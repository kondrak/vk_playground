#include <SDL.h>
#include "Application.hpp"
#include "StringHelpers.hpp"
#include "renderer/CameraDirector.hpp"
#include "renderer/RenderContext.hpp"

extern RenderContext  g_renderContext;
extern CameraDirector g_cameraDirector;


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

        m_q3map->OnWindowChanged();
        m_q3stats->OnWindowChanged();
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
    g_cameraDirector.AddCamera(Math::Vector3f(0.f, 0.f, 0.f),
                               Math::Vector3f(0.f, 0.f, 1.f),
                               Math::Vector3f(1.f, 0.f, 0.f),
                               Math::Vector3f(0.f, 1.f, 0.f));

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
        m_q3map->OnWindowChanged();
        m_q3stats->OnWindowChanged();
        return;
    }

    // render the bsp
    g_cameraDirector.GetActiveCamera()->UpdateView();
    m_q3map->OnRender();

    // render map stats
    switch (m_debugRenderState)
    {
    case RenderMapStats:
        m_q3stats->OnRender();
        break;
    default:
        break;
    }

    renderResult = g_renderContext.Present(false);

    // recreate swapchain if it's out of date
    if (renderResult == VK_ERROR_OUT_OF_DATE_KHR || renderResult == VK_SUBOPTIMAL_KHR)
    {
        m_q3map->OnWindowChanged();
        m_q3stats->OnWindowChanged();
    }
}

void Application::OnUpdate(float dt)
{
    UpdateCamera(dt);
}

void Application::OnTerminate()
{
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
