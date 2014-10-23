#include "stdafx.h"
#include "MyObject.h"

// z = f(x, y) to build object
static float f(float x, float y, bool &belongs)
{
  static float r1 = 0.5f;
  static float r2 = 0.8f;
  static float R = (r2 - r1) / 2;

  float cosf = 1 + (r1 - sqrt(x*x + y*y)) / R;
  if (fabs(cosf) >= 1.0f)
  {
    belongs = false;
    return -0.00000001f; // negative means point not from thorus
  }
  belongs = true;
  float sinf = sqrt(1 - cosf*cosf);
  return R*sinf;
}

MyObject::MyObject() : m_indices(nullptr), m_vertices(nullptr), m_indicesNumber(0), m_verticesNumber(0)
{
  float m_cathetusLength = 0.01f; // if it's too small, we can get problems: max WORD number contains only 65535 
  int verticesPerLine = static_cast<int>(2.0f / m_cathetusLength) + 1;

  m_verticesNumber = verticesPerLine * verticesPerLine;
  m_vertices = new SimpleVertex[m_verticesNumber];
  int numberOfTorusVertices = 0;
  bool belongs = false;

  for (int i = 0, index1 = 0; i < verticesPerLine; i++)
  {
    float y = m_cathetusLength * i - 1;
    float x = -1;
    float z = 0;
    for (int j = 0; j < verticesPerLine; j++, index1++)
    {
      m_vertices[index1].Pos.x = x;
      m_vertices[index1].Pos.y = y;
      z = m_vertices[index1].Pos.z = f(x, y, belongs);
      numberOfTorusVertices += (belongs);
      m_vertices[index1].Color = XMFLOAT4(1 - fabs(z), x*x + y*y, fabs(z), 1.0f);
      x += m_cathetusLength;
    }
  }
  numberOfTorusVertices *= 2;

  // precount size of needed array
  int numberOfTorusTriangles = 0;
  for (int i = 0, index1 = 0; i < verticesPerLine - 1; i++)
  {
    for (int j = 0; j < verticesPerLine - 1; j++, index1++)
    {
      if (m_vertices[index1].Pos.z >= 0 || m_vertices[index1 + 1].Pos.z >= 0 || m_vertices[index1 + verticesPerLine].Pos.z >= 0)
        numberOfTorusTriangles++;

      if (m_vertices[index1 + 1].Pos.z >= 0 || m_vertices[index1 + verticesPerLine].Pos.z >= 0 || m_vertices[index1 + verticesPerLine + 1].Pos.z >= 0)
        numberOfTorusTriangles++;
    }
  }

  m_indices = new WORD[3 * numberOfTorusTriangles];
  for (WORD i = 0, index1 = 0; i < verticesPerLine - 1; i++)
  {
    for (WORD j = 0; j < verticesPerLine - 1; j++, index1++)
    {
      if (m_vertices[index1].Pos.z >= 0 || m_vertices[index1 + 1].Pos.z >= 0 || m_vertices[index1 + verticesPerLine].Pos.z >= 0)
      {
        m_indices[m_indicesNumber++] = index1;
        m_indices[m_indicesNumber++] = index1 + 1;
        m_indices[m_indicesNumber++] = index1 + verticesPerLine;
      }
      if (m_vertices[index1 + 1].Pos.z >= 0 || m_vertices[index1 + verticesPerLine].Pos.z >= 0 || m_vertices[index1 + verticesPerLine + 1].Pos.z >= 0)
      {
        m_indices[m_indicesNumber++] = index1 + 1;
        m_indices[m_indicesNumber++] = index1 + verticesPerLine + 1;
        m_indices[m_indicesNumber++] = index1 + verticesPerLine;
      }
    }
    index1++;
  }

  XMMATRIX temp1 = XMMatrixIdentity() * XMMatrixTranslation(2.0f, 0.0f, 0.0f);
  XMMATRIX temp2 = XMMatrixRotationX(3.1415926f) * XMMatrixTranslation(2.0f, 0.0f, 0.0f);

  XMStoreFloat4x4(&m_World1, temp1);
  XMStoreFloat4x4(&m_World2, temp2);
}

MyObject::~MyObject()
{
  delete[] m_indices;
  delete[] m_vertices;
}

XMFLOAT4X4 MyObject::GetWorldMatrix1() const
{
  return m_World1;
}

XMFLOAT4X4 MyObject::GetWorldMatrix2() const
{
  return m_World2;
}

void  MyObject::RotateX(float angle)
{
  XMMATRIX rotate = XMMatrixRotationX(angle);
  XMMATRIX temp1 = rotate * XMLoadFloat4x4(&m_World1);
  XMMATRIX temp2 = rotate * XMLoadFloat4x4(&m_World2);
  XMStoreFloat4x4(&m_World1, temp1);
  XMStoreFloat4x4(&m_World2, temp2);
}

void MyObject::RotateY(float angle)
{
  XMMATRIX rotate = XMMatrixRotationY(angle);
  XMMATRIX temp1 = rotate * XMLoadFloat4x4(&m_World1);
  rotate = XMMatrixTranspose(rotate);
  XMMATRIX temp2 = rotate * XMLoadFloat4x4(&m_World2);
  XMStoreFloat4x4(&m_World1, temp1);
  XMStoreFloat4x4(&m_World2, temp2);
}