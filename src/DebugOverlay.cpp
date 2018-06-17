#include "DebugOverlay.hpp"

void DebugOverlay::OnUpdate( float dt )
{
    if( m_debugFlags & DEBUG_SHOW_FPS )
        m_fpsCounter.OnFrameStart(dt);
}

void DebugOverlay::OnRender()
{
    if (m_debugFlags & DEBUG_SHOW_FPS)
    {
        m_fpsCounter.OnRender();
    }
}

void DebugOverlay::OnKeyPress( KeyCode key )
{
    switch( key )
    {
    case KEY_F11:
        m_debugFlags ^= DEBUG_SHOW_FPS;
        break;
    }
}
