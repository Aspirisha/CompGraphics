#include "stdafx.h"
#include <xnamath.h>
#include <d3dcompiler.h>
#include <mesh.h>
#include "MyRender.h"
#include "MyObject.h"
#include "macros.h"
#include "MyNode.h"
#include "Log.h"

MyRender::MyRender(DWORD fps)
{
  m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
	m_pVertexLayout = nullptr;
	m_pVertexBuffer = nullptr;
  m_pIndexBuffer = nullptr;
  m_pConstantBuffer = nullptr;
  m_car = nullptr;
  m_verticesNumber = 0;
  m_indicesNumber = 0;
  m_enableDirectedLight = true;
  m_enableProjectorLight = true;
  m_enablePointLight = true;
  m_fps = fps;
  m_texSamplerState = nullptr;
  m_mipmapFiltration = MipmapFiltration::MIP_LINEAR;
  m_minFiltration = MinMagFiltration::MINMAG_LINEAR;
  m_magFiltration = MinMagFiltration::MINMAG_LINEAR;
  m_mipmapBias = 0.0f;
}


HRESULT MyRender::m_CompileShaderFromFile(LPCSTR fileName, LPCSTR EntryPoint, LPCSTR ShaderModel, ID3DBlob** ppBlobOut)
{
  HRESULT hr = S_OK;
  DWORD ShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
  #if defined( DEBUG ) || defined( _DEBUG )
	  ShaderFlags |= D3DCOMPILE_DEBUG;
  #endif
  ID3DBlob* pErrorBlob;
  hr = D3DX11CompileFromFile(fileName, 0, 0, EntryPoint, ShaderModel, 0, 0, 0, ppBlobOut, &pErrorBlob, 0);
  if(FAILED(hr) && pErrorBlob != NULL)
		OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
  _RELEASE(pErrorBlob);
  return hr;
}

bool MyRender::m_initFlower(size_t N)
{
  // load flower
  if (!m_scene)
    return false;

  MyObject *temp = m_scene->LoadObject("models/flower/", "stalk.3ds");
  if (!temp)
    return false;
  temp->SetBackfaceVisibility(true);
  m_flowerStalk = new StaticNode(temp);
  temp = m_scene->LoadObject("models/flower/", "receptacle.3ds");
  temp->SetBackfaceVisibility(true);

  m_flowerStalk->Translate(-5, -1.6f, 10);

  StaticNode *receptacleNode = new StaticNode(temp);
  m_flowerStalk->addChild(receptacleNode);

  if (N == 0)
    return true;

  float radius = 1;
  float dfi = XM_2PI / N;
  float fi = 0;
  float x = 0;
  float z = radius;
 
  
  float initialUpperAngle = 3 * XM_PIDIV4;
  float delay = 4.0f / (3.0f * N);

  for (size_t i = 0; i < N; i++)
  {
    float dz = radius * (cos(fi + dfi) - cos(fi));
    float dx = radius * (sin(fi + dfi) - sin(fi));
    
    temp = m_scene->LoadObject("models/flower/", "petal.blend");
    temp->Scale(0.3, 1, 1);
    temp->SetBackfaceVisibility(true);
    PetalNode *node1 = new PetalNode(temp, delay * i + 0.05f, 0.03f, true, 1.3f);
    
    node1->RotateAxis(0, 1, 0, fi);
    node1->Translate(x, 11.5, z);

    temp = m_scene->LoadObject("models/flower/", "petal.blend");
    temp->Scale(0.3, 1, 1);
    temp->SetBackfaceVisibility(true);

    PetalNode *node2 = new PetalNode(temp, delay*i, 0.01f);
    node2->RotateAxis(1, 0, 0, initialUpperAngle);
    node1->addChild(node2);
    receptacleNode->addChild(node1);
    x += dx;
    z += dz;
    fi += dfi;
    
    if (fi > XM_2PI)
      fi -= XM_2PI;
  }

  return true;
}

