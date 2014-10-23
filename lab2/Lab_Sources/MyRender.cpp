#include "stdafx.h"
#include <xnamath.h>
#include <d3dcompiler.h>
#include "MyRender.h"
#include "MyObject.h"
#include "MyMesh.h"
#include "macros.h"
#include "Log.h"

__declspec(align(16)) struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
  XMFLOAT4 vLightColor[2];
  XMFLOAT4 vLightDir[2];
  XMFLOAT4 vLightPos[2];
  XMFLOAT4 vBulbLightColor[1];
  XMFLOAT4 vBulbLightPos[1];
  XMFLOAT4 vDirectedLightColor[1];
  XMFLOAT4 vDirectedLightDir[1];
  UINT isLightEnabled[4];   
//  FLOAT vLightAngleX[2];
 // FLOAT vLightAngleY[2];
  XMFLOAT4 vOutputColor;
};

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
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // 12 is 3 * 4 for POSITION element
   
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

  m_scene = new MyScene;
  /*add directed light to scene*/
  DirectedLight light;
  light.Dir = XMFLOAT3(0.5, -0.5, -1);
  light.Color = XMFLOAT4(0.5, 0.5, 0, 0.1);
  m_scene->addDirectedLight(light);
  
  m_scene->LoadObject("car00.x");
  m_scene->LoadObject("sphere.obj");
  m_car = m_scene->GetObjectAt(1);
  m_sun = m_scene->GetObjectAt(2);
  m_sun->Translate(30, 10, 10);
  
  /* add lights to sun */
  SimpleLight bulb;
  bulb.Pos = XMFLOAT3(0, 0, 0);
  bulb.Color = XMFLOAT4(0, 1, 0, 0.5);
  m_sun->addPointLight(bulb);

  /* add lights to our car! */
  ProjectorLight newLight;
  newLight.Pos = XMFLOAT3(7, 0.5, -1.5);
  newLight.Color = XMFLOAT4(1, 0, 0, 0.5);
  newLight.angleX = 0.4;
  newLight.angleY = 0.4;
  newLight.Direction = XMFLOAT3(1, 0, 0);
  m_car->addProjector(newLight);

  newLight.Pos = XMFLOAT3(7, 0.5, 1.5);
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
  m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width/height, 0.1f, 100.0f );

  return true;
}

bool MyRender::Draw()
{
  static float dfi = 0.02;
  static float fi = 0;
  static float radius = 30;
  
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
  for (int i = 0, lightIndex = 0; i < m_scene->GetObjectsNumber(); i++)
  {
    MyObject *object = m_scene->GetObjectAt(i);
    for (int j = 0; j < object->GetPointLightsNumber(); j++, lightIndex++)
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
  cb.isLightEnabled[3] = 0;

  for (size_t idx = 0; idx < m_scene->GetObjectsNumber(); idx++)
  {
    MyObject *object = m_scene->GetObjectAt(idx);

    /****************************************************/

    D3D11_BUFFER_DESC bd;
    RtlZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * object->GetVerticesNumber();
	  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	  bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA data;
    RtlZeroMemory(&data, sizeof(data));
    data.pSysMem = object->GetVertices();

    HRESULT hr = m_pd3dDevice->CreateBuffer(&bd, &data, &m_pVertexBuffer);
    if (FAILED(hr))
    {
      Log::Get()->Err("CreateBuffer error for vertex buffer.");
      return false;
    }
    UINT stride = sizeof(SimpleVertex);
	  UINT offset = 0;

    m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
  
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(UINT) * object->GetIndicesNumber();
	  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	  bd.CPUAccessFlags = 0;
	  data.pSysMem = object->GetIndices();
	  hr = m_pd3dDevice->CreateBuffer(&bd, &data, &m_pIndexBuffer);
	  if(FAILED(hr))
    {
      Log::Get()->Err("CreateBuffer error for torus.");
		  return false;
    }
    m_pImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    /****************************************************/
    XMMATRIX world = XMLoadFloat4x4(&(object->GetWorldMatrix()));

    cb.mWorld = XMMatrixTranspose(world);
    cb.mView = XMMatrixTranspose(view);
	  cb.mProjection = XMMatrixTranspose(m_Projection);
    cb.vOutputColor = XMFLOAT4(0, 0, 0, 0);
	  m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);
  
	  m_pImmediateContext->VSSetShader(m_pVertexShader, NULL, 0);
	  m_pImmediateContext->VSSetConstantBuffers( 0, 1, &m_pConstantBuffer);
    if (object == m_sun) // absolutely black body
	    m_pImmediateContext->PSSetShader(m_pPixelShaderSolid, NULL, 0);
    else
      m_pImmediateContext->PSSetShader(m_pPixelShader, NULL, 0);
	  m_pImmediateContext->PSSetConstantBuffers( 0, 1, &m_pConstantBuffer);
    m_pImmediateContext->DrawIndexed(object->GetIndicesNumber(), 0, 0);

    _RELEASE(m_pVertexBuffer);
    _RELEASE(m_pIndexBuffer);
  }
	return true;
}

bool MyRender::Close()
{
  _RELEASE(m_pVertexBuffer);
	_RELEASE(m_pVertexLayout);
	_RELEASE(m_pVertexShader);
	_RELEASE(m_pPixelShader);
  _RELEASE(m_pIndexBuffer);
  _RELEASE(m_pConstantBuffer);
  delete m_car;
  return true;
}