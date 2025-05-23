#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
class Shader
{
private:
public:
	GLuint ID;
	GLuint compileShader(const char* source, GLenum shaderType);
	GLuint createShaderProgram();
	void setMat4(const std::string& name, const glm::mat4& value) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
	}
	void setVec3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(ID, name.c_str());
		if (location == -1)
		{
			std::cerr << "Warning: uniform " << name << " not found!" << std::endl;
			return;
		}
		glUniform3fv(location, 1, &value[0]);
	}
	void use();
};

