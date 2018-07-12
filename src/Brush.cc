#include "Brush.hh"
#include <list>
#include <algorithm>

Brush::Brush(MultiLayeredHeightmap *h){

   mHeightmap = h;
}

glow::SharedVertexArray Brush::getCircleVao() const
{
    return mCircleVao;
}

glm::mat4 Brush::GetCircleRotation(glm::vec3 normal, glm::vec3 intersection,  glm::vec3 upVector)
{
    glm::vec3 xVector = glm::cross(upVector, normal);
    glm::mat4 rot = glm::lookAt(intersection,
                                intersection + xVector,
                                normal);

    return inverse(rot);

}

void Brush::SetTextureBrush(int seletedTexture, std::vector<glm::vec4> &biomesMap, glow::SharedTexture2D mBiomesTexture){

    
    float sum;

    float Radius2 = mIntersectionRadius * mIntersectionRadius;

        for (unsigned int j = mIntersectionHeight - mIntersectionRadius; j < mIntersectionHeight + mIntersectionRadius; j++){

            for (unsigned int i = mIntersectionWidth - mIntersectionRadius; i < mIntersectionWidth + mIntersectionRadius; i++){

                if(j < 0 || j >= mHeightmap->mHeightmapDimensions.x || i < 0 || i >= mHeightmap->mHeightmapDimensions.x)
                    continue;


                float pointPositionx = glm::pow(mHeightmap->mPositions.at((j * mHeightmap->mHeightmapDimensions.x) + i).x - mHeightmap->mPositions.at((mIntersectionHeight * mHeightmap->mHeightmapDimensions.x) + mIntersectionWidth).x,2);
                float pointPositiony  = glm::pow(mHeightmap->mPositions.at((j * mHeightmap->mHeightmapDimensions.x) + i).y -mHeightmap->mPositions.at((mIntersectionHeight * mHeightmap->mHeightmapDimensions.x) + mIntersectionWidth).y,2);
                float pointPositionz = glm::pow(mHeightmap->mPositions.at((j * mHeightmap->mHeightmapDimensions.x) + i).z -mHeightmap->mPositions.at((mIntersectionHeight * mHeightmap->mHeightmapDimensions.x) + mIntersectionWidth).z,2);

                float distance = pointPositionx + pointPositiony + pointPositionz;

                    if(distance < Radius2 && distance > (0.7 * Radius2)){
                        biomesMap.at(j*mHeightmap->mHeightmapDimensions.x + i)[seletedTexture] += 0.2;
                    }

                    else if(distance < (0.7 * Radius2) && distance > (0.5 * Radius2)){
                        biomesMap.at(j*mHeightmap->mHeightmapDimensions.x + i)[seletedTexture] += 0.4;
                    }
                    else if(distance < (0.5 * Radius2)){
                        biomesMap.at(j*mHeightmap->mHeightmapDimensions.x + i)[seletedTexture] += 0.8;
                    }

                    sum = biomesMap.at(j*mHeightmap->mHeightmapDimensions.x + i).x + biomesMap.at(j*mHeightmap->mHeightmapDimensions.x + i).y + biomesMap.at(j*mHeightmap->mHeightmapDimensions.x + i).z + biomesMap.at(j*mHeightmap->mHeightmapDimensions.x + i).w;
                    biomesMap.at(j*mHeightmap->mHeightmapDimensions.x + i).y /= sum;
                    biomesMap.at(j*mHeightmap->mHeightmapDimensions.x + i).x /= sum;
                    biomesMap.at(j*mHeightmap->mHeightmapDimensions.x + i).z /= sum;
                    biomesMap.at(j*mHeightmap->mHeightmapDimensions.x + i).w /= sum;

                }
            }

        mBiomesTexture->bind().setData(GL_RGBA, mHeightmap->mHeightmapDimensions.x, mHeightmap->mHeightmapDimensions.y, biomesMap);
        mBiomesTexture->bind().generateMipmaps();
}

