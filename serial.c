/*
 * POSIX Serial Port C Library with Lua Bindings
 * See luaserialterm.lua for an example of how to use
 *
 * Uses posix library so it should work with Linux/UNIX
 * to communicate with devices that via a serial port
 *
 * HISTORY
 *
 * Forked from code found at:
 * https://github.com/biomood/LuaSerial
 *
 * That was derived from code available at:
 * https://github.com/rtacconi/arduino-serial-posix
 *
 * Which in turn was derived from code now found at:
 * https://github.com/todbot/arduino-serial
 *
 * My additions and corrections were derived from (among other places):
 * A very good description of all options:
 * https://blog.mbedded.ninja
 * /programming/operating-systems/linux/linux-serial-ports-using-c-cpp/
 *
 * Also the whole thing explained clearly:
 * https://support.dce.felk.cvut.cz/pos/cv5/doc/serial.html#config
 *
 * Compiled on Debian 12 for Lua5.1 using:
 * gcc -I /usr/include/lua5.1/ -fPIC -shared \
 * -l:liblua5.1.so.0 serial.c -o serial.so
 *
 * There are some Lua 5.1 specific calls (int, register, etc) that need
 * to be adressed to build for later Lua versions that no longer have
 * the calls used in this code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int iopen(lua_State *L);
int iread_no_bytes(lua_State *L);
int iread_until(lua_State *L);
int iwrite(lua_State *L);
int iclose(lua_State *L);
int isleep(lua_State *L);
int iusleep(lua_State *L);
int luaopen_serial(lua_State *L);

// set up the libray methods
const struct luaL_Reg serial [] = {
	{"open", iopen},
	{"write", iwrite},
	{"readbytes", iread_no_bytes},
	{"close", iclose},
	{"sleep", isleep},
	{"usleep", iusleep},
	{NULL, NULL}
};


int iopen(lua_State *L) {
	const char *serialport = luaL_checkstring(L, 1);
	int baud = luaL_checkint(L, 2);

	// attempt to open the serial port
	int fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd==-1) {
		lua_pushnumber(L, fd);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	// get terminal paramaters
	struct termios toptions;
	if (tcgetattr(fd, &toptions) < 0) {
		lua_pushnumber(L, fd);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	speed_t brate = baud;

	// set input/output speed
	cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

	toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    toptions.c_cflag &= ~CRTSCTS;
    toptions.c_cflag |= CREAD | CLOCAL;
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY);
    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    toptions.c_oflag &= ~OPOST;
	toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 20;

    // set terminal paramaters
    if (tcsetattr(fd, TCSANOW, &toptions) < 0) {
    	lua_pushnumber(L, fd);
    	lua_pushstring(L, strerror(errno));
    	return 2;
    }

	lua_pushnumber(L, fd);
	return 1;
}

int iwrite(lua_State *L) {
	int fd = luaL_checkint(L, 1);
	const char *value = luaL_checkstring(L, 2);
	int count = strlen(value);

	int r = write(fd, value, count);
	if (r!=count) {
		lua_pushnumber(L, r);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	lua_pushnumber(L, r);
	return 1;
}

int iread_no_bytes(lua_State *L) {
	int fd = luaL_checkint(L, 1);
	int count = luaL_checkint(L, 2);

	char* buf = (char*)calloc(count+1, sizeof(char));

	int r = read(fd, buf, count);
	if (r < 0) {
		lua_pushnumber(L, r);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	// return number of bytes read and data
	lua_pushnumber(L, r);
	lua_pushstring(L, buf);
	return 2;
}

int iclose(lua_State *L) {
	int fd = luaL_checkint(L, 1);
	int r = close(fd);

	if (r<0) {
		lua_pushnumber(L, r);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	lua_pushnumber(L, r);
	return 1;
}

int isleep(lua_State *L) {
	int seconds = luaL_checkint(L, 1);

	int r = sleep(seconds);
	if (r>0) {
		lua_pushnumber(L, r);
		lua_pushstring(L, "awoke early");
		return 2;
	}

	lua_pushnumber(L, r);
	return 1;
}

int iusleep(lua_State *L) {
	int useconds = luaL_checkint(L, 1);

	int r = usleep(useconds);
	if (r<0) {
		lua_pushnumber(L, r);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	lua_pushnumber(L, r);
	return 1;
}

int luaopen_serial(lua_State *L) {
	luaL_register(L, "serial", serial);
	return 1;
}

