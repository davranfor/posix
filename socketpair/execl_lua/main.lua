#!/usr/bin/env lua

local socket = require 'posix'
local fd = tonumber(arg[1])

while true do
    local line = socket.read(fd, 128)

    if line == nil then
        break
    end
    if line == "quit" then
        socket.close(fd)
        break
    end
    --local info = "parent says " .. line
    --print(info)
    io.write("parent says ", line, "\n")
    socket.write(fd, line)
end

