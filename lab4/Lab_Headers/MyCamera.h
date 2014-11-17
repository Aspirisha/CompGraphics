#pragma once
#include <xnamath.h>

enum CAMERA_TYPE 
{
  WASD,
  SPHERE
};

class MyCamera
{
public:
  MyCamera();
  void WalkForward(float units);
  void WalkLeft(float units);
  void Pitch(float angle);
  void Yaw(float angle);
  void RotateHor(float angle);
  void RotateVer(float angle);
  XMFLOAT4X4 GetViewMatrix();
  void Update();
  void SwitchType();
  CAMERA_TYPE GetType() { return m_type; }

  void IncreaseSpeed();
  void DecreaseSpeed();
private:
  void m_rotateHor();
  void m_rotateVer();
  void m_walk();
  
  CAMERA_TYPE m_type;
  XMFLOAT3 m_Position;
  XMFLOAT3 m_LookAt;
  XMFLOAT3 m_Up;
  XMFLOAT3 m_Right;

  float m_deltaHorAngle;
  float m_deltaVerAngle;
  float m_speed;
  XMFLOAT3 m_deltaPosition;
};