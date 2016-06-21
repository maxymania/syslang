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
	size_t sl,dl,sof,dof,ln;
	const char* sb = buflib_output(L, 1, &sl);
	char* db = buflib_input(L, 2, &dl);
	sof = lua_tointeger(L,3);
	dof = lua_tointeger(L,4);
	if(sof>sl)sof=sl;
	if(dof>dl)dof=dl;
	sb += sof; db += dof;
	sl -= sof; dl -= dof;
	if(sl>dl)sl = dl;
	ln = lua_tointeger(L,5);
	if((ln<1) || (ln>sl))ln = sl;
	memmove(db,sb,ln);
	return 0;
}

static int sl_strstr(lua_State *L){
	size_t sz1,sz2,p1,p2,p1f;
	const char* sp1 = buflib_output(L, 1, &sz1);
	const char* sp2 = buflib_output(L, 2, &sz2);
	p1 = lua_tointeger(L,3);
	lua_pushnumber(L,-1);
	if(!(sp1&&sp2))return 1;
	if(sz2<1)return 1;
	
	for(;p1<sz1;++p1){
		if(sp1[p1]!=*sp2) continue;
		for(p1f=p1+1,p2=1;(p1f<sz1) && (p2<sz2);++p1f,++p2){
			if(sp1[p1f]==sp2[p2]) continue;
			p2=0;
			break;
		}
		if(p2==sz2){
			lua_pushinteger(L,p1);
			return 1;
		}
	}
	return 1;
}

static int sl_div(lua_State *L){
	lua_settop(L,2);
	lua_pushliteral(L,"/");
	lua_insert(L,2);
	lua_concat(L,3);
	return 1;
}

void strlib_install(lua_State *ls){
	lua_register(ls,"strdup",sl_strdup);
	lua_register(ls,"arraycopy",sl_arraycopy);
	lua_register(ls,"strstr",sl_strstr);

	int top = lua_gettop(ls);
		lua_pushstring(ls,"");
		lua_getmetatable(ls,top+1);
		lua_pushstring(ls,"__div");
		lua_pushcfunction(ls,sl_div);
		lua_settable(ls,top+2);
	lua_settop(ls,top);
}


