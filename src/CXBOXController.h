#pragma once

#include <windows.h>
#include <xinput.h>

#define GINPUT_GAMEPAD_LTRIGGER_SHORT       0x10000
#define GINPUT_GAMEPAD_LTRIGGER_FULL        0x20000
#define GINPUT_GAMEPAD_RTRIGGER_SHORT       0x40000
#define GINPUT_GAMEPAD_RTRIGGER_FULL        0x80000
#define XINPUT_GAMEPAD_GUIDE                0x400
#define XINPUT_GAMEPAD_RESERVED             0x800

// returns 0 on success, 1167 on not connected. Might be others.
typedef int(__stdcall* GET_GAMEPAD_XINPUT) (int, XINPUT_STATE*);



class CXBOXController
{
	class Extra
	{
		Extra();
		GET_GAMEPAD_XINPUT XInputGetStateFunc;
		HINSTANCE xinput_dll;

	public:
		static DWORD XInputGetState(int, XINPUT_STATE*);
		~Extra();

	};


private:
  XINPUT_STATE _controllerState;
  int _controllerNum;

  GET_GAMEPAD_XINPUT loadProc();


public:
	
  CXBOXController(int playerNumber);

  XINPUT_STATE GetState();
  bool IsConnected();
  void Vibrate(int leftVal, int rightVal);
  int getNum() { return _controllerNum; }
};

