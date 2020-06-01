#include "Framework.h"
#include "World.h"

NS_HIME_BEGIN

#define ENABLE_VALIDATION       false
#define VERTEX_BUFFER_BIND_ID   0

Framework::Framework() : VulkanFramework(ENABLE_VALIDATION)
{
}

Framework::~Framework()
{
    vkDestroyPipeline(device, spritePipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    texture.destroy();
    uniformBuffer.destroy();
}

void Framework::prepare()
{
    VulkanFramework::prepare();
    setupVertexDescriptions();
    prepareBuffers();
    setupDescriptorSetLayout();
    preparePipelines();
    setupDescriptorPool();
    setupDescriptorSet();

    SpriteBatchRender::initialize(vulkanDevice, queue);

    prepared = true;
}

void Framework::render()
{
    SpriteBatchRender::reset();
    update(frameDeltaTime);
    World::update(frameDeltaTime);
    if (prepared) draw();
}

void Framework::buildCommandBuffer(VkCommandBuffer drawCmdBuffer, VkFramebuffer frameBuffer)
{
    VkCommandBufferBeginInfo cmdBufInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    VK_CHECK(vkBeginCommandBuffer(drawCmdBuffer, &cmdBufInfo));

    VkClearValue clearValues[2];
    clearValues[0].color = { { 1.0f, 1.0f, 1.0f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f, 0 };
    VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;
    renderPassBeginInfo.framebuffer = frameBuffer;
    vkCmdBeginRenderPass(drawCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
    vkCmdSetViewport(drawCmdBuffer, 0, 1, &viewport);

    VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
    vkCmdSetScissor(drawCmdBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(drawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
    vkCmdBindPipeline(drawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline);

    SpriteBatchRender::draw(drawCmdBuffer);

    drawUI(drawCmdBuffer);

    vkCmdEndRenderPass(drawCmdBuffer);
    VK_CHECK(vkEndCommandBuffer(drawCmdBuffer));
}

void Framework::draw()
{
    VulkanFramework::prepareFrame();

    VK_CHECK(vkWaitForFences(device, 1, &waitFences[currentBuffer], VK_TRUE, UINT64_MAX));
    VK_CHECK(vkResetFences(device, 1, &waitFences[currentBuffer]));

    // Build command buffer if needed
    //if (!drawCmdBuffersValid[currentBuffer]) {
        buildCommandBuffer(drawCmdBuffers[currentBuffer], frameBuffers[currentBuffer]);
    //    drawCmdBuffersValid[currentBuffer] = true;
    //}

    submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
    submitInfo.commandBufferCount = 1;
    VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, waitFences[currentBuffer]));

    VulkanFramework::submitFrame();
}

void Framework::setupVertexDescriptions()
{
    // Binding description
    vertices.bindingDescriptions = {
        vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, sizeof(SpriteVertex), VK_VERTEX_INPUT_RATE_VERTEX),
    };
    vertices.attributeDescriptions = {
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0),
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(SpriteVertex, inTexcoord)),
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 2, VK_FORMAT_B8G8R8A8_UNORM, offsetof(SpriteVertex, inColors)),
    };

    vertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertices.bindingDescriptions.size());
    vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
    vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.attributeDescriptions.size());
    vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
}

void Framework::setupDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
    };
    VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(
        static_cast<uint32_t>(poolSizes.size()),
        poolSizes.data(),
        2);
    VK_CHECK(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
}

void Framework::setupDescriptorSetLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        // Binding 0 : Vertex shader uniform buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT,
            0),
        // Binding 1 : sampler
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            1)
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings.data(), static_cast<uint32_t>(setLayoutBindings.size()));
    VK_CHECK(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
    VK_CHECK(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
}

void Framework::setupDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

    // Setup a descriptor image info for the current texture to be used as a combined image sampler
    VkDescriptorImageInfo textureDescriptor;
    textureDescriptor.imageView = texture.view;
    textureDescriptor.sampler = texture.sampler;
    textureDescriptor.imageLayout = texture.imageLayout;
    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        // Binding 0 : Vertex shader uniform buffer
        vks::initializers::writeDescriptorSet(
            descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            0,
            &uniformBuffer.descriptor),
        vks::initializers::writeDescriptorSet(
            descriptorSet,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            1,
            &textureDescriptor)
    };
    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}

void Framework::preparePipelines()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

    VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(
        VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);

    VkPipelineColorBlendAttachmentState blendAttachmentState = {};
    blendAttachmentState.blendEnable = VK_TRUE;
    blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

    VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(
        VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

    VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

    VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);

    std::vector<VkDynamicState> dynamicStateEnables = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(
        dynamicStateEnables.data(), static_cast<uint32_t>(dynamicStateEnables.size()), 0);

    // Load shaders
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
    shaderStages[0] = loadShader(getAssetPath() + "shaders/sprite.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader(getAssetPath() + "shaders/sprite.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(
        pipelineLayout, renderPass, 0);

    pipelineCreateInfo.pVertexInputState = &vertices.inputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();

    VK_CHECK(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &spritePipeline));
}

void Framework::loadTexture(const std::string& filename)
{
    texture.loadFromFile(filename, vulkanDevice, queue);
}

void Framework::prepareBuffers()
{
    uniformBuffer.create(vulkanDevice, vks::BufferType::device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(uboVS));
    float fov = 90;
    float eyed = height * 0.5f / std::tan(glm::radians(fov) * 0.5f);
    glm::vec3 eye = { 0.0f, 0.0f, -eyed };
    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)width / -(float)height, 0.1f, 1000.0f);
    glm::mat4 view = glm::lookAt(eye, glm::vec3(0, 0, 0), glm::vec3(0, 1.0, 0));
    view = glm::translate(view, glm::vec3(-(float)width / 2, -(float)height / 2, 0));
    uboVS.projection = projection * view;
    uniformBuffer.uploadFromStaging(&uboVS, sizeof(uboVS), queue);
}

NS_HIME_END