#ifndef DEBUGOVERLAY_INCLUDED
#define DEBUGOVERLAY_INCLUDED

#include "renderer/Font.hpp"
#include "InputHandlers.hpp"
#include <sstream>

class FPSCounter
{
public:
    FPSCounter() : m_numFrames( 0 ), m_numFramesToDraw( 1 ), m_lastTime( 1000 )
    {
        m_font = new Font( "res/font.png" );
        m_font->SetScale(Math::Vector2f(2.f, 2.f));
    }

    ~FPSCounter()
    {
        delete m_font;
    }

    void OnFrameStart()
    {
        m_numFrames++;
    }

    void OnRender()
    {
        m_font->RenderStart();

        std::stringstream sstream, sstream2;
        sstream << m_numFramesToDraw << " FPS";
        sstream2 << 1000.f / m_numFramesToDraw << " ms";
        m_font->RenderText( sstream.str().c_str(), -1.0f, 1.0f );
        m_font->RenderText( sstream2.str().c_str(), -1.0f, 0.95f );

        if( SDL_GetTicks() - m_lastTime >= 1000 )
        {
            m_numFramesToDraw = m_numFrames;
            m_numFrames = 0;
            m_lastTime = SDL_GetTicks();
        }

        m_font->RenderFinish();
    }

    void RebuildPipeline()
    {
        m_font->RebuildPipeline();
    }

private:
    Font *m_font;
    int m_numFrames;
    int m_numFramesToDraw;
    int m_lastTime;
};


class DebugOverlay
{
public:
    enum DebugFlag
    {
        DEBUG_NONE = 0,
        DEBUG_SHOW_FPS = 1 << 0
    };

    DebugOverlay() : m_debugFlags( DEBUG_SHOW_FPS )
    {
    }

    void OnUpdate( float dt );
    bool OnRender();
    void OnKeyPress( KeyCode key );
    bool DebugFlagSet( DebugFlag df ) { return ( m_debugFlags & df ) != 0; }
    void RebuildPipeline() { m_fpsCounter.RebuildPipeline(); }
private:
    FPSCounter m_fpsCounter;
    int  m_debugFlags;
};

#endif