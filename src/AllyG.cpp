

#include "AllyG.h"
#include "ConfigFile.h"

#include <algorithm>
#include "WinUser.h"

bool AllyG::_disabled = true;

#define STICK_KEY 0xFF

DWORD PadToCode(const std::string& name)
{
	static const std::map<std::string, DWORD> nameToCode =
	{
		{"UP", XINPUT_GAMEPAD_DPAD_UP },
		{"DOWN", XINPUT_GAMEPAD_DPAD_DOWN },
		{"LEFT", XINPUT_GAMEPAD_DPAD_LEFT },
		{"RIGHT", XINPUT_GAMEPAD_DPAD_RIGHT },
		{"DPAD_UP", XINPUT_GAMEPAD_DPAD_UP },
		{"DPAD_DOWN", XINPUT_GAMEPAD_DPAD_DOWN },
		{"DPAD_LEFT", XINPUT_GAMEPAD_DPAD_LEFT },
		{"DPAD_RIGHT", XINPUT_GAMEPAD_DPAD_RIGHT },
		{"START", XINPUT_GAMEPAD_START },
		{"MENU", XINPUT_GAMEPAD_START },
		{"BACK", XINPUT_GAMEPAD_BACK },
		{"VIEW", XINPUT_GAMEPAD_BACK },
		{"L3", XINPUT_GAMEPAD_LEFT_THUMB },
		{"LEFT_THUMB", XINPUT_GAMEPAD_LEFT_THUMB },
		{"R3", XINPUT_GAMEPAD_RIGHT_THUMB },
		{"RIGHT_THUMB", XINPUT_GAMEPAD_RIGHT_THUMB },
		{"L", XINPUT_GAMEPAD_LEFT_SHOULDER },
		{"L1", XINPUT_GAMEPAD_LEFT_SHOULDER },
		{"LEFT_SHOULDER", XINPUT_GAMEPAD_LEFT_SHOULDER },
		{"R", XINPUT_GAMEPAD_RIGHT_SHOULDER },
		{"R1", XINPUT_GAMEPAD_RIGHT_SHOULDER },
		{"RIGHT_SHOULDER", XINPUT_GAMEPAD_RIGHT_SHOULDER },
		{"A", XINPUT_GAMEPAD_A },
		{"B", XINPUT_GAMEPAD_B },
		{"X", XINPUT_GAMEPAD_X },
		{"Y", XINPUT_GAMEPAD_Y },
		{"L2", GINPUT_GAMEPAD_LTRIGGER_FULL },
		{"LEFT_TRIGGER", GINPUT_GAMEPAD_LTRIGGER_FULL },
		{"TRIGGER_LEFT", GINPUT_GAMEPAD_LTRIGGER_FULL },
		{"R2", GINPUT_GAMEPAD_RTRIGGER_FULL },
		{"RIGHT_TRIGGER", GINPUT_GAMEPAD_RTRIGGER_FULL },
		{"TRIGGER_RIGHT", GINPUT_GAMEPAD_RTRIGGER_FULL },
		{"L2SHORT", GINPUT_GAMEPAD_LTRIGGER_SHORT },
		{"LEFT_TRIGGER_SHORT", GINPUT_GAMEPAD_LTRIGGER_SHORT },
		{"R2SHORT", GINPUT_GAMEPAD_RTRIGGER_SHORT },
		{"RIGHT_TRIGGER_SHORT", GINPUT_GAMEPAD_RTRIGGER_SHORT },
		{"XBOX", XINPUT_GAMEPAD_GUIDE },
		{"GUIDE", XINPUT_GAMEPAD_GUIDE },
	};
	try {
		return nameToCode.at(name);
	}
	catch (...) {
		return strtol(name.c_str(), 0, 0);
	}
}

//Helper function
std::string osprintf(const char* fmt, ...)
{
	va_list args;
	char buf[1000];
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	return buf;
}

//Helper function
std::string upper_trim(const std::string& str)
{
	size_t s = str.find_first_not_of(" \n\r\t");
	if (s == std::string::npos)
		return "";

	size_t e = str.find_last_not_of(" \n\r\t");
	std::string result = str.substr(s, e - s + 1);
	std::transform(result.begin(), result.end(), result.begin(), ::toupper);
	return result;
}

