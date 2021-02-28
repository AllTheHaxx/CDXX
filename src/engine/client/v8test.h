#ifndef ENGINE_CLIENT_V8TEST_H
#define ENGINE_CLIENT_V8TEST_H

#include <v8.h>

#include <game/client/gameclient.h>

class CV8Test
{
public:
	CV8Test(IGameClient *pClient);
private:
	v8::MaybeLocal<v8::String> ReadFile(v8::Isolate *isolate, const char *name);
	bool ExecuteString(v8::Isolate *pIsolate, v8::Local<v8::String> Source, v8::Local<v8::Value> Name, bool PrintResult);
	void ReportException(v8::Isolate *pIsolate, v8::TryCatch *pTryCatch);
	const char* ToCString(const v8::String::Utf8Value& Str);

	static CGameClient * m_pCGameClient;
};

#endif
