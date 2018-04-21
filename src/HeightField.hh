#ifndef HEIGHTFIELD_H
#define HEIGHTFIELD_H

#include "NoiseGenerator.hh"
#include <glow/objects/VertexArray.hh>
#include <glm/common.hpp>
#include <vector>

class HeightField
{
public:
    HeightField();
    HeightField(NoiseGenerator* generator, unsigned int dimension);
    glow::SharedVertexArray createPerlinTerrain();
    void exportToFile();
    void init(NoiseGenerator* generator, unsigned int dimension);
private:
    NoiseGenerator* mGenerator;
    unsigned int mDimension;
    unsigned int mNumberOfVertices;
    std::vector<glm::vec3> mPositions;
    std::vector<glm::vec3> mNormals;
    std::vector<glm::vec4> mColors;
    std::vector<uint32_t> mIndices;
    std::vector<glow::SharedArrayBuffer> mAbs;
    glow::SharedElementArrayBuffer mEab;
    glow::SharedVertexArray mVao;
};

#endif // HEIGHTFIELD_H