bool MyRender::Init(HWND hwnd)
{
  HRESULT hr = S_OK;
  // compile vertex shader
  ID3DBlob *pVSBlob = nullptr;
  hr = m_CompileShaderFromFile("shader.fx", "VS", "vs_4_0", &pVSBlob);
  if(FAILED(hr))
	{
		Log::Get()->Err("Error compiling shader.fx. Error = %lu", hr);
		return false;
	}

  hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader);
  if(FAILED(hr))
	{	
		Log::Get()->Err("Error creating vertex shader.");
		_RELEASE(pVSBlob);
		return false;
	}

  D3D11_INPUT_ELEMENT_DESC layout[] = 
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
   
  };
  UINT numElements = ARRAYSIZE( layout );

  hr = m_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
  _RELEASE(pVSBlob);
	if( FAILED(hr))
  {
    Log::Get()->Err("CreateInputLayout error.");
		return false;
  }

  m_pImmediateContext->IASetInputLayout(m_pVertexLayout);

  ID3DBlob *pPSBlob = nullptr;
  hr = m_CompileShaderFromFile("shader.fx", "PS", "ps_4_0", &pPSBlob);
  if (FAILED(hr))
  {
    Log::Get()->Err("Unable to compile file shader.fx.");
    return false;
  }

  hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPixelShader);
  _RELEASE(pPSBlob);
  if(FAILED(hr))
	{	
		Log::Get()->Err("Unable to create pixel shader.");
		return false;
	}

  /*Load solid shader for debug purposes*/
  hr = m_CompileShaderFromFile("shader.fx", "PSSolid", "ps_4_0", &pPSBlob);
  if (FAILED(hr))
  {
    Log::Get()->Err("Unable to compile file shader.fx.");
    return false;
  }
  hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPixelShaderSolid);
  if(FAILED(hr))
	{	
		Log::Get()->Err("Unable to create pixel shader.");
		return false;
	}
  /**************************************************************/

  m_scene = new MyScene();
  /*add directed light to scene*/
  DirectedLight light;
  light.Dir = XMFLOAT3(0.5, -0.5, -1);
  light.Color = XMFLOAT4(0.5f, 0.5f, 0.0f, 0.1f);
  m_scene->addDirectedLight(light);
  
  m_car = m_scene->LoadObject("models/car/", "car00.x");
  m_sun = m_scene->LoadObject("models/sun/", "sphere.obj");
  m_house = m_scene->LoadObject("models/house/", "house.x");
  
  //m_scene->LoadObject("models/tree1/", "tree.3ds");
  m_table = m_scene->LoadObject("models/table/", "table.x");

  m_initFlower(20);


  for (size_t i = 0; i < m_scene->GetObjectsNumber(); i++)
  {
    MyObject *obj = m_scene->GetObjectAt(i); 
    if (obj->HasTextures())
      obj->LoadTextures(m_pd3dDevice);
  }

  m_sun->Translate(-30, 10, 10);
  m_table->Translate(10, -1.6f, 10);
  m_house->Translate(40, -1.6f, 10);
  /* add lights to sun */
  SimpleLight bulb;
  bulb.Pos = XMFLOAT3(0, 0, 0);
  bulb.Color = XMFLOAT4(0, 1, 0, 0.5);
  m_sun->addPointLight(bulb);

  /* add lights to our car! */
  ProjectorLight newLight;
  newLight.Pos = XMFLOAT3(7, 0.5f, -1.5f);
  newLight.Color = XMFLOAT4(1, 0, 0, 0.5f);
  newLight.angleX = 0.4f;
  newLight.angleY = 0.4f;
  newLight.Direction = XMFLOAT3(1, 0, 0);
  m_car->addProjector(newLight);

  newLight.Pos = XMFLOAT3(7, 0.5f, 1.5f);
  m_car->addProjector(newLight);
  /* lights added */

  if (FAILED(hr))
  {
    Log::Get()->Err("Load scene error.");
    return false;
  }

  D3D11_BUFFER_DESC bd;
  RtlZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(ConstantBuffer);
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = 0;
  hr = m_pd3dDevice->CreateBuffer(&bd, 0, &m_pConstantBuffer);
  if(FAILED(hr))
  {
    Log::Get()->Err("CreateBuffer error for Constant buffer.");
		return false;
  }

  m_camera = new MyCamera();

  float width = 1280.0f;
	float height = 1024.0f;
  m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width/height, 0.1f, 500.0f );
    
  m_createSamplerState();  
  return true;
}

