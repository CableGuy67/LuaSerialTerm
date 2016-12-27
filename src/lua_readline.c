#include <errno.h>
#include <lauxlib.h>
#include <lua.h>
#include <luaconf.h>
#include <lualib.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int l_readline_add_history(lua_State *const L) {
    add_history(luaL_checkstring(L, 1));
    lua_pushboolean(L, true);
    return 1;
}

#ifndef LIBEDIT
static int l_readline_add_history_time(lua_State *const L) {
    add_history_time(luaL_checkstring(L, 1));
    lua_pushboolean(L, true);
    return 1;
}
#endif

static int l_readline_stifle_history(lua_State *const L) {
    stifle_history(luaL_checkinteger(L, 1));
    lua_pushboolean(L, true);
    return 1;
}

static int l_readline_unstifle_history(lua_State *const L) {
    lua_pushinteger(L, unstifle_history());
    return 1;
}

static int l_readline_clear_history(lua_State *const L) {
    clear_history();
    lua_pushboolean(L, true);
    return 1;
}

static int l_readline_read_history(lua_State *const L) {
    const int s = read_history(luaL_optstring(L, 1, NULL));
    if (s == 0) {
        lua_pushboolean(L, true);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushstring(L, strerror(s));
        return 2;
    }
}

static int l_readline_write_history(lua_State *const L) {
    const int s = write_history(luaL_optstring(L, 1, NULL));
    if (s == 0) {
        lua_pushboolean(L, true);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushstring(L, strerror(s));
        return 2;
    }
}

#ifndef LIBEDIT
static int l_readline_append_history(lua_State *const L) {
    const int s = append_history(luaL_checkinteger(L, 1),
                                 luaL_optstring(L, 2, NULL));
    if (s == 0) {
        lua_pushboolean(L, true);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushstring(L, strerror(s));
        return 2;
    }
}
#endif

static int l_readline_history_is_stifled(lua_State *const L) {
    lua_pushboolean(L, history_is_stifled());
    return 1;
}

static int l_readline_readline(lua_State *const L) {
    const char *const prompt = luaL_optstring(L, 1, "");
    lua_pushstring(L, readline(prompt));
    return 1;
}

static const luaL_Reg l_readline[] = {
    {"add_history",        l_readline_add_history},
#ifndef LIBEDIT
    {"add_history_time",   l_readline_add_history_time},
    {"append_history",     l_readline_append_history},
#endif
    {"clear_history",      l_readline_clear_history},
    {"history_is_stifled", l_readline_history_is_stifled},
    {"read_history",       l_readline_read_history},
    {"readline",           l_readline_readline},
    {"stifle_history",     l_readline_stifle_history},
    {"unstifle_history",   l_readline_unstifle_history},
    {"write_history",      l_readline_write_history},
    {NULL, NULL}
};

int luaopen_readline(lua_State *const L) {
    using_history();
    luaL_newlib(L, l_readline);
    return 1;
}
