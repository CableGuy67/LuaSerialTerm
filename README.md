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

    gcc -shared -I /usr/include/lua5.1/ -l:liblua5.1.so -o lua_serial.so -fPIC lua_serial.c

Also build the library for readline support using:

    gcc -shared -I /usr/include/lua5.1/ -l:liblua5.1.so -o lua_readline.so -fPIC lua_readline.c

## usage
Require the serial library:
    
    local serial = require("lua_serial")

And to add readline history support:

    local readline = require("lua_readline")

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




lua-readline
============
:toc:

A Lua wrapper for the GNU readline library.


Usage
-----

To use lua-readline, simply put `local readline = require("readline")`
in your script.


Functions
---------

Unless otherwise specified, all functions return `true` on success, and
`nil` followed by an error message on failure.


[[readline]]
`readline.readline([prompt])`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Reads a line of input.

'prompt' is the string that is used as the... prompt.

'prompt' defaults to an empty string.


[[add_history]]
`readline.add_history(line)`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Adds 'line' to the history.


[[add_history_time]]
`readline.add_history_time(timestamp)`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sets the timestamp of the most recent line in the history to 'timestamp'.

'timestamp' is a string, by the way.


[[clear_history]]
`readline.clear_history()`
~~~~~~~~~~~~~~~~~~~~~~~~~~

Removes every entry from the history.


[[history_is_stifled]]
`readline.history_is_stifled()`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Returns `true` if the history is <<stifle_history,stifled>>, `false`
otherwise.


[[stifle_history]]
`readline.stifle_history(max_lines)`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prevents the history from storing more than 'max_lines' entries at a time.


[[unstifle_history]]
`readline.unstifle_history()`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Removes the limit from the history.

Returns the previous amount the history was stifled by.
The value is positive if the history was stifled, negative if it wasn't.


[[read_history]]
`readline.read_history(filename)`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Loads the history from a file.

'filename' is the file to load from.
If 'filename' is `nil`, the default history file is used
(usually `~/.history`).


[[write_history]]
`readline.write_history(filename)`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Writes the history to a file.

'filename' is the file to write to.
If 'filename' is `nil`, the default history file is used.

This function completely overwrites the history file;
use <<append_history,`append_history`>> if you just want to update it.


[[append_history]]
`readline.append_history(n_entries, filename)`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Appends the last 'n_entries' entries in the history to a file.

'filename' is the file to add the entries to.
If 'filename' is `nil`, the default history file is used.


Examples
--------

The following code is an (extremely) simple shell:

[source,lua]
----
local os = require("os")
local readline = require("readline")

while (true) do
  local cmdline = readline.readline("> ")  -- read a line

  if (cmdline == "exit") then              -- 'exit' builtin
    os.exit(0)                             --  end the program
  end

  readline.add_history(cmdline)            -- update command history

  os.execute(cmdline)                      -- execute command in a real shell
end
----

Here is a version that remembers the history across multiple invocations:

[source,lua]
----
local os = require("os")
local readline = require("readline")

readline.read_history(nil) -- load default history

while (true) do
  local cmdline = readline.readline("> ")

  if (cmdline == "exit") then
    assert(readline.write_history(nil)) -- write to default history file
    os.exit(0)
  end

  readline.add_history(cmdline)

  os.execute(cmdline)
end
----

And I can't resist adding tons of bells and whistles:

[source,lua]
----
local os = require("os")
local readline = require("readline")

if not (readline.read_history(nil)) then -- file probably doesn't exist
  assert(readline.write_history(nil)) -- create the default history file
end

local prompt = 'FAKESH> '

local builtins = {
  ['exit'] = function()
    os.exit(0)
  end,
}

while (true) do
  local cmdline = readline.readline(prompt)

  readline.add_history(cmdline)

  -- append most recent line to default history file
  assert(readline.append_history(1, nil))

  if (builtins[cmdline] ~= nil) then
    builtins[cmdline]()
    goto continue
  end

  os.execute(cmdline)

  ::continue::
end
----
