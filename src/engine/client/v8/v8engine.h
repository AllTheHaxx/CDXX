#ifndef ENGINE_CLIENT_V8ENGINE_H
#define ENGINE_CLIENT_V8ENGINE_H

#include <v8.h>

#include "jsfile.h"

#include <game/client/gameclient.h>

class CV8Engine
{
public:
	enum
	{
		MAX_JS_FILES=128,
	};

	CV8Engine(IGameClient *pClient);
    ~CV8Engine();

	void Init();

private:
    v8::Isolate *m_pIsolate;

	void LoadFiles();
	CJSFile *m_apJSFiles[MAX_JS_FILES]; // vector?

	static CGameClient *m_pCGameClient;
};

#endif
