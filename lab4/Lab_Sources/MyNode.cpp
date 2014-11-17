#include "stdafx.h"
#include "MyNode.h"
#include "MyObject.h"

void MyNode::addChild(MyNode *child)
{
  if (!child)
    return;
  m_children.push_back(child);
}

MyNode::MyNode(MyObject *object) : m_object(object) 
{
  m_matrix = m_object->GetWorldMatrix();
}

MyNode::~MyNode()
{
  
}

void MyNode::Translate(float x, float y, float z)
{
  XMStoreFloat4x4(&m_matrix, XMLoadFloat4x4(&m_matrix) * XMMatrixTranslation(x, y, z));
}

void MyNode::RotateAxis(float x, float y, float z, float angle)
{
  XMVECTOR axis = XMLoadFloat3(&XMFLOAT3(x,y,z));
  XMStoreFloat4x4(&m_matrix, XMMatrixRotationAxis(axis, angle) * XMLoadFloat4x4(&m_matrix));
}

bool MyNode::updateMatrix(float t, const XMFLOAT4X4 *parentMatrix)
{ 
  m_matrix = m_recounterFunction(t);
  XMFLOAT4X4 absolutMatrix = m_matrix;
  if (parentMatrix)
    XMStoreFloat4x4(&absolutMatrix, XMLoadFloat4x4(&m_matrix) * XMLoadFloat4x4(parentMatrix));

  m_object->SetWorldMatrix(absolutMatrix);
  for (MyNode *node : m_children)
    node->updateMatrix(t, &absolutMatrix);

  return true;
}

bool MyNode::Draw(ID3D11Device *device, ID3D11DeviceContext *deviceContext, ConstantBuffer &cb, const XMMATRIX &view, const XMMATRIX &projection, 
                    ID3D11VertexShader *vertexShader, ID3D11PixelShader *pixelShader, ID3D11Buffer *constantBuffer)
{
  if (!m_object)
    return false;
  m_object->Draw(device, deviceContext, cb, view, projection, vertexShader, pixelShader, constantBuffer);
  return true;
}

XMFLOAT4X4 PetalNode::m_recounterFunction(float t)
{
  static float fi = 0;
  if (t - m_delay < 0)
    return m_matrix;

  float dfi = m_angleAmplitude * sin(XM_PIDIV2 * (t - m_delay) * 3);
  fi += dfi;

  XMVECTOR axis = XMLoadFloat3(&XMFLOAT3(1, 0, 0));
  XMMATRIX rotationMatrix = XMMatrixRotationAxis(axis, dfi);
  XMMATRIX prevMatrix = XMLoadFloat4x4(&m_matrix);

  if (!m_isBottomPetal)
    XMStoreFloat4x4(&m_matrix, rotationMatrix * prevMatrix);
  else
    XMStoreFloat4x4(&m_matrix, XMMatrixTranslation(0, m_height, 0) * rotationMatrix * XMMatrixTranslation(0, -m_height, 0) * prevMatrix);
  
  return m_matrix;
}