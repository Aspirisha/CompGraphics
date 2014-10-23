#pragma once

#include <windef.h>

namespace D3D11Framework
{
  class Render
  {
  public:
    Render();
    virtual ~Render();

    bool CreateDevice(HWND hWnd);
    void BeginFrame();
    void EndFrame();
    void Shutdown();
    DWORD getFPS() { return m_fps; }
    virtual bool Init(HWND hWnd) = 0;
    virtual bool Draw() = 0;
    virtual bool Close() = 0;
  protected:
    DWORD m_fps;
    D3D_DRIVER_TYPE m_driverType;
    D3D_FEATURE_LEVEL m_featureLevel;
    ID3D11Device *m_pd3dDevice;
    ID3D11DeviceContext *m_pImmediateContext;
    IDXGISwapChain *m_pSwapChain;
    ID3D11RenderTargetView *m_pRenderTargetView;

    ID3D11Texture2D *m_pDepthStencil;
    ID3D11DepthStencilView *m_pDepthStencilView;
  };
}