std::map<std::string, DWORD> getVKNameMap() {
	std::map<std::string, DWORD> result = {
		{"SPACE",			VK_SPACE   },
		{"END",				VK_END     },
		{"HOME",			VK_HOME    },
		{"LEFT",			VK_LEFT    },
		{"UP",				VK_UP      },
		{"RIGHT",			VK_RIGHT   },
		{"DOWN",			VK_DOWN    },
		{"INSERT",			VK_INSERT  },
		{"DELETE",			VK_DELETE  },
		{"ESCAPE",			VK_ESCAPE  },
		{"RETURN",			VK_RETURN  },
		{"ENTER",			VK_RETURN  },
		{"NUMPAD_MULT",		VK_MULTIPLY },
		{"NUMPAD_ADD",		VK_ADD      },
		{"NUMPAD_SUB",		VK_SUBTRACT },
		{"NUMPAD_DECIMAL",  VK_DECIMAL  },
		{"NUMPAD_DIV",		VK_DIVIDE   },
		{"WIN",  	        VK_LWIN     },
		{"LWIN",  	        VK_LWIN     },
		{"RWIN",  	        VK_RWIN     },
		{"APPS",			VK_APPS     },
		{"SHIFT",           VK_SHIFT    },
		{"CTRL",            VK_CONTROL  },
		{"ALT", 			VK_MENU     },
		{"LSHIFT",          VK_LSHIFT   },
		{"RSHIFT",          VK_RSHIFT   },
		{"LCTRL",           VK_LCONTROL },
		{"RCTRL",           VK_RCONTROL },
		{"LALT",            VK_LMENU    },
		{"RALT",            VK_RMENU    },
		{"BACKSPACE",       VK_BACK     },
		{"TAB",             VK_TAB      },
		{"STICK",           STICK_KEY   },
	};

	for (char i = '0'; i <= '9'; i++)
		result[osprintf("%c", i)] = (DWORD)i;
	for (char i = 'A'; i <= 'Z'; i++)
		result[osprintf("%c", i)] = (DWORD)i;
	for (DWORD i = VK_NUMPAD0; i <= VK_NUMPAD9; i++)
		result[osprintf("NUMPAD_%i", i - VK_NUMPAD0)] = i;
	for (DWORD i = VK_F1; i <= VK_F24; i++)
		result[osprintf("F%i", i - VK_F1 + 1)] = i;

	return result;
}

DWORD KeyToCode(const std::string& name)
{
	static const auto nameToCode = getVKNameMap();

	if (!name.length())
		return STICK_KEY;

	try {
		return nameToCode.at(name);
	}
	catch (...) {
		return strtol(name.c_str(), 0, 0);
	}
	return 0;
}


std::vector<DWORD> KeysToCode(const std::string& name, DWORD(*mapper)(const std::string&))
{

	DWORD value = 0;
	std::vector<DWORD> result;

	std::stringstream ss(name);
	std::string item;

	while (getline(ss, item, '+')) {
		result.push_back(mapper(upper_trim(item)));
	}
	return result;
};

DWORD PadKeysToCode(const std::string& name) {
	DWORD result = 0;
	for (auto& key : KeysToCode(name, PadToCode))
		result |= key;
	return result;
}

DWORD VKeysToCode(const std::string& name) {
	for (auto& key : KeysToCode(name, KeyToCode))
		return key;
	return 0;
}

std::vector<WORD> VKeysToCodes(const std::string& name) {
	std::vector<WORD> result;
	for (auto& key : KeysToCode(name, KeyToCode))
		result.push_back((WORD)key);
	return result;
}

// Description:
//   Send a keyboard input to the system based on the key value
//     and its event type.
//
// Params:
//   cmd    The value of the key to send(see http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx)
//   flag   The KEYEVENT for the key
void inputKeyboard(WORD cmd, DWORD flag)
{
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = 0;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.wVk = cmd;
	input.ki.dwFlags = flag;
	SendInput(1, &input, sizeof(INPUT));
}

// Description:
//   Send a keyboard input based on the key value with the "pressed down" event.
//
// Params:
//   cmd    The value of the key to send
void inputKeyboardDown(WORD cmd)
{
	inputKeyboard(cmd, 0);
}

// Description:
//   Send a keyboard input based on the key value with the "released" event
//
// Params:
//   cmd    The value of the key to send
void inputKeyboardUp(WORD cmd)
{
	inputKeyboard(cmd, KEYEVENTF_KEYUP);
}

