#pragma once
#include "InputListener.h"
#include "MyRender.h"
#include <stdio.h>

using namespace D3D11Framework;

class MyInputListener : public InputListener
{
public:
  void SetRender(MyRender *render)
  {
    m_render = render;
  }

  bool KeyPressed(const KeyEvent &event);

  void SetMouseLocked(bool locked, int x = 0, int y = 0);
  bool MouseMove(const MouseEvent &arg);
  bool MouseWheel(const MouseEventWheel &arg);
private:
  MyRender *m_render;
  bool m_MouseLocked;
  POINT m_MousePos;
};