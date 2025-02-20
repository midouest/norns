local Playdate = {}

_norns.playdate = {}

_norns.playdate.add = function(id, name, dev)
    Playdate.dev = dev
    Playdate.add(id, name, dev)
end

_norns.playdate.remove = function(id)
    Playdate.dev = nil
    Playdate.remove(id)
end

local ebuffer = ""
_norns.playdate.event = function(id, line)
    local function evalplaydate(line)
        line = ebuffer..line
        ebuffer = ""
        if line:sub(1, 6) == "~ctl: " then
            if line:sub(7, 12) == "accel " then
                if Playdate.accel ~= nil then
                    local next = line:sub(13, #line):gmatch("%S+")
                    local x = tonumber(next())
                    local y = tonumber(next())
                    local z = tonumber(next())
                    Playdate.accel(x, y, z)
                end
            elseif line:sub(7, 12) == "crank " then
                if Playdate.crank ~= nil then
                    local d = tonumber(line:sub(13, #line))
                    Playdate.crank(d)
                end
            elseif line:sub(7, 10) == "btn " then
                if Playdate.button ~= nil then
                    local b = line:sub(11, 11)
                    local s = tonumber(line:sub(13, 13))
                    Playdate.button(b, s)
                end
            elseif line:sub(7, 16) == "crankdock " then
                if Playdate.crankdock ~= nil then
                    local s = tonumber(line:sub(17, 17))
                    Playdate.crankdock(s)
                end
            end
        else
            if Playdate.event ~= nil then
                Playdate.event(line)
            end
        end
    end

    local n, m = line:find("%c+")
    if not n then
        ebuffer = ebuffer..line
    else
        evalplaydate(line:sub(1, n-1))
        if m < #line then
            _norns.playdate.event(id, line:sub(m+1, -1))
        end
    end
end

--- Run a PDX on Playdate
function Playdate.run(path)
    if Playdate.dev then
        _norns.playdate_run(Playdate.dev, path)
    end
end

--- Send a string to Playdate
function Playdate.send(msg)
    if Playdate.dev then
        _norns.playdate_send(Playdate.dev, msg)
    end
end

--- Start controller mode on Playdate. Playdate will stream accelerometer,
-- button and crank events until controller_stop() is called or the lock button
-- is pressed.
function Playdate.controller_start()
    if Playdate.dev then
        _norns.playdate_controller_start(Playdate.dev)
    end
end

--- Stop controller mode on Playdate.
function Playdate.controller_stop()
    if Playdate.dev then
        _norns.playdate_controller_stop(Playdate.dev)
    end
end

--- True if Playdate is connected
function Playdate.connected()
    return Playdate.dev ~= nil
end

function Playdate.init()
    Playdate.add = function(id, name, dev)
        print(">>>>>> playdate.add / " .. id .. " / " .. name)
    end
    Playdate.remove = function(id) print(">>>>>> playdate.remove " .. id) end
    Playdate.event = nil
    Playdate.accel = nil
    Playdate.button = nil
    Playdate.crank = nil
    Playdate.crankdock = nil
end

Playdate.init()

return Playdate