bool MyRender::Draw()
{
  static float dfi = 0.02f;
  static float fi = 0.0f;
  static float radius = 40.0f;
  static float t = 0;

  float dz = radius * (cos(fi + dfi) - cos(fi));
  float dx = radius * (sin(fi + dfi) - sin(fi));
  m_car->RotateY(dfi);
  m_car->Translate(dx, 0, dz);
  fi += dfi;
  if (fi > XM_2PI)
    fi -= XM_2PI;

  ConstantBuffer cb;

  m_camera->Update();
  XMMATRIX view = XMLoadFloat4x4(&(m_camera->GetViewMatrix()));

  m_flowerStalk->updateMatrix(t, 0);
  t += 0.01f;

  // add all projectors info to constant buffer
  for (size_t i = 0, lightIndex = 0; i < m_scene->GetObjectsNumber(); i++)
  {
    MyObject *object = m_scene->GetObjectAt(i);
    for (size_t j = 0; j < object->GetProjectorLightsNumber(); j++, lightIndex++)
    {
      ProjectorLight *projector = object->GetProjectorAt(j);
      XMMATRIX projectorWorldMatrix = (XMLoadFloat4x4(&(object->GetWorldMatrix())));
      XMMATRIX projectorRotationMatrix = (XMLoadFloat4x4(&(object->GetRotationMatrix())));
      XMVECTOR tempPos = XMLoadFloat4(&(XMFLOAT4(projector->Pos.x, projector->Pos.y, projector->Pos.z, 1)));
      XMVECTOR tempDir = XMLoadFloat4(&(XMFLOAT4(projector->Direction.x, projector->Direction.y, projector->Direction.z, 1)));

      XMVECTOR projectorPos = XMVector4Transform(tempPos, projectorWorldMatrix);
      XMVECTOR projectorDir = XMVector4Transform(tempDir, projectorRotationMatrix);

      XMStoreFloat4(cb.vLightPos + lightIndex, projectorPos);
      XMStoreFloat4(cb.vLightDir + lightIndex, projectorDir);
	    cb.vLightColor[lightIndex] = projector->Color;
    }
  }
  // add all point lights info to constant buffer
  for (size_t i = 0, lightIndex = 0; i < m_scene->GetObjectsNumber(); i++)
  {
    MyObject *object = m_scene->GetObjectAt(i);
    for (size_t j = 0; j < object->GetPointLightsNumber(); j++, lightIndex++)
    {
      SimpleLight *bulb = object->GetPointLightAt(j);
      XMMATRIX bulbWorldMatrix = (XMLoadFloat4x4(&(object->GetWorldMatrix())));
      XMVECTOR tempPos = XMLoadFloat4(&(XMFLOAT4(bulb->Pos.x, bulb->Pos.y, bulb->Pos.z, 1)));

      XMVECTOR bulbPos = XMVector4Transform(tempPos, bulbWorldMatrix);

      XMStoreFloat4(cb.vBulbLightPos + lightIndex, bulbPos);
	    cb.vBulbLightColor[lightIndex] = bulb->Color;
    }
  }

  // add all directed lights info to constant buffer
  for (size_t i = 0, lightIndex = 0; i < m_scene->GetObjectsNumber(); i++)
  {
    MyObject *object = m_scene->GetObjectAt(i);
    for (size_t j = 0; j < object->GetDirectedLightsNumber(); j++, lightIndex++)
    {
      DirectedLight *dirLight = object->GetDirectedLightAt(j);
      XMMATRIX dirRotationMatrix = (XMLoadFloat4x4(&(object->GetRotationMatrix())));
      XMVECTOR tempDir = XMLoadFloat4(&(XMFLOAT4(dirLight->Dir.x, dirLight->Dir.y, dirLight->Dir.z, 1)));

      XMVECTOR bulbPos = XMVector4Transform(tempDir, dirRotationMatrix);

      XMStoreFloat4(cb.vDirectedLightDir + lightIndex, bulbPos);
	    cb.vDirectedLightColor[lightIndex] = dirLight->Color;
    }
  }

  cb.isLightEnabled[0] = m_enableProjectorLight;
  cb.isLightEnabled[1] = m_enablePointLight;
  cb.isLightEnabled[2] = m_enableDirectedLight;

  // draw all objects

  m_pImmediateContext->PSSetSamplers( 0, 1, &m_texSamplerState);
  for (size_t idx = 0; idx < m_scene->GetObjectsNumber(); idx++)
  {
    MyObject *object = m_scene->GetObjectAt(idx);
    ID3D11PixelShader *pixShader = m_pPixelShader;
    if (object == m_sun)
      pixShader = m_pPixelShaderSolid;

    bool drawResult = object->Draw(m_pd3dDevice, m_pImmediateContext, cb, view, m_Projection, m_pVertexShader, pixShader, m_pConstantBuffer);
    if (!drawResult)
      return false;
  }
	return true;
}

