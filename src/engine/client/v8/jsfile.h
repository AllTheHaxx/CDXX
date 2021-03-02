#ifndef ENGINE_CLIENT_V8FILE_H
#define ENGINE_CLIENT_V8FILE_H

#include <v8.h>

#include <game/client/gameclient.h>

class CJSFile
{
public:
	CJSFile(v8::Isolate *pIsolate, const char *pFileName);
    ~CJSFile();

    void Run();
private:
    v8::Isolate *m_pIsolate;
	v8::MaybeLocal<v8::String> ReadFile(const char *name);
	bool ExecuteString(v8::Local<v8::String> Source, v8::Local<v8::Value> Name, bool PrintResult);
	void ReportException(v8::TryCatch *pTryCatch);

	char m_aFileName[512];

	const char* ToCString(const v8::String::Utf8Value& Str); // move at some point
};

#endif
