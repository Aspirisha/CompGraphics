#pragma once
#include "Render.h"
#include "MyCamera.h"
#include "MyObject.h"
#include <xnamath.h>

using namespace D3D11Framework;

class MyRender : public Render
{
public:
  MyRender(DWORD fps);
  bool Init(HWND hWnd);
  bool Draw();
  bool Close();

  MyObject *GetMyObject() { return m_torus; };
  MyCamera *GetCamera() { return m_camera; }

  void Pitch(float angle)
  {
    m_camera->Pitch(angle);
  }

  void Yaw(float angle)
  {
    m_camera->Yaw(angle);
  }

  void *operator new(size_t s)
  {
    return _aligned_malloc(s, 16);
  }

  void operator delete(void *p)
  {
    _aligned_free(p);
  }
private:
  HRESULT m_CompileShaderFromFile(LPCSTR FileName, LPCSTR EntryPoint, LPCSTR ShaderModel, ID3DBlob** ppBlobOut);

  ID3D11Buffer *m_pVertexBuffer;
  ID3D11InputLayout *m_pVertexLayout;
  ID3D11VertexShader *m_pVertexShader;
  ID3D11PixelShader *m_pPixelShader;

	XMMATRIX m_Projection;

  ID3D11Buffer *m_pIndexBuffer;
	ID3D11Buffer *m_pConstantBuffer;

  MyCamera *m_camera;
  MyObject *m_torus;

  int m_verticesNumber;
  int m_indicesNumber;
};