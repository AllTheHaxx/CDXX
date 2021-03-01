#ifndef ENGINE_CLIENT_V8FILE_H
#define ENGINE_CLIENT_V8FILE_H

#include <v8.h>

#include <game/client/gameclient.h>

class CV8File
{
public:
	CV8File(v8::Isolate *pIsolate, const char *pFilename);
    ~CV8File();

    void Run();
private:
    v8::Isolate *m_pIsolate;
	v8::MaybeLocal<v8::String> ReadFile(const char *name);
	bool ExecuteString(v8::Local<v8::String> Source, v8::Local<v8::Value> Name, bool PrintResult);
	void ReportException(v8::TryCatch *pTryCatch);

	static CGameClient * m_pCGameClient;
};

#endif
