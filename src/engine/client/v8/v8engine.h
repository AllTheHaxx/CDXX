#ifndef ENGINE_CLIENT_V8ENGINE_H
#define ENGINE_CLIENT_V8ENGINE_H

#include <v8.h>

#include <game/client/gameclient.h>

class CV8Engine
{
public:
	CV8Engine(IGameClient *pClient);
    ~CV8Engine();

	static const char* ToCString(const v8::String::Utf8Value& Str);
private:
    v8::Isolate *m_pIsolate;
	v8::MaybeLocal<v8::String> ReadFile(const char *name);
	bool ExecuteString(v8::Local<v8::String> Source, v8::Local<v8::Value> Name, bool PrintResult);
	void ReportException(v8::TryCatch *pTryCatch);

	static CGameClient * m_pCGameClient;
};

#endif
