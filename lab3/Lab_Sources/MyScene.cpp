#include <mesh.h>
#include "stdafx.h"
#include "MyScene.h"
#include "Log.h"

using namespace D3D11Framework;

MyScene::MyScene()
{
  m_numMeshes = 1;
  m_primitiveTypes = new unsigned[1];
  m_primitiveTypes[0] = aiPrimitiveType_TRIANGLE;

  m_vertices = new SimpleVertex*[m_numMeshes];
  m_verticesNumber = new size_t[m_numMeshes];
  m_indicesNumber = new size_t[m_numMeshes];
  m_indices = new UINT*[m_numMeshes];

  m_verticesNumber[0] = 4;
  m_indicesNumber[0] = 6;
  FLOAT earth_width = 100.0f;
  FLOAT earth_y = -1.7f;

  m_vertices[0] = new SimpleVertex[m_verticesNumber[0]];

  m_vertices[0][0].Pos = XMFLOAT3(-earth_width, earth_y, -earth_width);
  m_vertices[0][0].Normal = XMFLOAT3(0, 1, 0);
  m_vertices[0][0].Color = XMFLOAT4(0.5, 0.5, 0, 0);
  m_vertices[0][1].Pos = XMFLOAT3(earth_width, earth_y, -earth_width);
  m_vertices[0][1].Normal = XMFLOAT3(0, 1, 0);
  m_vertices[0][1].Color = XMFLOAT4(0, 0.5, 0.5, 0);
  m_vertices[0][2].Pos = XMFLOAT3(-earth_width, earth_y, earth_width);
  m_vertices[0][2].Normal = XMFLOAT3(0, 1, 0);
  m_vertices[0][2].Color = XMFLOAT4(0.5, 0.5, 0.5, 0);
  m_vertices[0][3].Pos = XMFLOAT3(earth_width, earth_y, earth_width);
  m_vertices[0][3].Normal = XMFLOAT3(0, 1, 0);
  m_vertices[0][3].Color = XMFLOAT4(0.5, 0.5, 0.5, 0);
  
  m_indices[0] = new UINT[m_indicesNumber[0]];
  m_indices[0][0] = 1;
  m_indices[0][1] = 0;
  m_indices[0][2] = 2;

  m_indices[0][3] = 1;
  m_indices[0][4] = 2;
  m_indices[0][5] = 3;

  m_objects.push_back(this);
  for (size_t i = 0; i < m_projectorLights.size(); i++)
  {
    m_projectorObjectNumber.push_back(0);
  }
}

MyObject * MyScene::LoadObject(const char *path, const char *fileName)
{
  MyObject *newObj = new MyObject;

  HRESULT hr = newObj->LoadFromFile(path, fileName);
  if (FAILED(hr))
  {
    Log::Get()->Err("Error loading object from file %s", fileName);
    delete newObj;
    return nullptr;
  }

  m_objects.push_back(newObj);
  return newObj;
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

MyScene::~MyScene()
{
  for (MyObject *obj : m_objects)
  {
    if (obj != this)
      delete obj;
  }
}