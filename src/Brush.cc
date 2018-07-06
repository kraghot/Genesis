#include "Brush.hh"
#include <list>

Brush::Brush(MultiLayeredHeightmap *h){

   mHeightmap = h;
}

glow::SharedVertexArray Brush::getCircleVao() const
{
    return mCircleVao;
}

glm::mat4 Brush::GetCircleRotation()
{
    glm::vec3 upVector(0, 1, 0);
    glm::vec3 xVector = glm::cross(upVector, mIntersectionTriangle.normal);
    glm::mat4 rot = glm::lookAt(intersectionPoint,
                                intersectionPoint + xVector,
                                mIntersectionTriangle.normal);

    return inverse(rot);

}

void Brush::SetTextureBrush(int seletedTexture){

    float sum;

    float Radius2 = mIntersectionRadius * mIntersectionRadius;

        for (unsigned int j = 0; j < mHeightmap->mHeightmapDimensions.x; j++){ // NEEDS OPTIMIZATION; old: for (unsigned int j = mIntersectionHeight - mIntersectionRadius; j < mIntersectionHeight + mIntersectionRadius; j++)

            for (unsigned int i = 0; i < mHeightmap->mHeightmapDimensions.x; i++){


                float pointPositionx = glm::pow(mHeightmap->mPositions.at((j * mHeightmap->mHeightmapDimensions.x) + i).x - mHeightmap->mPositions.at((mIntersectionHeight * mHeightmap->mHeightmapDimensions.x) + mIntersectionWidth).x,2);
                float pointPositiony  = glm::pow(mHeightmap->mPositions.at((j * mHeightmap->mHeightmapDimensions.x) + i).y -mHeightmap->mPositions.at((mIntersectionHeight * mHeightmap->mHeightmapDimensions.x) + mIntersectionWidth).y,2);
                float pointPositionz = glm::pow(mHeightmap->mPositions.at((j * mHeightmap->mHeightmapDimensions.x) + i).z -mHeightmap->mPositions.at((mIntersectionHeight * mHeightmap->mHeightmapDimensions.x) + mIntersectionWidth).z,2);

                float distance = pointPositionx + pointPositiony + pointPositionz;

                    if(distance < Radius2 && distance > (0.7 * Radius2)){
                        mHeightmap->mSplatmap.at(j*mHeightmap->mHeightmapDimensions.x + i)[seletedTexture] += 0.2;
                    }

                    else if(distance < (0.7 * Radius2) && distance > (0.5 * Radius2)){
                        mHeightmap->mSplatmap.at(j*mHeightmap->mHeightmapDimensions.x + i)[seletedTexture] += 0.4;
                    }
                    else if(distance < (0.5 * Radius2)){
                        mHeightmap->mSplatmap.at(j*mHeightmap->mHeightmapDimensions.x + i)[seletedTexture] += 0.8;
                    }

                    sum = mHeightmap->mSplatmap.at(j*mHeightmap->mHeightmapDimensions.x + i).x + mHeightmap->mSplatmap.at(j*mHeightmap->mHeightmapDimensions.x + i).y + mHeightmap->mSplatmap.at(j*mHeightmap->mHeightmapDimensions.x + i).z;
                    mHeightmap->mSplatmap.at(j*mHeightmap->mHeightmapDimensions.x + i).y /= sum;
                    mHeightmap->mSplatmap.at(j*mHeightmap->mHeightmapDimensions.x + i).x /= sum;
                    mHeightmap->mSplatmap.at(j*mHeightmap->mHeightmapDimensions.x + i).z /= sum;

                }
            }


       mHeightmap->mSplatmapTexture->bind().setData(GL_RGB, mHeightmap->mHeightmapDimensions.x, mHeightmap->mHeightmapDimensions.y, mHeightmap->mSplatmap);
       mHeightmap->mSplatmapTexture->bind().generateMipmaps();
}

