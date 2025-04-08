
//https://github.com/ocornut/imgui/wiki/Getting-Started

#include "myGuiImplemnetation.h"
void myGuiImplementation::EndRender()
{
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
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

int myGuiImplementation::Render(float* rotations, bool cloud, float*& meshArgs, algorithmsEnum& mesh, bool running, float* pointSize)
{
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // UI Window
    ImGui::Begin("Options", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // Load File
    if (ImGui::Button("Load File"))
    {
        EndRender();
        return OpenFileDialog();//0
    }

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
        ImGui::InputFloat("Radius", &normalRadius);

        bool canCalcNormals = (numNeighbours > 0 || normalRadius > 0) && (numNeighbours * normalRadius) <= 0;
        if (!canCalcNormals) {
            ImGui::BeginDisabled();
        }
        if (e->GetCalculating())
        {
            if (ImGui::Button("Stop")) {

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
            ImGui::InputFloat("Voxel Size", &voxelSize);

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
            ImGui::InputFloat("IsoLevel", &isoLevel);

            static float mcVoxelSize = 0.01f;
            ImGui::InputFloat("MC VoxelSize", &mcVoxelSize);

            if (!running)
            {
                if (ImGui::Button("Start MC")) {
                    mesh = MARCHING_CUBES;
                    meshArgs[0] = isoLevel;
                    meshArgs[1] = mcVoxelSize;


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
            static float bpRadius = 1.0f;
            ImGui::InputFloat("BP Radius", &bpRadius);

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

    EndRender();
    return -10;
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

    e->StopCalculatingNormals();
    if (normalCalculating.joinable())
    {
        normalCalculating.join();
    }
}
