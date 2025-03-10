#include "E57.h"
#include "Window.h"
#include "Shader.h"
#include "Camera.h"

e57::ustring path = "D:\\Bakalarka\\e57Files\\bunnyFloat.e57";
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f;

float lastX = 800 / 2.0f;
float lastY = 600 / 2.0f;
bool firstMouse = true;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
Window win(1080, 720);
int main() {
    E57 e57(path);
    myGuiImplementation gui(win.getWindow(), &e57);

    if (e57.getCount() <= 0)
    {
        return -1;
    }

    win.setPointCount(e57.getCount());
    win.LoadPointCloudToGPU(e57.getPoints());
    win.setCallBacks(mouse_callback, scroll_callback, mouse_button_callback);
    Shader shader;
    GLuint shaderProgram = shader.createShaderProgram();
    while (!glfwWindowShouldClose(win.getWindow()))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        shader.use();
        win.ProcessInput(deltaTime, camera);
        win.Render(shader, camera, gui);
    }
    glfwTerminate();

    return 0;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (win.dragging)
    {
        firstMouse = true;
        float xoffset = xposIn - win.lastX;
        float yoffset = yposIn - win.lastY;
        win.lastX = xposIn;
        win.lastY = yposIn;

        // Adjust rotation based on movement
        //win.modelRotationX += yoffset * 0.1f;  // Sensitivity factor
        //win.modelRotationY += xoffset * 0.1f;

        glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(yoffset * 0.1f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around screen X
        glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(xoffset * 0.1f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around screen Y

        //Apply rotation relative to current model transformation
        win.model = rotY * rotX * win.model;

    }
    else
    {

        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;

        lastX = xpos;
        lastY = ypos;

        camera.ProcessMouseMovement(xoffset, yoffset, false);
    }
    ImGui_ImplGlfw_CursorPosCallback(window, xposIn, yposIn);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            win.dragging = true;
            glfwGetCursorPos(window, &win.lastX, &win.lastY);
        }
        else if (action == GLFW_RELEASE)
        {
            win.dragging = false;
        }
    }
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

}