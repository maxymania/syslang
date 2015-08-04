#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include "syscalls.h"
#include "../lua/lauxlib.h"

int lsc_pid (lua_State *L){
	lua_settop(L,1);
	pid_t* pid = lua_newuserdata(L,sizeof(pid_t));
	luaL_getmetatable(L,"pid_t");
	lua_setmetatable(L,2);
	*pid = lua_tointeger(L,1);
	return 1;
}
int lsc_uid (lua_State *L){
	lua_settop(L,1);
	uid_t* uid = lua_newuserdata(L,sizeof(uid_t));
	luaL_getmetatable(L,"uid_t");
	lua_setmetatable(L,2);
	*uid = lua_tointeger(L,1);
	return 1;
}
int lsc_gid (lua_State *L){
	lua_settop(L,1);
	gid_t* gid = lua_newuserdata(L,sizeof(gid_t));
	luaL_getmetatable(L,"gid_t");
	lua_setmetatable(L,2);
	*gid = lua_tointeger(L,1);
	return 1;
}

int lsc_fork (lua_State *L){
	lua_settop(L,0);
	pid_t* pid = lua_newuserdata(L,sizeof(pid_t));
	luaL_getmetatable(L,"pid_t");
	lua_setmetatable(L,1);
	*pid = fork();
	return 1;
}
int lsc_pipefork (lua_State *L){
	int pipefd[2];
	pipe(pipefd);
	lua_settop(L,0);
	pid_t* pid = lua_newuserdata(L,sizeof(pid_t));
	luaL_getmetatable(L,"pid_t");
	lua_setmetatable(L,1);
	*pid = fork();
	if(*pid>0){ // parent
		dup2(pipefd[0],0);
	}else if(*pid==0){ // child
		dup2(pipefd[1],1);
	}
	close(pipefd[0]);
	close(pipefd[1]);
	return 1;
}

typedef const char* STRING;

int lsc_exec (lua_State *L){
	int i;
	lua_settop(L,1);
	lua_len(L,1);
	int n = lua_tointeger(L,2);
	if(n<1)return 0;
	STRING *args = lua_newuserdata(L,sizeof(void*)*n+2); // 3
	args[n]=0;
	for(i=0;i<n;++i){
		lua_geti(L,1,i+1);
		args[i] = lua_tostring(L,4);
		if(!(args[i]))args[i]="";
		lua_settop(L,3);
	}
	if(!args[0])return 0;
	STRING p;
	int ispath=0;
	for(p=args[0];*p;++p) if(*p=='/') ispath=1;
	if(ispath)
		execv(args[0],(char *const *)args);
	else
		execvp(args[0],(char *const *)args);
	return 0;
}

int lsc_isvalid (lua_State *L){
	pid_t* pid = luaL_testudata(L,1,"pid_t");
	if(pid){
		lua_pushboolean(L,*pid<0);
		return 1;
	}
	return 0;
}
int lsc_is0 (lua_State *L){
	pid_t* pid = luaL_testudata(L,1,"pid_t");
	if(pid){
		lua_pushboolean(L,!*pid);
		return 1;
	}
	return 0;
}

int lsc_asint (lua_State *L){
	pid_t* pid = luaL_testudata(L,1,"pid_t");
	if(pid){
		lua_pushinteger(L,*pid);
		return 1;
	}
	uid_t* uid = luaL_testudata(L,1,"uid_t");
	if(uid){
		lua_pushinteger(L,*uid);
		return 1;
	}
	gid_t* gid = luaL_testudata(L,1,"gid_t");
	if(gid){
		lua_pushinteger(L,*gid);
		return 1;
	}
	printf("C: pid,uid,gid %p %p %p\n",pid,uid,gid);
	return 0;
}
int lsc_asstr (lua_State *L){
	pid_t* pid = luaL_testudata(L,1,"pid_t");
	if(pid){
		lua_pushfstring(L,"pid(%I)",(lua_Integer)(*pid));
		return 1;
	}
	uid_t* uid = luaL_testudata(L,1,"uid_t");
	if(uid){
		lua_pushfstring(L,"uid(%I)",(lua_Integer)(*uid));
		return 1;
	}
	gid_t* gid = luaL_testudata(L,1,"gid_t");
	if(gid){
		lua_pushfstring(L,"gid(%I)",(lua_Integer)(*gid));
		return 1;
	}
	const char* chr = lua_tostring(L,1);
	if(chr){
		lua_pushstring(L,chr);
		return 1;
	}
	lua_pushfstring(L,"<%p>",lua_topointer(L,1));
	return 1;
}

int lsc_setuid (lua_State *L){
	uid_t* uid = luaL_checkudata(L,1,"uid_t");
	lua_pushboolean(L,!setuid(*uid));
	return 1;
}

int lsc_setgid (lua_State *L){
	gid_t* gid = luaL_checkudata(L,1,"gid_t");
	lua_pushboolean(L,!setgid(*gid));
	return 1;
}

int lsc_waitpid (lua_State *L){
	int status;
	pid_t* pid = luaL_checkudata(L,1,"pid_t");
	lua_pushinteger(L,waitpid(*pid, &status, 0));
	//if(<=0)status=-1;
	lua_pushinteger(L,status);
	return 2;
}
int lsc_exit (lua_State *L){
	_exit(lua_tointeger(L,1));
	return 0;
}


#define TOSTR(L) (lua_pushcfunction(L, lsc_asstr),lua_setfield(L,table,"__tostring") )
//__tostring
//int lua_CFunction (lua_State *L){}

void syscalls_install(lua_State *ls){
	int top=lua_gettop(ls);
	int table = top+1;
	luaL_newmetatable(ls,"pid_t"); TOSTR(ls);
	lua_settop(ls,top);
	luaL_newmetatable(ls,"uid_t"); TOSTR(ls);
	lua_settop(ls,top);
	luaL_newmetatable(ls,"gid_t"); TOSTR(ls);
	lua_settop(ls,top);
	lua_register(ls,"pid_t"   ,lsc_pid);
	lua_register(ls,"uid_t"   ,lsc_uid);
	lua_register(ls,"gid_t"   ,lsc_gid);
	lua_register(ls,"fork"    ,lsc_fork);
	lua_register(ls,"pipefork",lsc_pipefork);
	lua_register(ls,"isvalid" ,lsc_isvalid);
	lua_register(ls,"is0"     ,lsc_is0);
	lua_register(ls,"int_t"   ,lsc_asint);
	lua_register(ls,"str_t"   ,lsc_asstr);
	lua_register(ls,"setuid"  ,lsc_setuid);
	lua_register(ls,"setgid"  ,lsc_setgid);
	lua_register(ls,"waitpid" ,lsc_waitpid);
	lua_register(ls,"exit"    ,lsc_exit);
	lua_register(ls,"exec"    ,lsc_exec);
	lua_settop(ls,top);
}


