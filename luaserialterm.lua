#!/usr/bin/lua5.1

local serial = require("serial")

if arg[1] == "help" then
	print('luaT "port-string"|"help" port-speed')
	os.exit()
end

local serialport = arg[1] or "/dev/ttyUSB0"
-- connectspeed = 115200 -- ESP 19200 1284P forth
local connectspeed = arg[2] or 115200
local port_handle
local data_out = ''
local data_in = ''
local ready_prompt = "\n> "


local function port_open()
	-- open connection to device
	local msg
	port_handle, msg = serial.open(serialport, connectspeed)
	if port_handle == -1 then
		error("Unable to open port - error: "..msg)
	end
end


local function port_close()
	-- close the connection
	local c, msg = serial.close(port_handle)
	if c < 0 then
		error("Unable to close connection: "..msg)
	end
end


local function port_TX(data)
	local c, msg = serial.write(port_handle, data)
	if c < 0 then
		error("Unable to write: "..msg)
	end
	serial.usleep(100000)
end


local function port_RX()
	-- read value back
	data_in = ''
	while true do
		local cc, msg = serial.readbytes(port_handle, 4096)
		serial.usleep(10000)
		if cc > 0 then -- bytes have been read from the port
			data_in = data_in .. msg
			if (string.sub(data_in,-(#ready_prompt), -1) == ready_prompt) then
				-- up to first "\n" is the echoed command
				-- better way to fix?
				local _, i = string.find(data_in, "\n")
				data_in = string.sub(data_in, i+1, -1)
				break
			end
		end
		-- show progress for long wait
		--io.write(". ")
	end
end


port_open()
-- give controller time to spin up
serial.usleep(100000) -- min to allow atmega to spin up forth

io.write("Welcome to LuaTerminal. <enter> to begin and '\\' to end\n")
--io.write(ready_prompt..data_in)
repeat
	data_out = io.read()
	if data_out == '\\' then break
	else port_TX(data_out.."\n")
	end
	serial.usleep(100000)
	port_RX()
	io.write(data_in)
until false

port_close()
