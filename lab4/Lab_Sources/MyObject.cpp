#include "stdafx.h"
#include "MyObject.h"
#include "Log.h"
#include "macros.h"
#include <cimport.h>
#include <scene.h>
#include <postprocess.h>

using namespace D3D11Framework;

HRESULT MyObject::m_LoadScene(const char *fileName)
{
  m_scene = aiImportFile(fileName, aiProcessPreset_TargetRealtime_MaxQuality);
  
  if (!m_scene)
    return S_FALSE;
  
  return S_OK;
  
}


MyObject::MyObject() : m_indices(nullptr), m_vertices(nullptr), m_indicesNumber(0), m_verticesNumber(0), 
  m_scene(0), m_worldMatrixUpdated(true), m_primitiveTypes(0), m_numMeshes(0), m_textureNames(0), 
  m_textures(0), m_hasTextures(false), m_hasVisibleBackfaces(false)
{
  XMStoreFloat4x4(&m_World, XMMatrixIdentity());
  XMStoreFloat4x4(&m_rotate, XMMatrixIdentity());
  XMStoreFloat4x4(&m_translate, XMMatrixIdentity());
}

MyObject::MyObject(const char *path, const char *fileName) : m_indices(nullptr), m_vertices(nullptr), m_indicesNumber(0), m_verticesNumber(0), 
  m_scene(0), m_worldMatrixUpdated(true), m_primitiveTypes(0), m_numMeshes(0), m_textureNames(0), 
  m_textures(0), m_hasTextures(false), m_hasVisibleBackfaces(false)
{
  XMStoreFloat4x4(&m_World, XMMatrixIdentity());
  XMStoreFloat4x4(&m_rotate, XMMatrixIdentity());
  XMStoreFloat4x4(&m_translate, XMMatrixIdentity());
  LoadFromFile(path, fileName);
}

