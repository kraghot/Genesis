#ifndef QUAD_TREE
#define QUAD_TREE

#include <stdio.h>
#include <glm/common.hpp>
#include <vector>

#include "MultiLayeredHeightmap.hh"


struct box{
    glm::vec2 max;
    glm::vec2 min;
};

struct quadtree_node{
    struct box area;
    float height_min;
    float height_max;
    bool isLeaf = false;
    std::vector<quadtree_node*> children;
};

struct quadtree_intersection{
    struct quadtree_node& node;
    float t_min;
    float t_max;
};


class QuadTree
{
public:
    QuadTree(MultiLayeredHeightmap *h);
    std::vector<quadtree_node> construct_quadtree();

private:
    MultiLayeredHeightmap *mHeightmap;

};




#endif //QUAD_TREE
