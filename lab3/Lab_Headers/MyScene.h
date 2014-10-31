#pragma once
#include <vector>
#include "MyObject.h"

class MyScene : public MyObject
{
public:
  MyScene();
  long LoadObject(const char *fileName);
  size_t GetObjectsNumber() const { return m_objects.size(); }
  MyObject *GetObjectAt(size_t idx);
  const std::vector<MyObject *> &GetObjects() const { return m_objects; } 
  XMFLOAT4X4 GetProjectorWorldMatrixAt(size_t idx) const;
  XMFLOAT4X4 GetProjectorRotationMatrixAt(size_t idx) const;
private:
  std::vector<MyObject *> m_objects; // NB first element is scene itself! so this vector usually is not empty
  std::vector<size_t> m_projectorObjectNumber; 
};