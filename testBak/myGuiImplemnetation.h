#pragma once

#include <GLFW/glfw3.h>
#include <iostream>

#include <fstream>
#include <vector>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "nfd.h"
#include "E57.h"

class myGuiImplementation
{
private:
public:
	E57* e;
	myGuiImplementation(GLFWwindow* window,E57* e57);
	int Render(float* rotations);
	int OpenFileDialog();
	~myGuiImplementation();
};

