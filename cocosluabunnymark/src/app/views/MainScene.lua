
local MainScene = class("MainScene", cc.load("mvc").ViewBase)


local function rrand(min_val, max_val)
    return math.random() * (max_val - min_val) + min_val
end

local textureRects = {
    cc.rect(2, 47, 26, 37),
    cc.rect(2, 86, 26, 37),
    cc.rect(2, 125, 26, 37),
    cc.rect(2, 164, 26, 37),
    cc.rect(2, 2, 26, 37)
}
local currentTexId = 0
local bunnies = {}
local texture
local label
function MainScene:addBunnies(amount)
    local tr = textureRects[currentTexId + 1]
    for i = 1, amount do
        local spr = cc.Sprite:createWithTexture(texture, tr)
        spr:setPosition(0, 0)
        local scale = rrand(0.5, 1.0)
        spr:setScale(scale)
        spr:setRotation((math.random() - 0.5) * 57.29577951)
        self:addChild(spr)
        table.insert(bunnies, { sprite = spr, speedx = math.random() * 10, speedy = math.random() * 10 - 5 })
    end
    label:setString(#bunnies .. "\nBUNNIES")
end

local maxX = 800
local maxY = 600
local function update(deltaTime)
    local d = 60 * deltaTime
    local gravityd = 0.5 * d
    for i = 1, #bunnies do
        local bunny = bunnies[i]
        local x, y = bunny.sprite:getPosition()
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
        bunny.sprite:setPosition(x, y)
    end
end

function MainScene:onCreate()
    texture = cc.Director:getInstance():getTextureCache():addImage("bunnys.png")

    self:scheduleUpdateWithPriorityLua(update, 0)
    local dispatcher = cc.Director:getInstance():getEventDispatcher()
    local listener = cc.EventListenerTouchAllAtOnce:create()
    listener:registerScriptHandler(function(touches, event)
        self:addBunnies(1000)
    end, cc.Handler.EVENT_TOUCHES_BEGAN)
    listener:registerScriptHandler(function(touches, event)
        currentTexId = (currentTexId + 1) % 5
    end, cc.Handler.EVENT_TOUCHES_ENDED)
    dispatcher:addEventListenerWithSceneGraphPriority(listener, self)


    label = cc.Label:createWithTTF("", "Roboto-Medium.ttf", 24);
    label:setTextColor(cc.c4b(255, 255, 255, 255));
    label:setAnchorPoint(0, 1);
    label:setPosition(10, maxY - 10);
    self:addChild(label, 1);
end

return MainScene
