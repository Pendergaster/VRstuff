#ifndef PAKKI_INPUTS
#define PAKKI_INPUTS
#include <Utils.h>
#include <MathUtil.h>
#include <glwf3/glfw3.h>
#include "file_system.h"
#include <sharedinputs.h>
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
#if 0
void key_callback(GLFWwindow*, int key, int , int action, int )
{
	if(action == GLFW_PRESS)
		set_key_up(key);	
	else if(action == GLFW_RELEASE)
		set_key_up(key);
}
#endif

void key_callback(GLFWwindow* , int key, int , int action, int )
{
	if (action == GLFW_PRESS)
	{
		set_key_down(key);
	}
	else if (action == GLFW_RELEASE)
	{
		set_key_up(key);
	}
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
#endif
