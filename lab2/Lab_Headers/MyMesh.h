#pragma once

struct aiScene;

class MyMesh
{
public:
  MyMesh();
  int LoadFromFile(const char *fileName);
public:
  const aiScene *m_scene;
};