// Description:
//   Send a mouse input based on a mouse event type.
//   See https://msdn.microsoft.com/en-us/library/windows/desktop/ms646310(v=vs.85).aspx
//
// Params:
//   dwFlags    The mouse event to send
//   mouseData  Additional information needed for certain mouse events (Optional)
void mouseEvent(DWORD dwFlags, DWORD mouseData = 0)
{
	INPUT input;
	input.type = INPUT_MOUSE;

	// Only set mouseData when using a supported dwFlags type
	if (dwFlags == MOUSEEVENTF_WHEEL ||
		dwFlags == MOUSEEVENTF_XUP ||
		dwFlags == MOUSEEVENTF_XDOWN ||
		dwFlags == MOUSEEVENTF_HWHEEL)
	{
		input.mi.mouseData = mouseData;
	}
	else
	{
		input.mi.mouseData = 0;
	}

	input.mi.dwFlags = dwFlags;
	input.mi.time = 0;
	SendInput(1, &input, sizeof(INPUT));
}

AllyG::AllyG(CXBOXController* controller)
	: _controller(controller)
{}

// Description:
//   Reads and parses the configuration file, assigning values to the
//     configuration variables.
void AllyG::loadConfigFile()
{
	ConfigFile cfg("config.ini");

	//--------------------------------
	// Configuration bindings
	//--------------------------------
	CONFIG_MOUSE_LEFT = PadKeysToCode(cfg.getValueOfKey<std::string>("CONFIG_MOUSE_LEFT"));
	CONFIG_MOUSE_RIGHT = PadKeysToCode(cfg.getValueOfKey<std::string>("CONFIG_MOUSE_RIGHT"));
	CONFIG_MOUSE_MIDDLE = PadKeysToCode(cfg.getValueOfKey<std::string>("CONFIG_MOUSE_MIDDLE"));
	CONFIG_HIDE = PadKeysToCode(cfg.getValueOfKey<std::string>("CONFIG_HIDE"));
	CONFIG_DISABLE = PadKeysToCode(cfg.getValueOfKey<std::string>("CONFIG_DISABLE"));
	CONFIG_DISABLE_VIBRATION = PadKeysToCode(cfg.getValueOfKey<std::string>("CONFIG_DISABLE_VIBRATION"));
	CONFIG_SPEED_CHANGE = PadKeysToCode(cfg.getValueOfKey<std::string>("CONFIG_SPEED_CHANGE"));
	CONFIG_THROTTLE = PadKeysToCode(cfg.getValueOfKey<std::string>("CONFIG_THROTTLE"));
	CONFIG_OSK = PadKeysToCode(cfg.getValueOfKey<std::string>("CONFIG_OSK"));

	//--------------------------------
	// Controller bindings
	//--------------------------------
	for (auto& key : cfg.getKeys("GAMEPAD")) {
		GAMEPAD_MAP[PadKeysToCode(key.c_str() + strlen("GAMEPAD "))] = VKeysToCodes(cfg.getValueOfKey<std::string>(key));
	}

	for (auto& key : cfg.getKeys("MAP")) {
		KEY_MAP[PadKeysToCode(key.c_str() + strlen("MAP "))] = VKeysToCodes(cfg.getValueOfKey<std::string>(key));
	}

	GAMEPAD_TRIGGER_LEFT = VKeysToCode(cfg.getValueOfKey<std::string>("GAMEPAD_TRIGGER_LEFT"));
	GAMEPAD_TRIGGER_RIGHT = VKeysToCode(cfg.getValueOfKey<std::string>("GAMEPAD_TRIGGER_RIGHT"));

	//--------------------------------
	// Advanced settings
	//--------------------------------

	// Acceleration factor
	acceleration_factor = strtof(cfg.getValueOfKey<std::string>("ACCELERATION_FACTOR").c_str(), 0);

	// Dead zones
	DEAD_ZONE = strtol(cfg.getValueOfKey<std::string>("DEAD_ZONE").c_str(), 0, 0);
	if (DEAD_ZONE == 0)
	{
		DEAD_ZONE = 6000;
	}

	SCROLL_DEAD_ZONE = strtol(cfg.getValueOfKey<std::string>("SCROLL_DEAD_ZONE").c_str(), 0, 0);
	if (SCROLL_DEAD_ZONE == 0)
	{
		SCROLL_DEAD_ZONE = 5000;
	}

	SCROLL_SPEED = strtof(cfg.getValueOfKey<std::string>("SCROLL_SPEED").c_str(), 0);
	if (SCROLL_SPEED < 0.00001f)
	{
		SCROLL_SPEED = 0.1f;
	}

	// Variable cursor speeds
	std::istringstream cursor_speed = std::istringstream(cfg.getValueOfKey<std::string>("CURSOR_SPEED"));
	int cur_speed_idx = 1;
	const float CUR_SPEED_MIN = 0.0001f;
	const float CUR_SPEED_MAX = 1.0f;
	for (std::string cur_speed; std::getline(cursor_speed, cur_speed, ',');)
	{
		std::istringstream cursor_speed_entry = std::istringstream(cur_speed);
		std::string cur_name, cur_speed_s;
		// Check to see if we are at the string that includes the equals sign.
		if (cur_speed.find_first_of('=') != std::string::npos)
		{
			std::getline(cursor_speed_entry, cur_name, '=');
		}
		else
		{
			std::ostringstream tmp_name;
			tmp_name << cur_speed_idx++;
			cur_name = tmp_name.str();
		}
		std::getline(cursor_speed_entry, cur_speed_s);
		float cur_speedf = strtof(cur_speed_s.c_str(), 0);
		// Ignore speeds that are not within the allowed range.
		if (cur_speedf > CUR_SPEED_MIN && cur_speedf <= CUR_SPEED_MAX)
		{
			speeds.push_back(cur_speedf);
			speed_names.push_back(cur_name);
		}
	}

	// If no cursor speeds were defined, add a set of default speeds.
	if (speeds.size() == 0)
	{
		speeds.push_back(0.005f);
		speeds.push_back(0.015f);
		speeds.push_back(0.025f);
		speeds.push_back(0.004f);
		speed_names.push_back("ULTRALOW");
		speed_names.push_back("LOW");
		speed_names.push_back("MED");
		speed_names.push_back("HIGH");
	}
	speed = speeds[0];  // Initialize the speed to the first speed stored. TODO: Set the speed to a saved speed that was last used when the application was closed last.

	// Swap stick functions
	SWAP_THUMBSTICKS = strtol(cfg.getValueOfKey<std::string>("SWAP_THUMBSTICKS").c_str(), 0, 0);

	// Set the initial window visibility
	setWindowVisibility(_hidden);
}

