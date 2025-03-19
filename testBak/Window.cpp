#include "Window.h"

bool Window::isCursorDisabled()
{
    return cursorDisabled;
}

void Window::setPointCount(int count)
{
    this->pointCount = count;
}

Window::Window(unsigned int width, unsigned int height)
{
    this->cubes = nullptr;
    this->renderMesh = false;
    this->cPressed = false;
    this->cursorDisabled = true;
    this->width = width;
    this->height = height;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(width, height, "Bakalarka", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

}
void Window::ProcessInput(float deltaTime, Camera& camera)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    //cursor on/off
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !cPressed)
    {
        cPressed = true;
        if (!cursorDisabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); 
        }
        cursorDisabled = !cursorDisabled;
    }

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
    {
        cPressed = false;  
    }
}

GLFWwindow* Window::getWindow()
{
    return this->window;
}

void Window::LoadPointCloudToGPU(const std::vector<float>& points) {

    if (VBO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_STATIC_DRAW);
                
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    else {
        // If the VBO already exists, update it with the new points
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void Window::LoadMeshToGPUFromCubes(MarchingCubes& cubes)
{
    this->renderMesh = true;
    this->cubes = &cubes;

    std::vector<float> triangles = std::vector<float>();
    /*for (Cube cube : cubes.getCubes())
    {
        for (Side side : cube.sides)
        {
            if (!side.active)
                continue;
            for (Triangle triangle : side.triangles)
            {
                triangles.push_back(triangle.a.x);
                triangles.push_back(triangle.a.y);
                triangles.push_back(triangle.a.z);

                triangles.push_back(triangle.b.x);
                triangles.push_back(triangle.b.y);
                triangles.push_back(triangle.b.z);

                triangles.push_back(triangle.c.x);
                triangles.push_back(triangle.c.y);
                triangles.push_back(triangle.c.z);

                triangles.push_back(triangle.normal.x);
                triangles.push_back(triangle.normal.y);
                triangles.push_back(triangle.normal.z);

                triangles.push_back(triangle.normal.x);
                triangles.push_back(triangle.normal.y);
                triangles.push_back(triangle.normal.z);

                triangles.push_back(triangle.normal.x);
                triangles.push_back(triangle.normal.y);
                triangles.push_back(triangle.normal.z);
            }
            
        }

        
    }*/

    for (Triangle triangle : cubes.getTriangles())
    {
        triangles.push_back(triangle.a.x);
        triangles.push_back(triangle.a.y);
        triangles.push_back(triangle.a.z);

        triangles.push_back(triangle.b.x);
        triangles.push_back(triangle.b.y);
        triangles.push_back(triangle.b.z);

        triangles.push_back(triangle.c.x);
        triangles.push_back(triangle.c.y);
        triangles.push_back(triangle.c.z);

        triangles.push_back(triangle.normal.x);
        triangles.push_back(triangle.normal.y);
        triangles.push_back(triangle.normal.z);

        triangles.push_back(triangle.normal.x);
        triangles.push_back(triangle.normal.y);
        triangles.push_back(triangle.normal.z);

        triangles.push_back(triangle.normal.x);
        triangles.push_back(triangle.normal.y);
        triangles.push_back(triangle.normal.z);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, triangles.size() * sizeof(float), triangles.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}



void Window::setCallBacks(GLFWcursorposfun mouse, GLFWscrollfun scroll)
{
    glfwSetCursorPosCallback(window, mouse);
    glfwSetScrollCallback(window, scroll);
}

void Window::Render(Shader& ourShader, Camera& camera, myGuiImplementation& gui)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ourShader.use();

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)this->width / (float)this->height, 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);

    // camera/view transformation
    glm::mat4 view = camera.GetViewMatrix();
    ourShader.setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);

    model = glm::rotate(model, glm::radians(rotations[0]), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotations[1]), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotations[2]), glm::vec3(0.0f, 0.0f, 1.0f));
    ourShader.setMat4("model", model);

    glm::vec3 lightPos(1.0f, 1.0f, 1.0f);
    ourShader.setVec3("lightPos", lightPos);

    // Light color (white light)
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    ourShader.setVec3("lightColor", lightColor);

    // Object color (can be any color you want)
    glm::vec3 objectColor(1.0f, 1.0f, 1.0f);
    ourShader.setVec3("objectColor", objectColor);

    glBindVertexArray(VAO);

    //glPointSize(3.0f);
    if (this->renderMesh)
        glDrawArrays(GL_TRIANGLES, 0, cubes->getTriangles().size() * 3 * 2);    
    else
        glDrawArrays(GL_POINTS, 0, pointCount);
    glBindVertexArray(0);
    
    auto result = gui.Render(rotations);
    if (result == 0)
    {
        this->cubes->InitGrid(gui.e);
        this->cubes->SetGrid(gui.e->getPoints(), gui.e);
        if (this->renderMesh)
        {
            this->LoadMeshToGPUFromCubes(*this->cubes);
        }
        else
        {
            this->LoadPointCloudToGPU(gui.e->getPoints());
        }
        this->setPointCount(gui.e->getCount());
    }
    if (result == -10)
    {
        if (this->renderMesh)
        {
            this->renderMesh = false;
            this->LoadPointCloudToGPU(gui.e->getPoints());
        }
        else
        {
            this->LoadMeshToGPUFromCubes(*this->cubes);
        }
    }
    glfwSwapBuffers(window);
    glfwPollEvents();
}

Window::~Window()
{
    glfwTerminate();
}

