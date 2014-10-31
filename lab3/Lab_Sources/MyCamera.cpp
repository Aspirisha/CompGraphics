#include "stdafx.h"
#include "MyCamera.h"
#include "Log.h"

const DWORD millisBetweenRecounts = 40; // 25 fps

MyCamera::MyCamera()
{
  m_Position = XMFLOAT3(0.0f, 0.0f, -3.0f);
  m_LookAt = XMFLOAT3(0.0f, 0.0f, 1.0f);
  m_Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
  m_Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
  m_deltaPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
  m_deltaHorAngle = 0;
  m_deltaVerAngle = 0;
  m_speed = 0.5f;
  m_type = SPHERE;
}

XMFLOAT4X4 MyCamera::GetViewMatrix()
{
	float x = -(m_Position.x * m_Right.x + m_Position.y * m_Right.y + m_Position.z * m_Right.z); // project position on camera axis
	float y = -(m_Position.x * m_Up.x + m_Position.y * m_Up.y + m_Position.z * m_Up.z);
	float z = -(m_Position.x * m_LookAt.x + m_Position.y * m_LookAt.y + m_Position.z * m_LookAt.z);

  XMFLOAT4X4 ret;

  ret._11 = m_Right.x; 
	ret._21  = m_Right.y; 
	ret._31  = m_Right.z; 
	ret._41  = x;

	ret._12 = m_Up.x;
	ret._22 = m_Up.y;
	ret._32 = m_Up.z;
	ret._42 = y;

	ret._13 = m_LookAt.x; 
	ret._23 = m_LookAt.y; 
	ret._33 = m_LookAt.z; 
	ret._43 = z;

	ret._14 = 0.0f;
	ret._24 = 0.0f;
	ret._34 = 0.0f;
	ret._44 = 1.0f;	
  
  return ret;
}

void MyCamera::WalkForward(float units)
{
  m_deltaPosition.z += units;
}

void MyCamera::WalkLeft(float units)
{
  m_deltaPosition.x += units;
}

void MyCamera::RotateHor(float angle)
{
  m_deltaHorAngle += angle;
}

void MyCamera::RotateVer(float angle)
{
  m_deltaVerAngle += angle;
}

void MyCamera::Pitch(float angle)
{
 // Rotate up and look vector about the right vector.
	const XMMATRIX rotation = XMMatrixRotationAxis(XMLoadFloat3(&m_Right), angle);

	XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), rotation));
	XMStoreFloat3(&m_LookAt, XMVector3TransformNormal(XMLoadFloat3(&m_LookAt), rotation));
}

void MyCamera::Yaw(float angle)
{
 // Rotate the basis vectors about the world y-axis.
	const XMMATRIX rotation = XMMatrixRotationY(angle);

	XMStoreFloat3(&m_Right, XMVector3TransformNormal(XMLoadFloat3(&m_Right), rotation));
	XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), rotation));
	XMStoreFloat3(&m_LookAt, XMVector3TransformNormal(XMLoadFloat3(&m_LookAt), rotation));
}

void MyCamera::Update()
{
  m_rotateHor();
  m_rotateVer();
  m_walk();
}

void MyCamera::m_rotateHor()
{
  if (m_type == SPHERE)
  {
    XMMATRIX rotation = XMMatrixRotationAxis(XMLoadFloat3(&m_Up), m_deltaHorAngle);

    XMStoreFloat3(&m_Right, XMVector3TransformNormal(XMLoadFloat3(&m_Right), rotation));
    XMStoreFloat3(&m_LookAt, XMVector3TransformNormal(XMLoadFloat3(&m_LookAt), rotation));
	  XMStoreFloat3(&m_Position, XMVector3TransformNormal(XMLoadFloat3(&m_Position), rotation));
  }
  else
  {
  	const XMMATRIX rotation = XMMatrixRotationY(m_deltaHorAngle);

	  XMStoreFloat3(&m_Right, XMVector3TransformNormal(XMLoadFloat3(&m_Right), rotation));
	  XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), rotation));
	  XMStoreFloat3(&m_LookAt, XMVector3TransformNormal(XMLoadFloat3(&m_LookAt), rotation));
  }
  
  m_deltaHorAngle = 0;
  D3D11Framework::Log::Get()->Debug("Camera position: x = %f, y = %f, z = %f", m_Position.x, m_Position.y, m_Position.z);
}

void MyCamera::m_rotateVer()
{
  if (m_type == SPHERE)
  {
    XMMATRIX rotation = XMMatrixRotationAxis(XMLoadFloat3(&m_Right), m_deltaVerAngle);

    XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), rotation));
    XMStoreFloat3(&m_LookAt, XMVector3TransformNormal(XMLoadFloat3(&m_LookAt), rotation));
	  XMStoreFloat3(&m_Position, XMVector3TransformNormal(XMLoadFloat3(&m_Position), rotation));
  }
  else
  {
	  const XMMATRIX rotation = XMMatrixRotationAxis(XMLoadFloat3(&m_Right), m_deltaVerAngle);

	  XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), rotation));
	  XMStoreFloat3(&m_LookAt, XMVector3TransformNormal(XMLoadFloat3(&m_LookAt), rotation));
  }
  m_deltaVerAngle = 0;

  D3D11Framework::Log::Get()->Debug("Camera position: x = %f, y = %f, z = %f", m_Position.x, m_Position.y, m_Position.z);
}

void MyCamera::m_walk()
{
  XMVECTOR eye = XMLoadFloat3(&m_Position);

  if (m_type == SPHERE)
  {
    float min_dist = 0.5f;
    float max_dist = 100.0f;
    float dist = sqrtf(XMVectorGetX(XMVector3Dot(eye, eye)));
    XMVECTOR walkVector = m_speed * eye / dist;
    float dz = m_deltaPosition.z;
    if (dist > min_dist && dz < 0)
    {
      eye -= walkVector;
    }
    else if (dist < max_dist && dz > 0)
    {
      eye += walkVector;
    }
  }
  else
  {
    XMVECTOR right = XMLoadFloat3(&m_Right);
    XMVECTOR lookAt = XMLoadFloat3(&m_LookAt);
    XMVECTOR walkVector = (m_deltaPosition.z * lookAt + m_deltaPosition.x * right);

    eye += 5 * walkVector;
  }
  XMStoreFloat3(&m_Position, eye);
  m_deltaPosition.z = 0;
  m_deltaPosition.x = 0;
}

void MyCamera::SwitchType()
{
  if (m_type == SPHERE)
    m_type = WASD;
  else
    m_type = SPHERE;
}

