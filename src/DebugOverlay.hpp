#ifndef DEBUGOVERLAY_INCLUDED
#define DEBUGOVERLAY_INCLUDED

#include "renderer/Font.hpp"
#include "InputHandlers.hpp"
#include <sstream>

class OverlayText
{
public:
    OverlayText() : m_numFrames(0), m_numFramesToDraw(1), m_numMSAASamples(1), m_time(0.f)
    {
        m_font = new Font( "res/font.png" );
        m_font->SetScale(Math::Vector2f(2.f, 2.f));
    }

    ~OverlayText()
    {
        delete m_font;
    }

    void OnFrameStart(float dt)
    {
        m_time += dt;
        m_numFrames++;
    }

    void OnRender()
    {
        m_font->RenderStart();

        std::stringstream sstream, sstream2, sstream3;
        sstream << m_numFramesToDraw << " FPS";
        sstream2 << (m_numFramesToDraw > 0 ? (1000.f / m_numFramesToDraw) : 0) << " ms";
        sstream3 << "MSAA:x" << m_numMSAASamples;

        m_font->RenderText(sstream.str(), -1.0f, 1.0f );
        m_font->RenderText(sstream2.str(), -1.0f, 0.95f );
        m_font->RenderText(sstream3.str(), -1.0f, 0.90f);

        if (m_time > .25f)
        {
            m_numFramesToDraw = int(m_numFrames / m_time);
            m_numFrames = 0;
            m_time = 0.f;
        }
        m_font->RenderFinish();
    }

    void SetMSAASamples(int samples)
    {
        m_numMSAASamples = samples;
    }

    void RebuildPipeline()
    {
        m_font->RebuildPipeline();
    }

private:
    Font *m_font;
    int m_numFrames;
    int m_numFramesToDraw;
    int m_numMSAASamples;
    float m_time;
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
    void OnRender();
    void OnKeyPress( KeyCode key );
    bool DebugFlagSet( DebugFlag df ) { return ( m_debugFlags & df ) != 0; }
    void SetMSAASamples(int samples) { m_text.SetMSAASamples(samples); }
    void RebuildPipeline() { m_text.RebuildPipeline(); }
private:
    OverlayText m_text;
    int  m_debugFlags;
};

#endif