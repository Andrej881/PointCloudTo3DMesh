#include "Poisson.h"

void Poisson::FillTree()
{
    for (E57Point& p : e57->getPoints())
    {
        tree.Insert(&p);
    }
}

void Poisson::SampleOctreeToGrid(OctreeNode* node, int depth)
{
    float minX = e57->getInfo().minX;
    float minY = e57->getInfo().minY;
    float minZ = e57->getInfo().minZ;

    float maxX = e57->getInfo().maxX;
    float maxY = e57->getInfo().maxY;
    float maxZ = e57->getInfo().maxZ;

    if (!node) return;

    if (node->points.size() > 0)
    //if(true)
    {
        /*for (auto& point : node->points) {
            float x = point->position.x, y = point->position.y, z = point->position.z;

            int indexX = (x - minX) / voxelSize;
            int indexY = (y - minY) / voxelSize;
            int indexZ = (z - minZ) / voxelSize;
            
            for (int dx = 0; dx <= 1; dx++)
                for (int dy = 0; dy <= 1; dy++)
                    for (int dz = 0; dz <= 1; dz++)
                    {
                        int ix = indexX + dx;
                        int iy = indexY + dy;
                        int iz = indexZ + dz;

                        float density = node->chi;

                        grid[ix][iy][iz] = density;
                        /*if (FLT_MIN == grid[ix][iy][iz])
                            grid[ix][iy][iz] = density;
                        else
                            grid[ix][iy][iz] += density;
                    }
            //grid[indexX][indexY][indexZ] = node->chi;
        }*/
        glm::vec3 min = glm::vec3(node->center.position - (node->size / 2));
        glm::vec3 max = glm::vec3(node->center.position + (node->size / 2));

        min.x = minX > min.x ? minX : min.x;
        min.y = minY > min.y ? minY : min.y;
        min.z = minZ > min.z ? minZ : min.z;

        max.x = maxX < max.x ? maxX : max.x;
        max.y = maxY < max.y ? maxY : max.y;
        max.z = maxZ < max.z ? maxZ : max.z;

        min = glm::vec3(min.x - minX, min.y - minY, min.z - minZ) / voxelSize;
        max = glm::vec3(max.x - minX, max.y - minY, max.z - minZ) / voxelSize;

        // Naplnenie gridu hodnotami chi
        for (int x = (int)min.x; x <= (int)max.x; ++x) {
            for (int y = (int)min.y; y <= (int)max.y; ++y) {
                for (int z = (int)min.z; z <= (int)max.z; ++z) {
                    this->grid[x][y][z] = node->chi;     
                }
            }
        }
    }

    // Rekurzívne spracovanie poduzlov
    for (int i = 0; i < 8; ++i) {
        if (node->children[i]) {
            SampleOctreeToGrid(node->children[i],depth+1);
        }
    }
}

float Poisson::GetDensity(OctreeNode* node, glm::vec3 position)
{
    if (!node)
        return 0.0f;

    if (node->points.size() > 0)
    {
        return node->chi;
    }
    int index = (position.x > node->center.position.x) 
        | ((position.y > node->center.position.y) << 1) 
        | ((position.z > node->center.position.z) << 2);
    if (node->children[index] != nullptr) {
        return GetDensity(node->children[index], position);
    }
    return 0.0f;
    
}

void Poisson::SetGrid()
{
    SampleOctreeToGrid(tree.GetRoot(),0);
}

void Poisson::IterateGaussSeidel(OctreeNode* node)
{
    if (!node || node->points.size() > 0) {
        return;
    }

    // Aktualizácia hodnôt vo vnútri uzla
    for (int i = 0; i < 8; ++i) {
        if (node->children[i]) {
            IterateGaussSeidel(node->children[i]);
        }
    }

    // Aktualizácia hodnoty funkcie chi na základe okolitých uzlov
    float sumChi = 0.0f;
    int neighborCount = 0;

    for (int i = 0; i < 8; ++i) {
        if (node->children[i]) {
            sumChi += node->children[i]->chi;
            neighborCount++;
        }
    }

    if (neighborCount > 0) {
        node->chi = (sumChi - node->divergence) / neighborCount;
    }
}

float Poisson::CalculateDensity(glm::vec3 point, glm::vec3 min, glm::vec3 index)
{
    int i = index.x;
    int j = index.y;
    int k = index.z;
    if (i < 0 || i >= GetVoxelsInDimX() || j < 0 || j >= GetVoxelsInDimY() || k < 0 || k >= GetVoxelsInDimZ()) {
        return 0.0;
    }
    float density = GetDensity(tree.GetRoot(), point);
    return density;
}

Poisson::Poisson(E57* e57) : MarchingCubes(0.1f, 0.01f,1,0.75, e57), tree({ glm::vec3(0) }, 1, 10)
{
    this->n = sqrt(e57->getCount());
    this->numOfIter = 500;

    this->scalarField = std::vector<std::vector<std::vector<float>>>();
}

Poisson::Poisson(E57* e57, int n, int numOfIter, int depth, E57Point center, float size, float voxelSize, int margin, float isolevel) : MarchingCubes(isolevel, voxelSize, margin, 0.75, e57), tree(center, size, depth)
{
	this->n = n;
	this->numOfIter = numOfIter;
    this->scalarField = std::vector<std::vector<std::vector<float>>>();
}

std::vector<std::vector<std::vector<float>>>& Poisson::GetField()
{
    return this->scalarField;
}

void Poisson::SetUp()
{
    if (tree.GetRoot() != nullptr)
    {
        float size = tree.GetRoot()->size;
        tree.Clear(tree.GetRoot());
        tree.GetRoot() = new OctreeNode({ glm::vec3(0) }, size);
    }
    if (!e57->GetHasNormals())
        e57->CalculateNormals(0.03,0);
    InitGrid();
    this->scalarField.clear();
    FillTree();
}

void Poisson::Run()
{
    tree.GetRoot()->computeDivergence();
    for (int iter = 0; iter < numOfIter; ++iter)
    {
        IterateGaussSeidel(tree.GetRoot());
    }
    SetGrid();
    this->GetTriangles().clear();
    GenerateMesh();
}