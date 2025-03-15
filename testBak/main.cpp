#include "E57.h"
#include "Window.h"
#include "Shader.h"
#include "Camera.h"

#include "MarginCubes.h"

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
    
    std::vector<float> points = std::vector<float>();

    /*for (float i = -0.5; i <= 0.5; i = i + 0.1)
    {
        for (float j = -0.5; j <= 0.5; j = j + 0.1)
        {
            for (float k = -0.5; k <= 0.5; k = k + 0.1)
            {
                points.push_back(i);
                points.push_back(j);
                points.push_back(k);
            }
        }
    }
    points.push_back(-0.5); points.push_back(-0.5); points.push_back(-0.5);

    MarginCubes cubes(1, 0, points);
    win.setPointCount(points.size() / 3);*/

    MarginCubes cubes(0.01,1,e57.getPoints());
    printf("Num of cubes: %d num of triangles: %d\n", cubes.getCubes().size(), cubes.numOfTriangels);

    if (e57.getCount() <= 0)
    {
        return -1;
    }

    win.setPointCount(e57.getCount());
    //win.LoadPointCloudToGPU(e57.getPoints());
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
