#!/usr/bin/lua5.1

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
