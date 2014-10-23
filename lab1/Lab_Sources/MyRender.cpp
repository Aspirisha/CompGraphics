#include "stdafx.h"
#include <xnamath.h>
#include <d3dcompiler.h>
#include "MyRender.h"
#include "MyObject.h"
#include "macros.h"
#include "Log.h"

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};

MyRender::MyRender(DWORD fps)
{
  m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
	m_pVertexLayout = nullptr;
	m_pVertexBuffer = nullptr;
  m_pIndexBuffer = nullptr;
  m_pConstantBuffer = nullptr;
  m_verticesNumber = 0;
  m_indicesNumber = 0;
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
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 } // 12 is 3 * 4 for POSITION element
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
		_RELEASE(pPSBlob);
		return false;
	}

  m_torus = new MyObject;

  D3D11_BUFFER_DESC bd;
  RtlZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(SimpleVertex) * m_torus->GetVerticesNumber();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

  D3D11_SUBRESOURCE_DATA data;
  RtlZeroMemory(&data, sizeof(data));
  data.pSysMem = m_torus->GetVertices();

  hr = m_pd3dDevice->CreateBuffer(&bd, &data, &m_pVertexBuffer);
  if (FAILED(hr))
  {
    Log::Get()->Err("CreateBuffer error for vertex buffer.");
    return false;
  }
  UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

  m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(WORD) * m_torus->GetIndicesNumber();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	data.pSysMem = m_torus->GetIndices();
	hr = m_pd3dDevice->CreateBuffer(&bd, &data, &m_pIndexBuffer);
	if(FAILED(hr))
  {
    Log::Get()->Err("CreateBuffer error for torus.");
		return false;
  }
  m_pImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
  m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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
	ConstantBuffer cb;

  m_camera->Update();
  XMMATRIX world1 = XMLoadFloat4x4(&(m_torus->GetWorldMatrix1()));
  XMMATRIX world2 = XMLoadFloat4x4(&(m_torus->GetWorldMatrix2()));
  XMMATRIX view = XMLoadFloat4x4(&(m_camera->GetViewMatrix()));

  cb.mWorld = XMMatrixTranspose(world1);
  cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(m_Projection);
	m_pImmediateContext->UpdateSubresource( m_pConstantBuffer, 0, NULL, &cb, 0, 0 );
  
	m_pImmediateContext->VSSetShader(m_pVertexShader, NULL, 0);
	m_pImmediateContext->VSSetConstantBuffers( 0, 1, &m_pConstantBuffer);
	m_pImmediateContext->PSSetShader(m_pPixelShader, NULL, 0);
  m_pImmediateContext->DrawIndexed(m_torus->GetIndicesNumber(), 0, 0);

  cb.mWorld = XMMatrixTranspose(world2);
  m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);
  m_pImmediateContext->DrawIndexed(m_torus->GetIndicesNumber(), 0, 0);
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
  delete m_torus;
  return true;
}