void Brush::SetHeightBrush(float factor){
    float Radius2 = mIntersectionRadius * mIntersectionRadius;
    factor /= 10;

    int jstart = clamp(mIntersectionHeight - mIntersectionRadius, 0.0f, static_cast<float>(mHeightmap->mHeightmapDimensions.y));
    int jend   = clamp(mIntersectionHeight + mIntersectionRadius, 0.0f, static_cast<float>(mHeightmap->mHeightmapDimensions.y));
    int istart = clamp(mIntersectionWidth  - mIntersectionRadius, 0.0f, static_cast<float>(mHeightmap->mHeightmapDimensions.x));
    int iend   = clamp(mIntersectionWidth  + mIntersectionRadius, 0.0f, static_cast<float>(mHeightmap->mHeightmapDimensions.x));

    for (unsigned int j = jstart; j < jend; j++){

        for (unsigned int i = istart; i < iend; i++){

            if(j < 0 || j >= mHeightmap->mHeightmapDimensions.x || i < 0 || i >= mHeightmap->mHeightmapDimensions.x)
                continue;


            float pointPositionx = glm::pow(mHeightmap->mPositions.at((j * mHeightmap->mHeightmapDimensions.x) + i).x - mHeightmap->mPositions.at((mIntersectionHeight * mHeightmap->mHeightmapDimensions.x) + mIntersectionWidth).x,2);
            float pointPositiony  = glm::pow(mHeightmap->mPositions.at((j * mHeightmap->mHeightmapDimensions.x) + i).y -mHeightmap->mPositions.at((mIntersectionHeight * mHeightmap->mHeightmapDimensions.x) + mIntersectionWidth).y,2);
            float pointPositionz = glm::pow(mHeightmap->mPositions.at((j * mHeightmap->mHeightmapDimensions.x) + i).z -mHeightmap->mPositions.at((mIntersectionHeight * mHeightmap->mHeightmapDimensions.x) + mIntersectionWidth).z,2);

            float distance = pointPositionx + pointPositiony + pointPositionz;

                if(distance < Radius2 && distance > (0.7 * Radius2)){
                    mHeightmap->mDisplacement.at(j*mHeightmap->mHeightmapDimensions.x + i) += factor < (100 * 0.004f)? (100 * 0.002f) : factor - (100 * 0.004f);
                }

                else if(distance < (0.7 * Radius2) && distance > (0.5 * Radius2)){
                    mHeightmap->mDisplacement.at(j*mHeightmap->mHeightmapDimensions.x + i) += factor < (100 * 0.002f)? (100 * 0.001f) : factor - (100 * 0.002f);
                }
                else if(distance < (0.5 * Radius2)){
                     mHeightmap->mDisplacement.at(j*mHeightmap->mHeightmapDimensions.x + i) += factor;
                }

                mHeightmap->mPositions.at(j*mHeightmap->mHeightmapDimensions.x + i).y = mHeightmap->mDisplacement.at(j*mHeightmap->mHeightmapDimensions.x + i);

            }
        }

//    if(j < 0 || j >= mHeightmap->mHeightmapDimensions.x || i < 0 || i >= mHeightmap->mHeightmapDimensions.x)
//        return;

   mHeightmap->CalculateNormalsTangents({istart, iend},
                                        {jstart, jend});
    //mHeightmap->LoadSplatmap();
    mHeightmap->MakeVertexArray();
}

bool Brush::IntersectAabb(const Ray &ray, const quadtree_node &node, float& tmin, float& tmax)
{
    glm::vec3 node_position_min;
    glm::vec3 node_position_max;

    node_position_min.x = node.area.min.x;
    node_position_min.y = node.height_min;
    node_position_min.z = node.area.min.y;

    node_position_max.x = node.area.max.x;
    node_position_max.y = node.height_max;
    node_position_max.z = node.area.max.y;

    tmin = (mHeightmap->LocalToWorldCoordinates(node_position_min).x - ray.origin.x) / ray.direction.x;
    tmax = (mHeightmap->LocalToWorldCoordinates(node_position_max).x - ray.origin.x) / ray.direction.x;

    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (mHeightmap->LocalToWorldCoordinates(node_position_min).y - ray.origin.y) / ray.direction.y;
    float tymax = (mHeightmap->LocalToWorldCoordinates(node_position_max).y - ray.origin.y) / ray.direction.y;

    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
    return false;

    if (tymin > tmin)
    tmin = tymin;

    if (tymax < tmax)
    tmax = tymax;

    float tzmin = (mHeightmap->LocalToWorldCoordinates(node_position_min).z - ray.origin.z) / ray.direction.z;
    float tzmax = (mHeightmap->LocalToWorldCoordinates(node_position_max).z - ray.origin.z) / ray.direction.z;

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
    return false;

    if (tzmin > tmin)
    tmin = tzmin;

    if (tzmax < tmax)
    tmax = tzmax;

    return true;
}

