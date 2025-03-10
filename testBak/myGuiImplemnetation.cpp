#include "myGuiImplemnetation.h"

myGuiImplementation::myGuiImplementation(GLFWwindow* window, E57* e57)
{
    e = e57;
    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();  // Set ImGui style (optional)

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

}

int myGuiImplementation::Render()
{
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // UI Window
    ImGui::Begin("Options");
    if (ImGui::Button("Load File"))
    {
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        return OpenFileDialog();
    }
    ImGui::End();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return 1;
}

int myGuiImplementation::OpenFileDialog()
{
    nfdchar_t* outPath = NULL;
    nfdresult_t result = NFD_OpenDialog("", NULL, &outPath);

    if (result == NFD_OKAY)
    {
        std::string filePath(outPath);
        free(outPath);
        return e->ReadFile(filePath);
    }
    else if (result == NFD_CANCEL)
    {
        std::cout << "User canceled file selection." << std::endl;
        return -1;
    }
    else
    {
        std::cerr << "Error opening file." << std::endl;
        return -2;
    }
}

myGuiImplementation::~myGuiImplementation()
{
}
