#pragma once
#include <xnamath.h>
#include <vector3.h>
#include <vector>

struct aiScene;

struct SimpleVertex
{
	XMFLOAT3 Pos;
  XMFLOAT3 Normal;
  XMFLOAT4 Color;
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
  MyObject(const char* fileName);
  virtual ~MyObject();
  HRESULT LoadFromFile(const char *fileName);

  const SimpleVertex *GetVertices() const { return m_vertices; }
  const UINT *GetIndices() const { return m_indices; }
  int GetVerticesNumber() const { return m_verticesNumber; }
  int GetIndicesNumber() const { return m_indicesNumber; }
  
  size_t GetProjectorLightsNumber() const { return m_projectorLights.size(); }
  size_t GetPointLightsNumber() const { return m_pointLights.size(); }
  size_t GetDirectedLightsNumber() const { return m_directedLights.size(); }

  ProjectorLight *GetProjectorAt(size_t idx);
  SimpleLight *GetPointLightAt(size_t idx);
  DirectedLight *GetDirectedLightAt(size_t idx);

  XMFLOAT4X4 GetWorldMatrix() const;
  XMFLOAT4X4 GetRotationMatrix() const;
  
  void addProjector(const ProjectorLight &projector);
  void addPointLight(const SimpleLight &bulb);
  void addDirectedLight(const DirectedLight &sun);

  void RotateX(float angle);
  void RotateY(float angle);
  void RotateZ(float angle);
  void Translate(float dx, float dy, float dz);
protected:
  HRESULT m_LoadScene(const char *fileName);

  mutable XMFLOAT4X4 m_World; // we use symmetry of torus: it's top is mirrored bottom part
  XMFLOAT4X4 m_rotate;
  XMFLOAT4X4 m_translate;

  UINT *m_indices;
  const aiScene *m_scene;
  SimpleVertex *m_vertices;
  size_t m_verticesNumber;
  size_t m_indicesNumber;
  mutable bool m_worldMatrixUpdated;

  std::vector<ProjectorLight*> m_projectorLights;
  std::vector<SimpleLight *> m_pointLights;
  std::vector<DirectedLight *> m_directedLights;
};
