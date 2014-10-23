#pragma once

#include "Window.h"
#include "Render.h"
#include "InputMgr.h"
#include "Log.h"

namespace D3D11Framework
{
  class Framework
  {
  public: 
    Framework();
		~Framework();

		bool Init(const DescWindow &desc);
    void SetMouseLocked(bool lock, int x = 0, int y = 0) 
    { 
      m_window->SetMouseLocked(lock, x, y); 
    }
		void Run();
		void Close();

    void SetRender(Render *render) {m_render = render;}
		void AddInputListener(InputListener *listener);
  protected:	
		bool m_frame();	

		Window *m_window;
		Render *m_render;
		InputMgr *m_inputMgr;
		Log *m_log;
		bool m_init;		// if was inited
  };

}