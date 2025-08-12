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


speed_t get_baud_bitmask (int baud) {
	switch (baud){
		// matching to baud defines found in
		// <bits/termios.h> and <bits/termios-baud.h>
		case    300:  return B300;
		case   1200:  return B1200;
		case   4800:  return B4800;
		case   9600:  return B9600;
		case  19200:  return B19200;
		case  38400:  return B38400;
		case  57600:  return B57600;
		case 115200:  return B115200;
		case 230400:  return B230400;
		case 460800:  return B460800;
		case 921600:  return B921600;
		// baud rate not found so map to hangup
		default:      return B0;
		break;
    }
}


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
	else
		// called due to open() O_NDELAY
		fcntl(fd, F_SETFL, 0);

	// get terminal paramaters
	struct termios toptions;
	if (tcgetattr(fd, &toptions) < 0) {
		lua_pushnumber(L, fd);
		lua_pushstring(L, strerror(errno));
		return 2;
	}

	// set input/output speed
	// or use cfsetispeed() and cfsetospeed()
	cfsetspeed(&toptions, get_baud_bitmask(baud));

	// disable software flow control
	toptions.c_iflag &= ~(IXON | IXOFF | IXANY);
	// Disable any special handling of received bytes
	toptions.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

	// Prevent special interpretation of output bytes (e.g. newline chars)
	toptions.c_oflag &= ~OPOST;
	// Prevent conversion of newline to carriage return/line feed
	toptions.c_oflag &= ~ONLCR;

    toptions.c_cflag &= ~CSIZE;   // Clear all the size bits, then
    toptions.c_cflag |=  CS8;     // 8 bits
	toptions.c_cflag &= ~PARENB;  // Clear parity bit, disabling parity
    toptions.c_cflag &= ~CSTOPB;  // Clear stop field, only one stop bit used
    toptions.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control
    toptions.c_cflag |=  CREAD | CLOCAL; // Turn on READ & ignore ctrl lines

	// Disable: canonical, echo, erasure, n/l echo also INTR, QUIT and SUSP
    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);

	// VMIN = 0, VTIME = 0 : read() grabs what is available and returns
	// VMIN > 0, VTIME = 0 : read() waits for VMIN bytes before returning
	// VMIN = 0, VTIME > 0 : read() waits for VTIME then returns
	// VMIN > 0, VTIME > 0 : read() for VMIN bytes or there is a VTIME gap
	//                       in time between chars recieved
	toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 10;

    // set terminal paramaters
    if (tcsetattr(fd, TCSANOW, &toptions) < 0) {
		// on failure
    	lua_pushnumber(L, fd);
    	lua_pushstring(L, strerror(errno));
    	return 2;
    }
	// on success
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
	free(buf);
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

