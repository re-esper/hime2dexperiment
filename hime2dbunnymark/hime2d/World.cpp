#include "World.h"
#include "SpriteBatchRender.h"

#include <unordered_map>
#include <deque>

NS_HIME_BEGIN

struct Transform {
    glm::vec2   position;
    float       rotation;
    glm::vec2   scale;
};
struct AffineTransform {
    float a, b, c, d;
    float tx, ty;
};
struct SpriteData {
    uint32_t    texture;
    glm::vec4   vertexPositions[4];
};

namespace World
{
    // basic
    std::vector<uint32_t>           e_signatures; // the lower 24 bit is e_type_t, the higher 8 bit is signature count.
    std::vector<uint32_t>           e_states;
    // heirarchy
    std::vector<Id>                 e_parents;
    std::unordered_multimap<Id, Id> e_children;
    // node
    std::vector<float>              e_zorders; // NOT like cocos2d-x. In hime2d, child's zorder = parent's zorder + child's local zorder.
    // transform
    std::vector<Transform>          e_transforms;
    std::vector<AffineTransform>    e_localAffines;
    std::vector<glm::mat4>          e_worldMatrices;
    // sprite
    std::vector<SpriteData>         e_spriteDatas;
    std::vector<glm::vec4>          e_localColors;
    std::vector<glm::vec4>          e_worldColors;
    std::vector<SpriteQuad>         e_vertices;

    // privates 
    Id _maxId = 0;
    std::deque<Id> _freeQueue;

    // sprite system privates
    bool _isRenderOrderDirty = false;
    std::vector<Id> _renderOrder;
};

using namespace World;
void World::initialize()
{
    e_signatures.reserve(_initial_size);
    e_states.reserve(_initial_size);
    e_parents.reserve(_initial_size);
    e_zorders.reserve(_initial_size);
    e_transforms.reserve(_initial_size);
    e_localAffines.reserve(_initial_size);
    e_worldMatrices.reserve(_initial_size);
    e_spriteDatas.reserve(_initial_size);
    e_localColors.reserve(_initial_size);
    e_worldColors.reserve(_initial_size);
    e_vertices.reserve(_initial_size);
}

inline void _components_resize(uint32_t size)
{
    e_signatures.resize(size);
    e_states.resize(size);
    e_parents.resize(size);
    e_zorders.resize(size);
    e_transforms.resize(size);
    e_localAffines.resize(size);
    e_worldMatrices.resize(size);
    e_spriteDatas.resize(size);
    e_localColors.resize(size);
    e_worldColors.resize(size);
    e_vertices.resize(size);
}

inline void _entity_init(Id index, uint8_t signature, uint32_t type)
{
    e_signatures[index] = ((uint32_t)signature << 24) | type;
    e_parents[index] = index; // no parents
    e_transforms[index] = {
        glm::vec2(0, 0), // position
        0,                  // rotation
        glm::vec2(1, 1),    // scale
    };
    e_worldMatrices[index] = glm::mat4(1.0);
    e_zorders[index] = 0;

    e_states[index] = e_state::transform_dirty;
}

Id World::entity_create(uint32_t type)
{
    _isRenderOrderDirty = true;

    if (_freeQueue.empty()) {
        Id id = _maxId++;
        assert(id < _max_entities);
        _components_resize(id + 1);
        _entity_init(id, 0, type);
        return id;
    }
    else {
        Id index = _freeQueue.front();
        assert((e_signatures[index] & 0xFFFFFF) == e_type::none);
        _freeQueue.pop_front();
        uint8_t signature = (uint8_t)(e_signatures[index] >> 24) + 1;
        _entity_init(index, signature, type);
        return make_entityid(index, signature);
    }
}

bool World::entity_isnull(Id id)
{
    uint32_t signature = e_signatures[entityidx(id)];
    if ((signature & 0xFFFFFF) == e_type::none)
        return true;
    // even if a valid entity already exists at the slot, it does not mean the Id is valid,
    // the signature still needs to be validated.
    return (signature >> 24) != (id >> 24);
}

inline void _deleteEntityByIndex(Id index)
{
    _freeQueue.push_back(index);
    e_signatures[index] = (e_signatures[index] & 0xFF000000) | e_type::none;
    // delete children
    auto children = e_children.equal_range(index);
    if (children.first != e_children.end()) {
        for (auto itr = children.first; itr != children.second; ++itr) {
            _deleteEntityByIndex(itr->second);
        }
        e_children.erase(index);
    }
}

