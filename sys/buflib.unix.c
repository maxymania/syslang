//#include <stdio.h>
#include "syscalls.h"
#include "../lua/lauxlib.h"


static int isInteger(const char* str){
	return ((*str) == 0) || (  ((*str)>='0') && ((*str)<='9') );
}

static int buflib_index(lua_State *L){
	int idx;
	size_t size;
	const unsigned char* str;
	lua_settop(L,2);
	if(lua_type(L,2)==LUA_TSTRING){
		if(!isInteger(lua_tostring(L,2))){
			lua_gettable(L,lua_upvalueindex(1));
			return 1;
		}
	}

	str = lua_tolstring(L,1,&size);
	idx = lua_tointeger(L,2);
	if(idx<0) goto onErr;
	if(idx>=size) goto onErr;
	lua_pushinteger(L,str[idx]);
	return 1;
	onErr:
	lua_pushliteral(L, "index out of bounds");
	return lua_error(L);
}

static int blhb_len(lua_State *L){
	size_t* size = luaL_checkudata(L,1,"buflib.HeapBuffer");
	lua_pushinteger(L,*size);
	return 1;
}
static int blhb_index(lua_State *L){
	size_t* size = luaL_checkudata(L,1,"buflib.HeapBuffer");
	int idx = lua_tointeger(L,2);
	if(idx<0) goto onErr;
	if(idx>=*size) goto onErr;
	unsigned char *str = (unsigned char*)&size[1];
	lua_pushinteger(L,str[idx]);
	return 1;
	onErr:
	lua_pushliteral(L, "index out of bounds");
	return lua_error(L);
}
static int blhb_newindex(lua_State *L){
	size_t* size = luaL_checkudata(L,1,"buflib.HeapBuffer");
	int idx = lua_tointeger(L,2);
	if(idx<0) goto onErr;
	if(idx>=*size) goto onErr;
	unsigned char *str = (unsigned char*)&size[1];
	str[idx] = lua_tointeger(L,3);
	return 0;
	onErr:
	lua_pushliteral(L, "index out of bounds");
	return lua_error(L);
}
static int blhb_allocate(lua_State *L){
	size_t size;
	size_t* buf;
	size = lua_tointeger(L,1);
	lua_settop(L,0);
	buf = lua_newuserdata(L,size+sizeof(size_t));
	*buf = size;
	luaL_getmetatable(L,"buflib.HeapBuffer");
	lua_setmetatable(L,1);
	return ;
}

struct DirectBuffer {
	size_t size;
	unsigned char* ptr;
	void* handle;
	void (*free)(void* handle);
};
static int bldb_len(lua_State *L){
	struct DirectBuffer* buf = luaL_checkudata(L,1,"buflib.DirectBuffer");
	lua_pushinteger(L,buf->size);
	return 1;
}
static int bldb_index(lua_State *L){
	struct DirectBuffer* buf = luaL_checkudata(L,1,"buflib.DirectBuffer");
	int idx = lua_tointeger(L,2);
	if(idx<0) goto onErr;
	if(idx>=buf->size) goto onErr;
	lua_pushinteger(L,buf->ptr[idx]);
	return 1;
	onErr:
	lua_pushliteral(L, "index out of bounds");
	return lua_error(L);
}
static int bldb_newindex(lua_State *L){
	struct DirectBuffer* buf = luaL_checkudata(L,1,"buflib.DirectBuffer");
	int idx = lua_tointeger(L,2);
	if(idx<0) goto onErr;
	if(idx>=buf->size) goto onErr;
	buf->ptr[idx] = lua_tointeger(L,3);
	return 0;
	onErr:
	lua_pushliteral(L, "index out of bounds");
	return lua_error(L);
}
static int bldb_gc(lua_State *L){
	struct DirectBuffer* buf = luaL_checkudata(L,1,"buflib.DirectBuffer");
	if(buf->free)
		buf->free(buf->handle);
	return 0;
}


