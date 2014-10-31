#include "stdafx.h"
#include "Framework.h"
#include "Log.h"
#include "macros.h"

namespace D3D11Framework
{
  Framework::Framework() : m_window(nullptr), m_render(nullptr), m_inputMgr(nullptr), m_init(false)
	{
    Log::Create();
	}

  Framework::~Framework()
	{
	}

  void Framework::AddInputListener(InputListener *listener)
	{
		if (m_inputMgr)
			m_inputMgr->AddListener(listener);
	}

  void Framework::Run()
  {
    static DWORD prevTime = GetTickCount();
    DWORD needed_dt = 1000 / m_render->getFPS();
    bool run = true;

    if (m_init)
    {
			while (run)
      {
        DWORD curTime = GetTickCount();
        if (curTime - prevTime > needed_dt)
        {
          run = m_frame();
          prevTime = curTime;
        }
       // else // this is bad, but it's effecient
       // {
       //   Sleep(curTime - prevTime);
       // }
      }
    }
  }

  bool Framework::Init(const DescWindow &desc)
  {
    m_window = new Window();
		m_inputMgr = new InputMgr();

    if (!m_window || !m_inputMgr)
    {
      Log::Get()->Err("Error constructing window or input manager");
      return false;
    }

    m_inputMgr->Init();

		if (!m_window->Create(desc))
		{
			Log::Get()->Err("Unable to create window");
			return false;
		}
		m_window->SetInputMgr(m_inputMgr);

		if (!m_render->CreateDevice(m_window->GetHWND()))
		{
			Log::Get()->Err("Error initializing render");
			return false;
		}

		m_init = true;
    
		return true;
  }

  bool Framework::m_frame()
	{
		m_window->RunEvent(); // loop getting messages from OS is here
		if (!m_window->IsActive())
			return true;

		if (m_window->IsExit()) // finish working
			return false;

		if (m_window->IsResize()) // if window changed it's size
		{
		}

    m_render->BeginFrame();
		if (!m_render->Draw())
			return false;
    m_render->EndFrame();
    
    m_window->SetMousePos();
		return true;
	}

  void Framework::Close()
	{
		m_init = false;
		m_render->Shutdown();
    _DELETE(m_render);
		_CLOSE(m_window);
		_CLOSE(m_inputMgr);
	}

}