inline void _heirarchy_removeFromParent(Id index)
{
    if (e_parents[index] != index) { // has a parent
        Id parentidx = e_parents[index];
        auto children = e_children.equal_range(parentidx);
        for (auto itr = children.first; itr != children.second; ++itr) {
            if (itr->second == index) {
                e_children.erase(itr);
                break;
            }
        }
    }
}

void World::entity_delete(Id id)
{
    assert(!entity_isnull(id));
    Id index = entityidx(id);
    _deleteEntityByIndex(index);
    _heirarchy_removeFromParent(index);
}

FORCE_INLINE float _entity_localZOrder(Id index)
{
    float z = e_zorders[index];
    Id pindex = e_parents[index];
    if (pindex != index) {
        z -= e_zorders[pindex];
    }
    return z;
}
FORCE_INLINE void _entity_addZOrder(Id index, float zorderdiff) {
    if (zorderdiff != 0) {
        _isRenderOrderDirty = true;
        e_zorders[index] += zorderdiff;
        auto children = e_children.equal_range(index);
        if (children.first != e_children.end()) {
            for (auto itr = children.first; itr != children.second; ++itr) {
                _entity_addZOrder(itr->second, zorderdiff);
            }
        }
    }
}

void World::entity_addChild(Id id, Id childId)
{
    assert(!entity_isnull(id) && !entity_isnull(childId));
    Id childidx = entityidx(childId);
    Id pidx = entityidx(id);
    // z-order
    float oldz = e_zorders[childidx];
    float localz = _entity_localZOrder(childidx);
    // remove relationship from old parent
    _heirarchy_removeFromParent(childidx);
    // add to new parent
    e_parents[childidx] = pidx;
    e_children.insert({ pidx, childidx });
    // reset z-order
    float z = e_zorders[pidx] + localz;
    _entity_addZOrder(childidx, z - oldz);
}

FORCE_INLINE void _entity_updateLocalFromTransform(Id index)
{
    Transform& transform = e_transforms[index];    
    float c = std::cos(transform.rotation);
    float s = std::sin(transform.rotation);
    auto& affine = e_localAffines[index];
    affine.a = c * transform.scale.x;
    affine.b = s * transform.scale.x;
    affine.c = -s * transform.scale.y;
    affine.d = c * transform.scale.y;
    affine.tx = transform.position.x;
    affine.ty = transform.position.y;
}

FORCE_INLINE glm::mat4 _affineToMatrix(const AffineTransform& t) {
    return glm::mat4(
        t.a, t.b, 0, 0,
        t.c, t.d, 0, 0,
        0, 0, 1, 0,
        t.tx, t.ty, 0, 1
    );
}

FORCE_INLINE void _affineToMatrix(const AffineTransform& t, glm::mat4& m) {
    m[0][0] = t.a;
    m[0][1] = t.b;
    m[1][0] = t.c;
    m[1][1] = t.d;
    m[3][0] = t.tx;
    m[3][1] = t.ty;
}

FORCE_INLINE const glm::mat4& _entity_getWorldMatrix(Id index)
{
    if (e_states[index] & e_state::transform_dirty) {
        // update local matrix
        _entity_updateLocalFromTransform(index);
        // update world matrix
        if (e_parents[index] != index) {
            e_worldMatrices[index] = _entity_getWorldMatrix(e_parents[index]) * _affineToMatrix(e_localAffines[index]);
        }
        else {
            _affineToMatrix(e_localAffines[index], e_worldMatrices[index]);
        }
        e_states[index] &= ~e_state::transform_dirty;
        e_states[index] &= ~e_state::worldtransform_dirty;
        e_states[index] |= e_state::sprite_dirty;
    }
    else if (e_states[index] & e_state::worldtransform_dirty) {
        if (e_parents[index] != index) {
            e_worldMatrices[index] = _entity_getWorldMatrix(e_parents[index]) * _affineToMatrix(e_localAffines[index]);
        }
        else {
            _affineToMatrix(e_localAffines[index], e_worldMatrices[index]);
        }
        e_states[index] &= ~e_state::worldtransform_dirty;
        e_states[index] |= e_state::sprite_dirty;
    }
    return e_worldMatrices[index];
}

