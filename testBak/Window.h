#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include "myGuiImplemnetation.h"

#include "Shader.h"
#include "Camera.h"


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
	bool isCursorDisabled, isCPressed;
public:
	bool dragging = false;
	double lastX, lastY;
	float modelRotationX = 0.0f; // Rotation around X-axis
	float modelRotationY = 0.0f; // Rotation around Y-axis
	glm::mat4 model = glm::mat4(1.0f);

	void setPointCount(int count);
	void ProcessInput(float deltaTime, Camera& camera);
	Window(unsigned int width, unsigned int height);
	void setCallBacks(GLFWcursorposfun mouse, GLFWscrollfun scroll, GLFWmousebuttonfun button);
	void Render(Shader& ourShader, Camera& camera, myGuiImplementation& gui);
	GLFWwindow* getWindow();
	void LoadPointCloudToGPU(const std::vector<float>& points);
	~Window();
};

