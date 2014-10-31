#pragma once

#include "InputCodes.h"

namespace D3D11Framework
{
  struct MouseEvent
  {
    MouseEvent(int _x, int _y) : x(_x), y(_y) {}

    int x;
    int y;
  };

  struct MouseEventClick : public MouseEvent
  {
    MouseEventClick(eMouseKeyCodes b, int _x, int _y) : MouseEvent(_x, _y), btn(b) {}

    eMouseKeyCodes btn;
  };

  struct MouseEventWheel : public MouseEvent
  {
    MouseEventWheel(int _wheel, int _x, int _y) : MouseEvent(_x, _y), wheel(_wheel) {}

    int wheel;
  };

  struct KeyEvent
  {
    KeyEvent(wchar_t _c, eKeyCodes _ek) : wc(_c), code(_ek) {}

    wchar_t wc;
    eKeyCodes code;
  };

  class InputListener
  {
  public:
    virtual bool MousePressed(const MouseEventClick &arg) { return false; }
    virtual bool MouseReleased(const MouseEventClick &arg) { return false; }
    virtual bool MouseWheel(const MouseEventWheel &arg) { return false; }
    virtual bool MouseMove(const MouseEvent &arg) { return false; }
		virtual bool KeyPressed(const KeyEvent &arg) { return false; }
		virtual bool KeyReleased(const KeyEvent &arg) { return false; }
  };
}