bool Brush::IntersectAabb2(const Ray &ray, const quadtree_node &node, float& t)
{
    glm::vec3 lb = mHeightmap->LocalToWorldCoordinates({node.area.min.x, node.height_min, node.area.min.y});
    glm::vec3 rt = mHeightmap->LocalToWorldCoordinates({node.area.max.x, node.height_max, node.area.max.y});

//    printf("lb: %f %f %f rb: %f %f %f\n", lb.x, lb.y, lb.z, rt.x, rt.y, rt.z);
//    std::cout << std::flush;

    glm::vec3 dirfrac;
    // ray.direction is unit direction vector of ray
    dirfrac.x = 1.0f / ray.direction.x;
    dirfrac.y = 1.0f / ray.direction.y;
    dirfrac.z = 1.0f / ray.direction.z;
    // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
    // r.org is origin of ray
    float t1 = (lb.x - ray.origin.x)*dirfrac.x;
    float t2 = (rt.x - ray.origin.x)*dirfrac.x;
    float t3 = (lb.y - ray.origin.y)*dirfrac.y;
    float t4 = (rt.y - ray.origin.y)*dirfrac.y;
    float t5 = (lb.z - ray.origin.z)*dirfrac.z;
    float t6 = (rt.z - ray.origin.z)*dirfrac.z;

    float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
    float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

//    printf("tmin %f, tmax %f", tmin, tmax);
//    std::cout << std::flush;

    // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
    if (tmax < 0)
    {
        t = tmax;
        return false;
    }

    // if tmin > tmax, ray doesn't intersect AABB
    if (tmin > tmax)
    {
        t = tmax;
        return false;
    }

    t = tmin;
    return true;
}

glm::dvec3 Brush::getIntersectionPoint() const
{
    return mIntersection;
}

bool Brush::intersectTriangle(const Face& _face, const glm::vec3& _normal, const Ray& _ray)
{

    glm::vec3 bary;
    auto temp_t = _t;
    auto temp_intersection = mIntersection;

    float dotRN = glm::dot(_ray.direction, _normal);

    float planeDist = glm::dot((_face.p0 - _ray.origin), _normal);

    _t = planeDist / dotRN;


    mIntersection = _ray.origin + _t * _ray.direction;

    bary_coord(mIntersection, _face.p0, _face.p1, _face.p2, bary);


    if(bary[0] >= 0 && bary[1] >= 0 && bary[2] >= 0 && bary[0] <= 1 && bary[1] <= 1 && bary[2] <= 1 && _t > epsilon){

        return true;
    }

    _t = temp_t;
    mIntersection = temp_intersection;

    return false;
}

