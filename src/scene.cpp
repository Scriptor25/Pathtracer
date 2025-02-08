#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <pathtracer/scene.hpp>

glm::vec3 pathtracer::Triangle::Center() const
{
    return (P0 + P1 + P2) / 3.0f;
}

void pathtracer::Triangle::AddBounds(glm::vec3 &min, glm::vec3 &max) const
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

void pathtracer::Scene::LoadModel(const std::filesystem::path &path, const unsigned int flags)
{
    Assimp::Importer importer;
    const auto scene = importer.ReadFile(
        path.string(),
        flags | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
    if (!scene)
        throw std::runtime_error("failed to load model from " + path.string() + ": " + importer.GetErrorString());

    const unsigned first = m_Triangles.size();

    for (unsigned mi = 0; mi < scene->mNumMeshes; ++mi)
    {
        const auto mesh = scene->mMeshes[mi];

        for (unsigned fi = 0; fi < mesh->mNumFaces; ++fi)
        {
            const auto face = mesh->mFaces[fi];

            std::vector<glm::vec3> p;
            std::vector<glm::vec3> n;
            std::vector<glm::vec2> uv;
            for (unsigned i = 0; i < face.mNumIndices; ++i)
            {
                const auto idx = face.mIndices[i];

                auto pos = mesh->mVertices[idx];
                p.emplace_back(pos.x, pos.y, pos.z);

                auto normal = mesh->mNormals[idx];
                n.emplace_back(normal.x, normal.y, normal.z);

                if (mesh->HasTextureCoords(0))
                {
                    auto tex = mesh->mTextureCoords[0][idx];
                    uv.emplace_back(tex.x, tex.y);
                }
                else
                {
                    uv.emplace_back(0.0, 0.0);
                }
            }

            m_Triangles.emplace_back(
                p[0],
                p[1],
                p[2],
                n[0],
                n[1],
                n[2],
                uv[0],
                uv[1],
                uv[2],
                m_Materials.size() + mesh->mMaterialIndex);
        }
    }

    for (unsigned mi = 0; mi < scene->mNumMaterials; ++mi)
    {
        const auto material = scene->mMaterials[mi];

        aiColor3D diffuse, emission;
        auto roughness = 0.9f, metalic = 0.1f, opacity = 0.0f, ir = 1.5f;

        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        material->Get(AI_MATKEY_COLOR_EMISSIVE, emission);
        material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
        material->Get(AI_MATKEY_METALLIC_FACTOR, metalic);
        material->Get(AI_MATKEY_OPACITY, opacity);
        material->Get(AI_MATKEY_REFRACTI, ir);

        m_Materials.emplace_back(
            glm::vec3(diffuse.r, diffuse.g, diffuse.b),
            -1,
            glm::vec3(emission.r, emission.g, emission.b),
            -1,
            roughness,
            metalic,
            1.0f - opacity,
            ir);
    }

    importer.FreeScene();

    const auto root = GenerateBVHTree(first, m_Triangles.size(), 100);
    m_Models.emplace_back(root, glm::mat4(1.0f), glm::mat4(1.0f));
}

size_t pathtracer::Scene::GenerateBVHTree(const unsigned start, const unsigned end, const unsigned depth)
{
    glm::vec3 min{std::numeric_limits<float>::infinity()}, max{-std::numeric_limits<float>::infinity()};

    for (auto i = start; i < end; ++i)
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

    const auto longest_axis = dx > dy ? (dx >= dz ? 0 : 2) : dy >= dz ? 1 : 2;

    std::sort(
        m_Triangles.begin() + start,
        m_Triangles.begin() + end,
        [longest_axis](const Triangle &a, const Triangle &b)-> bool
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

pathtracer::Model &pathtracer::Scene::GetModel(const size_t i)
{
    return m_Models[i];
}

pathtracer::Model &pathtracer::Scene::GetLastModel()
{
    return m_Models.back();
}
