#include "jsfile.h"

CJSFile::CJSFile(v8::Isolate *pIsolate)
{
    m_pIsolate = pIsolate;
}

v8::MaybeLocal<v8::String> CJSFile::ReadFile(const char *name) { // maybe do this with tw's io? (keeping this c style for now xd)
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
	v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(m_pIsolate, chars, v8::NewStringType::kNormal, static_cast<int>(size));
	delete[] chars;
	return result;
}

bool CJSFile::ExecuteString(v8::Local<v8::String> Source, v8::Local<v8::Value> Name, bool PrintResult)
{
	v8::HandleScope HandleScope(m_pIsolate);
	v8::TryCatch TryCatch(m_pIsolate);
	v8::ScriptOrigin Origin(Name);
	v8::Local<v8::Context> Context(m_pIsolate->GetCurrentContext());
	v8::Local<v8::Script> Script;
	if(!v8::Script::Compile(Context, Source, &Origin).ToLocal(&Script))
	{
		ReportException(&TryCatch);
		return false;
	}
	else
	{
		v8::Local<v8::Value> Result;
		if(!Script->Run(Context).ToLocal(&Result))
		{
			ReportException(&TryCatch);
			return false;
		}
		else
		{
			if(PrintResult && !Result->IsUndefined())
			{
				v8::String::Utf8Value StrV8(m_pIsolate, Result);
				const char* pStr = ToCString(StrV8);
				dbg_msg("v8test", "%s", pStr);
			}
			return true;
		}
	}
}

void CJSFile::ReportException(v8::TryCatch* TryCatch) {
	v8::HandleScope HandleScope(m_pIsolate);
	v8::String::Utf8Value Exception(m_pIsolate, TryCatch->Exception());
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
		v8::String::Utf8Value Filename(m_pIsolate, Message->GetScriptOrigin().ResourceName());
		v8::Local<v8::Context> Context(m_pIsolate->GetCurrentContext());
		const char *pFilenameStr = ToCString(Filename);
		int LineNum = Message->GetLineNumber(Context).FromJust();
		dbg_msg("v8test", "%s:%i: %s", pFilenameStr, LineNum, pExceptionString);
		
		// line
		v8::String::Utf8Value Sourceline(m_pIsolate, Message->GetSourceLine(Context).ToLocalChecked());
		const char *pSourceLine = ToCString(Sourceline);
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
			v8::String::Utf8Value StackTraceV8(m_pIsolate, StackTrace);
			const char* pStackTraceStr = ToCString(StackTraceV8);
			dbg_msg("v8test", "%s", pStackTraceStr);
		}
	}
}