void MyRender::SwitchMipmapFiltration()
{
  switch (m_mipmapFiltration)
  {
  case MIP_NONE:
    m_mipmapFiltration = MIP_LINEAR;
    break;
  case MIP_LINEAR:
    m_mipmapFiltration = MIP_NEAREST;
    break;
  case MIP_NEAREST:
    m_mipmapFiltration = MIP_NONE;
    break;
  }
  
  m_createSamplerState();
}

void MyRender::SwitchMinFiltration()
{
  switch (m_minFiltration)
  {
  case MINMAG_LINEAR:
    m_minFiltration = MINMAG_NEAREST;
    break;
  case MINMAG_NEAREST:
    m_minFiltration = MINMAG_LINEAR;
    break;
  }

  m_createSamplerState();
}

void MyRender::SwitchMagFiltration()
{
  switch (m_magFiltration)
  {
  case MINMAG_LINEAR:
    m_magFiltration = MINMAG_NEAREST;
    break;
  case MINMAG_NEAREST:
    m_magFiltration = MINMAG_LINEAR;
    break;
  }

  m_createSamplerState();
}

void MyRender::m_createSamplerState()
{
  _RELEASE(m_texSamplerState);
  D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));

  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  int filteringCode = 0;

  if (m_mipmapFiltration == MIP_LINEAR)
    filteringCode |= 1;

  if (m_minFiltration == MINMAG_LINEAR)
    filteringCode |= 16;

  if (m_magFiltration == MINMAG_LINEAR)
    filteringCode |= 4;

  D3D11_FILTER filter = (D3D11_FILTER)filteringCode; // oh well... XD

   sampDesc.Filter = filter;

  if (m_mipmapFiltration == MIP_NONE)
    sampDesc.MaxLOD = 0;

	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; 
  sampDesc.MinLOD = 0;
  sampDesc.MipLODBias = m_mipmapBias;
  
  HRESULT hr = m_pd3dDevice->CreateSamplerState(&sampDesc, &m_texSamplerState);
}

void MyRender::IncreaseMipmapBias()
{
  m_mipmapBias += 0.2f;
  m_createSamplerState();
}

void MyRender::DecreaseMipmapBias()
{
  m_mipmapBias -= 0.2f;
  m_createSamplerState();
}


bool MyRender::Close()
{
  delete m_scene;
  _RELEASE(m_pVertexBuffer);
	_RELEASE(m_pVertexLayout);
	_RELEASE(m_pVertexShader);
	_RELEASE(m_pPixelShader);
  _RELEASE(m_pPixelShaderSolid);
  _RELEASE(m_pIndexBuffer);
  _RELEASE(m_pConstantBuffer);
  _RELEASE(m_texSamplerState);
  return true;
}