#include "stdafx.h"
#include "MyObject.h"
#include "Log.h"
#include <cimport.h>
#include <scene.h>
#include <postprocess.h>

using namespace D3D11Framework;

HRESULT MyObject::m_LoadScene(const char *fileName)
{
  m_scene = aiImportFile(fileName, aiProcessPreset_TargetRealtime_MaxQuality);
  
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


MyObject::MyObject() : m_indices(nullptr), m_vertices(nullptr), m_indicesNumber(0), m_verticesNumber(0), 
  m_scene(0), m_worldMatrixUpdated(true), m_primitiveTypes(0), m_numMeshes(0)
{
  XMStoreFloat4x4(&m_World, XMMatrixIdentity());
  XMStoreFloat4x4(&m_rotate, XMMatrixIdentity());
  XMStoreFloat4x4(&m_translate, XMMatrixIdentity());
}

MyObject::MyObject(const char *fileName) : m_indices(nullptr), m_vertices(nullptr), m_indicesNumber(0), m_verticesNumber(0), 
  m_scene(0), m_worldMatrixUpdated(true), m_primitiveTypes(0), m_numMeshes(0)
{
  XMStoreFloat4x4(&m_World, XMMatrixIdentity());
  XMStoreFloat4x4(&m_rotate, XMMatrixIdentity());
  XMStoreFloat4x4(&m_translate, XMMatrixIdentity());
  LoadFromFile(fileName);
}

HRESULT MyObject::LoadFromFile(const char *fileName)
{
  m_LoadScene(fileName);

  m_numMeshes = m_scene->mNumMeshes;
  m_vertices = new SimpleVertex*[m_numMeshes];
  m_verticesNumber = new size_t[m_numMeshes];
  m_indicesNumber = new size_t[m_numMeshes];
  m_indices = new UINT*[m_numMeshes];
  m_primitiveTypes = new unsigned[m_numMeshes];

  for (size_t i = 0; i < m_scene->mNumMeshes; i++)
  {
    m_verticesNumber[i] =  m_scene->mMeshes[i]->mNumVertices;
    m_primitiveTypes[i] = m_scene->mMeshes[i]->mPrimitiveTypes;

    switch (m_primitiveTypes[i])
    {
    case aiPrimitiveType_TRIANGLE:
      m_indicesNumber[i] = (m_scene->mMeshes[i]->mNumFaces * 3);
      break;
    case aiPrimitiveType_LINE:
      m_indicesNumber[i] = (m_scene->mMeshes[i]->mNumFaces * 2);
      break;
    default:
      Log::Get()->Err("Cannot draw not this type of primitives!");
      break;
    }
    
    m_vertices[i] = new SimpleVertex[m_verticesNumber[i]];  
    m_indices[i] = new UINT[m_indicesNumber[i]];
  }
  
  

  for (size_t j = 0; j < m_scene->mNumMeshes; j++)
  {
    aiMesh *mesh = m_scene->mMeshes[j];
    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
      m_vertices[j][i].Pos = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
      m_vertices[j][i].Normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
      if (mesh->mColors[0] != nullptr)
      {
        if (!mesh->mColors[0][i].IsBlack())
          m_vertices[j][i].Color = XMFLOAT4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
        else
          m_vertices[j][i].Color = XMFLOAT4(0, 0, 0, 0);
      }
      else
         m_vertices[j][i].Color = XMFLOAT4(0, 1, 0, 1);
    }
  }

  
  for (size_t k = 0; k < m_scene->mNumMeshes; k++)
  {
    size_t offset = 0;
    aiMesh *mesh = m_scene->mMeshes[k];  
    size_t faceSize = 3;

    switch (mesh->mPrimitiveTypes)
    {
    case aiPrimitiveType_TRIANGLE:
      faceSize = 3;
      break;
    case aiPrimitiveType_LINE:
      faceSize = 2;
      break;
    }

    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
      for (size_t j = 0; j < faceSize; j++)
        m_indices[k][offset + j] = mesh->mFaces[i].mIndices[j];
      offset += faceSize;
    }
  }

  return S_OK;
}

MyObject::~MyObject()
{
  if (m_indices)
    delete[] m_indices;
  if (m_vertices)
    delete[] m_vertices;
  
  for (ProjectorLight *light: m_projectorLights) 
    delete light;
  for (DirectedLight *light: m_directedLights)
    delete light;
  for (SimpleLight *light: m_pointLights)
    delete light;

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

size_t MyObject::GetMeshesNumber() 
{ 
  return m_numMeshes;
}

unsigned MyObject::GetMeshPrimitiveTypeAt(size_t idx) 
{ 
  return m_primitiveTypes[idx]; 
}