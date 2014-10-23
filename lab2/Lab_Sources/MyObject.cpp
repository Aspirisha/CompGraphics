#include "stdafx.h"
#include "MyObject.h"
#include "Log.h"
#include <cimport.h>
#include <scene.h>
#include <postprocess.h>

typedef const aiScene *(*impFile)(const char *, unsigned int);
using namespace D3D11Framework;

HRESULT MyObject::m_LoadScene(const char *fileName)
{
  HMODULE handler = LoadLibrary("assimp.dll");
  if (handler == nullptr)
  {
    Log::Get()->Err("assimp.dll not found in %PATH% and in application directory. Shutting down...");
    return S_FALSE;
  }
  impFile f = (impFile)GetProcAddress(handler, "aiImportFile");
  m_scene = f(fileName, aiProcessPreset_TargetRealtime_MaxQuality);
  
  if (!m_scene)
    return S_FALSE;
  
  Log::Get()->Debug("m_scene has %i cameras", m_scene->mNumCameras);
  Log::Get()->Debug("m_scene has %i animations", m_scene->mNumAnimations);
  Log::Get()->Debug("m_scene has %i lights", m_scene->mNumLights);
  Log::Get()->Debug("m_scene has %i meshes", m_scene->mNumMeshes);
  Log::Get()->Debug("m_scene->mesh[0] has %i faces", m_scene->mMeshes[0]->mNumFaces);
  for (int i = 0; i < 100; i++)
    Log::Get()->Debug("m_scene->mesh[0]->face[i] has %i vertices", m_scene->mMeshes[0]->mFaces[i].mNumIndices);
  return S_OK;
  
}


MyObject::MyObject() : m_indices(nullptr), m_vertices(nullptr), m_indicesNumber(0), m_verticesNumber(0), m_scene(0), m_worldMatrixUpdated(true)
{
  XMStoreFloat4x4(&m_World, XMMatrixIdentity());
  XMStoreFloat4x4(&m_rotate, XMMatrixIdentity());
  XMStoreFloat4x4(&m_translate, XMMatrixIdentity());
}

MyObject::MyObject(const char *fileName) : m_indices(nullptr), m_vertices(nullptr), m_indicesNumber(0), m_verticesNumber(0), m_scene(0), m_worldMatrixUpdated(true)
{
  XMStoreFloat4x4(&m_World, XMMatrixIdentity());
  XMStoreFloat4x4(&m_rotate, XMMatrixIdentity());
  XMStoreFloat4x4(&m_translate, XMMatrixIdentity());
  LoadFromFile(fileName);
}

HRESULT MyObject::LoadFromFile(const char *fileName)
{
  m_LoadScene(fileName);
  
  for (size_t i = 0; i < m_scene->mNumMeshes; i++)
  {
    if (m_scene->mMeshes[i]->mPrimitiveTypes != aiPrimitiveType::aiPrimitiveType_TRIANGLE) 
    {
      Log::Get()->Err("Cannot draw not triangles!");
    }
    m_verticesNumber +=  m_scene->mMeshes[i]->mNumVertices;
    m_indicesNumber += (m_scene->mMeshes[i]->mNumFaces * 3);
  }
  m_vertices = new SimpleVertex[m_verticesNumber];  
  m_indices = new UINT[m_indicesNumber];

  for (int j = 0; j < m_scene->mNumMeshes; j++)
  {
    aiMesh *mesh = m_scene->mMeshes[j];
    for (int i = 0; i < mesh->mNumVertices; ++i)
    {
      m_vertices[i].Pos = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
      m_vertices[i].Normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
      if (mesh->mColors[0] != nullptr)
      {
        if (!mesh->mColors[0][i].IsBlack())
          m_vertices[i].Color = XMFLOAT4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
        else
          m_vertices[i].Color = XMFLOAT4(0, 0, 0, 0);
      }
      else
         m_vertices[i].Color = XMFLOAT4(0, 1, 0, 1);
    }
  }

  int offset = 0;
  int indicesOffset = 0;
  for (int k = 0; k < m_scene->mNumMeshes; k++)
  {
    aiMesh *mesh = m_scene->mMeshes[k];  
    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
      for (size_t j = 0; j < 3; j++)
        m_indices[offset + j] = mesh->mFaces[i].mIndices[j] + indicesOffset;
      offset += 3;
    }
    indicesOffset += (mesh->mNumFaces * 3);
  }

  return S_OK;
}

MyObject::~MyObject()
{
  if (m_indices)
    delete[] m_indices;
  if (m_vertices)
    delete[] m_vertices;
 // m_scene->~aiScene();
}

XMFLOAT4X4 MyObject::GetWorldMatrix() const
{
  if (!m_worldMatrixUpdated)
  {
    XMStoreFloat4x4(&m_World, XMMatrixMultiply(XMLoadFloat4x4(&m_rotate), XMLoadFloat4x4(&m_translate)));
    m_worldMatrixUpdated = true;
  }

  return m_World;
}

void  MyObject::RotateX(float angle)
{
  XMStoreFloat4x4(&m_rotate, XMMatrixRotationX(angle) * XMLoadFloat4x4(&m_rotate));
  m_worldMatrixUpdated = false;
}

void MyObject::RotateY(float angle)
{
  XMStoreFloat4x4(&m_rotate, XMMatrixRotationY(angle) * XMLoadFloat4x4(&m_rotate));
  m_worldMatrixUpdated = false;
}

ProjectorLight *MyObject::GetProjectorAt(size_t idx)
{
  if (idx >= m_projectorLights.size())
    return 0;
  return m_projectorLights[idx];
}

SimpleLight *MyObject::GetPointLightAt(size_t idx)
{
  if (idx >= m_pointLights.size())
    return 0;
  return m_pointLights[idx];
}

DirectedLight *MyObject::GetDirectedLightAt(size_t idx)
{
  if (idx >= m_directedLights.size())
    return 0;
  return m_directedLights[idx];
}

void MyObject::Translate(float dx, float dy, float dz)
{
   XMStoreFloat4x4(&m_translate, XMMatrixTranslation(dx, dy, dz) * XMLoadFloat4x4(&m_translate));
   m_worldMatrixUpdated = false;
}

XMFLOAT4X4 MyObject::GetRotationMatrix() const
{
  return m_rotate;
}

void MyObject::addProjector(const ProjectorLight &projector)
{
  ProjectorLight *newProjector = new ProjectorLight(projector);

  m_projectorLights.push_back(newProjector);
}

void MyObject::addPointLight(const SimpleLight &bulb)
{
  SimpleLight *newBulb = new SimpleLight(bulb);

  m_pointLights.push_back(newBulb);
}

void MyObject::addDirectedLight(const DirectedLight &sun)
{
  DirectedLight *newSun = new DirectedLight(sun);
  m_directedLights.push_back(newSun);
}