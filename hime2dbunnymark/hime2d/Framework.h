#pragma once

#include "hime2d.h"

#include "VulkanFramework.h"
#include "VulkanBuffer.hpp"
#include "VulkanDevice.hpp"
#include "VulkanTexture.hpp"

#include "SpriteBatchRender.h"

NS_HIME_BEGIN

class Framework : public VulkanFramework {
public:
    Framework();
    virtual ~Framework();
    // TODO: TextureManager
    void loadTexture(const std::string& filename);
    // Prepare and initialize uniform buffer containing shader uniforms
    void prepareBuffers();
    virtual void prepare();
    virtual void render();
    virtual void update(float deltaTime) {}
private:
    void buildCommandBuffer(VkCommandBuffer drawCmdBuffer, VkFramebuffer frameBuffer);
    void draw();
    void setupVertexDescriptions();
    void setupDescriptorPool();
    void setupDescriptorSetLayout();
    void setupDescriptorSet();
    void preparePipelines();
protected:
    vks::Texture2D texture;
private:
    struct {
        VkPipelineVertexInputStateCreateInfo inputState;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    } vertices;
    struct {
        glm::mat4 projection;
    } uboVS;
    vks::Buffer uniformBuffer;

    VkPipeline spritePipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;
};

NS_HIME_END

