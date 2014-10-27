#include "stdafx.h"
#include "MyInputListener.h"

using namespace D3D11Framework;

bool MyInputListener::MouseMove(const MouseEvent &arg)
{
  if (m_mousePressed) // otherwise we don't react on mouse moves
  {
    if (m_MousePos.y == arg.y && m_MousePos.x == arg.x)
      return true;
	  float angle_hor = (arg.x - m_MousePos.x) * 0.005f;
    float angle_ver = (arg.y - m_MousePos.y) * 0.005f;

    m_render->GetCamera()->RotateHor(-angle_hor);
    m_render->GetCamera()->RotateVer(-angle_ver);
  }

  if (!m_MouseLocked)
  {
    m_MousePos.y = arg.y;
    m_MousePos.x = arg.x;
  }

  return true; 
}

bool MyInputListener::MousePressed(const MouseEventClick &arg) 
{
  if (arg.btn == eMouseKeyCodes::MOUSE_LEFT)
    m_mousePressed = true;
  return true;
}

bool MyInputListener::MouseReleased(const MouseEventClick &arg)
{
  if (arg.btn == eMouseKeyCodes::MOUSE_LEFT)
    m_mousePressed = false;
  return true;
}

void MyInputListener::SetMouseLocked(bool locked, int x, int y)
{
  m_MouseLocked = locked;
  if (locked)
  {
    m_MousePos.x = x;
    m_MousePos.y = y;
  }
}

bool MyInputListener::KeyPressed(const KeyEvent &event) 
{
  switch (event.code)
  {
  case KEY_DOWN:
    m_render->GetMyObject()->RotateX(0.05f);
    break;
  case KEY_UP:
    m_render->GetMyObject()->RotateX(-0.05f);
    break;
  case KEY_LEFT:
    m_render->GetMyObject()->RotateY(-0.05f);
    break;
  case KEY_RIGHT:
    m_render->GetMyObject()->RotateY(0.05f);
    break;
  default:
    printf("key press %c\n", event.wc);
  }
	return true; 
}

bool MyInputListener::MouseWheel(const MouseEventWheel &arg)
{
  if (arg.wheel > 0)
    m_render->GetCamera()->Walk(-1);
  else
    m_render->GetCamera()->Walk(1);

  return true;
}