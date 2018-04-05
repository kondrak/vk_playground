#include "DebugOverlay.hpp"

void DebugOverlay::OnUpdate( float dt )
{
    if( m_debugFlags & DEBUG_SHOW_FPS )
        m_fpsCounter.OnFrameStart();
}

bool DebugOverlay::OnRender()
{
    if (m_debugFlags & DEBUG_SHOW_FPS)
    {
        m_fpsCounter.OnRender();
    }

    return m_debugFlags != DEBUG_NONE;
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