// Description:
//   The main program loop. Handles the gamepad inputs and converts them
//     to system inputs based on the mapping provided by the configuration
//     file.
void AllyG::loop()
{
	if (!_controller->getNum())
		Sleep(SLEEP_AMOUNT);

	bool connected = _controller->IsConnected();
	if (connected != _connected ) {
		_connected = connected;
		if (_connected)
		{
			int intense = _disabled ? 10000 : 65000;
			pulseVibrate(400, intense, intense);
		}
	}
	if (!_connected) {
		return;
	}

	_currentState = _controller->GetState();

	// Map to Keyboard sequences
	for (auto& padToKey : KEY_MAP) {
		mapKeyboard(padToKey.first, padToKey.second);
	}
	handleNotPressed();

	// Disable AllyG
	handleDisableButton();

	if (_disabled)
	{
		return;
	}

	// Vibration
	handleVibrationButton();

	// Mouse functions
	if (CONFIG_THROTTLE)
	{
		setXboxClickState(CONFIG_THROTTLE);
		_throttle = _xboxClickStateLastIteration[CONFIG_THROTTLE];

	}
	handleMouseMovement();
	handleScrolling();

	if (CONFIG_MOUSE_LEFT)
	{
		mapMouseClick(CONFIG_MOUSE_LEFT, MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP);
	}
	if (CONFIG_MOUSE_RIGHT)
	{
		mapMouseClick(CONFIG_MOUSE_RIGHT, MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP);
	}
	if (CONFIG_MOUSE_MIDDLE)
	{
		mapMouseClick(CONFIG_MOUSE_MIDDLE, MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP);
	}

	// Hides the console
	if (CONFIG_HIDE)
	{
		setXboxClickState(CONFIG_HIDE);
		if (_xboxClickIsDown[CONFIG_HIDE])
		{
			toggleWindowVisibility();
		}
	}

	// Toggle the on-screen keyboard
	if (CONFIG_OSK)
	{
		setXboxClickState(CONFIG_OSK);
		if (_xboxClickIsDown[CONFIG_OSK])
		{
			// Get the otk window
			HWND otk_win = getOskWindow();
			if (otk_win == NULL)
			{
				printf("Please start the On-screen keyboard first\n");
			}
			else if (IsIconic(otk_win))
			{
				ShowWindow(otk_win, SW_RESTORE);
			}
			else
			{
				ShowWindow(otk_win, SW_MINIMIZE);
			}
		}
	}

	// Will change between the current speed values
	setXboxClickState(CONFIG_SPEED_CHANGE);
	if (_xboxClickIsDown[CONFIG_SPEED_CHANGE])
	{
		const int CHANGE_SPEED_VIBRATION_INTENSITY = 65000;   // Speed of the vibration motors when changing cursor speed.
		const int CHANGE_SPEED_VIBRATION_DURATION = 450;      // Duration of the cursor speed change vibration in milliseconds.

		speed_idx++;
		if (speed_idx >= speeds.size())
		{
			speed_idx = 0;
		}
		speed = speeds[speed_idx];
		printf("Setting speed to %f (%s)...\n", speed, speed_names[speed_idx].c_str());
		pulseVibrate(CHANGE_SPEED_VIBRATION_DURATION, CHANGE_SPEED_VIBRATION_INTENSITY, CHANGE_SPEED_VIBRATION_INTENSITY);
	}

	// Map to Keyboard sequences
	for (auto& padToKey : GAMEPAD_MAP)	{
		mapKeyboard(padToKey.first, padToKey.second);
	}
}

