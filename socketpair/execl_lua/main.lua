#!/usr/bin/env lua

local socket = require 'posix'
local fd = tonumber(arg[1])

while true do
    local line = socket.read(fd, 128)

    if line == nil then
        break
    end
    io.write("Parent sent ", line, "\n")
    if line == "quit" then
        socket.close(fd)
        io.write("Child says Bye!\n")
        break
    end
    socket.write(fd, line)
end

