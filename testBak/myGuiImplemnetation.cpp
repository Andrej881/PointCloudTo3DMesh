
//https://github.com/ocornut/imgui/wiki/Getting-Started

#include "myGuiImplemnetation.h"
void MyGuiImplementation::EndRender()
{
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
MyGuiImplementation::MyGuiImplementation(GLFWwindow* window, E57* e57)
{
    e = e57;
    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

}

int MyGuiImplementation::Render(float* rotations, bool cloud, float*& meshArgs, AlgorithmsEnum& mesh, bool running, float* pointSize, bool& refresh, int& refreshTimes)
{
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // UI Window
    ImGui::Begin("Options", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // Load File    
    static bool allPoints = false;
    if (ImGui::Button("Load File"))
    {
        EndRender();
        return OpenFileDialog(allPoints);
    }
    ImGui::SameLine();
    ImGui::Checkbox("Load All Points", &allPoints);

	//Point Size
    ImGui::Separator();
    int mi = 1;
    int ma = 10;
    ImGui::SliderFloat("Point Size", pointSize, mi, ma, "%.1f");

    // Cloud/Mesh
	ImGui::Separator();
    const char* cloudMesh = cloud ? "Show Mesh" : "Show Point Cloud";
    if (ImGui::Button(cloudMesh))
    {
        EndRender();
        return 1;
    }

    // Rotation
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Rotation")) 
    {
        int min = -180;
        int max = 180;
        ImGui::SliderFloat3("Rotation", rotations, min, max, "%.1f");
    }

    // Normals
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Normals")) 
    {
        if (e->GetHasNormals())
		{
			ImGui::Text("Normals are already calculated");
			ImGui::Text("You can recalculate them");
		}
		else
		{
			ImGui::Text("Normals are not calculated");
        }
        static int numNeighbours = 0;
        ImGui::InputInt("Neighbours", &numNeighbours);

        static float normalRadius = 0.03f;
        ImGui::InputFloat("Radius", &normalRadius,0, 0, "%.6f");

        bool canCalcNormals = (numNeighbours > 0 || normalRadius > 0) && (numNeighbours * normalRadius) <= 0;
        if (!canCalcNormals) {
            ImGui::BeginDisabled();
        }
        if (e->GetCalculating())
        {
            if (ImGui::Button("Stop Calculating Normals")) {

                printf("Stopping\n");
                e->StopCalculatingNormals();
                if (normalCalculating.joinable())
                {
                    normalCalculating.join();
                }
                printf("Stoped\n");

            }
        }
        else
        {
            if (ImGui::Button("Calculate Normals")) {

                if (normalCalculating.joinable())
                {
                    normalCalculating.join(); 
                }

                normalCalculating = std::thread(&E57::CalculateNormals, e, normalRadius, numNeighbours);

            }
        }
        
        if (!canCalcNormals) {
            ImGui::EndDisabled();
        }
    }

    // Mesh
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Mesh")) {
		static bool stop = false;
        if (ImGui::TreeNode("Cubes")) {
            static int cubeMargin = 0;
            ImGui::InputInt("Cube Margin", &cubeMargin);

            static float voxelSize = 0.01f;
            ImGui::InputFloat("Voxel Size", &voxelSize, 0, 0, "%.6f");

            if (!running)
            {
                if (ImGui::Button( "Start Cubes")) {
                    mesh = CUBES;
                    meshArgs[0] = (float)cubeMargin;
                    meshArgs[1] = voxelSize;

                    ImGui::TreePop();
                    EndRender();
                    return 2;
                }
			}
			else
			{
				if (ImGui::Button("Stop Cubes")) {
                    stop = true;
				}
			}            

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Marching Cubes")) {
            static float isoLevel = 0.02f;
            ImGui::InputFloat("IsoLevel", &isoLevel, 0, 0, "%.6f");

            static float mcVoxelSize = 0.01f;
            ImGui::InputFloat("MC VoxelSize", &mcVoxelSize, 0, 0, "%.6f");

            static float mcSigmaMultiplier = 0.75f;
            ImGui::InputFloat("MC Sigma Multiplier", &mcSigmaMultiplier, 0, 0, "%.6f");

            static int mcMargin = 1;
            ImGui::InputInt("MC Margin", &mcMargin);

            if (!running)
            {
                if (ImGui::Button("Start MC")) {
                    mesh = MARCHING_CUBES;
                    meshArgs[0] = isoLevel;
                    meshArgs[1] = mcVoxelSize;
                    meshArgs[2] = mcSigmaMultiplier;
                    meshArgs[3] = (float)mcMargin;

                    ImGui::TreePop();
                    EndRender();
                    return 2;
                }
            }
            else
            {
                if (ImGui::Button("Stop MC")) {
                    stop = true;
                }
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Ball Pivoting")) {
            static float bpRadius = 0.015f;
            ImGui::InputFloat("BP Radius", &bpRadius, 0, 0, "%.6f");

            if (!running)
            {
                if (ImGui::Button("Start BP")) {
                    mesh = BALL_PIVOTING;
                    meshArgs[0] = bpRadius;

                    ImGui::TreePop();
                    EndRender();
                    return 2;
                }
            }
            else
            {
                if (ImGui::Button("Stop BP")) {
                    stop = true;
                }
            }

            ImGui::TreePop();
        }

        // Global Stop
        ImGui::Separator();
        if (!running) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Stop")) {
			stop = true;
        }
        if (!running) {
            ImGui::EndDisabled();
        }

        if (stop)
        {
            stop = !stop;
            EndRender();
            return 3;
        }
    }
    // Refresh
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Refresh"))
    {
        int minR = 1;
        int maxR = 60;
        ImGui::SliderInt("Refresh per sec", &refreshTimes, minR, maxR);
        if (ImGui::Button(refresh ? "Off" : "On"))
        {
            refresh = !refresh;
        }
    }

    //Point Size
    ImGui::Separator();


    EndRender();
    return -10;
}

//https://github.com/btzy/nativefiledialog-extended
int MyGuiImplementation::OpenFileDialog(bool allPoints)
{
    nfdchar_t* outPath = NULL;
    nfdresult_t result = NFD_OpenDialog("e57", NULL, &outPath);

    if (result == NFD_OKAY)
    {
        std::string filePath(outPath);
        free(outPath);
        return e->ReadFile(filePath, allPoints);
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

MyGuiImplementation::~MyGuiImplementation()
{

    e->StopCalculatingNormals();
    if (normalCalculating.joinable())
    {
        normalCalculating.join();
    }
}
