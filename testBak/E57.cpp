#include "E57.h"

E57::E57(e57::ustring path)
{
    points = std::vector<float>();
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
        points = std::vector<float>(); // X, Y, Z interleaved
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
            points.push_back(XP[i]);
            points.push_back(YP[i]);
            points.push_back(ZP[i]);
            if (hasNormals)
            {
                normals.push_back(NorX[i]);
                normals.push_back(NorY[i]);
                normals.push_back(NorZ[i]);
            }
        }



        std::cout << "Successfully read " << count << " points!" << std::endl;
        //NORMILIZE
        float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
        float maxX = FLT_MIN, maxY = FLT_MIN, maxZ = FLT_MIN;

        for (int i = 0; i < count; i++) {
            minX = std::min(minX, points[i * 3]);
            minY = std::min(minY, points[i * 3 + 1]);
            minZ = std::min(minZ, points[i * 3 + 2]);
            maxX = std::max(maxX, points[i * 3]);
            maxY = std::max(maxY, points[i * 3 + 1]);
            maxZ = std::max(maxZ, points[i * 3 + 2]);
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
        for (int i = 0; i < count; i++) {
            points[i * 3] = (points[i * 3] - centerX) / maxDim;
            points[i * 3 + 1] = (points[i * 3 + 1] - centerY) / maxDim;
            points[i * 3 + 2] = (points[i * 3 + 2] - centerZ) / maxDim;
        }

    }
    catch (const e57::E57Exception& e) {
        std::cerr << "E57 error: " << e.what() << std::endl;
        std::cerr << "E57 errorCode: " << e.errorCode() << std::endl;

        return -1;
    }

    return 0;
}

std::vector<float>& E57::getPoints()
{
    return this->points;
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
