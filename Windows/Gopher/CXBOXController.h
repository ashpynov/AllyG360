#pragma once

#include <windows.h>
#include <xinput.h>

#define GINPUT_GAMEPAD_LTRIGGER_SHORT       0x10000
#define GINPUT_GAMEPAD_LTRIGGER_FULL        0x20000
#define GINPUT_GAMEPAD_RTRIGGER_SHORT       0x40000
#define GINPUT_GAMEPAD_RTRIGGER_FULL        0x80000
#define XINPUT_GAMEPAD_GUIDE                0x400
#define XINPUT_GAMEPAD_RESERVED             0x800


class CXBOXController
{
private:
  XINPUT_STATE _controllerState;
  int _controllerNum;
public:
  CXBOXController(int playerNumber);
  XINPUT_STATE GetState();
  bool IsConnected();
  void Vibrate(int leftVal, int rightVal);
  int getNum() { return _controllerNum; }
};

