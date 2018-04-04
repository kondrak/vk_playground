#include <SDL.h>
#include "Application.hpp"
#include "renderer/CameraDirector.hpp"

extern RenderContext  g_renderContext;
extern CameraDirector g_cameraDirector;

struct Vertex
{
    float x;
    float y;
    float z;
    float u;
    float v;
};

void Application::OnWindowResize(int newWidth, int newHeight)
{
    // fast window resizes return incorrect results from polled event - Vulkan surface query does it better
    Math::Vector2f windowSize = g_renderContext.WindowSize();

    if (windowSize.m_x > 0 && windowSize.m_y > 0)
    {
        m_noRedraw = false;
        g_renderContext.width  = (int)windowSize.m_x;
        g_renderContext.height = (int)windowSize.m_y;
        g_renderContext.halfWidth  = g_renderContext.width >> 1;
        g_renderContext.halfHeight = g_renderContext.height >> 1;
        g_renderContext.scrRatio = (float)windowSize.m_x / (float)windowSize.m_y;
        g_renderContext.left  = -g_renderContext.scrRatio;
        g_renderContext.right =  g_renderContext.scrRatio;

        g_renderContext.RecreateSwapChain(m_commandPool, m_renderPass);
        RebuildPipelines();
    }
    else
        m_noRedraw = true;
}

void Application::OnWindowMinimized(bool minimized)
{
    m_noRedraw = minimized;
    // force swap chain rebuilding when restoring the window
    if (!minimized)
        OnWindowResize(g_renderContext.width, g_renderContext.height);
}

void Application::OnStart(int argc, char **argv)
{
    m_pipeline.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VK_VERIFY(vk::createRenderPass(g_renderContext.device, g_renderContext.swapChain, &m_renderPass));
    VK_VERIFY(vk::createCommandPool(g_renderContext.device, &m_commandPool));

    // build the swap chain
    g_renderContext.RecreateSwapChain(m_commandPool, m_renderPass);

    m_texture = TextureManager::GetInstance()->LoadTexture("res/block_blue.png", m_commandPool);

    // create a common descriptor set layout and vertex buffer info
    m_vbInfo.bindingDescriptions.push_back(vk::getBindingDescription(sizeof(Vertex)));
    m_vbInfo.attributeDescriptions.push_back(vk::getAttributeDescription(inVertex, VK_FORMAT_R32G32B32_SFLOAT, 0));
    m_vbInfo.attributeDescriptions.push_back(vk::getAttributeDescription(inTexCoord, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 3));
    CreateDescriptorSetLayout();

    // single shared uniform buffer
    VK_VERIFY(vk::createUniformBuffer(g_renderContext.device, sizeof(UniformBufferObject), &m_uniformBuffer));

    /* Create buffers here */
    m_descriptor.setLayout = m_dsLayout;

    const Vertex verts[4] = { { 0.0, 0.0, -1.0, 0.0, 0.0 },
                              { 0.0, 1.0, -1.0, 0.0, 1.0 },
                              { 1.0, 0.0, -1.0, 1.0, 0.0 },
                              { 1.0, 1.0, -1.0, 1.0, 1.0 } };

    const uint32_t indices[6] = { 0, 1, 2, 1, 3, 2 };

    // vertex buffer and index buffer with staging buffer
    vk::createVertexBuffer(g_renderContext.device, m_commandPool,
                           verts, sizeof(Vertex) * 4, &m_vertexBuffer);
    vk::createIndexBuffer(g_renderContext.device, m_commandPool,
                          indices, sizeof(uint32_t) * 6, &m_indexBuffer);
    const vk::Texture *textureSet[1] = { *m_texture };
    CreateDescriptor(textureSet, &m_descriptor);

    RebuildPipelines();
    VK_VERIFY(vk::createCommandBuffers(g_renderContext.device, m_commandPool, m_commandBuffers, g_renderContext.frameBuffers.size()));

    g_cameraDirector.AddCamera(Math::Vector3f(0.5f, 0.5f, 0.f),
                               Math::Vector3f(0.f, 1.f, 0.f),
                               Math::Vector3f(1.f, 0.f, 0.f),
                               Math::Vector3f(0.f, 0.f, -1.f));

    // set to "clean" perspective matrix
    g_cameraDirector.GetActiveCamera()->SetMode(Camera::CAM_FPS);
}

