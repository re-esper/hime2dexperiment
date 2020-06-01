
local function rrand(min_val, max_val)
    return math.random() * (max_val - min_val) + min_val
end

local textureRects = {
    { 2, 47, 26, 37 },
    { 2, 86, 26, 37 },
    { 2, 125, 26, 37 },
    { 2, 164, 26, 37 },
    { 2, 2, 26, 37 }
}
local currentTexId = 0
local bunnies = {}

local function addBunnies(amount)
    local tr = textureRects[currentTexId + 1]
    for i = 1, amount do
        local spr = hime.sprite_create(0, unpack(tr))
        hime.entity_setPosition(spr, 0, 0)
        local scale = rrand(0.5, 1.0)
        hime.entity_setScale(spr, scale)
        hime.entity_setRotation(spr, math.random() - 0.5)
        table.insert(bunnies, { sprite = spr, speedx = math.random() * 10, speedy = math.random() * 10 - 5 })
    end
    print("BUNNIES: " .. #bunnies)
end

function onTouch(event)
    if event == "down" then
        addBunnies(5000)
    elseif event == "up" then
        currentTexId = (currentTexId + 1) % 5
    end
end


local ffi = require "ffi"
local ffi_setPosition = ffi.cast("void(*)(unsigned int, float, float)", _ptr_setPosition)
local _ffi_getPosition = ffi.cast("void(*)(unsigned int, float*, float*)", _ptr_getPosition)
local tmp = ffi.new "float[2]"
local function ffi_getPosition(entity)
    _ffi_getPosition(entity, tmp, tmp+1)
    return tmp[0], tmp[1]
end
_ptr_setPosition = nil
_ptr_getPosition = nil

local maxX = 800
local maxY = 600
function tick(deltaTime)
    local d = 60 * deltaTime
    local gravityd = 0.5 * d

    for i = 1, #bunnies do
        local bunny = bunnies[i]
        --local x, y = hime.entity_getPosition(bunny.sprite)
        local x, y = ffi_getPosition(bunny.sprite)
        x = x + bunny.speedx * d
        y = y + bunny.speedy * d
        bunny.speedy = bunny.speedy + gravityd
        if x > maxX then
            bunny.speedx = -bunny.speedx
            x = maxX
        elseif x < 0 then
            bunny.speedx = -bunny.speedx
            x = 0
        end
        if y > maxY then
            bunny.speedy = -0.85 * bunny.speedy
            y = maxY
            if math.random() > 0.5 then
                bunny.speedy = bunny.speedy - math.random() * 6
            end
        elseif y < 0 then
            bunny.speedy = 0
            y = 0
        end
        --hime.entity_setPosition(bunny.sprite, x, y)
        ffi_setPosition(bunny.sprite, x, y)
    end
end