#include "v8engine.h"

#include <libplatform/libplatform.h>
#include <engine/external/v8pp/class.hpp>
#include <engine/external/v8pp/module.hpp>

#include <base/system.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CGameClient * CV8Engine::m_pCGameClient = 0;	

CV8Engine::CV8Engine(IGameClient *pClient)
{
	CV8Engine::m_pCGameClient = (CGameClient*)pClient;
	Init();
}

CV8Engine::~CV8Engine()
{
    // dispose the Isolate and tear down v8
	m_pIsolate->Dispose();
	v8::V8::Dispose();
	v8::V8::ShutdownPlatform();
}

void CV8Engine::Init()
{
	// initialize v8
	v8::V8::InitializeICUDefaultLocation(".");
	v8::V8::InitializeExternalStartupData(".");
	std::unique_ptr<v8::Platform> Platform = v8::platform::NewDefaultPlatform();
	v8::V8::InitializePlatform(Platform.get());
	v8::V8::Initialize();
	
    // create a new Isolate and make it the current one
	v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	m_pIsolate = v8::Isolate::New(create_params);
	// create a stack-allocated handle scope
	v8::HandleScope HandleScope(m_pIsolate);
	v8::Local<v8::Context> Context = v8::Context::New(m_pIsolate);
	// enter the Context for compiling and running the hello world script
	v8::Context::Scope ContextScope(Context);

	// bindings
	v8pp::module Game(m_pIsolate), GameConsole(m_pIsolate);
	GameConsole
		.set("print", [](int Level, const char *pFrom, const char *pStr, bool Highlighted) { CV8Engine::m_pCGameClient->Console()->Print(Level, pFrom, pStr, Highlighted); })
		.set_const("tedst", 123)
	;
	Game
		.set("console", GameConsole)
	;

	// set bindings in global object
	m_pIsolate->GetCurrentContext()->Global()->Set(Context, v8pp::to_v8(m_pIsolate, "game"), Game.new_instance());

	delete create_params.array_buffer_allocator;

	LoadFiles();
}

void CV8Engine::LoadFiles()
{
	// one lonely file for now :c
	m_apJSFiles[0] = new CJSFile(m_pIsolate, "test.js");
}
