#ifndef QUAD_TREE
#define QUAD_TREE

#include <stdio.h>
#include <glm/common.hpp>
#include <vector>

#include "MultiLayeredHeightmap.hh"

/**
 * @brief XZ extent of the node
 */
struct box{
    glm::vec2 max;
    glm::vec2 min;
};

/**
 * @brief The quadtree_node struct contains all the data needed to create a node
 * @param height_min is the minimum height of a node
 * @param height_max @see height_min
 * @param isLeaf indicates wether the current node is a leaf
 * @param children is a vector of pointer to all children. Empty if leaf
 */
struct quadtree_node{
    struct box area;
    float height_min;
    float height_max;
    bool isLeaf = false;
    std::vector<quadtree_node*> children;
};

struct quadtree_intersection{
    struct quadtree_node* node;
    float t_min;
};


class QuadTree
{
public:
    QuadTree(MultiLayeredHeightmap *h);
    std::vector<quadtree_node> ConstructQuadtree();

private:
    MultiLayeredHeightmap* mHeightmap;

};




#endif //QUAD_TREE
