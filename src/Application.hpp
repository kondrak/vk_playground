#ifndef APPLICATION_INCLUDED
#define APPLICATION_INCLUDED

#include <map>
#include "InputHandlers.hpp"
#include "Math.hpp"

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
    inline void SetKeyPressed(KeyCode key, bool pressed) { m_keyStates[key] = pressed; }

    bool m_running     = true;    // application is running
    bool m_noRedraw    = false;   //  do not perform window redraw

    std::map<KeyCode, bool> m_keyStates;
};

#endif
