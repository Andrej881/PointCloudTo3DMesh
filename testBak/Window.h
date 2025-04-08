#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <thread>

#include "myGuiImplemnetation.h"

#include "Shader.h"
#include "Camera.h"

#include "AlgorithmControl.h"
#include <chrono>

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
class Window
{
private:
	GLFWwindow* window;
	GLuint VAO, VBO;
	int pointCount; //tmp
	int width, height;
	bool cursorDisabled, cPressed;
	float rotations[3];
	bool renderMesh;
	bool refresh;
	float pointSize;

	std::thread meshCalculating;

	std::chrono::steady_clock::time_point lastRefreshTime;
	const std::chrono::milliseconds refreshInterval{ 1000 };
public:

	bool isCursorDisabled();

	void setPointCount(int count);
	void ProcessInput(float deltaTime, Camera& camera);
	Window(unsigned int width, unsigned int height);
	void setCallBacks(GLFWcursorposfun mouse, GLFWscrollfun scroll);
	void Render(Shader& ourShader, Camera& camera, myGuiImplementation& gui, AlgorithmControl& algorithmsEnum);
	GLFWwindow* getWindow();
	void LoadPointCloudToGPU(E57 & e57);
	void LoadMeshToGPU(AlgorithmControl& algorithmsEnum);

	~Window();
};