// Description:
//   Sends a vibration pulse to the controller for a duration of time.
//     This is a BLOCKING call. Any inputs during the vibration will be IGNORED.
//
// Params:
//   duration   The length of time in milliseconds to vibrate for
//   l          The speed (intensity) of the left vibration motor
//   r          The speed (intensity) of the right vibration motor
void AllyG::pulseVibrate(const int duration, const int l, const int r) const
{
	if (!_vibrationDisabled)
	{
		_controller->Vibrate(0, 0);
		for (int i = 0; i < 2; i++) {
			_controller->Vibrate(l, 0);
			Sleep(duration/4);
			_controller->Vibrate(0, r);
			Sleep(duration/4);
			_controller->Vibrate(0, 0);
		}
	}
}

// Description:
//   Toggles the controller mapping after checking for the disable configuration command.
void AllyG::handleDisableButton()
{
	setXboxClickState(CONFIG_DISABLE);
	if (_xboxClickIsDown[CONFIG_DISABLE])
	{
		int duration = 0;   // milliseconds
		int intensity = 0;  // vibration intensity

		_disabled = !_disabled;

		if (_disabled)
		{
			// Transition to a disabled state.
			duration = 400;
			intensity = 10000;

			// Release all keys currently pressed by the AllyG mapping.
			while (_pressedKeys.size() != 0)
			{
				std::list<WORD>::iterator it = _pressedKeys.begin();

				// Handle mouse buttons
				if (*it == VK_LBUTTON)
				{
					mouseEvent(MOUSEEVENTF_LEFTUP);
				}
				else if (*it == VK_RBUTTON)
				{
					mouseEvent(MOUSEEVENTF_RIGHTUP);
				}
				else if (*it == VK_MBUTTON)
				{
					mouseEvent(MOUSEEVENTF_MIDDLEUP);
				}
				// Handle keys (TODO: support mouse X1 and X2 buttons)
				else
				{
					inputKeyboardUp(*it);
				}

				_pressedKeys.erase(it);
			}
		}
		else
		{
			duration = 400;
			intensity = 65000;
		}

		pulseVibrate(duration, intensity, intensity);
	}
}

// Description:
//   Toggles the vibration support after checking for the diable vibration command.
//   This function will BLOCK to prevent rapidly toggling the vibration.
void AllyG::handleVibrationButton()
{
	setXboxClickState(CONFIG_DISABLE_VIBRATION);
	if (_xboxClickIsDown[CONFIG_DISABLE_VIBRATION])
	{
		_vibrationDisabled = !_vibrationDisabled;
		printf("Vibration %s\n", _vibrationDisabled ? "Disabled" : "Enabled");
		Sleep(1000);
	}
}

