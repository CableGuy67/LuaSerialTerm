## POSIX Serial Port C Library with Lua Bindings
Uses posix library so it should work with Linux/UNIX environments.

Using this library you'll be able to send and receive data to a device via
the serial port to connected devices that have a serial port, have a usb to
serial bridge built in or with a usb to serial module.
The goal here is to be able to talk with a range or REPLs whether it is forth
on an Atmega, Lua on an ESP running nodeMCU or Python on a Raspberry Pi Pico.

See luaserialterm.lua for an example of sending and receiving data.

## history
Forked from code found at:
[biomood/LuaSerial](https://github.com/biomood/LuaSerial)

That was derived from code available at:
[rtacconi/arduino-serial-posix](https://github.com/rtacconi/arduino-serial-posix)

Which in turn was derived from code now found at:
[todbot/arduino-serial](https://github.com/todbot/arduino-serial)

My additions and corrections were derived from (among other places):

A very good description of all options:
[mbedded ninja blog post](https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/)

Also the whole thing explained clearly:
[Serial Programming in POSIX](https://support.dce.felk.cvut.cz/pos/cv5/doc/serial.html#config)

## requirements

Lua 5.1.x

## build
Compiled on Debian 12 for Lua5.1 using:

    gcc -shared -I /usr/include/lua5.1/ -l:liblua5.1.so.0 -o serial.so -fPIC serial.c

-l:liblua5.1.so.0 will depend on what your shared library is actually named. The same goes for -I to point at the correct headers include path.

Also build the library for readline support using:

    gcc -shared -I /usr/include/lua5.1/ -l:liblua5.1.so.0 -o readline.so -fPIC lua_readline.c

## usage
Require the library:
    
    local serial = require("serial")

### Open a connection to a device
port_handle is the file descriptor  
msg is the error message if opening failed

    port_handle, msg = serial.open("/dev/ttyUSB0", 115200)
    if port_handle==-1 then
      error("Unable to open port - error: "..msg)
    end

### Write a string
c is number of bytes written  
msg is the error message if writing failed

    c, msg = serial.write(port_handle, "hello")
    if c < 0 then
      error("Unable to write: "..msg)
    end

### Read up to X number of bytes
c is actual number of bytes read  
msg is error message if reading failed, or the string of bytes that were read

    c, msg = serial.readbytes(port_handle, x)

### Close connection

    c, msg = serial.close(port_handle)
