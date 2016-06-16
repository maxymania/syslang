#include <stddef.h>
#include "../lua/lua.h"

void syscalls_install(lua_State *ls);
void luanoise_install(lua_State *ls);
void buflib_install(lua_State *ls);
void strlib_install(lua_State *ls);

void buflib_pushbuf(lua_State *ls,char* buf,size_t size,void* handle,void (*free)(void*));

const char* buflib_output(lua_State *ls, int index, size_t* size);
char* buflib_input(lua_State *ls, int index, size_t* size);


