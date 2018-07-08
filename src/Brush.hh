#ifndef BRUSH_H
#define BRUSH_H

#include "MultiLayeredHeightmap.hh"
#include "QuadTree.hh"


struct Ray
{
    glm::vec3 origin = {0, 0, 0};
    glm::vec3 direction = {0, 0, 0};
};

struct Face
{
    glm::vec3 p0;
    glm::vec3 p1;
    glm::vec3 p2;

    glm::vec3 normal;
};

class Brush
{
public:
    Brush(MultiLayeredHeightmap *h);

    void intersect(const Ray& _ray );
    bool IntersectNode(const Ray& ray, const quadtree_node *node);
    glm::vec3 intersect_quadtree(const Ray& _ray, std::vector<quadtree_node> nodes);

    glm::dvec3 getIntersectionPoint() const;

    glow::SharedVertexArray getCircleVao() const;
    glm::mat4 GetCircleRotation();
    void GenerateArc(float r);

    void SetTextureBrush(int seletedTexture);
    void SetHeightBrush(float factor);
    bool IntersectAabb(const Ray& ray, const quadtree_node& node, float& tmin, float& tmax);
    bool IntersectAabb2(const Ray& ray, const quadtree_node& node, float &t);

private:

    bool bary_coord(const glm::vec3& _p, const glm::vec3& _u, const glm::vec3& _v, const glm::vec3& _w, glm::vec3& _result) const;
    bool intersectTriangle(const Face& _face, const glm::vec3 &_normal, const Ray &_ray);
    static bool CompareQuadtreeIntersections(const quadtree_intersection& a, const quadtree_intersection& b)
    {
        return a.t_min < b.t_min;
    }
    glm::vec3 intersectionPoint = {0.f, 0.f, 0.f};
    float _t = 0.f;
    float epsilon = 0.001f;

    Face mIntersectionTriangle;
    glow::SharedVertexArray mCircleVao;
    unsigned int mIntersectionHeight = 0;
    unsigned int mIntersectionWidth = 0;
    float mIntersectionRadius = 0.f;

    MultiLayeredHeightmap *mHeightmap;
};

#endif // BRUSH_H
