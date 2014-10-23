#pragma once
#include <xnamath.h>

struct SimpleVertex
{
	XMFLOAT3 Pos;
  XMFLOAT4 Color;
};

class MyObject
{
public:
  MyObject();
  virtual ~MyObject();

  const SimpleVertex *GetVertices() const { return m_vertices; }
  const WORD *GetIndices() const { return m_indices; }
  int GetVerticesNumber() const { return m_verticesNumber; }
  int GetIndicesNumber() const { return m_indicesNumber; }
  XMFLOAT4X4 GetWorldMatrix1() const;
  XMFLOAT4X4 GetWorldMatrix2() const;
  void RotateX(float angle);
  void RotateY(float angle);
  void RotateZ(float angle);
private:
  XMFLOAT4X4 m_World1; // we use symmetry of torus: it's top is mirrored bottom part
  XMFLOAT4X4 m_World2;

  WORD *m_indices;
  SimpleVertex *m_vertices;
  int m_verticesNumber;
  int m_indicesNumber;
  int m_cathetusLength;
};

static float f(float x, float y, bool &belongs);