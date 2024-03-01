#include "CXBOXController.h"
#include "TCHAR.h"

CXBOXController::Extra::Extra()
{
    try {
        TCHAR xinput_dll_path[MAX_PATH];
        GetSystemDirectory(xinput_dll_path, sizeof(xinput_dll_path) - 1);
        _tcscat_s(xinput_dll_path, MAX_PATH, _T("\\xinput1_4.dll"));
        xinput_dll = LoadLibrary(xinput_dll_path);
        XInputGetStateFunc = xinput_dll ? (GET_GAMEPAD_XINPUT)GetProcAddress(xinput_dll, (LPCSTR)100) : NULL; // load ordinal 100
    }
    catch (...)
    {
    }
}

CXBOXController::Extra::~Extra()
{
    if (xinput_dll) FreeLibrary(xinput_dll);
}

DWORD CXBOXController::Extra::XInputGetState(int num, XINPUT_STATE* state)
{
    static Extra _extraProc;
    return _extraProc.XInputGetStateFunc ? _extraProc.XInputGetStateFunc(num, state) : XInputGetState(num, state);
}


CXBOXController::CXBOXController(int playerNumber)
{
    _controllerNum = playerNumber - 1; //set number
}

XINPUT_STATE CXBOXController::GetState()
{
    ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));
    Extra::XInputGetState(_controllerNum, &_controllerState);

    return _controllerState;
}

bool CXBOXController::IsConnected()
{
    ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));
    DWORD Result = XInputGetState(_controllerNum, &_controllerState);

    return (Result == ERROR_SUCCESS);
}

void CXBOXController::Vibrate(int leftVal, int rightVal)
{
    // Create a Vibraton State
    XINPUT_VIBRATION Vibration;

    // Zeroise the Vibration
    ZeroMemory(&Vibration, sizeof(XINPUT_VIBRATION));

    // Set the Vibration Values
    Vibration.wLeftMotorSpeed = leftVal;
    Vibration.wRightMotorSpeed = rightVal;

    // Vibrate the controller
    XInputSetState(_controllerNum, &Vibration);
}