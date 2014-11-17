#include "stdafx.h"
#include "Window.h"
#include "Log.h"
#include "InputMgr.h"

namespace D3D11Framework
{
  Window *Window::m_window = nullptr;

  Window::Window() : m_inputMgr(nullptr), m_hWnd(nullptr), m_isExit(false), m_active(true), m_minimized(false), 
    m_maximized(false), m_isResize(false), m_MouseLock(false)
  {
    if (!m_window)
      m_window = this;
    else
      Log::Get()->Err("Window is already created");
  }

  bool Window::Create(const DescWindow &desc)
  {
    Log::Get()->Debug("Window Create()");
    m_desc = desc;

    WNDCLASSEXW wndClass;

    wndClass.cbSize = sizeof(WNDCLASSEXW);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = StaticWndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = GetModuleHandle(0);
    wndClass.hCursor = LoadCursor(0, IDC_ARROW);
    wndClass.hIcon = LoadIcon(0, MAKEINTRESOURCE(IDI_APPLICATION));
    wndClass.hIconSm = wndClass.hIcon;
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName = 0;
    wndClass.lpszClassName = L"D3D11F";

    if (!RegisterClassExW(&wndClass))
    {
      Log::Get()->Err("Error registering window class");
      return false;
    }

    RECT rect = {0, 0, m_desc.width, m_desc.height};
    // gets rect containing desired size of client part of window and returns full size of window in this rect
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);

    long lwidth = rect.right - rect.left;
    long lheight = rect.bottom - rect.top;
    long lleft = (long)m_desc.posx;	
		long ltop = (long)m_desc.posy;

    m_hWnd = CreateWindowExW(NULL, (LPCWSTR)L"D3D11F", (LPCWSTR)m_desc.caption.c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE,  lleft, ltop, lwidth, lheight, NULL, NULL, NULL, NULL);
  
    if (!m_hWnd)
		{
			Log::Get()->Err("Error creating window");
			return false;
		}

    ShowWindow(m_hWnd, SW_NORMAL);
		UpdateWindow(m_hWnd);

    Log::Get()->Debug("Window is created!");
    return true;
  }

  void Window::RunEvent()
  {
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  void Window::Close()
  {
    if (m_hWnd)
			DestroyWindow(m_hWnd);
		m_hWnd = nullptr;

		Log::Get()->Debug("Window Close");
  }

  LRESULT Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    switch (msg)
    {
    case WM_CREATE:
      Log::Get()->Debug("WM_CREATE message");
      return 0;
    case WM_CLOSE:
      m_isExit = true;
      return 0;
    case WM_ACTIVATE:
      if (LOWORD(wParam) != WA_INACTIVE)
        m_active = true;
      else
        m_active = false;
      return 0;
    case WM_MOVE:
      m_desc.posx = LOWORD(lParam);
      m_desc.posy = HIWORD(lParam);
      m_UpdateWindowState();
      return 0;
    case WM_SIZE:
      if (!m_desc.is_resizing)
        return 0;
      m_desc.width = LOWORD(lParam);
			m_desc.height = HIWORD(lParam);
      m_isResize = true;

      if (wParam == SIZE_MINIMIZED)
      {
        m_active = false;
        m_minimized = true;
        m_maximized = false;
      }
      else if (wParam == SIZE_MAXIMIZED)
      {
        m_active = true;
        m_minimized = false;
        m_maximized = true;
      }
      else if (wParam == SIZE_RESTORED)
      {
        if (m_minimized)
        {
          m_active = true;
          m_minimized = false;
        }
        else if (m_maximized)
        {
          m_active = true;
          m_maximized = false;
        }
      }
      m_UpdateWindowState();
			return 0;

    case WM_MOUSEMOVE:
    case WM_LBUTTONUP: 
    case WM_LBUTTONDOWN: 
    case WM_MBUTTONUP: 
    case WM_MBUTTONDOWN:
    case WM_RBUTTONUP: 
    case WM_RBUTTONDOWN: 
    case WM_MOUSEWHEEL: 
    case WM_KEYDOWN: 
    case WM_KEYUP:
      if (m_inputMgr)
        m_inputMgr->Run(msg, wParam, lParam);
      if (msg == WM_KEYDOWN  && static_cast<eKeyCodes>(wParam) == eKeyCodes::KEY_ESCAPE)
      {
        Log::Get()->Debug("Post Quit message!!");
        m_isExit = true;
      }
      return 0;
    default:
      return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    
  }

  void Window::SetMousePos()
  {
    if (!m_MouseLock)
      return;
    POINT pt = m_MousePos;
    ClientToScreen(m_hWnd, &pt);
    SetCursorPos(pt.x, pt.y);
  }

  void Window::SetInputMgr(InputMgr *mgr)
  {
    m_inputMgr = mgr;
		m_UpdateWindowState();	
  }

  void Window::m_UpdateWindowState()
  {
    RECT ClientRect;
		ClientRect.left = m_desc.posx;
		ClientRect.top = m_desc.posy;
		ClientRect.right = m_desc.width;
		ClientRect.bottom = m_desc.height;
		if (m_inputMgr)
			m_inputMgr->SetWinRect(ClientRect);
  }

  LRESULT CALLBACK D3D11Framework::StaticWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
	{
		return Window::Get()->WndProc(hWnd, nMsg, wParam, lParam );
	}
}
