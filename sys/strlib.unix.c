#include <string.h>
#include "syscalls.h"
#include "../lua/lauxlib.h"


static int sl_strdup(lua_State *L){
	size_t size;
	const char* str = buflib_output(L, 1, &size);
	size_t beg = lua_tointeger(L,2);
	size_t end = lua_tointeger(L,3);
	if((end<1) || (end>size))end=size;
	if((beg<1))beg=0;
	else if((beg>size))beg=size;
	if(end<=beg)
		lua_pushliteral(L,"");
	else
		lua_pushlstring(L,str+beg,end-beg);
	return 1;
}

static int sl_arraycopy(lua_State *L){
	size_t sl,dl,of,ln;
	const char* sb = buflib_output(L, 1, &sl);
	char* db = buflib_input(L, 2, &dl);
	if(sl>dl)sl = dl;
	of = lua_tointeger(L,3);
	if(sl<of)return 0;
	sb += of; db += of;
	sl -= of; dl -= of;
	ln = lua_tointeger(L,4);
	if((ln<1) || (ln>sl))ln = sl;
	memmove(db,sb,ln);
	return 0;
}


void strlib_install(lua_State *ls){
	lua_register(ls,"strdup",sl_strdup);
	lua_register(ls,"arraycopy",sl_arraycopy);
}