void Brush::SetHeightBrush(float factor){
    float Radius2 = mIntersectionRadius * mIntersectionRadius;
    factor /= 10;

    for (unsigned int j = 0; j < mHeightmap->mHeightmapDimensions.x; j++){ // 2m world = 2 u heightmapu
        for (unsigned int i = 0; i < mHeightmap->mHeightmapDimensions.x; i++){


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

    mHeightmap->CalculateNormalsTangents(mHeightmap->mHeightmapDimensions.x, mHeightmap->mHeightmapDimensions.y);
    //mHeightmap->LoadSplatmap();
    mHeightmap->MakeVertexArray();
}

bool Brush::IntersectAabb(const Ray &ray, const quadtree_node &node, float& tmin, float& tmax)
{

    tmin = (node.area.min.x - ray.origin.x) / ray.direction.x;
    tmax = (node.area.max.x - ray.origin.x) / ray.direction.x;

    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (node.area.min.y - ray.origin.y) / ray.direction.y;
    float tymax = (node.area.max.y - ray.origin.y) / ray.direction.y;

    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
    return false;

    if (tymin > tmin)
    tmin = tymin;

    if (tymax < tmax)
    tmax = tymax;

    float tzmin = (node.height_min - ray.origin.z) / ray.direction.z;
    float tzmax = (node.height_max - ray.origin.z) / ray.direction.z;

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
    return false;

    if (tzmin > tmin)
    tmin = tzmin;

    if (tzmax < tmax)
    tmax = tzmax;

    return true;
}

glm::dvec3 Brush::getIntersectionPoint() const
{
    return intersectionPoint;
}

bool Brush::intersectTriangle(const Face& _face, const glm::vec3& _normal, const Ray& _ray)
{

    glm::vec3 bary;
    auto temp_t = _t;
    auto temp_intersection = intersectionPoint;

    float dotRN = glm::dot(_ray.direction, _normal);

    float planeDist = glm::dot((_face.p0 - _ray.origin), _normal);

    _t = planeDist / dotRN;


    intersectionPoint = _ray.origin + _t * _ray.direction;

    bary_coord(intersectionPoint, _face.p0, _face.p1, _face.p2, bary);


    if(bary[0] >= 0 && bary[1] >= 0 && bary[2] >= 0 && bary[0] <= 1 && bary[1] <= 1 && bary[2] <= 1 && _t > epsilon){

        return true;
    }

    _t = temp_t;
    intersectionPoint = temp_intersection;

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
                intersectionPoint = _ray.origin + temp_t * _ray.direction * 0.9;
            }

            else if(intersectTriangle(Triangle2, Normal2, _ray) && _t < temp_t){
                temp_t = _t;
                Triangle2.normal = Normal2;
                mIntersectionTriangle = Triangle2;
                mIntersectionHeight = j;
                mIntersectionWidth = i;
                intersectionPoint = _ray.origin + temp_t * _ray.direction* 0.9;
            }

        }

    }

//    if(intersection)
//        intersectionPoint = _ray.origin + temp_t * _ray.direction;

}

bool Brush::IntersectNode(const Ray &ray, const quadtree_node &node)
{
    int dimX = mHeightmap->mHeightmapDimensions.x;
    Face Triangle1, Triangle2;
    glm::vec3 Normal1, Normal2;
    float temp_t = std::numeric_limits<float>::max();

    for (int j = node.area.min.x; j <= node.area.max.x; j++ )
    {
        for (int i = node.area.min.y; i <= node.area.max.y; i++ )
        {

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
                intersectionPoint = ray.origin + temp_t * ray.direction * 0.9;
            }

            else if(intersectTriangle(Triangle2, Normal2, ray) && _t < temp_t){
                temp_t = _t;
                Triangle2.normal = Normal2;
                mIntersectionTriangle = Triangle2;
                mIntersectionHeight = j;
                mIntersectionWidth = i;
                intersectionPoint = ray.origin + temp_t * ray.direction* 0.9;
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

glm::vec3 Brush::intersect_quadtree(const Ray& _ray ){
    QuadTree quadtree(mHeightmap);

    std::vector<quadtree_node> nodes;

    std::vector<quadtree_node> queue;
    std::list<quadtree_intersection> intersected;

    nodes = quadtree.construct_quadtree();

    queue.push_back(nodes.at(0));

    while(!queue.empty()){
        quadtree_node node = queue.back();
        queue.pop_back();
        float tmin, tmax;
        bool isIntersecting = IntersectAabb(_ray, node, tmin, tmax);

        if(isIntersecting){
            if(node.isLeaf)
            {
                intersected.push_back({node, tmin, tmax});
            }
            else
            {
                for(auto it: node.children)
                    queue.push_back(*it);
            }
        }

    }

//    std::sort(intersected.begin(), intersected.end(), CompareQuadtreeIntersections);
    intersected.sort(CompareQuadtreeIntersections);

    while(intersected.size() > 0)
    {
        auto& closestIntersection = intersected.front();
        if(IntersectNode(_ray, closestIntersection.node))
            return intersectionPoint;
        else
        {
            intersected.pop_front();
        }
    }

    // No intersection found. Figure out what to do
    return {-10, -10, -10};
}


