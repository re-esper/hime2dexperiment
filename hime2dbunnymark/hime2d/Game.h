#pragma once

#include "Framework.h"
#include "World.h"
extern "C" {
#include "lua.h"
}

class Hime2DExperimentGame : public hime2d::Framework {
public:
    Hime2DExperimentGame();
    virtual ~Hime2DExperimentGame();
    virtual void prepare();
    virtual void update(float deltaTime);
    virtual void onUpdateUIOverlay(vks::UIOverlay* overlay);
private:
    void addBunnies(int32_t amount);
    bool isClickDown();
private:
    lua_State* L;
    struct Bunny {
        hime2d::Id entity;
        float speedX, speedY;
    };
    std::vector<Bunny> bunnies;
    uint32_t bunnyCount = 0;
    uint32_t currentTexId = 0;
    float clickDownTime = -1.f;
    bool isDown = false;
};
