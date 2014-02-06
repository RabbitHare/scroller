#include <Keys.h>
#include <stddef.h>
#include <SDL/SDL.h>
#include <iostream>

static const int NUM_KEYS = 512;
static Key keys[NUM_KEYS];
static Joystick joysticks[NUM_JOYSTICKS];
static InputDevice inputDevices[] ={
	InputDevice(KEYBOARD1,true),
	InputDevice(KEYBOARD2,true),
	InputDevice(JOYSTICK1,false),
	InputDevice(JOYSTICK2,false),
	InputDevice(JOYSTICK3,false),
	InputDevice(JOYSTICK4,false)};
static DeviceName deviceNames[] ={
	{KEYBOARD1, "keyboard1"},
	{KEYBOARD2, "keyboard2"},
	{JOYSTICK1, "joystick1"},
	{JOYSTICK2, "joystick2"},
	{JOYSTICK3, "joystick3"},
	{JOYSTICK4, "joystick4"},
	{KEYBOARD1, NULL}};

static KeyName keyNames[] ={
	{SDLK_BACKSPACE, "keyBackspace"},
	{SDLK_RETURN, "keyEnter"},
	{SDLK_SPACE, "keySpace"},
	{SDLK_TAB, "keyTab"},
	{SDLK_UP, "keyUp"},
	{SDLK_DOWN, "keyDown"},
	{SDLK_RIGHT, "keyRight"},
	{SDLK_LEFT, "keyLeft"},
	{SDLK_RSHIFT, "keyRShift"},
	{SDLK_LSHIFT, "keyLShift"},
	{SDLK_RCTRL, "keyRCtrl"},
	{SDLK_LCTRL, "keyLCtrl"},
	{SDLK_RALT, "keyRAlt"},
	{SDLK_LALT, "keyLAlt"},
	{SDLK_a, "keyA"},
	{SDLK_b, "keyB"},
	{SDLK_c, "keyC"},
	{SDLK_d, "keyD"},
	{SDLK_e, "keyE"},
	{SDLK_f, "keyF"},
	{SDLK_g, "keyG"},
	{SDLK_h, "keyH"},
	{SDLK_i, "keyI"},
	{SDLK_l, "keyL"},
	{SDLK_m, "keyM"},
	{SDLK_n, "keyN"},
	{SDLK_o, "keyO"},
	{SDLK_p, "keyP"},
	{SDLK_e, "keyQ"},
	{SDLK_r, "keyR"},
	{SDLK_s, "keyS"},
	{SDLK_t, "keyT"},
	{SDLK_u, "keyU"},
	{SDLK_v, "keyV"},
	{SDLK_w, "keyW"},
	{SDLK_x, "keyX"},
	{SDLK_y, "keyY"},
	{SDLK_z, "keyZ"},
	{0, NULL}};

Key* GetKeys ()
{
	return keys;
}
Joystick* GetJoysticks()
{
	return joysticks;
}
InputDevice* GetInputDevice (InputType type)
{
	return inputDevices+type;
}

void ClearReleasedKeys ()
{
	for (int i=0; i < NUM_KEYS; i++)
		if (keys[i].state == KS_RELEASED)
			keys[i].state = KS_NONE;
	for (int j=0; j < NUM_JOYSTICKS; j++)
	{
		for (int i=0; i < NUM_JOYBUTTONS; i++)
			if (joysticks[j].buttons[i].state == KS_RELEASED)
				joysticks[j].buttons[i].state = KS_NONE;
		for (int i=0; i < 4; i++)
			if (joysticks[j].buttons[JOY_DIR+i].state == KS_RELEASED)
				joysticks[j].buttons[JOY_DIR+i].state = KS_NONE;
	}

}
KeyName* GetKeyNames ()
{
	return keyNames;
}
DeviceName* GetDeviceNames ()
{
	return deviceNames;
}

int
InputDevice::Key1(int bind)
{
	KeyBinding *b = &bindings[bind];
	if (b->key1 < 0) return KS_NONE;
	if (type < JOYSTICK1)
	{
		return keys[b->key1].state;
	}
	else return joysticks[type-JOYSTICK1].buttons[b->key1].state;
}
int
InputDevice::Key2(int bind)
{
	KeyBinding *b = &bindings[bind];
	if (b->key2 < 0) return KS_NONE;
	if (type < JOYSTICK1)
		return keys[b->key2].state;
	else return joysticks[type-JOYSTICK1].buttons[b->key2].state;
}