void World::update(float deltaTime)
{
    // update transforms
    for (Id idx = 0; idx < _maxId; ++idx) {
        if ((e_signatures[idx] & 0xFFFFFF) == e_type::none) continue;

        if (e_states[idx] & e_state::transform_dirty) {
            // update local matrix
            _entity_updateLocalFromTransform(idx);

            // update world matrix
            Id pindex = e_parents[idx];
            if (pindex != idx) { // has a parent
                e_worldMatrices[idx] = _entity_getWorldMatrix(e_parents[idx]) * _affineToMatrix(e_localAffines[idx]);
            }
            else {
                _affineToMatrix(e_localAffines[idx], e_worldMatrices[idx]);
            }
            e_states[idx] &= ~e_state::transform_dirty;
            e_states[idx] &= ~e_state::worldtransform_dirty;
            e_states[idx] |= e_state::sprite_dirty;
        }
        else if (e_states[idx] & e_state::worldtransform_dirty) {
            Id pindex = e_parents[idx];
            if (pindex != idx) { // has a parent
                e_worldMatrices[idx] = _entity_getWorldMatrix(e_parents[idx]) * _affineToMatrix(e_localAffines[idx]);
            }
            else {
                _affineToMatrix(e_localAffines[idx], e_worldMatrices[idx]);
            }
            e_states[idx] &= ~e_state::worldtransform_dirty;
            e_states[idx] |= e_state::sprite_dirty;
        }
    }

    // update sprites
    for (Id idx = 0; idx < _maxId; ++idx) {
        if ((e_signatures[idx] & 0xFFFFFF) != e_type::sprite) continue;
        if (e_states[idx] & e_state::sprite_dirty) {
            auto& verts = e_vertices[idx].verts;
            auto& vpos = e_spriteDatas[idx].vertexPositions;
            auto& mat = e_worldMatrices[idx];
            for (int i = 0; i < 4; ++i) {
                verts[i].inPosition = mat * vpos[i];
            }
            e_states[idx] &= ~e_state::sprite_dirty;
        }
    }

    // sort
    if (_isRenderOrderDirty) {
        _renderOrder.clear();
        _renderOrder.reserve(_maxId);
        for (Id idx = 0; idx < _maxId; ++idx) {
            if ((e_signatures[idx] & 0xFFFFFF) == e_type::sprite) {
                _renderOrder.push_back(idx);
            }
        }
        std::stable_sort(_renderOrder.begin(), _renderOrder.end(), [](const Id idx1, const Id idx2) {
            return e_zorders[idx1] < e_zorders[idx2];
        });
        _isRenderOrderDirty = false;
    }

#ifndef NORENDER
    // finally, render them all by order!!
    for (auto idx : _renderOrder) {
        // TODO: Test good or not that 'entity_delete' makes render order dirty
        if ((e_states[idx] & e_state::hidden) == 0 && (e_signatures[idx] & 0xFFFFFF) != e_type::none) {
            SpriteBatchRender::renderSprite(0, e_vertices[idx]);
        }
    }
    SpriteBatchRender::flush();
#endif
}

FORCE_INLINE void _entity_setWorldTransformDirty(Id index)
{
    if ((e_states[index] & e_state::worldtransform_dirty) == 0) {
        e_states[index] |= e_state::worldtransform_dirty;
        auto children = e_children.equal_range(index);
        if (children.first != e_children.end()) {
            for (auto itr = children.first; itr != children.second; ++itr) {
                _entity_setWorldTransformDirty(itr->second);
            }
        }
    }
}
FORCE_INLINE void _entity_setTransformDirty(Id index)
{
    if ((e_states[index] & e_state::transform_dirty) == 0) {
        e_states[index] |= e_state::transform_dirty;
        auto children = e_children.equal_range(index);
        if (children.first != e_children.end()) {
            for (auto itr = children.first; itr != children.second; ++itr) {
                _entity_setWorldTransformDirty(itr->second);
            }
        }
    }
}

void World::entity_setVisible(Id id, bool visible)
{
}

void World::entity_setPostion(Id id, const glm::vec2& position)
{
    assert(!entity_isnull(id));
    Id index = entityidx(id);
    e_transforms[index].position = position;
    _entity_setTransformDirty(index);
}

