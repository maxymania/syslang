#include <stdio.h>

#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "sys/syscalls.h"

typedef const char* STRING;

void lps(lua_State* ls,int beg,int argc,const STRING* argv){
	int top = lua_gettop(ls);
	int i,j;
	lua_createtable(ls,argc,0);
	for(i=beg,j=1;i<argc;++i,j++){
		lua_pushstring(ls,argv[i]);
		lua_seti(ls,top+1,j);
	}
	lua_setglobal(ls,"ARGV");
	lua_settop(ls,top);
}
int main(int argc,const STRING* argv){
	lua_State *ls = luaL_newstate();
	luaL_openlibs(ls);
	syscalls_install(ls);
	luanoise_install(ls);
	lps(ls,1,argc,argv);
	if(argc<2) return -1;
	if(luaL_loadfile(ls,argv[1])) return -1;
	if(lua_pcall(ls, 0, 1, 0)) return -1;
	return (int)(lua_tointeger(ls,1));
}


