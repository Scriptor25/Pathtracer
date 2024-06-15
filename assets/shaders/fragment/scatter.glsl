#version 450 core
#include "common.incl"

layout (binding = 1, std430) readonly buffer MaterialBuffer {
    Material materials[];
};

bool Scatter(inout Ray ray, in Record rec, inout vec3 contribution, inout vec3 light) {
    Material mat = materials[rec.material];

    if (mat.emissive.length() > 0.0) {
        light += contribution * mat.emissive;
        return false;
    }

    contribution *= mat.diffuse;

    vec3 direction = normalize(rec.normal + RandomUnitVec3());
    if (direction.length() < 1e-5) direction = rec.normal;

    ray.origin = rec.p;
    ray.direction = direction;

    return true;
}
