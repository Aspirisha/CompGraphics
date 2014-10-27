#include "stdafx.h"
#include "MyInputListener.h"

using namespace D3D11Framework;

bool MyInputListener::MouseMove(const MouseEvent &arg)
{
  if (m_mousePressed)
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
  if (m_render->GetCamera()->GetType() == WASD)
  {
    switch (event.code)
    {
    case KEY_W:
      m_render->GetCamera()->WalkForward(0.05f);
      break;
    case KEY_S:
      m_render->GetCamera()->WalkForward(-0.05f);
      break;
    case KEY_A:
      m_render->GetCamera()->WalkLeft(-0.05f);
      break;
    case KEY_D:
      m_render->GetCamera()->WalkLeft(0.05f);
      break;
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
    case KEY_SPACE:
      m_render->GetCamera()->SwitchType();
      break;
    case KEY_1:
      m_render->SwitchDirectedLights();
      break;
    case KEY_2:
      m_render->SwitchPointLights();
      break;
    case KEY_3:
      m_render->SwitchProjectorLights();
      break;
    default:
      printf("key press %c\n", event.wc);
    }
  }
  else if (m_render->GetCamera()->GetType() == SPHERE)
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
    case KEY_SPACE:
      m_render->GetCamera()->SwitchType();
      break;
    default:
      printf("key press %c\n", event.wc);
    }
  }
	return true; 
}

bool MyInputListener::MouseWheel(const MouseEventWheel &arg)
{
  if (m_render->GetCamera()->GetType() == SPHERE)
  {
    if (arg.wheel > 0)
      m_render->GetCamera()->WalkForward(-1);
    else
      m_render->GetCamera()->WalkForward(1);
  }
  return true;
}