void World::ffi_setPosition(Id id, float x, float y)
{
    Id index = entityidx(id);
    e_transforms[index].position.x = x;
    e_transforms[index].position.y = y;
    _entity_setTransformDirty(index);
}

const glm::vec2& World::entity_getPosition(Id id)
{
    assert(!entity_isnull(id));
    Id index = entityidx(id);
    return e_transforms[index].position;
}

void World::ffi_getPosition(Id id, float *x, float *y)
{
    Id index = entityidx(id);
    auto& pos = e_transforms[index].position;
    *x = pos.x; *y = pos.y;
}

void World::entity_setRotation(Id id, float rotation)
{
    assert(!entity_isnull(id));
    Id index = entityidx(id);
    e_transforms[index].rotation = rotation;
    _entity_setTransformDirty(index);
}

void World::entity_setScale(Id id, float x, float y)
{
    assert(!entity_isnull(id));
    Id index = entityidx(id);
    e_transforms[index].scale.x = x;
    e_transforms[index].scale.y = y;
    _entity_setTransformDirty(index);
}

void World::entity_setLocalZOrder(Id id, float zorder)
{
    assert(!entity_isnull(id));
    Id index = entityidx(id);
    _entity_addZOrder(index, zorder - _entity_localZOrder(index));
}

Id World::sprite_create(uint32_t texId, const glm::vec4& rect)
{
    Id id = entity_create(e_type::sprite);
    Id index = entityidx(id);

    //glm::vec2 texSize = TextureCache::getTextureSize(texId);
    float texw = 30, texh = 203; // TODO:  implement TextureCache

    float tx = rect[0], ty = rect[1], tw = rect[2], th = rect[3];
    float halfw = tw * 0.5f;
    float halfh = th * 0.5f;

    auto& spritedata = e_spriteDatas[index];
    spritedata.texture = texId;
    spritedata.vertexPositions[0] = { halfw, halfh, 0.0, 1.0 };
    spritedata.vertexPositions[1] = { -halfw, halfh, 0.0, 1.0 };
    spritedata.vertexPositions[2] = { -halfw, -halfh, 0.0, 1.0 };
    spritedata.vertexPositions[3] = { halfw, -halfh, 0.0, 1.0 };

    auto& verts = e_vertices[index].verts;
    verts[0].inTexcoord = { (tw + tx) / texw, (th + ty) / texh };
    verts[1].inTexcoord = { tx / texw, (th + ty) / texh };
    verts[2].inTexcoord = { tx / texw, ty / texh };
    verts[3].inTexcoord = { (tw + tx) / texw, ty / texh };

    sprite_setColor(id, glm::vec4(1.0));

    e_states[index] |= e_state::sprite_dirty;
    return id;
}

FORCE_INLINE void _updateVertexColor(Id index)
{
    glm::u8vec4 color4b = e_worldColors[index] * 255.0f;
    auto& verts = e_vertices[index].verts;
    for (int i = 0; i < 4; ++i) {
        verts[i].inColors = color4b;
    }
}

FORCE_INLINE void _setChildWorldColor(Id index, Id pindex)
{
    e_worldColors[index] = e_worldColors[pindex] * e_localColors[index];
    _updateVertexColor(index);
    auto children = e_children.equal_range(index);
    if (children.first != e_children.end()) {
        for (auto itr = children.first; itr != children.second; ++itr) {
            _setChildWorldColor(itr->second, index);
        }
    }
}

void World::sprite_setColor(Id id, const glm::vec4& color)
{
    // unlike Transform, Color is updated immediately, rather than calculated in 'update'.
    assert(!entity_isnull(id));
    Id index = entityidx(id);
    e_localColors[index] = color;
    if (e_parents[index] != index) {
        e_worldColors[index] = e_worldColors[e_parents[index]] * e_localColors[index];
    }
    else {
        e_worldColors[index] = color;
    }
    _updateVertexColor(index);

    auto children = e_children.equal_range(index);
    if (children.first != e_children.end()) {
        for (auto itr = children.first; itr != children.second; ++itr) {
            _setChildWorldColor(itr->second, index);
        }
    }
}

void World::sprite_setColorCascaded(Id id, bool enabled)
{
}

NS_HIME_END