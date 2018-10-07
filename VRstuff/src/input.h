#ifndef PAKKI_INPUTS
#define PAKKI_INPUTS
#include <Utils.h>
#include <MathUtil.h>
#include <glwf3/glfw3.h>
#include "file_system.h"
namespace INPUTS
{
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
	static Input* g_inputs = 0;
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
	void set_key_down(int key)
	{
		int realKey = key - GLFW_KEY_A + 1;
		if(BETWEEN(0,realKey,30))
		{
			BIT_SET(g_inputs->keys, (1 << (realKey - 1)));
		}
	}
	void set_key_up(int key)
	{
		int realKey = key - GLFW_KEY_A + 1;
		if (BETWEEN(0, realKey, 30))
		{
			BIT_UNSET(g_inputs->keys, (1 << (realKey - 1)));
		}
	}
	void key_callback(GLFWwindow*, int key, int , int action, int )
	{
		if(action == GLFW_PRESS)
			set_key_up(key);	
		else if(action == GLFW_RELEASE)
			set_key_up(key);
	}

	void mouse_callback(GLFWwindow* , int button, int action, int )
	{
		if(button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if(action == GLFW_PRESS)
				g_inputs->mousebuttons[0] = true;
			else if(action == GLFW_RELEASE)
				g_inputs->mousebuttons[0] = false;
		}
		else if(button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			if(action == GLFW_PRESS)
				g_inputs->mousebuttons[1] = true;
			else if(action == GLFW_RELEASE)
				g_inputs->mousebuttons[1] = false;
		}
	}
	void cursor_position_callback(GLFWwindow* ,double xpos , double ypos)
	{
		g_inputs->mpos = MATH::vec2((float)xpos,(float)ypos);	
	}
	void init_inputs(Input* in) 
	{
		g_inputs = in;
		in->keys = 0;
		in->lastkeys = 0;
		in->joyStick.keys = 0;
		in->joyStick.lastKeys = 0;
		in->lastmpos = MATH::vec2(0,0);
		in->mpos = MATH::vec2(0,0);
		in->lastmousebuttons[0] = false;
		in->lastmousebuttons[1] = false;
		in->mousebuttons[0] = false;
		in->mousebuttons[1] = false;
	}
	void update_keys()
	{
		g_inputs->lastkeys = g_inputs->keys;
		g_inputs->lastmpos = g_inputs->mpos;
		g_inputs->joyStick.lastKeys = g_inputs->joyStick.keys;
		g_inputs->joyStick.keys = 0;

		int joyStickPresent = glfwJoystickPresent(GLFW_JOYSTICK_1);
		if(joyStickPresent)
		{
			if(!g_inputs->joyStick.active)
			{
				const char* name = glfwGetJoystickName(GLFW_JOYSTICK_1);
				LOG("Connected controller :: %s \n",name);
				g_inputs->joyStick.active = true;
			}
			int buttonCount = 0;
			const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1,&buttonCount);
			//printf("%d \n",buttonCount);
			if(buttons[0] == GLFW_PRESS){
				BIT_SET(g_inputs->joyStick.keys,JoyKey::KEY_ARROW_LEFT);
			}
			if(buttons[1] == GLFW_PRESS){
				BIT_SET(g_inputs->joyStick.keys,JoyKey::KEY_ARROW_DOWN);
			}
			if(buttons[2] == GLFW_PRESS){
				BIT_SET(g_inputs->joyStick.keys,JoyKey::KEY_ARROW_UP);
			}
			if(buttons[3] == GLFW_PRESS){
				BIT_SET(g_inputs->joyStick.keys,JoyKey::KEY_ARROW_RIGHT);
			}
			if(buttons[4] == GLFW_PRESS){
				BIT_SET(g_inputs->joyStick.keys,JoyKey::KEY_TRIANGLE);
			}
			if(buttons[5] == GLFW_PRESS){
				BIT_SET(g_inputs->joyStick.keys,JoyKey::KEY_SQUARE);
			}
			if(buttons[6] == GLFW_PRESS){
				BIT_SET(g_inputs->joyStick.keys,JoyKey::KEY_X);
			}
			if(buttons[7] == GLFW_PRESS){
				BIT_SET(g_inputs->joyStick.keys,JoyKey::KEY_CIRCLE);
			}
#if 0
			for(unsigned char* i = (unsigned char*)buttons; i < buttons + buttonCount ; i++)
			{
				if(GLFW_PRESS == *i)
				{
					printf("%d \n",(int)(i - buttons));
				}
			}
#endif

		}
		else if(g_inputs->joyStick.active){
			LOG("JoyStick Disconnected \n");
			g_inputs->joyStick.active = false;
		}

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
}
#endif