// Description:
//   Toggles the visibility of the window.
void AllyG::toggleWindowVisibility()
{
	_hidden = !_hidden;
	printf("Window %s\n", _hidden ? "hidden" : "unhidden");
	setWindowVisibility(_hidden);
}

// Description:
//   Either hides or shows the window.
//
// Params:
//   hidden   Hides the window when true
void AllyG::setWindowVisibility(const bool& hidden) const
{
	HWND hWnd = GetConsoleWindow();
	ShowWindow(hWnd, _hidden ? SW_HIDE : SW_SHOW);
}

template <typename T>
int sgn(T val)
{
	return (T(0) < val) - (val < T(0));
}

// Description:
//   Determines if the thumbstick value is valid and converts it to a float.
//
// Params:
//   t  Analog thumbstick value to check and convert
//
// Returns:
//   If the value is valid, t will be returned as-is as a float. If the value is
//     invalid, 0 will be returned.
float AllyG::getDelta(short t)
{
	//filter non-32768 and 32767, wireless ones can glitch sometimes and send it to the edge of the screen, it'll toss out some HUGE integer even when it's centered
	if (t > 32767) t = 0;
	if (t < -32768) t = 0;

	return t;
}

// Description:
//   Calculates a multiplier for an analog thumbstick based on the update rate.
//
// Params:
//   tValue     The thumbstick value
//   deadzone   The dead zone to use for this thumbstick
//   accel      An exponent to use to create an input curve (Optional). 0 to use a linear input
//
// Returns:
//   Multiplier used to properly scale the given thumbstick value.
float AllyG::getMult(float lengthsq, float deadzone, float accel = 0.0f)
{
	// Normalize the thumbstick value.
	float mult = (sqrt(lengthsq) - deadzone) / (MAXSHORT - deadzone);

	mult = min(1.0f, mult);

	float linear_factor = 0.1;
	float linear_scale = 2.5;

	float exp_factor = 5;
	float exp_scale = 1;

	mult = mult * linear_factor * linear_scale + exp_scale * (exp(mult * exp_factor) - 1) / exp(exp_factor);

	if (accel > 0.0001f)
	{
	  mult = mult * accel;
	}

	return mult / FPS;
}

// Description:
//   Controls the mouse cursor movement by reading the left thumbstick.
void AllyG::handleMouseMovement()
{
	POINT cursor;
	GetCursorPos(&cursor);

	short tx;
	short ty;

	if (SWAP_THUMBSTICKS == 0)
	{
		// Use left stick
		tx = _currentState.Gamepad.sThumbLX;
		ty = _currentState.Gamepad.sThumbLY;
	}
	else
	{
		// Use right stick
		tx = _currentState.Gamepad.sThumbRX;
		ty = _currentState.Gamepad.sThumbRY;
	}

	float x = cursor.x + _xRest;
	float y = cursor.y + _yRest;

	float dx = 0;
	float dy = 0;

	// Handle dead zone
	float lengthsq = tx * tx + ty * ty;
	if (lengthsq > DEAD_ZONE * DEAD_ZONE)
	{
		float mult = speed * getMult(lengthsq, DEAD_ZONE, _throttle ? acceleration_factor : 1 );

		dx = getDelta(tx) * mult;
		dy = getDelta(ty) * mult;
	}

	x += dx;
	_xRest = x - (float)((int)x);

	y -= dy;
	_yRest = y - (float)((int)y);

	INPUT mouseInput;
	mouseInput.type = INPUT_MOUSE;
	mouseInput.mi.dx = dx;
	mouseInput.mi.dy = -dy;
	mouseInput.mi.mouseData = 0;
	mouseInput.mi.time = 0;
	mouseInput.mi.dwFlags = MOUSEEVENTF_MOVE;
	mouseInput.mi.dwExtraInfo = 0;
	SendInput(1, &mouseInput, sizeof(mouseInput));
}

