#include "stdafx.h"
#include "Render.h"
#include "macros.h"
#include "Log.h"
namespace D3D11Framework
{
  Render::Render()
  {
    m_driverType = D3D_DRIVER_TYPE_NULL;
		m_featureLevel = D3D_FEATURE_LEVEL_11_0;
		m_pd3dDevice = nullptr;
		m_pImmediateContext = nullptr;
		m_pSwapChain = nullptr;
		m_pRenderTargetView = nullptr;
    m_pDepthStencilView = nullptr;
    m_fps = 25;
  }

  Render::~Render()
  {
  }

  bool Render::CreateDevice(HWND hWnd)
  {
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    UINT createDeviceFlags = 0;
  	#ifdef _DEBUG
		  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	  #endif

    D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes = ARRAYSIZE(driverTypes);

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    RtlZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = true;

    for (UINT i = 0; i < numDriverTypes; ++i)
    {
      m_driverType = driverTypes[i];
      hr = D3D11CreateDeviceAndSwapChain(0, m_driverType, 0, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &sd,
        &m_pSwapChain, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
      if (SUCCEEDED(hr))
        break;
    }

    if (FAILED(hr))
    {
      Log::Get()->Err("D3D11CreateDeviceAndSwapChain error");
      return false;
    }

    ID3D11Texture2D *pBackBuffer = nullptr;
    hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
    {
      Log::Get()->Err("GetBuffer error");
      return false;
    }

    hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, 0, &m_pRenderTargetView);
    _RELEASE(pBackBuffer);
    if (FAILED(hr))
    {
       Log::Get()->Err("CreateRenderTargetView error");
      return false;
    }

    D3D11_TEXTURE2D_DESC descDepth;
    RtlZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

    hr = m_pd3dDevice->CreateTexture2D(&descDepth, NULL, &m_pDepthStencil);
	  if (FAILED(hr))
	  	return false;

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    RtlZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = m_pd3dDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
    if (FAILED(hr))
    {
       Log::Get()->Err("CreateRenderTargetView error");
      return false;
    }

    m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
    D3D11_VIEWPORT vp;
    vp.Height = (FLOAT)height;
    vp.Width = (FLOAT)width;
    vp.MaxDepth = 1;
    vp.MinDepth = 0;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pImmediateContext->RSSetViewports(1, &vp);
    
    return Init(hWnd);
  }

  void Render::BeginFrame()
  {
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
    m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, ClearColor);
    m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
  }

  void Render::EndFrame()
  {
    m_pSwapChain->Present(0, 0);
  }

  void Render::Shutdown()
	{
		Close();

		if(m_pImmediateContext) 
			m_pImmediateContext->ClearState();
    
		_RELEASE(m_pRenderTargetView);
    m_pSwapChain->SetFullscreenState(false, NULL);
    _RELEASE(m_pSwapChain);
    _RELEASE(m_pDepthStencil);
  	_RELEASE(m_pDepthStencilView);
    _RELEASE(m_pImmediateContext);
   
     // uncomment for debug purposes
    /*   ID3D11Debug *debugDev = nullptr;
     hr = m_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugDev));
    if (debugDev != nullptr)
    {
      debugDev->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
      debugDev = nullptr;
    }*/
    _RELEASE(m_pd3dDevice);
    
	}
}
