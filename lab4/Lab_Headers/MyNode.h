#pragma once
#include <xnamath.h>
#include <vector>

class MyObject;
struct ConstantBuffer;

class MyNode
{
public:
  MyNode(MyObject *object);
  void addChild(MyNode *child);
  virtual bool updateMatrix(float t, const XMFLOAT4X4 *parentMatrix = 0); // updates own matrix and calls update for children
  bool Draw(ID3D11Device *device, ID3D11DeviceContext *deviceContext, ConstantBuffer &cb, const XMMATRIX &view, const XMMATRIX &projection, 
                    ID3D11VertexShader *vertexShader, ID3D11PixelShader *pixelShader, ID3D11Buffer *constantBuffer);

  void Translate(float x, float y, float z);
  void RotateAxis(float x, float y, float z, float angle);
  virtual ~MyNode();
protected:
  std::vector<MyNode*> m_children;
  XMFLOAT4X4 m_matrix;
  virtual XMFLOAT4X4 m_recounterFunction(float t) = 0;
  MyObject *m_object;
};

class StaticNode : public MyNode
{
public:
  StaticNode(MyObject *object) : MyNode(object) {}
protected:
  XMFLOAT4X4 m_recounterFunction(float t) { return m_matrix; }
};

class PetalNode : public MyNode
{
public:
  PetalNode(MyObject *object, float delay = 0, float angleAmplitude = 10.0f, bool isBottom = false, float petalHeight = 1) : MyNode(object), m_delay(delay), m_angleAmplitude(angleAmplitude), m_isBottomPetal(isBottom), m_height(petalHeight) {}
  void setDelay(float delay) { m_delay = delay; }
protected:
  XMFLOAT4X4 m_recounterFunction(float t);
  float m_delay;
  float m_angleAmplitude;
  bool m_isBottomPetal;
  float m_height;
};