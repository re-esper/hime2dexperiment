#pragma once

#include "hime2d.h"
#include "VulkanBuffer.hpp"
#include "VulkanDevice.hpp"

struct SpriteVertex {
    glm::vec4 inPosition;
    glm::vec2 inTexcoord;
    glm::u8vec4 inColors;
};
struct SpriteQuad {
    SpriteVertex verts[4];
};

NS_HIME_BEGIN

namespace SpriteBatchRender {

    const int _max_sprite_batch_size = 8192; // cocos2d-x: 65536 / 4 = 16384
    const int _batch_quad_reserved_size = 64; // cocos2d-x: 64

    // constructor
    void initialize(vks::VulkanDevice* device, VkQueue queue);

    // build draw commands
    void draw(const VkCommandBuffer commandBuffer);

    // destroy current batchVertexBuffers
    void reset();

    // render a sprite in batch
    extern vks::VulkanDevice* _device;
    extern std::vector<vks::Buffer> _batchVertexBuffers;
    extern std::vector<SpriteQuad> _verts;
    FORCE_INLINE void renderSprite(uint32_t texId, const SpriteQuad& quad) {
        assert(_verts.size() <= _max_sprite_batch_size);
        if (UNLIKELY(_verts.size() == _max_sprite_batch_size)) {
            _batchVertexBuffers.emplace_back();
            auto& buffer = _batchVertexBuffers.back();
            size_t bufferSize = _verts.size() * sizeof(SpriteQuad);
            buffer.create(_device, vks::BufferType::transient, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, bufferSize);
            buffer.upload(_verts.data(), bufferSize);
            _verts.clear();
        }
        _verts.push_back(quad);
    }

    void flush();
};

NS_HIME_END