bool Brush::bary_coord(const glm::vec3& _p, const glm::vec3& _u, const glm::vec3& _v, const glm::vec3& _w, glm::vec3& _result) const
{
    glm::vec3 vu = _v - _u;
    glm::vec3 wu = _w - _u;
    glm::vec3 pu = _p - _u;

    double nx = vu[1] * wu[2] - vu[2] * wu[1];
    double ny = vu[2] * wu[0] - vu[0] * wu[2];
    double nz = vu[0] * wu[1] - vu[1] * wu[0];
    double ax = fabs(nx);
    double ay = fabs(ny);
    double az = fabs(nz);

    unsigned char max_coord;

    if (ax > ay)
    {
        if (ax > az)
        {
            max_coord = 0;
        }
        else
        {
            max_coord = 2;
        }
    }
    else
    {
        if (ay > az)
        {
            max_coord = 1;
        }
        else
        {
            max_coord = 2;
        }
    }

    switch (max_coord)
    {
    case 0:
        if (1.0 + ax == 1.0)
            return false;
        _result[1] = 1.0 + (pu[1] * wu[2] - pu[2] * wu[1]) / nx - 1.0;
        _result[2] = 1.0 + (vu[1] * pu[2] - vu[2] * pu[1]) / nx - 1.0;
        _result[0] = 1.0 - _result[1] - _result[2];
        break;

    case 1:
        if (1.0 + ay == 1.0)
            return false;
        _result[1] = 1.0 + (pu[2] * wu[0] - pu[0] * wu[2]) / ny - 1.0;
        _result[2] = 1.0 + (vu[2] * pu[0] - vu[0] * pu[2]) / ny - 1.0;
        _result[0] = 1.0 - _result[1] - _result[2];
        break;

    case 2:
        if (1.0 + az == 1.0)
            return false;
        _result[1] = 1.0 + (pu[0] * wu[1] - pu[1] * wu[0]) / nz - 1.0;
        _result[2] = 1.0 + (vu[0] * pu[1] - vu[1] * pu[0]) / nz - 1.0;
        _result[0] = 1.0 - _result[1] - _result[2];
        break;
    }

    return true;
}

void Brush::intersect(const Ray& _ray )
{

    int dimX = mHeightmap->mHeightmapDimensions.x, dimY = mHeightmap->mHeightmapDimensions.y;
    Face Triangle1, Triangle2;
    glm::vec3 Normal1, Normal2;
    float temp_t = 1000000.f;

    for (int j = 0; j < dimY-1; j++ )
    {
        for (int i = 0; i < dimX-1; i++ )
        {
           // unsigned int index = ( j * dimX ) + i;

            Triangle1.p0 = mHeightmap->mPositions.at((j * dimX ) + i);
            Triangle1.p1 = mHeightmap->mPositions.at((j+1) * dimX  + i);
            Triangle1.p2 = mHeightmap->mPositions.at((j+1) * dimX + i+1);

//            Triangle1.p0.y = mHeightmap->mDisplacement.at((j * dimX ) + i);
//            Triangle1.p1.y = mHeightmap->mDisplacement.at((j+1) * dimX  + i);
//            Triangle1.p2.y = mHeightmap->mDisplacement.at((j+1) * dimX + i+1);

            Triangle2.p0 = mHeightmap->mPositions.at((j+1) * dimX + i+1);
            Triangle2.p1 = mHeightmap->mPositions.at((j* dimX) + i+1);
            Triangle2.p2 = mHeightmap->mPositions.at((j * dimX ) + i);

//            Triangle2.p0.y = mHeightmap->mDisplacement.at((j+1) * dimX + i+1);
//            Triangle2.p1.y = mHeightmap->mDisplacement.at((j* dimX) + i+1);
//            Triangle2.p2.y = mHeightmap->mDisplacement.at((j * dimX ) + i);

            Normal1 = glm::normalize(glm::cross(Triangle1.p0-Triangle1.p1, Triangle1.p1-Triangle1.p2));
            Normal2 = glm::normalize(glm::cross(Triangle2.p0-Triangle2.p1, Triangle2.p1-Triangle2.p2));

            if(intersectTriangle(Triangle1, Normal1, _ray) && _t < temp_t){
                temp_t = _t;
                Triangle1.normal = Normal1;
                mIntersectionTriangle = Triangle1;
                mIntersectionHeight = j;
                mIntersectionWidth = i;
                mIntersection = _ray.origin + temp_t * _ray.direction * 0.9;
            }

            else if(intersectTriangle(Triangle2, Normal2, _ray) && _t < temp_t){
                temp_t = _t;
                Triangle2.normal = Normal2;
                mIntersectionTriangle = Triangle2;
                mIntersectionHeight = j;
                mIntersectionWidth = i;
                mIntersection = _ray.origin + temp_t * _ray.direction* 0.9;
            }

        }

    }

//    if(intersection)
//        intersectionPoint = _ray.origin + temp_t * _ray.direction;

}

