#pragma once

#include "hime2d.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

NS_HIME_BEGIN

namespace e_type {
    enum e_type_t {
        none = 0, // deleted
        entity = 1,
        sprite = 2
    };
};
namespace e_state {
    enum e_state_t {
        none = 0,
        hidden = 1 << 0,

        transform_dirty = 1 << 1,
        worldtransform_dirty = 1 << 2,
        sprite_dirty = 1 << 3,

        color_cascaded = 1 << 4,
    };
};

/**
 *  entity ID:
 *  size:       uint32_t (for lua reason)
 *  layout:     FF           FFFFFF
 *              (signature)  (index)
 */
typedef uint32_t Id;
#define make_entityid(index, sig) ((uint32_t)sig << 24 | index)
#define entityidx(id) (id & 0xFFFFFF)

namespace World {

#if HM_PLATFORM_MOBILE
    const uint32_t _initial_size = 128;
#else
    const uint32_t _initial_size = 1024;
#endif
    const uint32_t _max_entities = 0xFFFFFF;

    // constructor
    void initialize();
    
    // main loop
    void update(float deltaTime);

    // interfaces
    Id entity_create(uint32_t type = e_type::entity);
    void entity_delete(Id id);
    bool entity_isnull(Id id);
    void entity_addChild(Id id, Id childId);

    // node interfaces
    void entity_setVisible(Id id, bool visible);
    void entity_setPostion(Id id, const glm::vec2& position);
    const glm::vec2& entity_getPosition(Id id);
    void entity_setRotation(Id id, float rotation);
    void entity_setScale(Id id, float x, float y);
    // NOT like cocos2d-x. In hime2d, child's zorder = parent's zorder + child's local zorder.
    void entity_setLocalZOrder(Id id, float zorder);

    // sprite interfaces
    Id sprite_create(uint32_t texId, const glm::vec4& rect);
    void sprite_setColor(Id id, const glm::vec4& color);
    void sprite_setColorCascaded(Id id, bool enabled);

    // for ffi binding
    void ffi_setPosition(Id id, float x, float y);
    void ffi_getPosition(Id id, float *x, float *y);
};

NS_HIME_END