void Application::OnRender()
{
    if (m_noRedraw)
        return;

    VkResult renderResult = g_renderContext.RenderStart();

    // incompatile swapchain - recreate it and skip this frame
    if (renderResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        g_renderContext.RecreateSwapChain(m_commandPool, m_renderPass);
        RebuildPipelines();
        return;
    }

    // render the bsp
    g_cameraDirector.GetActiveCamera()->UpdateView();
    //m_q3map->OnRender();
    m_ubo.ModelViewProjectionMatrix = g_renderContext.ModelViewProjectionMatrix;

    void *data;
    vmaMapMemory(g_renderContext.device.allocator, m_uniformBuffer.allocation, &data);
    memcpy(data, &m_ubo, sizeof(m_ubo));
    vmaUnmapMemory(g_renderContext.device.allocator, m_uniformBuffer.allocation);

    // record new set of command buffers including only visible faces and patches
    RecordCommandBuffers();

    // render visible faces
    VK_VERIFY(g_renderContext.Submit(m_commandBuffers));

    renderResult = g_renderContext.Present(false);

    // recreate swapchain if it's out of date
    if (renderResult == VK_ERROR_OUT_OF_DATE_KHR || renderResult == VK_SUBOPTIMAL_KHR)
    {
        g_renderContext.RecreateSwapChain(m_commandPool, m_renderPass);
        RebuildPipelines();
    }
}

void Application::OnUpdate(float dt)
{
    UpdateCamera(dt);
}

void Application::OnTerminate()
{
    vkDeviceWaitIdle(g_renderContext.device.logical);
    vk::destroyPipeline(g_renderContext.device, m_pipeline);
    vkDestroyDescriptorPool(g_renderContext.device.logical, m_descriptor.pool, nullptr);
    vk::freeBuffer(g_renderContext.device, m_uniformBuffer);
    vk::freeBuffer(g_renderContext.device, m_vertexBuffer);
    vk::freeBuffer(g_renderContext.device, m_indexBuffer);
    vk::freeCommandBuffers(g_renderContext.device, m_commandPool, m_commandBuffers);
    vk::destroyRenderPass(g_renderContext.device, m_renderPass);
    vkDestroyCommandPool(g_renderContext.device.logical, m_commandPool, nullptr);
    vkDestroyDescriptorSetLayout(g_renderContext.device.logical, m_dsLayout, nullptr);
}

bool Application::KeyPressed(KeyCode key)
{
    // to be 100% no undefined state exists
    if (m_keyStates.find(key) == m_keyStates.end())
        m_keyStates[key] = false;

    return m_keyStates[key];
}

void Application::OnKeyPress(KeyCode key)
{
    SetKeyPressed(key, true);

    switch (key)
    {
    case KEY_ESC:
        Terminate();
        break;
    default:
        break;
    }
}

void Application::OnKeyRelease(KeyCode key)
{
    SetKeyPressed(key, false);
}

void Application::OnMouseMove(int x, int y)
{
    g_cameraDirector.GetActiveCamera()->OnMouseMove(x, y);
}

void Application::UpdateCamera(float dt)
{
    static const float movementSpeed = 8.f;

    if (KeyPressed(KEY_A))
        g_cameraDirector.GetActiveCamera()->Strafe(-movementSpeed * dt);

    if (KeyPressed(KEY_D))
        g_cameraDirector.GetActiveCamera()->Strafe(movementSpeed * dt);

    if (KeyPressed(KEY_W))
        g_cameraDirector.GetActiveCamera()->MoveForward(-movementSpeed * dt);

    if (KeyPressed(KEY_S))
        g_cameraDirector.GetActiveCamera()->MoveForward(movementSpeed * dt);

    // do the barrel roll!
    if (KeyPressed(KEY_Q))
        g_cameraDirector.GetActiveCamera()->rotateZ(2.f * dt);

    if (KeyPressed(KEY_E))
        g_cameraDirector.GetActiveCamera()->rotateZ(-2.f * dt);

    // move straight up/down
    if (KeyPressed(KEY_R))
        g_cameraDirector.GetActiveCamera()->MoveUpward(movementSpeed * dt);

    if (KeyPressed(KEY_F))
        g_cameraDirector.GetActiveCamera()->MoveUpward(-movementSpeed * dt);
}

