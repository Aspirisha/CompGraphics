#pragma once

#include <windef.h>
#include <list>
#include "InputCodes.h"

namespace D3D11Framework
{
  class InputListener;

  class InputMgr
  {
  public:
    void Init(); 
    void Close();
    void Run(const UINT &msg, WPARAM wParam, LPARAM lParam); // gets message from window and dispatches it to listeners
    void AddListener(InputListener *listener);
    void SetWinRect(const RECT &winrect);

  private:
    // these methods are called from Run when dispatching message
    void m_event_cursor(); // mouse move event
    void m_event_mouse(const eMouseKeyCodes key_code, bool press); // mouse click
    void m_MouseWheel(short val);
    void m_event_key(const eKeyCodes key_code, const wchar_t ch, bool press);

    std::list<InputListener*> m_Listener;
    RECT m_windowrect;
		int m_curx;
		int m_cury;
		int m_mouse_wheel;
  };
}