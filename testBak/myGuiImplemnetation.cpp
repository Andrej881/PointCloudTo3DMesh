
//https://github.com/ocornut/imgui/wiki/Getting-Started

#include "myGuiImplemnetation.h"
myGuiImplementation::myGuiImplementation(GLFWwindow* window, E57* e57)
{
    e = e57;
    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

}

int myGuiImplementation::Render(float* rotations)
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
    bool change = ImGui::Button("Cloud/Mesh");
    int min = -180;
    int max = 180;
    ImGui::SliderFloat("Rotation X", &rotations[0], min, max, "%.1f");
    ImGui::SliderFloat("Rotation Y", &rotations[1], min, max, "%.1f");
    ImGui::SliderFloat("Rotation Z", &rotations[2], min, max, "%.1f");
    ImGui::End();
    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (change)
        return -10;
    return 1;
}
//https://github.com/btzy/nativefiledialog-extended
int myGuiImplementation::OpenFileDialog()
{
    nfdchar_t* outPath = NULL;
    nfdresult_t result = NFD_OpenDialog("e57", NULL, &outPath);

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
