#include "stdafx.h"
#include "MyScene.h"
#include "Log.h"

using namespace D3D11Framework;

MyScene::MyScene()
{
  m_verticesNumber = 4;
  m_indicesNumber = 6;
  m_indices = new UINT[m_indicesNumber];
  m_vertices = new SimpleVertex[m_verticesNumber];
  FLOAT earth_width = 100.0f;
  FLOAT earth_y = -1.7f;

  m_vertices[0].Pos = XMFLOAT3(-earth_width, earth_y, -earth_width);
  m_vertices[0].Normal = XMFLOAT3(0, 1, 0);
  m_vertices[0].Color = XMFLOAT4(0.5, 0.5, 0, 0);
  m_vertices[1].Pos = XMFLOAT3(earth_width, earth_y, -earth_width);
  m_vertices[1].Normal = XMFLOAT3(0, 1, 0);
  m_vertices[1].Color = XMFLOAT4(0, 0.5, 0.5, 0);
  m_vertices[2].Pos = XMFLOAT3(-earth_width, earth_y, earth_width);
  m_vertices[2].Normal = XMFLOAT3(0, 1, 0);
  m_vertices[2].Color = XMFLOAT4(0.5, 0.5, 0.5, 0);
  m_vertices[3].Pos = XMFLOAT3(earth_width, earth_y, earth_width);
  m_vertices[3].Normal = XMFLOAT3(0, 1, 0);
  m_vertices[3].Color = XMFLOAT4(0.5, 0.5, 0.5, 0);
  
  m_indices[0] = 1;
  m_indices[1] = 0;
  m_indices[2] = 2;

  m_indices[3] = 1;
  m_indices[4] = 2;
  m_indices[5] = 3;

  m_objects.push_back(this);
  for (size_t i = 0; i < m_projectorLights.size(); i++)
  {
    m_projectorObjectNumber.push_back(0);
  }
}

HRESULT MyScene::LoadObject(const char *fileName)
{
  MyObject *newObj = new MyObject;

  HRESULT hr = newObj->LoadFromFile(fileName);
  if (FAILED(hr))
  {
    Log::Get()->Err("Error loading object from file %s", fileName);
    delete newObj;
    return hr;
  }

  m_objects.push_back(newObj);
  return S_OK;
}

MyObject *MyScene::GetObjectAt(size_t idx)
{
  if (idx >= m_objects.size())
    return 0;
  return m_objects[idx];
}

XMFLOAT4X4 MyScene::GetProjectorWorldMatrixAt(size_t idx) const
{
  return m_objects[m_projectorObjectNumber[idx]]->GetWorldMatrix();
}

XMFLOAT4X4 MyScene::GetProjectorRotationMatrixAt(size_t idx) const
{
  return m_objects[m_projectorObjectNumber[idx]]->GetRotationMatrix();
}