#include "stdafx.h"
#include "InputMgr.h"
#include "InputListener.h"
#include "Log.h"

namespace D3D11Framework
{
  void InputMgr::Init()
  {
    m_mouse_wheel = m_curx = m_cury = 0;
    Log::Get()->Debug("InputMgr init");
  }

  void InputMgr::Close()
  {
    if (!m_Listener.empty())
			m_Listener.clear();
		Log::Get()->Debug("InputMgr close");
  }

  void InputMgr::SetWinRect(const RECT &winrect)
	{
		m_windowrect.left = winrect.left;
		m_windowrect.right = winrect.right;
		m_windowrect.top = winrect.top;
		m_windowrect.bottom = winrect.bottom;
	}

  void InputMgr::AddListener(InputListener *Listener)
	{
		m_Listener.push_back(Listener);
	}

  void InputMgr::Run(const UINT &msg, WPARAM wParam, LPARAM lParam)
	{
    if (m_Listener.empty())
      return;

    eKeyCodes key_index;
    wchar_t buffer;
    BYTE lpKeyState[256];

    m_event_cursor();

    switch (msg) 
    {
    case WM_KEYDOWN:
      key_index = static_cast<eKeyCodes>(wParam);
      GetKeyboardState(lpKeyState);
      ToUnicode(wParam, HIWORD(lParam) & 0xFF, lpKeyState, &buffer, 1, 0);
      m_event_key(key_index, buffer, true);
      break;
    case WM_KEYUP:
      key_index = static_cast<eKeyCodes>(wParam);
      GetKeyboardState(lpKeyState);
      ToUnicode(wParam, HIWORD(lParam)&0xFF, lpKeyState, &buffer, 1, 0);
      m_event_key(key_index, buffer, false);
      break;
    case WM_LBUTTONDOWN:
      m_event_mouse(MOUSE_LEFT, true);
      break;
    case WM_LBUTTONUP:
			m_event_mouse(MOUSE_LEFT,false);
			break;
    case WM_RBUTTONDOWN:
			m_event_mouse(MOUSE_RIGHT,true);
			break;
		case WM_RBUTTONUP:
			m_event_mouse(MOUSE_RIGHT,false);
			break;
    case WM_MBUTTONDOWN:
			m_event_mouse(MOUSE_MIDDLE,true);
			break;
		case WM_MBUTTONUP:
			m_event_mouse(MOUSE_MIDDLE,false);
			break;
		case WM_MOUSEWHEEL:
			m_MouseWheel(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
			break;
    }
  }

  void InputMgr::m_event_cursor()
  {
    POINT position;
    GetCursorPos(&position);

    position.x -= m_windowrect.left;
    position.y -= m_windowrect.top;

    if (m_curx == position.x && m_cury == position.y)
      return;

    m_curx = position.x;
    m_cury = position.y;

    for (auto it = m_Listener.begin(); it != m_Listener.end(); it++)
    {
      if (!(*it))
        continue;
      if ((*it)->MouseMove(MouseEvent(m_curx, m_cury)) == true)
        return;
    }
  }

  void InputMgr::m_event_key(eKeyCodes key_code, wchar_t ch, bool press)
  {
    if (press)
    {
      for (auto it = m_Listener.begin(); it != m_Listener.end(); it++)
      {
        if (!(*it))
          continue;

        if ((*it)->KeyPressed(KeyEvent(ch, key_code)))
          return;
      }
    }
    else
    {
      for (auto it = m_Listener.begin(); it != m_Listener.end(); it++)
      {
        if (!(*it))
          continue;
        if ((*it)->KeyReleased(KeyEvent(ch, key_code)))
					return;
      }
    }
  }

  void InputMgr::m_event_mouse(eMouseKeyCodes key_code, bool press)
  {
    if (press)
    {
      for (auto it = m_Listener.begin(); it != m_Listener.end(); it++)
      {
        if (!(*it))
          continue;
        if ((*it)->MousePressed(MouseEventClick(key_code, m_curx, m_cury)))
          return;
      }
    }
    else
    {
      for (auto it = m_Listener.begin(); it != m_Listener.end(); it++)
      {
        if (!(*it))
          continue;
        if ((*it)->MouseReleased(MouseEventClick(key_code, m_curx, m_cury)))
          return;
      }
    }
  }

  void InputMgr::m_MouseWheel(short val) 
  {
    for (auto it = m_Listener.begin(); it != m_Listener.end(); it++)
    {
      if (!(*it))
        continue;
      if ((*it)->MouseWheel(MouseEventWheel(val, m_curx, m_cury)))
        return;
    }
  }
}