
#include "SpriteBatchRender.h"

NS_HIME_BEGIN

namespace SpriteBatchRender
{
    vks::VulkanDevice* _device;

    vks::Buffer _batchIndexBuffer;
    std::vector<vks::Buffer> _batchVertexBuffers;
    std::vector<SpriteQuad> _verts;

    uint32_t _lastTextureId;
}

void SpriteBatchRender::initialize(vks::VulkanDevice* device, VkQueue queue)
{
    _device = device;
    _verts.reserve(_batch_quad_reserved_size);
    _lastTextureId = -1;

    const uint16_t indices1[6] = { 0, 1, 2, 2, 3, 0 };
    std::vector<uint16_t> indices;
    for (int i = 0; i < _max_sprite_batch_size; ++i)
        for (int n = 0; n < 6; ++n)
            indices.push_back(i * 4 + indices1[n]);
    _batchIndexBuffer.create(_device, vks::BufferType::device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices.size() * sizeof(uint16_t));
    _batchIndexBuffer.uploadFromStaging(indices.data(), indices.size() * sizeof(uint16_t), queue);
}

#define VERTEX_BUFFER_BIND_ID 0
void SpriteBatchRender::draw(const VkCommandBuffer commandBuffer)
{
    VkDeviceSize offsets[1] = { 0 };
    for (auto& vkbuffer : _batchVertexBuffers) {
        vkCmdBindVertexBuffers(commandBuffer, VERTEX_BUFFER_BIND_ID, 1, &vkbuffer.buffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, _batchIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
        uint32_t indexCount = vkbuffer.descriptor.range / sizeof(SpriteQuad) * 6;
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }
}

void SpriteBatchRender::reset()
{
    for (auto& buffer : _batchVertexBuffers) {
        buffer.destroy();
    }
    _batchVertexBuffers.clear();
}

void SpriteBatchRender::flush()
{
    if (_verts.size() > 0) {
        _batchVertexBuffers.emplace_back();
        auto& buffer = _batchVertexBuffers.back();
        size_t bufferSize = _verts.size() * sizeof(SpriteQuad);
        buffer.create(_device, vks::BufferType::transient, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, bufferSize);
        buffer.upload(_verts.data(), bufferSize);
        _verts.clear();
    }
}

NS_HIME_END