bool Brush::IntersectNode(const Ray &ray, const quadtree_node *node, glm::vec3& intersectionPoint)
{
    int dimX = mHeightmap->mHeightmapDimensions.x;
    Face Triangle1, Triangle2;
    glm::vec3 Normal1, Normal2;
    float temp_t = std::numeric_limits<float>::max();

    for (int j = node->area.min.y; j < (node->area.max.y); j++ )
    {
        if(j >= mHeightmap->mHeightmapDimensions.y)
            continue;
        for (int i = node->area.min.x; i < (node->area.max.x); i++ )
        {
            if(i >= dimX)
                continue;

            Triangle1.p0 = mHeightmap->mPositions.at((j * dimX ) + i);
            Triangle1.p1 = mHeightmap->mPositions.at((j+1) * dimX  + i);
            Triangle1.p2 = mHeightmap->mPositions.at((j+1) * dimX + i+1);

            Triangle2.p0 = mHeightmap->mPositions.at((j+1) * dimX + i+1);
            Triangle2.p1 = mHeightmap->mPositions.at((j* dimX) + i+1);
            Triangle2.p2 = mHeightmap->mPositions.at((j * dimX ) + i);

            Normal1 = glm::normalize(glm::cross(Triangle1.p0-Triangle1.p1, Triangle1.p1-Triangle1.p2));
            Normal2 = glm::normalize(glm::cross(Triangle2.p0-Triangle2.p1, Triangle2.p1-Triangle2.p2));

            if(intersectTriangle(Triangle1, Normal1, ray) && _t < temp_t){
                temp_t = _t;
                Triangle1.normal = Normal1;
                mIntersectionTriangle = Triangle1;
                mIntersectionHeight = j;
                mIntersectionWidth = i;
                intersectionPoint = ray.origin + temp_t * ray.direction * 0.8;
            }

            else if(intersectTriangle(Triangle2, Normal2, ray) && _t < temp_t){
                temp_t = _t;
                Triangle2.normal = Normal2;
                mIntersectionTriangle = Triangle2;
                mIntersectionHeight = j;
                mIntersectionWidth = i;
                intersectionPoint = ray.origin + temp_t * ray.direction* 0.8;
            }

        }

    }

    // Check if anything intersected
    return temp_t != std::numeric_limits<float>::max();
}



void Brush::GenerateArc(float r)
{
    mIntersectionRadius = r;

    std::vector<glm::vec3> circlePoints;
    for(auto i=0.0f; i < 2.0f * M_PI; i+= 0.1f)
    {
        circlePoints.push_back(glm::vec3(sin(i) * r, 0.0f, cos(i) * r));
    }

    auto ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aPosition");
    ab->bind().setData(circlePoints);
    ab->setObjectLabel(ab->getAttributes()[0].name + " of " + "Circle");
    mCircleVao = glow::VertexArray::create(ab, GL_LINES);

}

glm::vec3 Brush::intersect_quadtree(const Ray& _ray, std::vector<quadtree_node> nodes){
    QuadTree quadtree(mHeightmap);

    std::vector<quadtree_node*> queue;
    std::list<quadtree_intersection> intersected;

    queue.push_back(&nodes.at(0));

    while(!queue.empty()){
        quadtree_node* node = queue.back();
        queue.pop_back();
        float t;
        bool isIntersecting = IntersectAabb2(_ray, *node, t);

        if(isIntersecting){
            if(node->isLeaf)
            {
                intersected.push_back({node, t});
            }
            else
            {
                for(auto it: node->children)
                    queue.push_back(it);
            }
        }

    }

    glm::vec3 closestIntersection;
    intersected.sort(CompareQuadtreeIntersections);

    while(intersected.size() > 0)
    {
        auto& closestNode = intersected.front();
        if(IntersectNode(_ray, closestNode.node, closestIntersection))
        {
            mIntersection = closestIntersection;
            return mIntersection;
        }
        else
        {
            intersected.pop_front();
        }
    }

    // No intersection found. Figure out what to do
    return {-10, -10, -10};
}


