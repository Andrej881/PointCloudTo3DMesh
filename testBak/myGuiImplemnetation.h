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

#include "AlgorithmControl.h"

class MyGuiImplementation
{
private:
	void EndRender();
	std::thread normalCalculating;
	int OpenFileDialog(bool allPoints);
public:
	E57* e;
	MyGuiImplementation(GLFWwindow* window,E57* e57);
	int Render(float* rotations, bool cloud, float*& meshArgs, AlgorithmsEnum& mesh, bool running, float* pointSize, bool& refresh, int& refreshTimes);
	~MyGuiImplementation();
};

