#include "stdafx.h"
#include "macros.h"
#include "Framework.h"
#include "MyRender.h"
#include "MyInputListener.h"

#ifndef _WIN32
  #define _WIN32 
#endif
/*
#ifdef _MSC_VER
#    pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif
*/

using namespace D3D11Framework;

int main(void)
{
  Framework framework;
  MyRender *render = new MyRender(60);
  MyInputListener *input = new MyInputListener();
  DescWindow desc;
  
  desc.width = 640;
  desc.height = 480;

  input->SetRender(render);
  framework.SetRender(render);
  framework.Init(desc);
  //framework.SetMouseLocked(true, desc.width >> 1, desc.height >> 1);
  framework.AddInputListener(input);

  framework.Run();
  framework.Close();
  return 0;
}