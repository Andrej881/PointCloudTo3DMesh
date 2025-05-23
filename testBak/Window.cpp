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
    this->refresh = false;
    this->renderMesh = false;
    this->cPressed = false;
    this->cursorDisabled = true;
    this->width = width;
    this->height = height;
    this->pointSize = 1.0f;

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

void Window::LoadPointCloudToGPU(E57& e57) {

	std::vector<float> points = std::vector<float>();
    for (E57Point& point : e57.getPoints())
    {
		points.push_back(point.position.x);
		points.push_back(point.position.y);
		points.push_back(point.position.z);

		points.push_back(point.normal.x);
		points.push_back(point.normal.y);
		points.push_back(point.normal.z);
    }
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void Window::LoadMeshToGPU(AlgorithmControl& Algorithms)
{
    this->renderMesh = true;
	std::vector<Triangle>& triangles = Algorithms.GetTriangles();
	numOfTriangles = triangles.size();
    //printf("num of triangles: %d\n", triangles.size());
    std::vector<float> data;
    for (Triangle triangle : triangles)
    {
        data.push_back(triangle.a.position.x);
        data.push_back(triangle.a.position.y);
        data.push_back(triangle.a.position.z);

        data.push_back(triangle.normal.x);
        data.push_back(triangle.normal.y);
        data.push_back(triangle.normal.z);

        data.push_back(triangle.b.position.x);
        data.push_back(triangle.b.position.y);
        data.push_back(triangle.b.position.z);

        data.push_back(triangle.normal.x);
        data.push_back(triangle.normal.y);
        data.push_back(triangle.normal.z);

        data.push_back(triangle.c.position.x);
        data.push_back(triangle.c.position.y);
        data.push_back(triangle.c.position.z);

        data.push_back(triangle.normal.x);
        data.push_back(triangle.normal.y);
        data.push_back(triangle.normal.z);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Window::setCallBacks(GLFWcursorposfun mouse, GLFWscrollfun scroll)
{
    glfwSetCursorPosCallback(window, mouse);
    glfwSetScrollCallback(window, scroll);
}
void Window::Render(float deltaTime,Shader& ourShader, Camera& camera, MyGuiImplementation& gui, AlgorithmControl& algorithms)
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

    glm::vec3 lightPos(1.0f);
    ourShader.setVec3("lightPos", lightPos);

    // Light color (white light)
    glm::vec3 lightColor(1.0f);
    ourShader.setVec3("lightColor", lightColor);

    // Object color (can be any color you want)
    glm::vec3 objectColor(1.0f);
    ourShader.setVec3("objectColor", objectColor);

    glBindVertexArray(VAO);

    glPointSize(this->pointSize);

    if (this->renderMesh)
        glDrawArrays(GL_TRIANGLES, 0, numOfTriangles * 3 * 2);    
    else
        glDrawArrays(GL_POINTS, 0, pointCount*2);
    glBindVertexArray(0);
    
    AlgorithmsEnum algEnum = algorithms.GetActiveAlgorithm();
    bool running = algorithms.getRunning();
    float* args = new float[4];

    if (this->refresh)
    {
        this->timeToRefresh -= deltaTime;
        if (this->timeToRefresh <= 0)
        {
            this->timeToRefresh = 1 / this->refreshTimePerSec;
            if (this->renderMesh)
            {
                if (algorithms.GetTriangleMutex() != nullptr)
                {
                    std::unique_lock<std::mutex> lock(*algorithms.GetTriangleMutex());
                    this->LoadMeshToGPU(algorithms);
                    lock.unlock();
                }
            }
            else
            {
                this->LoadPointCloudToGPU(*gui.e);
            }
        }
        else
        {
            //printf("%f\n", this->timeToRefresh);
        }
    }

	int oldTime = this->refreshTimePerSec;
    auto result = gui.Render(rotations, !this->renderMesh, args, algEnum, running, &this->pointSize, this->refresh, this->refreshTimePerSec);
        
    switch (result)
    {
    case 0:
    {
        this->renderMesh = false;
        this->LoadPointCloudToGPU(*gui.e);
        this->setPointCount(gui.e->getCount());
        break;
    }
    case 1:
    {
        if (this->renderMesh)
        {
            this->renderMesh = false;
            this->LoadPointCloudToGPU(*gui.e);
        }
        else
        {
            this->renderMesh = true;
            this->LoadMeshToGPU(algorithms);
        }
        break;
    }
    case 2:
    {
        if (meshCalculating.joinable())
        {
            meshCalculating.join(); // Wait for the previous thread to finish
        }

		algorithms.ChangeAlgorithm(algEnum);
        algorithms.ChangeParams(args);
        algorithms.SetUp();
		meshCalculating = std::thread(&AlgorithmControl::Run, &algorithms);
        break;
    }
    case 3:
    {
        algorithms.Stop();
        printf("STOPPING\n");
        if (meshCalculating.joinable())
        {
            meshCalculating.join(); // Wait for the previous thread to finish
        }
        printf("STOPED\n");
        break;
    }
    }

    delete[] args;
    glfwSwapBuffers(window);
    glfwPollEvents();
}

Window::~Window()
{    
    if (meshCalculating.joinable())
    {
        meshCalculating.join(); 
    }
    glfwTerminate();
}

