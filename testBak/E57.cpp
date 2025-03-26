#include "E57.h"

void E57::SetUpTree()
{
    tree.Clear();
    for (E57Point& point : this->points)
    {
        tree.Insert(&point);
    }
}

E57::E57(e57::ustring path)
{   
    points = std::vector<E57Point>();
    count = 0;
    if (ReadFile(path) != 0)
    {
        std::cerr << "Could not read file" << std::endl;
    }
}

int E57::ReadFile(e57::ustring & path)
{    
    try {
        // Create ReaderOptions (use default options for now)
        e57::ReaderOptions options;

        // Pass the filename and options to the Reader constructor
        e57::Reader reader(path, options);

        std::cout << "E57 file successfully opened!" << std::endl;

        if (count > 0)
        {
            count = 0;
            points.clear();        
        }

        // Get the number of point clouds in the file
        size_t numPointClouds = reader.GetData3DCount();
        std::cout << "Number of point clouds: " << numPointClouds << std::endl;

        if (numPointClouds == 0) {
            std::cerr << "No point clouds found in the file!" << std::endl;
            return -1;
        }

        // Get the point count of the first cloud
        e57::Data3D data3DHeader;
        reader.ReadData3D(0, data3DHeader);
        int pointCount = data3DHeader.pointCount;
        std::cout << "Point count: " << pointCount << std::endl;

        hasNormals = data3DHeader.pointFields.normalXField && data3DHeader.pointFields.normalYField && data3DHeader.pointFields.normalZField;

        // Allocate memory for points
        points = std::vector<E57Point>(); // X, Y, Z interleaved
        std::vector<float> XP(pointCount), YP(pointCount), ZP(pointCount);
        std::vector<float> NorX, NorY, NorZ;
        // Prepare buffers for the reader
        e57::Data3DPointsFloat buffer;
        buffer.cartesianX = XP.data();
        buffer.cartesianY = YP.data();
        buffer.cartesianZ = ZP.data();

        if (hasNormals)
        {
            buffer.normalX = NorX.data();
            buffer.normalY = NorY.data();
            buffer.normalZ = NorZ.data();        
        }

        // Read points from the first cloud
        e57::CompressedVectorReader vectorReader = reader.SetUpData3DPointsData(0, static_cast<int64_t>(data3DHeader.pointCount), buffer);

        // Read all points
        count = vectorReader.read();
        vectorReader.close();


        for (int i = 0; i < count; i++) {
            E57Point point = { glm::vec3(XP[i], YP[i], ZP[i])};
            point.hasNormal = hasNormals;
            if (hasNormals)
                point.normal = glm::vec3(NorX[i], NorY[i], NorZ[i]);   
			points.push_back(point);
        }



        std::cout << "Successfully read " << count << " points!" << std::endl;
        //NORMILIZE
        float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
        float maxX = FLT_MIN, maxY = FLT_MIN, maxZ = FLT_MIN;

        for (E57Point& point : points) {
            minX = std::min(minX, point.position.x);
            minY = std::min(minY, point.position.y);
            minZ = std::min(minZ, point.position.z);
            maxX = std::max(maxX, point.position.x);
            maxY = std::max(maxY, point.position.y);
            maxZ = std::max(maxZ, point.position.z);
        }

        float centerX = (minX + maxX) / 2.0f;
        float centerY = (minY + maxY) / 2.0f;
        float centerZ = (minZ + maxZ) / 2.0f;
        float maxDim = std::max({ maxX - minX, maxY - minY, maxZ - minZ });
        
        info.minX = (minX - centerX) / maxDim;
        info.maxX = (maxX - centerX) / maxDim;
        info.minY = (minY - centerY) / maxDim;
        info.maxY = (maxY - centerY) / maxDim;
        info.minZ = (minZ - centerZ) / maxDim;
        info.maxZ = (maxZ - centerZ) / maxDim;
        
        printf("minX[%f], maxX[%f], minY[%f], maxY[%f], minZ[%f], maxZ[%f]\n", info.minX, info.maxX, info.minY, info.maxY, info.minZ, info.maxZ);
        for (E57Point& point : points) {
            point.position.x = (point.position.x - centerX) / maxDim;
            point.position.y = (point.position.y - centerY) / maxDim;
            point.position.z = (point.position.z - centerZ) / maxDim;
        }

    }
    catch (const e57::E57Exception& e) {
        std::cerr << "E57 error: " << e.what() << std::endl;
        std::cerr << "E57 errorCode: " << e.errorCode() << std::endl;

        return -1;
    }

    return 0;
}

std::vector<E57Point>& E57::getPoints()
{
    return this->points;
}

KDTree& E57::getTree()
{
    return this->tree;
}

void E57::CalculateNormals()
{
    float radius = 0.2f;

    if (hasNormals)
    {
        return;
    }

	SetUpTree();
    KDTreeNode* seedPoint = this->tree.GetRoot();
    if (!seedPoint) {
        throw std::runtime_error("Point cloud is empty!");
    }

    std::vector<KDTreeNode*> neighbors = tree.GetNeighborsWithinRadius(seedPoint, radius);  

	hasNormals = true;

    for (int i = 0; i < count; i++) {
        KDTreeNode* seedPoint = tree.FindNode(&this->points[i]);
        if (seedPoint == nullptr)
            continue;

        std::vector<KDTreeNode*> neighbors = tree.GetNeighborsWithinRadius(seedPoint, radius);
        if (neighbors.size() < 3)
            continue;

        glm::vec3 centroid(0.0f);
        int neighborCount = 0;

        for (auto neighbor : neighbors) {
            centroid += neighbor->point->position;
        }
        centroid /= static_cast<float>(neighbors.size());

         glm::mat3 covariance = glm::mat3(0.0f);
        for (auto neighbor : neighbors) {
            glm::vec3 diff = neighbor->point->position - centroid;
            covariance += glm::outerProduct(diff, diff);
        }
        covariance /= static_cast<float>(neighbors.size());
       
        Eigen::Matrix3f eigenCovariance;
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                eigenCovariance(row, col) = covariance[col][row]; // glm is column-major
            }
        }

        // Compute eigenvalues & eigenvectors
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(eigenCovariance);
        Eigen::Vector3f normalEigen = solver.eigenvectors().col(0); // Smallest eigenvector

        glm::vec3 normal(normalEigen.x(), normalEigen.y(), normalEigen.z());
        normal = -glm::normalize(normal);        

        glm::vec3 avgNeighborNormal(0.0f);
        int validNormals = 0;

        for (auto neighbor : neighbors) {
            if (neighbor->point->hasNormal) {
                avgNeighborNormal += neighbor->point->normal;
                validNormals++;
            }
        }

        if (validNormals > 0) {
            avgNeighborNormal = glm::normalize(avgNeighborNormal);
            if (glm::dot(normal, avgNeighborNormal) < 0) {
                normal = -normal;  
            }
        }

        points[i].normal = normal;
        points[i].hasNormal = true;
    }
}

bool E57::GetHasNormals()
{
    return this->hasNormals;
}

int E57::getCount()
{
    return this->count;
}

NormilizedPointsInfo& E57::getInfo()
{
    return this->info;
}

E57::~E57()
{
}
