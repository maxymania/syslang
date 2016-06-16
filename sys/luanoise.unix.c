#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linenoise.h"
#include "syscalls.h"
#include "../lua/lauxlib.h"

lua_State *LNT;

static int luno_dummy(lua_State *L){
	return 0;
}

static int luno_add(lua_State *L){
	linenoiseCompletions *lc = lua_touserdata(L,lua_upvalueindex(1));
	const char* c = lua_tostring(L,1);
	if(c) linenoiseAddCompletion(lc,c);
	return 0;
}

static void luno_completion(const char *buf, linenoiseCompletions *lc) {
	lua_settop(LNT,0);
	lua_getfield(LNT,LUA_REGISTRYINDEX,"callback-handler");
	lua_pushstring(LNT,buf);
	lua_pushlightuserdata(LNT,lc);
	lua_pushcclosure(LNT,luno_add,1);
	lua_pcall(LNT,2,0,0);
}

static int luno_linenoise (lua_State *L){
	lua_settop(L,1);
	const char* prompt = lua_tostring(L,1);
	if(!prompt) prompt = "$ ";
	const char* data = linenoise(prompt);
	if(!data) return 0;
	lua_pushstring(L,data);
	return 1;
}

static int luno_linenoiseSetCompletionCallback (lua_State *L){
	lua_settop(L,1);
	if(lua_type(L,1)!=LUA_TFUNCTION)
		lua_pushcfunction(L,luno_dummy);
	lua_setfield(L,LUA_REGISTRYINDEX,"callback-handler");
	return 0;
}

void luanoise_install(lua_State *ls){
	int top=lua_gettop(ls);
	lua_register(ls,"linenoise",luno_linenoise);
	lua_register(ls,"linenoiseSetCompletionCallback",luno_linenoiseSetCompletionCallback);
	lua_register(ls,"linenoise_set_cb",luno_linenoiseSetCompletionCallback);
	LNT = lua_newthread(ls);
	lua_setfield(ls,LUA_REGISTRYINDEX,"callback-thread");
	lua_pushcfunction(ls,luno_dummy);
	lua_setfield(ls,LUA_REGISTRYINDEX,"callback-handler");
	lua_settop(ls,top);
	linenoiseSetCompletionCallback(luno_completion);
}


