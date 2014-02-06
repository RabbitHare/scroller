#ifndef __KEYS_H__
#define __KEYS_H__

#define NUM_JOYSTICKS 4
#define NUM_JOYBUTTONS 20

#define NUM_PLAYER_COMMANDS 8
enum JoyDir
{
	JOY_DIR=NUM_JOYBUTTONS,
	JOY_LEFT=NUM_JOYBUTTONS,
	JOY_RIGHT,
	JOY_UP,
	JOY_DOWN
};
enum KeyState
{
	KS_NONE = 0,
	KS_PRESSED = 1<<0,
	KS_RELEASED = 1<<1
};

enum InputType
{
	KEYBOARD1=0,
	KEYBOARD2,
	JOYSTICK1,
	JOYSTICK2,
	JOYSTICK3,
	JOYSTICK4
};

struct Key
{
	int state;
	inline Key() { state = KS_NONE; }
};
struct Button
{
	int state;
	inline Button() { state = KS_NONE; }
};
struct Joystick
{
	Button buttons[NUM_JOYBUTTONS+4]; //plus right,left,up,down
};

struct KeyBinding
{
	int key1,key2;
	inline KeyBinding () { key1 = -1; key2 = -1;}
};
struct InputDevice
{
	InputType type;
	bool connected;
	KeyBinding bindings[NUM_PLAYER_COMMANDS];
	InputDevice (InputType _type, bool _connected)
	{
		type = _type;
		connected = _connected;
	}
	int Key1(int bind);
	int Key2(int bind);
};

// used for parsing config file
struct KeyName
{
	int key;
	const char *name;
};
struct DeviceName
{
	InputType type;
	const char *name;
};



// returns list of keys
Key* GetKeys ();

Joystick* GetJoysticks ();
// the keys used to control the player

InputDevice* GetInputDevice (InputType type);

// clears releases
void ClearReleasedKeys ();

// function used when parsing the config file

KeyName* GetKeyNames ();

DeviceName* GetDeviceNames ();

#endif

