#pragma once
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace D3D11Framework
{
  class InputMgr;

  struct DescWindow
  {
    DescWindow() : caption(L""), width(640), height(480), posx(200), posy(20), is_resizing(true) {}

    int posx;
    int posy;
    std::wstring caption; 
    int width;
    int height;
    bool is_resizing;
  };

  class Window
  {
  public:
    Window();

    static Window* Get() { return m_window; };
    bool Create(const DescWindow &desc);
    void RunEvent();
    void Close();
    void SetInputMgr(InputMgr *inputMgr);
    HWND GetHWND() const { return m_hWnd; }

    int GetLeft() const { return m_desc.posx; }
    int GetWidth() const { return m_desc.width; }
    int GetTop() const { return m_desc.posy; }
    int GetHeight() const { return m_desc.height; }
    const std::wstring &GetCaption() const { return m_desc.caption; }

    void SetMouseLocked(bool lock, int x, int y) 
    { 
      m_MouseLock = lock; 
      m_MousePos.x = x;
      m_MousePos.y = y;
    }

    void SetMousePos();
    bool IsExit() const { return m_isExit; }
    bool IsActive() const { return m_active; }
    bool IsResize() 
		{
			bool ret = m_isResize;
			m_isResize = false;
			return ret;
		}

    LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

  private:
    void m_UpdateWindowState();
    static Window *m_window;
    DescWindow m_desc;
    InputMgr *m_inputMgr;
    HWND m_hWnd;
    bool m_isExit;
    bool m_active;
    bool m_minimized;
    bool m_maximized;
    bool m_isResize;
    bool m_MouseLock;
    POINT m_MousePos;
  };

  static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}