// Description:
//   Controls the scroll wheel movement by reading the right thumbstick.
void AllyG::handleScrolling()
{
	float tx;
	float ty;

	if (SWAP_THUMBSTICKS == 0)
	{
		// Use right stick
		tx = getDelta(_currentState.Gamepad.sThumbRX);
		ty = getDelta(_currentState.Gamepad.sThumbRY);
	}
	else
	{
		// Use left stick
		tx = getDelta(_currentState.Gamepad.sThumbLX);
		ty = getDelta(_currentState.Gamepad.sThumbLY);
	}

	// Handle dead zone
	float magnitude = sqrt(tx * tx + ty * ty);

	if (magnitude > SCROLL_DEAD_ZONE)
	{
		mouseEvent(MOUSEEVENTF_HWHEEL, tx * getMult(tx * tx, SCROLL_DEAD_ZONE, _throttle ? acceleration_factor : 1) * SCROLL_SPEED);
		mouseEvent(MOUSEEVENTF_WHEEL, ty * getMult(ty * ty, SCROLL_DEAD_ZONE, _throttle ? acceleration_factor : 1) * SCROLL_SPEED);
	}
}

DWORD AllyG::readPadState()
{
	bool lTriggerIsFullDown = _currentState.Gamepad.bLeftTrigger > TRIGGER_DEAD_ZONE;
	bool rTriggerIsFullDown = _currentState.Gamepad.bRightTrigger > TRIGGER_DEAD_ZONE;

	bool lTriggerIsDown = _currentState.Gamepad.bLeftTrigger && _currentState.Gamepad.bLeftTrigger < TRIGGER_DEAD_ZONE;
	bool rTriggerIsDown = _currentState.Gamepad.bRightTrigger && _currentState.Gamepad.bRightTrigger < TRIGGER_DEAD_ZONE;
	return _currentState.Gamepad.wButtons
		| (lTriggerIsFullDown ? GINPUT_GAMEPAD_LTRIGGER_FULL : 0)
		| (lTriggerIsDown ? GINPUT_GAMEPAD_LTRIGGER_SHORT : 0)
		| (rTriggerIsFullDown ? GINPUT_GAMEPAD_RTRIGGER_FULL : 0)
		| (rTriggerIsDown ? GINPUT_GAMEPAD_RTRIGGER_SHORT : 0);

}

// Description:
//   Handles the state of a controller button press.
//
// Params:
//   STATE  The AllyG state, or command, to update
void AllyG::setXboxClickState(DWORD STATE)
{
	_xboxClickIsDown[STATE] = false;
	_xboxClickIsUp[STATE] = false;

	if (!this->xboxClickStateExists(STATE))
	{
		_xboxClickStateLastIteration[STATE] = false;
	}

	bool isDown = (readPadState() & STATE) == STATE;

	// Detect if the button has been pressed.
	if (isDown && !_xboxClickStateLastIteration[STATE])
	{
		_xboxClickStateLastIteration[STATE] = true;
		_xboxClickIsDown[STATE] = true;
		_xboxClickDownLength[STATE] = 0;
		_xboxClickIsDownLong[STATE] = false;
	}

	// Detect if the button has been held as a long press.
	if (isDown && _xboxClickStateLastIteration[STATE])
	{
		const int LONG_PRESS_TIME = 200;  // milliseconds

		++_xboxClickDownLength[STATE];
		if (_xboxClickDownLength[STATE] * SLEEP_AMOUNT > LONG_PRESS_TIME)
		{
			_xboxClickIsDownLong[STATE] = true;
		}
	}

	// Detect if the button has been released.
	if (!isDown && _xboxClickStateLastIteration[STATE])
	{
		_xboxClickStateLastIteration[STATE] = false;
		_xboxClickIsUp[STATE] = true;
		_xboxClickIsDownLong[STATE] = false;
	}

	_xboxClickStateLastIteration[STATE] = isDown;
}

// Description:
//   Check to see if a controller state exists in AllyG's button map.
//
// Params:
//   xinput   The AllyG state, or command, to search for
//
// Returns:
//   true if the state is present in the map.
bool AllyG::xboxClickStateExists(DWORD STATE)
{
	auto it = _xboxClickStateLastIteration.find(STATE);
	if (it == _xboxClickStateLastIteration.end())
	{
		return false;
	}

	return true;
}

// Description:
//   Presses or releases a key based on a mapped AllyG state.
//
// Params:
//   STATE  The AllyG state, or command, to trigger a key event
//   key    The key value to input to the system
void AllyG::mapKeyboard(DWORD STATE, WORD key)
{
	mapKeyboard(STATE, std::vector<WORD>{key});
}

