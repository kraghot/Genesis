#include <QuadTree.hh>

QuadTree::QuadTree(MultiLayeredHeightmap *h){

   mHeightmap = h;
}

std::vector<quadtree_node> QuadTree::construct_quadtree(){
    //MultiLayeredHeightmap heightmap(20.0f,1.0f);
    int d = std::log2(mHeightmap->mNumberOfVertices/8500);

    int nodes_count = 0;
    int leaf_first_index = 0;

    for(int i=0; i<=d; i++){
        nodes_count += pow(4,i);

        if(i<(d-1))
            leaf_first_index += pow(4,i+1);
    }

    std::vector<quadtree_node> nodes(nodes_count);

    nodes.at(0).area.min.x = 0;
    nodes.at(0).area.min.y = 0;
    nodes.at(0).area.max.x = mHeightmap->mHeightmapDimensions.x;
    nodes.at(0).area.max.y = mHeightmap->mHeightmapDimensions.y;

    for(int i = 1; i < nodes_count; i += 4){
        quadtree_node parent = nodes.at((i-1)/4);

        glm::vec2 half_length = parent.area.min + ((parent.area.max - parent.area.min)/2.0);

        //NorthWestQuadrant
        nodes.at(i).area.min.y = half_length.y;
        nodes.at(i).area.min.x = parent.area.min.x;

        nodes.at(i).area.max.y = parent.area.max.y;
        nodes.at(i).area.max.x = half_length.x;


        //SouthWestQuadrant
        nodes.at(i+2).area.min = parent.area.min;
        nodes.at(i+2).area.max = half_length;

        //NorthEastQuadrant
        nodes.at(i+1).area.min =  half_length;
        nodes.at(i+1).area.max = parent.area.max;

        //SouthEastQuadrant

        nodes.at(i+3).area.min.x = half_length.x;
        nodes.at(i+3).area.min.y = parent.area.min.y;

        nodes.at(i+3).area.max.x = parent.area.max.x;
        nodes.at(i+3).area.max.y = half_length.y;
    }

    for (int i = nodes_count - 1; i >= 0; i--){
        nodes.at(i).height_max = 0;
        nodes.at(i).height_min = 1000;

        if(i >= leaf_first_index){
            for(int j = nodes.at(i).area.min.y; j <= nodes.at(i).area.max.y; j++){
                if(j >= mHeightmap->mHeightmapDimensions.y)
                    continue;
                for(int k = nodes.at(i).area.min.x; k <= nodes.at(i).area.max.x; k++){
                    if(k >= mHeightmap->mHeightmapDimensions.x)
                        continue;
                    float height_value = mHeightmap->GetDisplacementAt({k, j});
                    auto& currentNode = nodes.at(i);
                    currentNode.height_max = std::max(nodes.at(i).height_max, height_value);
                    currentNode.height_min = std::min(nodes.at(i).height_max, height_value);
                    currentNode.isLeaf = true;
                }
            }
        }

        else{
            int c = (i * 4) + 1;
            int max = c + 4;

            for(; c < max; c++){
                quadtree_node& child = nodes.at(c);
                nodes.at(i).children.push_back(&child);
                nodes.at(i).height_max = std::max(child.height_max, nodes.at(i).height_max);
                nodes.at(i).height_min = std::min(child.height_min, nodes.at(i).height_min);
            }
            nodes.at(i).isLeaf = false;
        }
    }

        return nodes;
}