HRESULT MyObject::LoadFromFile(const char *path, const char *fileName)
{
  std::string fullFileName = path;
  fullFileName.append(fileName);

  m_LoadScene(fullFileName.c_str());

  m_hasTextures = false;

  if (m_scene->HasTextures())
    Log::Get()->Debug("Model in %s conatins textures", fullFileName.c_str());
  else
    Log::Get()->Debug("Model in %s has NO textures", fullFileName.c_str());

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
  
  aiString aiPath(path);

  for (size_t j = 0; j < m_numMeshes; j++)
  {
    aiMesh *mesh = m_scene->mMeshes[j];
    SimpleVertex *vertices = m_vertices[j];
    aiMaterial *mat = m_scene->mMaterials[mesh->mMaterialIndex]; // TODO now we load as many textures as there are meshes, fix it then
   
    aiTexture tex;
    aiString str;

    Log::Get()->Debug("there are %i textures of type %i", mat->GetTextureCount(aiTextureType_DIFFUSE), aiTextureType_DIFFUSE);
    
    if (AI_SUCCESS == mat->GetTexture(aiTextureType_DIFFUSE, 0, &str)) 
    {
      Log::Get()->Debug("FOUND TEXTURE %i with index %i!", aiTextureType_DIFFUSE, 0);
      if (!m_hasTextures) // now we assume very simple: if one mesh has texture, than we assume ALL meshes in this model have 1 texture
      {
        m_textureNames = new aiString[m_numMeshes];
        m_hasTextures = true;
      }
      m_textureNames[j] = aiPath;
      m_textureNames[j].Append(str.C_Str());
    } else {
      for (int i = 0; i < 0xD; i++)
      {
        //int p = mat->GetTextureCount();
        if (AI_SUCCESS == mat->GetTexture((aiTextureType)i, 0, &str)) 
        {
           Log::Get()->Debug("FOUND TEXTURE %i with index %i!", i, 0);
        }
      }
    }

    
    if(AI_SUCCESS == mat->Get(AI_MATKEY_NAME, str)) 
    {
      Log::Get()->Debug("Mat name = %s", str.C_Str());
    }

    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
      vertices[i].Pos = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
      vertices[i].Normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
       
      if (mesh->mColors[0] != nullptr)
      {
        if (!mesh->mColors[0][i].IsBlack())
          vertices[i].Color = XMFLOAT4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
        else
          vertices[i].Color = XMFLOAT4(0, 0, 0, 0);
      }
      else
         vertices[i].Color = XMFLOAT4(0, 1, 0, 1);
    }

    if (mesh->mTextureCoords[0] != 0)
    {
      for (size_t i = 0; i < mesh->mNumVertices; ++i)
      {
        vertices[i].texCoord = XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
      }
    } 
    else 
    {
      for (size_t i = 0; i < mesh->mNumVertices; ++i)
      {
        vertices[i].texCoord = XMFLOAT2(0, 0);
      }
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

  if (m_hasTextures)
  {
    for (size_t i = 0; i < m_numMeshes; i++)
    {
      _RELEASE(m_textures[i]);
    }
  }

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

void MyObject::RotateZ(float angle)
{
  XMStoreFloat4x4(&m_rotate, XMMatrixRotationZ(angle) * XMLoadFloat4x4(&m_rotate));
  m_worldMatrixUpdated = false;
}

void MyObject::RotateAxis(float x_axis, float y_axis, float z_axis, float angle)
{
  XMVECTOR axis = XMLoadFloat3(&XMFLOAT3(x_axis, y_axis, z_axis));
  XMStoreFloat4x4(&m_rotate, XMMatrixRotationAxis(axis, angle) * XMLoadFloat4x4(&m_rotate));
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

HRESULT MyObject::LoadTextures(ID3D11Device *device) 
{
  m_textures = new ID3D11ShaderResourceView*[m_numMeshes];
  HRESULT hr = S_OK;

  	// Create resource view descriptor
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	RtlZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = D3D11_RESOURCE_MISC_GENERATE_MIPS;

  for (size_t i = 0; i < m_numMeshes; i++)
  {
    hr = D3DX11CreateShaderResourceViewFromFile(device, m_textureNames[i].C_Str(), NULL, NULL, m_textures + i, NULL);
    if (FAILED(hr))
      m_textures[i] = nullptr;
    
  }
  return S_OK;
}

ID3D11ShaderResourceView *MyObject::GetTextureAt(size_t idx)
{
  if (idx >= m_numMeshes)
    return 0;
  return m_textures[idx];
}

bool MyObject::Draw(ID3D11Device *device, ID3D11DeviceContext *deviceContext, ConstantBuffer &cb, const XMMATRIX &view, const XMMATRIX &projection, 
                    ID3D11VertexShader *vertexShader, ID3D11PixelShader *pixelShader, ID3D11Buffer *constantBuffer)
{
  XMMATRIX world = XMMatrixMultiply(XMLoadFloat4x4(&m_rotate), XMLoadFloat4x4(&m_translate));
  cb.mWorld = XMMatrixTranspose(world);
  cb.mView = XMMatrixTranspose(view);
  cb.mProjection = XMMatrixTranspose(projection);

  /*** culling settings ***/
  D3D11_RASTERIZER_DESC rastDesc;
  ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
  rastDesc.FillMode = D3D11_FILL_SOLID;
  if (m_hasVisibleBackfaces)
	  rastDesc.CullMode = D3D11_CULL_NONE;
  else
	  rastDesc.CullMode = D3D11_CULL_BACK;
  	  
  ID3D11RasterizerState* noCull = 0;
  device->CreateRasterizerState(&rastDesc, &noCull);
  deviceContext->RSSetState(noCull);

  for (size_t meshNumber = 0; meshNumber < m_numMeshes; meshNumber++)
  {
    ID3D11Buffer *vertexBuffer;
    ID3D11Buffer *indexBuffer;

    D3D11_BUFFER_DESC bd;
    RtlZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * m_verticesNumber[meshNumber];
	  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	  bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA data;
    RtlZeroMemory(&data, sizeof(data));
    data.pSysMem = m_vertices[meshNumber];

    HRESULT hr = device->CreateBuffer(&bd, &data, &vertexBuffer);
    if (FAILED(hr))
    {
      Log::Get()->Err("CreateBuffer error for vertex buffer.");
      return false;
    }
    UINT stride = sizeof(SimpleVertex);
	  UINT offset = 0;

    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
  
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(UINT) * m_indicesNumber[meshNumber];
	  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	  bd.CPUAccessFlags = 0;
    data.pSysMem = m_indices[meshNumber];
	  hr = device->CreateBuffer(&bd, &data, &indexBuffer);
	  if(FAILED(hr))
    {
      Log::Get()->Err("CreateBuffer error for torus.");
		  return false;
    }
    deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    switch (GetMeshPrimitiveTypeAt(meshNumber))
    {
    case aiPrimitiveType_TRIANGLE:
      deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
      break;
    case aiPrimitiveType_LINE:
      deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
      break;
    default:
      Log::Get()->Err("Don't know what topology to use!");
      break;
  }

  cb.vOutputColor = XMFLOAT4(0, 0, 0, 0);
  cb.isLightEnabled[3] = 0; // here we store info about if texture for this mesh is enabled
  if (HasTextures()) 
  {
    if (m_textures[meshNumber] != nullptr)
    {  
      deviceContext->PSSetShaderResources( 0, 1, m_textures + meshNumber);
      cb.isLightEnabled[3] = 1;
    }
  }
	deviceContext->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
  
	deviceContext->VSSetShader(vertexShader, NULL, 0);
	deviceContext->VSSetConstantBuffers( 0, 1, &constantBuffer);
  deviceContext->PSSetShader(pixelShader, NULL, 0);



	deviceContext->PSSetConstantBuffers( 0, 1, &constantBuffer);
  deviceContext->DrawIndexed(m_indicesNumber[meshNumber], 0, 0);

   _RELEASE(vertexBuffer);
   _RELEASE(indexBuffer);
 }
  _RELEASE(noCull);

  return true;
}

void MyObject::SetWorldMatrix(XMFLOAT4X4 world)
{
  m_rotate = world;
  m_translate = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
  m_worldMatrixUpdated = false;
}

void MyObject::Scale(float x_ratio, float y_ratio, float z_ratio)
{
  for (size_t meshNumber = 0; meshNumber < m_numMeshes; meshNumber++)
  {
    aiMesh *mesh = m_scene->mMeshes[meshNumber];
    SimpleVertex *vertices = m_vertices[meshNumber];
    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
      vertices[i].Pos.x *= x_ratio;
      vertices[i].Pos.y *= y_ratio;
      vertices[i].Pos.z *= z_ratio;
    }
  }
}

void MyObject::ApplyMatrixTransform(XMFLOAT4X4 matrix)
{
  XMStoreFloat4x4(&m_rotate, XMMatrixMultiply(XMLoadFloat4x4(&matrix), XMLoadFloat4x4(&m_rotate)));
  m_worldMatrixUpdated = false;
}

void MyObject::ApplyMatrixTranslate(XMFLOAT4X4 matrix)
{
  XMStoreFloat4x4(&m_translate, XMMatrixMultiply(XMLoadFloat4x4(&matrix), XMLoadFloat4x4(&m_translate)));
  m_worldMatrixUpdated = false;
}