void buflib_install(lua_State *ls){
	int top=lua_gettop(ls);
	int table = top+1;

	lua_pushstring(ls,"");
	lua_getmetatable(ls,table);            //table+1
	lua_pushstring(ls,"__index");          //table+2
	lua_gettable(ls,table+1);
	lua_pushcclosure(ls,buflib_index,1);
	lua_pushstring(ls,"__index");          //table+3
	lua_insert(ls,table+2);
	lua_settable(ls,table+1);

	lua_settop(ls,top);

	luaL_newmetatable(ls,"buflib.HeapBuffer");
	lua_pushstring(ls,"__len");
	lua_pushcfunction(ls,blhb_len);
	lua_settable(ls,table);
	lua_pushstring(ls,"__index");
	lua_pushcfunction(ls,blhb_index);
	lua_settable(ls,table);
	lua_pushstring(ls,"__newindex");
	lua_pushcfunction(ls,blhb_newindex);
	lua_settable(ls,table);
	lua_register(ls,"allocate",blhb_allocate);

	lua_settop(ls,top);

	luaL_newmetatable(ls,"buflib.DirectBuffer");
	lua_pushstring(ls,"__len");
	lua_pushcfunction(ls,bldb_len);
	lua_settable(ls,table);
	lua_pushstring(ls,"__index");
	lua_pushcfunction(ls,bldb_index);
	lua_settable(ls,table);
	lua_pushstring(ls,"__newindex");
	lua_pushcfunction(ls,bldb_newindex);
	lua_settable(ls,table);
	lua_pushstring(ls,"__gc");
	lua_pushcfunction(ls,bldb_gc);
	lua_settable(ls,table);

	lua_settop(ls,top);
}

void buflib_pushbuf(lua_State *ls,char* buf,size_t size,void* handle,void (*free)(void*)){
	int top = lua_gettop(ls);
	struct DirectBuffer* dbf = lua_newuserdata(ls,sizeof(struct DirectBuffer));
	dbf->ptr = buf;
	dbf->size = size;
	dbf->handle = handle;
	dbf->free = free;
	luaL_getmetatable(ls,"buflib.DirectBuffer");
	lua_setmetatable(ls,top+1);
	lua_settop(ls,top+1);
}

const char* buflib_output(lua_State *ls, int index, size_t* size){
	struct DirectBuffer* dbuf;
	size_t *hbuf;
	int top = lua_gettop(ls);
	if(!lua_getmetatable(ls,index)) return lua_tolstring(ls,index,size);
	// top+1
	luaL_getmetatable(ls,"buflib.HeapBuffer"  ); // top+2
	luaL_getmetatable(ls,"buflib.DirectBuffer"); // top+3
	if(lua_rawequal(ls,top+1,top+2)) {
		hbuf = luaL_checkudata(ls,index,"buflib.HeapBuffer");
		*size = *hbuf;
		lua_settop(ls,top);
		return (const char*)&hbuf[1];
	}
	if(lua_rawequal(ls,top+1,top+3)) {
		dbuf = luaL_checkudata(ls,index,"buflib.DirectBuffer");
		*size = dbuf->size;
		lua_settop(ls,top);
		return dbuf->ptr;
	}
	lua_settop(ls,top);
	return lua_tolstring(ls,index,size);
}

char* buflib_input(lua_State *ls, int index, size_t* size){
	struct DirectBuffer* dbuf;
	size_t *hbuf;
	int top = lua_gettop(ls);
	if(!lua_getmetatable(ls,index))return NULL;
	// top+1
	luaL_getmetatable(ls,"buflib.HeapBuffer"  ); // top+2
	luaL_getmetatable(ls,"buflib.DirectBuffer"); // top+3
	if(lua_rawequal(ls,top+1,top+2)) {
		hbuf = luaL_checkudata(ls,index,"buflib.HeapBuffer");
		*size = *hbuf;
		lua_settop(ls,top);
		return (char*)&hbuf[1];
	}
	if(lua_rawequal(ls,top+1,top+3)) {
		dbuf = luaL_checkudata(ls,index,"buflib.DirectBuffer");
		*size = dbuf->size;
		lua_settop(ls,top);
		return dbuf->ptr;
	}
	lua_settop(ls,top);
	return NULL;
}


