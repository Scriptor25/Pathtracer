#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <pathtracer/scene.hpp>

glm::vec3 pathtracer::Triangle::Center() const
{
    return (P0 + P1 + P2) / 3.0f;
}

void pathtracer::Triangle::AddBounds(glm::vec3& min, glm::vec3& max) const
{
    min = glm::min(glm::min(glm::min(min, P0), P1), P2);
    max = glm::max(glm::max(glm::max(max, P0), P1), P2);
}

pathtracer::Scene::Scene()
    : m_TriangleBuffer(GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW),
      m_MaterialBuffer(GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW),
      m_ModelBuffer(GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW),
      m_BVHNodeBuffer(GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW)
{
}

void pathtracer::Scene::LoadModel(const std::filesystem::path& path, unsigned int flags)
{
    Assimp::Importer importer;
    if (const auto pScene = importer.ReadFile(path.string(), flags | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices))
    {
        const unsigned first = m_Triangles.size();

        for (unsigned mi = 0; mi < pScene->mNumMeshes; ++mi)
        {
            const auto pMesh = pScene->mMeshes[mi];

            for (unsigned fi = 0; fi < pMesh->mNumFaces; ++fi)
            {
                const auto face = pMesh->mFaces[fi];

                std::vector<glm::vec3> p;
                std::vector<glm::vec3> n;
                std::vector<glm::vec2> uv;
                for (unsigned i = 0; i < face.mNumIndices; ++i)
                {
                    const auto idx = face.mIndices[i];

                    auto pos = pMesh->mVertices[idx];
                    p.emplace_back(pos.x, pos.y, pos.z);

                    auto normal = pMesh->mNormals[idx];
                    n.emplace_back(normal.x, normal.y, normal.z);

                    if (pMesh->HasTextureCoords(0))
                    {
                        auto tex = pMesh->mTextureCoords[0][idx];
                        uv.emplace_back(tex.x, tex.y);
                    }
                    else
                    {
                        uv.emplace_back(0.0, 0.0);
                    }
                }

                m_Triangles.emplace_back(p[0], p[1], p[2], n[0], n[1], n[2], uv[0], uv[1], uv[2], m_Materials.size() + pMesh->mMaterialIndex);
            }
        }

        for (unsigned mi = 0; mi < pScene->mNumMaterials; ++mi)
        {
            const auto pMaterial = pScene->mMaterials[mi];

            /*for (unsigned pi = 0; pi < pMaterial->mNumProperties; ++pi)
            {
                const auto pProp = pMaterial->mProperties[pi];
                const auto key = pProp->mKey.C_Str();
                std::cout << key << ": ";

                switch (pProp->mType)
                {
                case aiPTI_Float:
                    {
                        float value = 0.0f;
                        pMaterial->Get(key, 0, 0, value);
                        std::cout << value << std::endl;
                    }
                    break;
                case aiPTI_Double:
                    {
                        double value = 0.0;
                        pMaterial->Get(key, 0, 0, value);
                        std::cout << value << std::endl;
                    }
                    break;
                case aiPTI_String:
                    {
                        aiString value;
                        pMaterial->Get(key, 0, 0, value);
                        std::cout << value.C_Str() << std::endl;
                    }
                    break;
                case aiPTI_Integer:
                    {
                        int value = 0;
                        pMaterial->Get(key, 0, 0, value);
                        std::cout << value << std::endl;
                    }
                    break;
                case aiPTI_Buffer:
                    std::cout << "Buffer" << std::endl;
                    break;
                case _aiPTI_Force32Bit:
                    std::cout << "Force 32 Bit" << std::endl;
                    break;
                }
            }*/

            aiColor3D diffuse, emissive;
            float roughness = 0.9f, metalness = 0.1f, opacity = 0.0f, ir = 1.5f;

            pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
            pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);
            pMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
            pMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metalness);
            pMaterial->Get(AI_MATKEY_OPACITY, opacity);
            pMaterial->Get(AI_MATKEY_REFRACTI, ir);

            m_Materials.emplace_back(glm::vec3(diffuse.r, diffuse.g, diffuse.b), -1, glm::vec3(emissive.r, emissive.g, emissive.b), -1, roughness, metalness, 1.0f - opacity, ir);
        }

        importer.FreeScene();

        const auto root = GenerateBVHTree(first, m_Triangles.size(), 100);
        m_Models.emplace_back(root, glm::mat4(1.0f), glm::mat4(1.0f));
    }
    else
    {
        throw std::runtime_error("Failed to load model from " + path.string());
    }
}

size_t pathtracer::Scene::GenerateBVHTree(const unsigned start, const unsigned end, const unsigned depth)
{
    glm::vec3 min{std::numeric_limits<float>::infinity()}, max{-std::numeric_limits<float>::infinity()};

    for (unsigned i = start; i < end; ++i)
        m_Triangles[i].AddBounds(min, max);

    const auto dx = max.x - min.x;
    const auto dy = max.y - min.y;
    const auto dz = max.z - min.z;

    if (dx < 0.01f)
    {
        min.x -= 0.01f;
        max.x += 0.01f;
    }
    if (dy < 0.01f)
    {
        min.y -= 0.01f;
        max.y += 0.01f;
    }
    if (dz < 0.01f)
    {
        min.z -= 0.01f;
        max.z += 0.01f;
    }

    const auto idx = m_BVHNodes.size();
    const auto count = end - start;

    if (depth == 0 || count < 2)
    {
        m_BVHNodes.emplace_back(min, max, 0, 0, start, end);
        return idx;
    }

    m_BVHNodes.emplace_back(min, max, 0, 0, 0, 0);

    const int longest_axis = dx > dy ? (dx >= dz ? 0 : 2) : dy >= dz ? 1 : 2;

    std::sort(m_Triangles.begin() + start, m_Triangles.begin() + end, [longest_axis](const Triangle& a, const Triangle& b)-> bool
    {
        return a.Center()[longest_axis] < b.Center()[longest_axis];
    });

    const auto mid = start + count / 2;
    m_BVHNodes[idx].Left = GenerateBVHTree(start, mid, depth - 1);
    m_BVHNodes[idx].Right = GenerateBVHTree(mid, end, depth - 1);

    return idx;
}

void pathtracer::Scene::Upload() const
{
    m_TriangleBuffer.Bind();
    m_TriangleBuffer.Data(m_Triangles.size() * sizeof(Triangle), m_Triangles.data());
    m_TriangleBuffer.Unbind();

    m_MaterialBuffer.Bind();
    m_MaterialBuffer.Data(m_Materials.size() * sizeof(Material), m_Materials.data());
    m_MaterialBuffer.Unbind();

    m_ModelBuffer.Bind();
    m_ModelBuffer.Data(m_Models.size() * sizeof(Model), m_Models.data());
    m_ModelBuffer.Unbind();

    m_BVHNodeBuffer.Bind();
    m_BVHNodeBuffer.Data(m_BVHNodes.size() * sizeof(BVHNode), m_BVHNodes.data());
    m_BVHNodeBuffer.Unbind();

    m_TriangleBuffer.BindBase(0);
    m_MaterialBuffer.BindBase(1);
    m_ModelBuffer.BindBase(2);
    m_BVHNodeBuffer.BindBase(3);
}

pathtracer::Model& pathtracer::Scene::GetModel(const size_t i)
{
    return m_Models[i];
}

pathtracer::Model& pathtracer::Scene::GetLastModel()
{
    return m_Models.back();
}
