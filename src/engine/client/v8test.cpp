#include "v8test.h"

#include <libplatform/libplatform.h>
#include <engine/external/v8pp/class.hpp>
#include <engine/external/v8pp/module.hpp>

#include <base/system.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CGameClient * CV8Test::m_pCGameClient = 0;	

CV8Test::CV8Test(IGameClient *pClient)
{
	// initialize pointers to bind to v8
	CV8Test::m_pCGameClient = (CGameClient*)pClient;

	// initialize v8
	v8::V8::InitializeICUDefaultLocation(".");
	v8::V8::InitializeExternalStartupData(".");
	std::unique_ptr<v8::Platform> Platform = v8::platform::NewDefaultPlatform();
	v8::V8::InitializePlatform(Platform.get());
	v8::V8::Initialize();
	
    // create a new Isolate and make it the current one
	v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	v8::Isolate *pIsolate = v8::Isolate::New(create_params);
	{
		// create a stack-allocated handle scope
		v8::HandleScope HandleScope(pIsolate);
		v8::Local<v8::Context> Context = v8::Context::New(pIsolate);
		// enter the Context for compiling and running the hello world script
		v8::Context::Scope ContextScope(Context);

		// bindings
		v8pp::module Game(pIsolate), GameConsole(pIsolate);
		GameConsole
			.set("print", [](int Level, const char *pFrom, const char *pStr, bool Highlighted) { CV8Test::m_pCGameClient->Console()->Print(Level, pFrom, pStr, Highlighted); })
			.set_const("tedst", 123)
		;
		Game
			.set("console", GameConsole)
		;

		// set bindings in global object
		pIsolate->GetCurrentContext()->Global()->Set(Context, v8pp::to_v8(pIsolate, "game"), Game.new_instance());


		const char *pFileName = "test.js";
		v8::Local<v8::String> FileName = v8::String::NewFromUtf8(pIsolate, pFileName).ToLocalChecked();
		v8::Local<v8::String> Source;
		if(!ReadFile(pIsolate, pFileName).ToLocal(&Source))
			dbg_msg("v8test", "Error reading '%s'", pFileName);

		bool success = ExecuteString(pIsolate, Source, FileName, true);
		while(v8::platform::PumpMessageLoop(Platform.get(), pIsolate))
			continue;
	}

	// dispose the Isolate and tear down v8
	pIsolate->Dispose();
	v8::V8::Dispose();
	v8::V8::ShutdownPlatform();
	delete create_params.array_buffer_allocator;
}

v8::MaybeLocal<v8::String> CV8Test::ReadFile(v8::Isolate *isolate, const char *name) { // maybe do this with tw's io? (keeping this c style for now xd)
	FILE* file = fopen(name, "rb");
	if(file == NULL)
		return v8::MaybeLocal<v8::String>();

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	rewind(file);

	char* chars = new char[size + 1];
	chars[size] = '\0';
	for(size_t i = 0; i < size;)
	{
		i += fread(&chars[i], 1, size - i, file);
		if(ferror(file)) {
			fclose(file);
			return v8::MaybeLocal<v8::String>();
		}
	}
	fclose(file);
	v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(isolate, chars, v8::NewStringType::kNormal, static_cast<int>(size));
	delete[] chars;
	return result;
}

bool CV8Test::ExecuteString(v8::Isolate *pIsolate, v8::Local<v8::String> Source, v8::Local<v8::Value> Name, bool PrintResult)
{
	v8::HandleScope HandleScope(pIsolate);
	v8::TryCatch TryCatch(pIsolate);
	v8::ScriptOrigin Origin(Name);
	v8::Local<v8::Context> Context(pIsolate->GetCurrentContext());
	v8::Local<v8::Script> Script;
	if(!v8::Script::Compile(Context, Source, &Origin).ToLocal(&Script))
	{
		ReportException(pIsolate, &TryCatch);
		return false;
	}
	else
	{
		v8::Local<v8::Value> Result;
		if(!Script->Run(Context).ToLocal(&Result))
		{
			ReportException(pIsolate, &TryCatch);
			return false;
		}
		else
		{
			if(PrintResult && !Result->IsUndefined())
			{
				v8::String::Utf8Value StrV8(pIsolate, Result);
				const char* pStr = ToCString(StrV8);
				dbg_msg("v8test", "%s", pStr);
			}
			return true;
		}
	}
}

void CV8Test::ReportException(v8::Isolate* pIsolate, v8::TryCatch* TryCatch) {
	v8::HandleScope HandleScope(pIsolate);
	v8::String::Utf8Value Exception(pIsolate, TryCatch->Exception());
	const char *pExceptionString = ToCString(Exception);
	v8::Local<v8::Message> Message = TryCatch->Message();
	if (Message.IsEmpty())
	{
		// no extra information, just print the exception
		dbg_msg("v8test", "%s\n", pExceptionString);
	}
	else
	{
		// filename, line number and message
		v8::String::Utf8Value Filename(pIsolate, Message->GetScriptOrigin().ResourceName());
		v8::Local<v8::Context> Context(pIsolate->GetCurrentContext());
		const char *pFilenameStr = ToCString(Filename);
		int LineNum = Message->GetLineNumber(Context).FromJust();
		dbg_msg("v8test", "%s:%i: %s", pFilenameStr, LineNum, pExceptionString);
		
		// line
		v8::String::Utf8Value sourceline(pIsolate, Message->GetSourceLine(Context).ToLocalChecked());
		const char *pSourceLine = ToCString(sourceline);
		dbg_msg("v8test", "%s", pSourceLine);
		
		// underlining
		int Start = Message->GetStartColumn(Context).FromJust();
		int End = Message->GetEndColumn(Context).FromJust();
		char aUnderlining[512]; 
		for(int i = 0; i < Start; i++)
			str_append(aUnderlining, " ", sizeof(aUnderlining));
		for(int i = Start; i < End; i++)
			str_append(aUnderlining, "^", sizeof(aUnderlining));
		
		dbg_msg("v8test", "%s", aUnderlining);
		
		// stack trace
		v8::Local<v8::Value> StackTrace;
		if(TryCatch->StackTrace(Context).ToLocal(&StackTrace) &&
			StackTrace->IsString() &&
			v8::Local<v8::String>::Cast(StackTrace)->Length() > 0)
		{
			v8::String::Utf8Value StackTraceV8(pIsolate, StackTrace);
			const char* pStackTraceStr = ToCString(StackTraceV8);
			dbg_msg("v8test", "%s", pStackTraceStr);
		}
	}
}

const char* CV8Test::ToCString(const v8::String::Utf8Value& Str) {
  return *Str ? *Str : "<string conversion failed>";
}
