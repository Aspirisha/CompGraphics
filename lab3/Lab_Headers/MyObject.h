#pragma once
#include <xnamath.h>
#include <vector3.h>
#include <vector>

struct aiScene;
struct aiString;


struct SimpleVertex
{
	XMFLOAT3 Pos;
  XMFLOAT3 Normal;
  XMFLOAT4 Color;
  XMFLOAT2 texCoord;
};

struct SimpleLight
{
  XMFLOAT3 Pos;
  XMFLOAT4 Color;
};

struct DirectedLight
{
  XMFLOAT3 Dir;
  XMFLOAT4 Color;
};

struct ProjectorLight : SimpleLight
{
  XMFLOAT3 Direction;
  FLOAT angleX;
  FLOAT angleY;
};

class MyObject
{
public:
  MyObject();
  MyObject(const char *path, const char* fileName);
  virtual ~MyObject();
  HRESULT LoadFromFile(const char *path, const char *fileName);

  bool HasTextures() const { return m_hasTextures; }
  HRESULT LoadTextures(ID3D11Device *device); // must be called if has textures

  const SimpleVertex *GetVerticesAt(size_t idx) const { return m_vertices[idx]; }
  const UINT *GetIndicesAt(size_t idx) { return m_indices[idx]; }
  size_t GetMeshesNumber();// {return m_numMeshes;}
  unsigned GetMeshPrimitiveTypeAt(size_t idx);// {return m_primitiveTypes[idx]; }
  int GetVerticesNumberAt(size_t idx) const { return m_verticesNumber[idx]; }
  int GetIndicesNumberAt(size_t idx) const { return m_indicesNumber[idx]; }
  
  inline size_t GetProjectorLightsNumber() const { return m_projectorLights.size(); }
  inline size_t GetPointLightsNumber() const { return m_pointLights.size(); }
  inline size_t GetDirectedLightsNumber() const { return m_directedLights.size(); }

  ProjectorLight *GetProjectorAt(size_t idx);
  SimpleLight *GetPointLightAt(size_t idx);
  DirectedLight *GetDirectedLightAt(size_t idx);

  XMFLOAT4X4 GetWorldMatrix() const;
  XMFLOAT4X4 GetRotationMatrix() const;
  void scale(double ratio);

  void addProjector(const ProjectorLight &projector);
  void addPointLight(const SimpleLight &bulb);
  void addDirectedLight(const DirectedLight &sun);

  void RotateX(float angle);
  void RotateY(float angle);
  void RotateZ(float angle);
  void Translate(float dx, float dy, float dz);

  ID3D11ShaderResourceView *GetTextureAt(size_t idx);
  ID3D11SamplerState *GetSampler();
protected:
  HRESULT m_LoadScene(const char *fileName);

  const aiScene *m_scene;

  /**
   * Object data
   */
  mutable XMFLOAT4X4 m_World;
  XMFLOAT4X4 m_rotate;
  XMFLOAT4X4 m_translate;
  size_t m_numMeshes;

  SimpleVertex **m_vertices;
  UINT **m_indices; // indices array for every mesh
  size_t *m_verticesNumber;
  size_t *m_indicesNumber;

  // Textures data
  bool m_hasTextures;
  aiString *m_textureNames;
  ID3D11ShaderResourceView** m_textures;
  ID3D11SamplerState* m_texSamplerState;
  
  unsigned *m_primitiveTypes;
  mutable bool m_worldMatrixUpdated;

  std::vector<ProjectorLight*> m_projectorLights;
  std::vector<SimpleLight *> m_pointLights;
  std::vector<DirectedLight *> m_directedLights;
};
