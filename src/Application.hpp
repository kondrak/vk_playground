#ifndef APPLICATION_INCLUDED
#define APPLICATION_INCLUDED

#include <map>
#include "DebugOverlay.hpp"
#include "InputHandlers.hpp"
#include "Math.hpp"
#include "renderer/RenderContext.hpp"
#include "renderer/TextureManager.hpp"

/*
 * main application
 */

class Application
{
public:
    void OnWindowResize(int newWidth, int newHeight);
    void OnWindowMinimized(bool minimized);

    void OnStart(int argc, char **argv);
    void OnRender();
    void OnUpdate(float dt);
    void OnTerminate();

    inline bool Running() const { return m_running; }
    inline void Terminate() { m_running = false; }

    bool KeyPressed(KeyCode key);
    void OnKeyPress(KeyCode key);
    void OnKeyRelease(KeyCode key);
    void OnMouseMove(int x, int y);
private:

    void UpdateCamera(float dt);
    void RenderQuad();
    inline void SetKeyPressed(KeyCode key, bool pressed) { m_keyStates[key] = pressed; }

    bool m_running     = true;    // application is running
    bool m_noRedraw    = false;   //  do not perform window redraw

    std::map<KeyCode, bool> m_keyStates;

    // rendering Vulkan buffers and pipelines
    void CreateDescriptorSetLayout();
    void CreateDescriptor(const vk::Texture **textures, vk::Descriptor *descriptor);
    void RebuildPipelines();
    void Draw();

    UniformBufferObject m_ubo;
    vk::Buffer m_uniformBuffer;
    vk::Buffer m_vertexBuffer;
    vk::Buffer m_indexBuffer;
    vk::Pipeline   m_pipeline; // used for rendering standard faces
    vk::Descriptor m_descriptor;
    GameTexture *m_texture = nullptr;

    // all faces and patches use shared vertex buffer info and descriptor set layout
    vk::VertexBufferInfo  m_vbInfo;
    VkDescriptorSetLayout m_dsLayout;

    DebugOverlay *m_debugOverlay = nullptr;
};

#endif
