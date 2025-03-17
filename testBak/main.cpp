#include "E57.h"
#include "Window.h"
#include "Shader.h"
#include "Camera.h"

#include "MarchingCubes.h"
#include "Cubes.h"

e57::ustring path = "D:\\Bakalarka\\e57Files\\bunnyFloat.e57";
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

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

    MarchingCubes cubes(0.01,5,e57);
    //Cubes cubes(0.01, 0, e57);
    //printf("Num of cubes: %d num of triangles: %d\n", cubes.getCubes().size(), cubes.numOfTriangels);
    if (e57.getCount() <= 0)
    {
        return -1;
    }

    win.setPointCount(e57.getCount());
    win.LoadMeshToGPUFromCubes(cubes);
    win.setCallBacks(mouse_callback, scroll_callback);
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
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (!win.isCursorDisabled())
        firstMouse = true;

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
    ImGui_ImplGlfw_CursorPosCallback(window, xposIn, yposIn);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

}