// Description:
//   Presses or releases a key based on a mapped AllyG state.
//
// Params:
//   STATE  The AllyG state, or command, to trigger a key event
//   key    The key value to input to the system
void AllyG::mapKeyboard(DWORD STATE, std::vector<WORD> keys)
{
	setXboxClickState(STATE);
	if (_xboxClickIsDown[STATE]) for (auto& key : keys)
	{
		if (key == STICK_KEY)
			continue;

		if (std::find(_pressedKeys.begin(), _pressedKeys.end(), key) == std::end(_pressedKeys))
		{
			inputKeyboardDown(key);

			// Add key to the list of pressed keys.
			_pressedKeys.push_back(key);
		}
	}

	bool keepNext = false;
	if (_xboxClickIsUp[STATE]) for (auto& key : keys)
	{
		if (key == STICK_KEY) {
			keepNext = ((readPadState() & STATE) != 0);
			continue;
		}

		if (!keepNext)
		{
			inputKeyboardUp(key);
			// Remove key from the list of pressed keys.
			erasePressedKey(key);
		}
		keepNext = false;
	}
}

void AllyG::handleNotPressed() {
	if (!readPadState() && _pressedKeys.size())
	{
		for (auto& key : _pressedKeys) inputKeyboardUp(key);
		_pressedKeys.clear();
	}
}

// Description:
//   Presses or releases a mouse button based on a mapped AllyG state
//
// Params:
//   STATE    The AllyG state, or command, to trigger a mouse event
//   keyDown  The button down event for a mouse event
//   keyUp    The button up event for a mouse event
void AllyG::mapMouseClick(DWORD STATE, DWORD keyDown, DWORD keyUp)
{
	setXboxClickState(STATE);
	if (_xboxClickIsDown[STATE])
	{
		mouseEvent(keyDown);

		// Add key to the list of pressed keys.
		if (keyDown == MOUSEEVENTF_LEFTDOWN)
		{
			_pressedKeys.push_back(VK_LBUTTON);
		}
		else if (keyDown == MOUSEEVENTF_RIGHTDOWN)
		{
			_pressedKeys.push_back(VK_RBUTTON);
		}
		else if (keyDown == MOUSEEVENTF_MIDDLEDOWN)
		{
			_pressedKeys.push_back(VK_MBUTTON);
		}
	}

	if (_xboxClickIsUp[STATE])
	{
		mouseEvent(keyUp);

		// Remove key from the list of pressed keys.
		if (keyUp == MOUSEEVENTF_LEFTUP)
		{
			erasePressedKey(VK_LBUTTON);
		}
		else if (keyUp == MOUSEEVENTF_RIGHTUP)
		{
			erasePressedKey(VK_RBUTTON);
		}
		else if (keyUp == MOUSEEVENTF_MIDDLEUP)
		{
			erasePressedKey(VK_MBUTTON);
		}
	}

	/*if (_xboxClickIsDownLong[STATE])
	{
	  mouseEvent(keyDown | keyUp);
	  mouseEvent(keyDown | keyUp);
	}*/
}

// Description:
//   Callback function used for the EnumWindows call to determine if we
//     have found the On-Screen Keyboard window.
//
// Params:
//   curWnd   The current window to check
//   lParam   A callback parameter used to store the window if it is found
//
// Returns:
//   FALSE when the the desired window is found.
static BOOL CALLBACK EnumWindowsProc(HWND curWnd, LPARAM lParam)
{
	TCHAR title[256];
	// Check to see if the window title matches what we are looking for.
	if (GetWindowText(curWnd, title, 256) && !_tcscmp(title, _T("On-Screen Keyboard")))
	{
		*(HWND*)lParam = curWnd;
		return FALSE;  // Correct window found, stop enumerating through windows.
	}

	return TRUE;
}

// Description:
//   Finds the On-Screen Keyboard if it is open.
//
// Returns:
//   If found, the handle to the On-Screen Keyboard handle. Otherwise, returns NULL.
HWND AllyG::getOskWindow()
{
	HWND ret = NULL;
	EnumWindows(EnumWindowsProc, (LPARAM)&ret);
	return ret;
}

// Description:
//   Removes an entry for a pressed key from the list.
//
// Params:
//   key  The key value to remove from the pressed key list.
//
// Returns:
//   True if the given key was found and removed from the list.
bool AllyG::erasePressedKey(WORD key)
{
	for (std::list<WORD>::iterator it = _pressedKeys.begin();
		it != _pressedKeys.end();
		++it)
	{
		if (*it == key)
		{
			_pressedKeys.erase(it);
			return true;
		}
	}

	return false;
}
