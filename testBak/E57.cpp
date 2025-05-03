#include "E57.h"

void E57::OrientNormals(std::unordered_map<E57Point*, std::vector<KDTreeNode*>>& neighborsCache)
{
    std::queue<KDTreeNode*> queue;
    std::unordered_set<KDTreeNode*> visited;
    
    KDTreeNode* start = tree.GetRandomNode();
    if (!start) return;

    queue.push(start);
    visited.insert(start);

    while (!queue.empty() && !this->stopCalulatingNormals) {
        KDTreeNode* node = queue.front();
        queue.pop();

        // Získame susedov
        std::vector<KDTreeNode*> neighbors = neighborsCache[node->point];

        for (auto& neighbor : neighbors) {
            if (visited.find(neighbor) == visited.end()) {
                // Porovnanie normál - ak sú opaèné, otoèíme
                if (glm::dot(node->point->normal, neighbor->point->normal) < 0) {
                    neighbor->point->normal = -neighbor->point->normal;
                }
                queue.push(neighbor);
                visited.insert(neighbor);
            }
        }
    }
}

void E57::CalculateNormalsThread(std::unordered_map<E57Point*, std::vector<KDTreeNode*>>& neighborsCache, int startIndex, int endIndex, int numOfNeigbours, float radius)
{
    std::vector<std::pair<E57Point*, std::vector<KDTreeNode*>>> neighborsToCache;
    int size = endIndex - startIndex;
    for (int i = startIndex; i < endIndex; i++) {

        if(this->stopCalulatingNormals)
			return;

        if ((i - startIndex) % (size / 10) == 0)
            printf("Calculating normals on thread[%lu] %d%%\n", std::this_thread::get_id(), ((i - startIndex) * 100) / size);

        std::vector<KDTreeNode*> neighbors = std::vector<KDTreeNode*>();
        if(radius > 0)
            neighbors = tree.GetNeighborsWithinRadius(points[i].position, radius);
        if(numOfNeigbours > 0)
            neighbors = tree.GetKNearestNeighbors(points[i].position, numOfNeigbours);

        neighborsToCache.push_back({ &points[i], neighbors});

        if (neighbors.size() < 3)
            continue;

        glm::vec3 centroid(0.0f);

        for (auto neighbor : neighbors) {
            centroid += neighbor->point->position;
        }
        centroid /= static_cast<float>(neighbors.size());

        Eigen::Matrix3f covariance = Eigen::Matrix3f::Zero();
        for (auto& neighbor : neighbors) {
            glm::vec3 diff = neighbor->point->position - centroid;
            Eigen::Vector3f d(diff.x, diff.y, diff.z);
            covariance += d * d.transpose();
        }
        covariance /= neighbors.size();

        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(covariance);
        Eigen::Vector3f normalEigen = solver.eigenvectors().col(0);

        points[i].normal = glm::vec3(normalEigen.x(), normalEigen.y(), normalEigen.z());
        points[i].hasNormal = true;
    }

    for (const auto& entry : neighborsToCache) {

        std::unique_lock<std::mutex> lock(this->mutex);
        neighborsCache[entry.first] = entry.second;
        lock.unlock();
    }
}

void E57::SetUpTree()
{
	if (tree.GetRoot() != nullptr)
		tree.Clear();

    std::vector<E57Point*> pointPtrs;
    for (E57Point& point : this->points) {
        pointPtrs.push_back(&point); // Add pointer to point
    }

    tree.InsertPoints(pointPtrs);

	/*for (E57Point& point : this->points)
	{
		tree.Insert(&point);
	}*/
}

E57::E57()
{
    points = std::vector<E57Point>();
    count = 0;
	hasNormals = false;
}

E57::E57(e57::ustring path)
{   
    points = std::vector<E57Point>();
    count = 0;
    if (ReadFile(path, true) != 0)
    {
        std::cerr << "Could not read file" << std::endl;
    }
}

int E57::ReadFile(e57::ustring & path, bool allClounds)
{    
    try {
        if (tree.GetRoot() != nullptr)
            tree.Clear();
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

		numPointClouds = allClounds ? numPointClouds : 1; // Read only the first cloud if not all clouds are needed
        
        points = std::vector<E57Point>(); 
        for (int cloud = 0; cloud < numPointClouds; ++cloud)
        {
            reader.ReadData3D(cloud, data3DHeader);
            int pointCount = data3DHeader.pointCount;
            std::cout << "Point count: " << pointCount << std::endl;

            hasNormals = data3DHeader.pointFields.normalXField && data3DHeader.pointFields.normalYField && data3DHeader.pointFields.normalZField;

            // Allocate memory for points
            std::vector<float> XP(pointCount), YP(pointCount), ZP(pointCount);
            std::vector<float> NorX(pointCount), NorY(pointCount), NorZ(pointCount);
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
            count += vectorReader.read();
            vectorReader.close();


            for (int i = 0; i < pointCount; i++) {
                E57Point point = { glm::vec3(XP[i], YP[i], ZP[i]) };
                point.hasNormal = hasNormals;
                if (hasNormals)
                    point.normal = glm::vec3(NorX[i], NorY[i], NorZ[i]);
                points.push_back(point);
            }
            std::cout << "Successfully read " << pointCount << " points in " << cloud << "point cloud!" << std::endl;
        }
        
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

            if (hasNormals)
            {
                // Renormalizácia normály
                point.normal = glm::normalize(point.normal);
            }
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

void E57::CalculateNormals(float radius, int numOfNeigbors)
{
    this->calculating = true;
    if (hasNormals)
    {
        hasNormals = false;
    }
    std::unordered_map<E57Point*, std::vector<KDTreeNode*>> neighborsCache;
    if (tree.GetRoot() == nullptr)
        SetUpTree();
    KDTreeNode* seedPoint = this->tree.GetRoot();
    if (!seedPoint) {
        throw std::runtime_error("Point cloud is empty!");
    }


    int numThreads = std::thread::hardware_concurrency();
    int chunkSize = count / numThreads;

    std::vector<std::thread> threads;

    // Create threads to process the points in parallel
    for (int t = 0; t < numThreads; ++t)
    {
        int startIdx = t * chunkSize;
        int endIdx = (t == numThreads - 1) ? count : (startIdx + chunkSize);

        threads.push_back(std::thread(&E57::CalculateNormalsThread, this, std::ref(neighborsCache), startIdx, endIdx, numOfNeigbors, radius));
    }

    // Join all threads to ensure they complete before continuing
    for (auto& t : threads)
    {
        t.join();
    }

    printf("Orienting Normals\n");
    OrientNormals(neighborsCache);
    hasNormals = true;
    printf("Normals calculated\n");
    calculating = stopCalulatingNormals = false;
}

bool E57::GetHasNormals()
{
    return this->hasNormals;
}

bool E57::GetCalculating()
{
    return this->calculating;
}

void E57::StopCalculatingNormals()
{
    if(this->calculating)
        this->stopCalulatingNormals = false;
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
