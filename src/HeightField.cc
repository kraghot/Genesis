#include "HeightField.hh"
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <fstream>

using namespace glow;

HeightField::HeightField()
{

}

HeightField::HeightField(NoiseGenerator* generator, unsigned int dimension)
{
    init(generator, dimension);
}

glow::SharedVertexArray HeightField::createPerlinTerrain()
{
    const int dim = 100;
    const uint32_t restart = 65535;
    mPositions.resize(mNumberOfVertices);
    mNormals.resize(mNumberOfVertices);
    mColors.resize(mNumberOfVertices);

    for(int i = 0; i < dim; i++)
    {
        for(int j = 0; j < dim; j++)
        {
            float x = 10 * ((float)i / dim), y = 10 * ((float)j/dim);
            float noise = mGenerator->noise(x, y, 0.8);
            mPositions.at(i*dim + j) = {i, 5 * noise, j};
            mNormals.at(i*dim + j) = {0, 1, 0};
            float colornoise = noise + 1.0f / 2.0f;
            mColors.at(i*dim + j) = {colornoise, colornoise, colornoise, 1.0f};
            if(i != dim - 1)
            {
                mIndices.push_back(i* dim + j);
                mIndices.push_back((i+1) * dim + j);
            }
        }
        mIndices.push_back(restart);
    }

    auto ab = ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aPosition");
    ab->bind().setData(mPositions, GL_DYNAMIC_DRAW);
    mAbs.push_back(ab);

    ab = ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aNormal");
    ab->bind().setData(mNormals);
    mAbs.push_back(ab);

    ab = ArrayBuffer::create();
    ab->defineAttribute<glm::vec4>("aColor");
    ab->bind().setData(mColors);
    mAbs.push_back(ab);

    for (auto const& ab : mAbs)
        ab->setObjectLabel(ab->getAttributes()[0].name + " of " + "Perlin");

    mEab = ElementArrayBuffer::create(mIndices);
    mEab->setObjectLabel("Heightamp");
    mVao = VertexArray::create(mAbs, mEab, GL_TRIANGLE_STRIP);
    mVao->setObjectLabel("Heightmap");

    return mVao;
}

void HeightField::exportToFile()
{
    std::ostringstream filename;
    filename << "terrain-8bbp-" << mDimension << "x" << mDimension << ".raw";
    std::ofstream file (filename.str(), std::ios::out | std::ios::binary);
    std::vector<uint8_t> byteField;
    byteField.reserve(mNumberOfVertices);
    for(auto it : mPositions)
    {
        byteField.push_back(it.y * (255/5));
    }
    file.write((char *)byteField.data(), byteField.size());
}

void HeightField::init(NoiseGenerator* generator, unsigned int dimension)
{
    mGenerator = generator;
    mDimension = dimension;
    mNumberOfVertices = dimension * dimension;
}

