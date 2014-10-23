#pragma once
#include <xnamath.h>

class MyCamera
{
public:
  MyCamera();
  void Walk(float units);
  void Pitch(float angle);
  void Yaw(float angle);
  void RotateHor(float angle);
  void RotateVer(float angle);
  XMFLOAT4X4 GetViewMatrix();
  void Update();
private:
  void m_rotateHor();
  void m_rotateVer();
  void m_walk();

  XMFLOAT3 m_Position;
  XMFLOAT3 m_LookAt;
  XMFLOAT3 m_Up;
  XMFLOAT3 m_Right;

  float m_deltaHorAngle;
  float m_deltaVerAngle;
  XMFLOAT3 m_deltaPosition;
};