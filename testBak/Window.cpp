#include "Window.h"

void Window::setPointCount(int count)
{
    this->pointCount = count;
}

Window::Window(unsigned int width, unsigned int height)
{
    this->isCPressed = false;
    this->isCursorDisabled = true;
    this->width = width;
    this->height = height;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);
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
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(ROTATE_LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(ROTATE_RIGHT, deltaTime);

    // Toggle cursor mode on/off with the 'C' key
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !isCPressed)
    {
        isCPressed = true;
        if (!isCursorDisabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide and lock the cursor
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Restore the cursor to normal mode
        }
        isCursorDisabled = !isCursorDisabled; // Toggle the state
    }

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
    {
        isCPressed = false;  // Allow the key to be pressed again in the future
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

        // Tell OpenGL how to interpret the data (3 floats per vertex)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    else {
        // If the VBO already exists, update it with the new points
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_STATIC_DRAW);

        // Tell OpenGL how to interpret the data (3 floats per vertex)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void Window::setCallBacks(GLFWcursorposfun mouse, GLFWscrollfun scroll, GLFWmousebuttonfun button)
{
    glfwSetCursorPosCallback(window, mouse);
    glfwSetScrollCallback(window, scroll);
    glfwSetMouseButtonCallback(window, button);
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

    glBindVertexArray(VAO);

    //glm::mat4 model = glm::mat4(1.0f); 
    //model = glm::rotate(model, glm::radians(modelRotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y
    //model = glm::rotate(model, glm::radians(modelRotationX), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X

    ourShader.setMat4("model", model);
    //glPointSize(3.0f);
    glDrawArrays(GL_POINTS, 0, pointCount);
    glBindVertexArray(0);

    if (gui.Render() == 0)
    {
        this->LoadPointCloudToGPU(gui.e->getPoints());
        this->setPointCount(gui.e->getCount());
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
}

Window::~Window()
{
    glfwTerminate();
}

