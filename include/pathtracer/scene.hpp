#pragma once

#include <filesystem>
#include <vector>
#include <glm/glm.hpp>
#include <pathtracer/buffer.hpp>

namespace path_tracer
{
    struct Triangle
    {
        [[nodiscard]] glm::vec3 Center() const;

        void AddBounds(glm::vec3 &min, glm::vec3 &max) const;

        alignas(16) glm::vec3 P0;
        alignas(16) glm::vec3 P1;
        alignas(16) glm::vec3 P2;
        alignas(16) glm::vec3 N0;
        alignas(16) glm::vec3 N1;
        alignas(16) glm::vec3 N2;
        alignas(8) glm::vec2 UV0;
        alignas(8) glm::vec2 UV1;
        alignas(8) glm::vec2 UV2;
        alignas(4) unsigned Material;
    };

    struct BVHNode
    {
        alignas(16) glm::vec3 Min;
        alignas(16) glm::vec3 Max;
        alignas(4) unsigned Left;
        alignas(4) unsigned Right;
        alignas(4) unsigned Start;
        alignas(4) unsigned End;
    };

    struct Material
    {
        alignas(16) glm::vec3 Diffuse;
        alignas(4) int DiffuseTexture;
        alignas(16) glm::vec3 Emission;
        alignas(4) int EmissionTexture;
        alignas(4) float Roughness;
        alignas(4) float Metalic;
        alignas(4) float Transparency;
        alignas(4) float IR;
    };

    struct Model
    {
        alignas(4) unsigned Root;
        alignas(16) glm::mat4 Transform;
        alignas(16) glm::mat4 InverseTransform;
        alignas(16) glm::mat3 NormalTransform;
    };

    class Scene
    {
    public:
        Scene();

        void LoadModel(const std::filesystem::path &path, unsigned int flags);

        size_t GenerateBVHTree(unsigned start, unsigned end, unsigned depth);

        void Upload() const;

        Model &GetModel(size_t i);

        Model &GetLastModel();

    private:
        std::vector<Triangle> m_Triangles;
        std::vector<Material> m_Materials;
        std::vector<Model> m_Models;
        std::vector<BVHNode> m_BVHNodes;

        Buffer m_TriangleBuffer;
        Buffer m_MaterialBuffer;
        Buffer m_ModelBuffer;
        Buffer m_BVHNodeBuffer;
    };
}
