#pragma once
#include "Render.h"
#include "MyCamera.h"
#include "MyObject.h"
#include "MyScene.h"
#include <xnamath.h>

using namespace D3D11Framework;

class MyNode;

enum MipmapFiltration
{
  MIP_LINEAR,
  MIP_NEAREST,
  MIP_NONE
};

enum MinMagFiltration
{
  MINMAG_LINEAR,
  MINMAG_NEAREST
};

class MyRender : public Render
{
public:
  MyRender(DWORD fps);
  bool Init(HWND hWnd);
  bool Draw();
  bool Close();

  MyObject *GetMyObject() { return m_car; };
  MyCamera *GetCamera() { return m_camera; }

  void *operator new(size_t s)
  {
    return _aligned_malloc(s, 16);
  }

  void operator delete(void *p)
  {
    _aligned_free(p);
  }
  void SwitchPointLights() { m_enablePointLight = !m_enablePointLight; }
  void SwitchDirectedLights() { m_enableDirectedLight = !m_enableDirectedLight; }
  void SwitchProjectorLights() { m_enableProjectorLight = !m_enableProjectorLight; }

  void SwitchMipmapFiltration();
  void SwitchMinFiltration();
  void SwitchMagFiltration();
  void IncreaseMipmapBias();
  void DecreaseMipmapBias();
private:
  HRESULT m_CompileShaderFromFile(LPCSTR FileName, LPCSTR EntryPoint, LPCSTR ShaderModel, ID3DBlob** ppBlobOut);
  void m_createSamplerState();
  bool m_initFlower(size_t N);

  ID3D11Buffer *m_pVertexBuffer;
  ID3D11InputLayout *m_pVertexLayout;
  ID3D11VertexShader *m_pVertexShader;
  ID3D11PixelShader *m_pPixelShader;

	XMMATRIX m_Projection;

  ID3D11Buffer *m_pIndexBuffer;
	ID3D11Buffer *m_pConstantBuffer;
  ID3D11PixelShader *m_pPixelShaderSolid;
  MyCamera *m_camera;

  MyObject *m_car; // currently contains object = m_scene->m_objects[1], which is car
  MyObject *m_sun; 
  MyObject *m_table; // NB do not delete any objects except m_scene, for scene will delete it its own 
  MyObject *m_house;

  MyNode *m_flowerStalk;

  MyScene *m_scene;
  ID3D11SamplerState *m_texSamplerState;

  int m_verticesNumber;
  int m_indicesNumber;

  bool m_enableDirectedLight;
  bool m_enablePointLight;
  bool m_enableProjectorLight;
  MipmapFiltration m_mipmapFiltration;
  MinMagFiltration m_minFiltration;
  MinMagFiltration m_magFiltration;
  float m_mipmapBias;
};