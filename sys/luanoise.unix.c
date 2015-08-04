#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linenoise.h"
#include "syscalls.h"
#include "../lua/lauxlib.h"

lua_State *LNT;

void completion(const char *buf, linenoiseCompletions *lc) {
	lua_settop(LNT,0);
	lua_getfield(LNT,LUA_REGISTRYINDEX,"callback-handler");
	
	lua_pushstring(LNT,buf);
	
}

int luno_linenoise (lua_State *L){
	lua_settop(L,1);
	const char* prompt = lua_tostring(L,1);
	if(!prompt) prompt = "$ ";
	const char* data = linenoise(prompt);
	if(!data) return 0;
	lua_pushstring(L,data);
	return 1;
}

void luanoise_install(lua_State *ls){
	int top=lua_gettop(ls);
	lua_register(ls,"linenoise",luno_linenoise);
	LNT = lua_newthread(ls);
	lua_setfield(ls,LUA_REGISTRYINDEX,"callback-handler");
	lua_settop(ls,top);
}