void Application::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding bindings[] = { uboLayoutBinding, samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;

    VK_VERIFY(vkCreateDescriptorSetLayout(g_renderContext.device.logical, &layoutInfo, nullptr, &m_dsLayout));
}

void Application::CreateDescriptor(const vk::Texture **textures, vk::Descriptor *descriptor)
{
    // create descriptor pool
    VkDescriptorPoolSize poolSizes[3];
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 1;

    VK_VERIFY(vkCreateDescriptorPool(g_renderContext.device.logical, &poolInfo, nullptr, &descriptor->pool));

    // create descriptor set
    VK_VERIFY(vk::createDescriptorSet(g_renderContext.device, descriptor));
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.offset = 0;
    bufferInfo.buffer = m_uniformBuffer.buffer;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textures[0]->imageView;
    imageInfo.sampler = textures[0]->sampler;

    VkWriteDescriptorSet descriptorWrites[2];
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptor->set;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;
    descriptorWrites[0].pImageInfo = nullptr;
    descriptorWrites[0].pTexelBufferView = nullptr;
    descriptorWrites[0].pNext = nullptr;
    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptor->set;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = nullptr;
    descriptorWrites[1].pImageInfo = &imageInfo;
    descriptorWrites[1].pTexelBufferView = nullptr;
    descriptorWrites[1].pNext = nullptr;

    vkUpdateDescriptorSets(g_renderContext.device.logical, 2, descriptorWrites, 0, nullptr);
}

void Application::RebuildPipelines()
{
    vkDeviceWaitIdle(g_renderContext.device.logical);
    vk::destroyPipeline(g_renderContext.device, m_pipeline);

    // todo: pipeline derivatives https://github.com/SaschaWillems/Vulkan/blob/master/examples/pipelines/pipelines.cpp
    const char *shaders[] = { "res/Basic_vert.spv", "res/Basic_frag.spv" };
    VK_VERIFY(vk::createPipeline(g_renderContext.device, g_renderContext.swapChain, m_renderPass, m_dsLayout, &m_vbInfo, &m_pipeline, shaders));
}

void Application::RecordCommandBuffers()
{
    for (size_t i = 0; i < m_commandBuffers.size(); ++i)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        VkResult result = vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);
        LOG_MESSAGE_ASSERT(result == VK_SUCCESS, "Could not begin command buffer: " << result);

        VkClearValue clearColors[2];
        clearColors[0].color = { 0.f, 0.f, 0.f, 1.f };
        clearColors[1].depthStencil = { 1.0f, 0 };
        VkRenderPassBeginInfo renderBeginInfo = {};
        renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderBeginInfo.renderPass = m_renderPass.renderPass;
        renderBeginInfo.framebuffer = g_renderContext.frameBuffers[i];
        renderBeginInfo.renderArea.offset = { 0, 0 };
        renderBeginInfo.renderArea.extent = g_renderContext.swapChain.extent;
        renderBeginInfo.clearValueCount = 2;
        renderBeginInfo.pClearValues = clearColors;

        vkCmdBeginRenderPass(m_commandBuffers[i], &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // queue standard faces
        vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.pipeline);

        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, &m_vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(m_commandBuffers[i], m_indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.layout, 0, 1, &m_descriptor.set, 0, nullptr);
        vkCmdDrawIndexed(m_commandBuffers[i], 6, 1, 0, 0, 0);

        vkCmdEndRenderPass(m_commandBuffers[i]);

        result = vkEndCommandBuffer(m_commandBuffers[i]);
        LOG_MESSAGE_ASSERT(result == VK_SUCCESS, "Error recording command buffer: " << result);
    }
}
