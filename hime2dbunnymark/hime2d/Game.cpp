#include "Game.h"
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "bindings.h"

USING_NS_HIME;

const float bunniesAddingThreshold = 0.1f; // in seconds
const float gravity = 0.5f;

inline float rrand() { return (float)rand() / (float)RAND_MAX; }
inline float rrand(float a, float b) {
    float r = (float)rand() / (float)RAND_MAX;
    return a + r * (b - a);
}

#define BUNNY_AMOUNT_EACH_TIME 5000

std::vector<glm::vec4> textureRects;

Hime2DExperimentGame::Hime2DExperimentGame() : Framework()
{
    title = "HIME 2D";
    settings.overlay = true;
}

Hime2DExperimentGame::~Hime2DExperimentGame()
{
#ifdef HIME2D_LUA
    lua_close(L);
#endif
}

void Hime2DExperimentGame::prepare()
{
    loadTexture(getAssetPath() + "textures/bunnys.png");
    textureRects.push_back({ 2, 47, 26, 37 });
    textureRects.push_back({ 2, 86, 26, 37 });
    textureRects.push_back({ 2, 125, 26, 37 });
    textureRects.push_back({ 2, 164, 26, 37 });
    textureRects.push_back({ 2, 2, 26, 37 });
    Framework::prepare();

#ifdef HIME2D_LUA
    L = luaL_newstate();
    luaL_openlibs(L);
    register_hime2d_lib(L);
    luaL_dofile(L, (getAssetPath() + "game.lua").c_str());
#endif
}

void Hime2DExperimentGame::addBunnies(int32_t amount)
{
    auto tr = textureRects[currentTexId];
    size_t sidx = bunnies.size();
    bunnies.resize(bunnies.size() + amount);
    for (size_t i = sidx; i < sidx + amount; ++i) {
        Bunny& bunny = bunnies[i];
        Id entity = World::sprite_create(0, tr);
        World::entity_setPostion(entity, glm::vec2(0, 0));
        float scale = rrand(0.5, 1.0);
        World::entity_setScale(entity, scale, scale);
        World::entity_setRotation(entity, rrand() - 0.5f);
        bunny.speedX = rrand() * 10;
        bunny.speedY = rrand() * 10 - 5;
        bunny.entity = entity;
    }
    bunnyCount += amount;
}

inline bool Hime2DExperimentGame::isClickDown() {
#if defined(_WIN32)
    return mouseButtons.left;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    return touchDown;
#endif
}

void Hime2DExperimentGame::update(float deltaTime)
{
#ifndef HIME2D_LUA
    if (isClickDown()) {
        if (clickDownTime < 0) {
            clickDownTime = deltaTime;
            addBunnies(BUNNY_AMOUNT_EACH_TIME);
        }
        else {
            clickDownTime += deltaTime;
            if (clickDownTime > bunniesAddingThreshold) {
                // add bunnies!
                addBunnies(BUNNY_AMOUNT_EACH_TIME);
                clickDownTime -= bunniesAddingThreshold;
            }
        }
    }
    else if (clickDownTime > 0) {
        clickDownTime = -1.0f;
        currentTexId++;
        currentTexId %= 5;
    }

    // update bunnies!
    float maxX = (float)width;
    float maxY = (float)height;
    float d = 60.f * deltaTime; // pixijs's bunnymark work at 60 fps
    float gravityd = gravity * d;
    for (auto& bunny : bunnies) {
        glm::vec2 position = World::entity_getPosition(bunny.entity);
        position.x += bunny.speedX * d;
        position.y += bunny.speedY * d;
        bunny.speedY += gravityd;

        if (position.x > maxX) {
            bunny.speedX *= -1;
            position.x = maxX;
        }
        else if (position.x < 0) {
            bunny.speedX *= -1;
            position.x = 0;
        }
        if (position.y > maxY) {
            bunny.speedY *= -0.85f;
            position.y = maxY;
            if (rand() & 1) { // if (Math.rrand() > 0.5)
                bunny.speedY -= rrand() * 6;
            }
        }
        else if (position.y < 0) {
            bunny.speedY = 0;
            position.y = 0;
        }
        World::entity_setPostion(bunny.entity, position);
    }
#else
    if (isClickDown() != isDown) {
        isDown = isClickDown();
        lua_getglobal(L, "onTouch");
        lua_pushstring(L, isDown ? "down" : "up");
        int error = lua_pcall(L, 1, 0, 0);
        if (error) {
            printf("ERROR: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
    lua_getglobal(L, "tick");
    lua_pushnumber(L, deltaTime);
    int error = lua_pcall(L, 1, 0, 0);
    if (error) {
        printf("ERROR: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
#endif
}

void Hime2DExperimentGame::onUpdateUIOverlay(vks::UIOverlay* overlay)
{
    static uint32_t lastCount = -1;
    static char str[0x80];
    if (lastCount != bunnyCount) {
        lastCount = bunnyCount;
        sprintf(str, "%d\nBUNNIES", bunnyCount);
    }
    overlay->text(str);
}