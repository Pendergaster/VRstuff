#ifndef PAKKI_SHARED_INPUTS
#define PAKKI_SHARED_INPUTS
#include <Utils.h>
#include <MathUtil.h>
enum class Key : unsigned int
{
	KEY_A = 1 << 0/*= GLFW_KEY_A*/,
	KEY_B = 1 << 1,
	KEY_C = 1 << 2,
	KEY_D = 1 << 3,
	KEY_E = 1 << 4,
	KEY_F = 1 << 5,
	KEY_G = 1 << 6,
	KEY_H = 1 << 7,
	KEY_I = 1 << 8,
	KEY_J = 1 << 9,
	KEY_K = 1 << 10,
	KEY_L = 1 << 11,
	KEY_M = 1 << 12,
	KEY_N = 1 << 13,
	KEY_O = 1 << 14,
	KEY_P = 1 << 15,
	KEY_Q = 1 << 16,
	KEY_R = 1 << 17,
	KEY_S = 1 << 18,
	KEY_T = 1 << 19,
	KEY_U = 1 << 20,
	KEY_V = 1 << 21,
	KEY_W = 1 << 22,
	KEY_X = 1 << 23,
	KEY_Y = 1 << 24,
	KEY_MAX = 1 << 30,
	//max_keys
};
enum JoyKey : unsigned int
{
	KEY_X = 1 << 1,
	KEY_TRIANGLE = 1 << 2,
	KEY_CIRCLE = 1 << 3,
	KEY_SQUARE = 1 << 4,
	KEY_ARROW_UP = 1 << 5,
	KEY_ARROW_DOWN = 1 << 6,
	KEY_ARROW_LEFT = 1 << 7,
	KEY_ARROW_RIGHT = 1 << 8,
	//max_keys
};

struct Input;
static Input* g_inputs = NULL;
struct JoyStick
{
	bool active = 0;
	uint keys = 0;
	uint lastKeys = 0;
};
struct Input
{
	uint		keys;
	uint		lastkeys;
	MATH::vec2	mpos;
	MATH::vec2	lastmpos;
	bool		mousebuttons[2];
	bool		lastmousebuttons[2];
	bool		inputDisabled;
	JoyStick	joyStick;
};

static inline void set_joy_button_down(JoyKey key)
{
	BIT_SET(g_inputs->joyStick.keys, key);
}
bool key_pressed(Key key)
{
	return	BIT_CHECK(g_inputs->keys, (uint)key) && !(BIT_CHECK(g_inputs->lastkeys, (uint)key));	
}
bool joy_key_pressed(JoyKey key)
{
	return	BIT_CHECK(g_inputs->joyStick.keys, (uint)key) && 
		!(BIT_CHECK(g_inputs->joyStick.lastKeys, (uint)key));	
}
bool joy_key_down(JoyKey key)
{
	return BIT_CHECK(g_inputs->joyStick.keys,(uint)key);
}
bool key_down(Key key)
{
	return BIT_CHECK(g_inputs->keys,(uint)key);
}
static void set_input_context(Input* inputs)
{
	ASSERT_MESSAGE(!g_inputs,"Inputs already set!");
	g_inputs = inputs;
}

static MATH::vec2 get_mouse_position()
{
	return g_inputs->mpos;
}

static MATH::vec2 get_last_mouse_position()
{
	return g_inputs->lastmpos;
}

static MATH::vec2 get_mouse_movement()
{
	return g_inputs->mpos - g_inputs->lastmpos;
}
#endif
