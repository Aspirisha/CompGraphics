//#include <d3dx9anim.h>
#include <d3dx9.h>
//#include <winnt.h>
// assimp include files. These three are usually needed.
#include <cimport.h>
#include <scene.h>
#include <postprocess.h>
#include "MyMesh.h"
#include "Log.h"

#pragma comment(lib, "assimpd.lib")
#pragma comment(lib, "zlibstaticd.lib")
using namespace D3D11Framework;
typedef const aiScene *(*impFile)(const char *, unsigned int);

MyMesh::MyMesh() : m_scene(nullptr)
{

}

int MyMesh::LoadFromFile(const char *fileName)
{
  HMODULE handler = LoadLibrary("assimp.dll");
  if (handler == nullptr)
  {
    Log::Get()->Err("assimp.dll not found in %PATH% and in application directory. Shutting down...");
    return 0;
  }
  impFile f = (impFile)GetProcAddress(handler, "aiImportFile");
  m_scene = f(fileName, aiProcessPreset_TargetRealtime_MaxQuality);
  
  if (!m_scene)
    return 0;
  if (m_scene->HasCameras())
    Log::Get()->Debug("m_scene has %i cameras", m_scene->mNumCameras);
  if (m_scene->HasAnimations())
     Log::Get()->Debug("m_scene has %i animations", m_scene->mNumAnimations);
  if (m_scene->HasLights())
   Log::Get()->Debug("m_scene has %i lights", m_scene->mNumLights);
  Log::Get()->Debug("m_scene has %i meshes", m_scene->mNumMeshes);
  
  return 1;
  
}