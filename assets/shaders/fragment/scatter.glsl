#version 450 core

#include "common.incl"

layout (binding = 1, std430) readonly buffer MaterialBuffer {
    Material materials[];
};

float schlick(in float cosine, in float ref_idx) {
    float r0 = (1.0 - ref_idx) / (1.0 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow((1.0 - cosine), 5.0);
}

vec3 get_direction(in Ray ray, in Record rec) {
    vec3 reflected = reflect(ray.direction, rec.normal);

    Material mat = materials[rec.material];
    if (mat.transparency > 0.0) {
        float eta = rec.front_face ? 1.0 / mat.ir : mat.ir;
        vec3 refracted = refract(ray.direction, rec.normal, eta);

        if (refracted == vec3(0.0)) {
            return reflected;
        }

        float cosine = -dot(ray.direction, rec.normal) / length(ray.direction);
        float probability = Random();
        if (probability > schlick(cosine, mat.ir)) {
            return refracted;
        }

        return reflected;
    }

    vec3 direction = reflected;
    if (mat.metalness < 1.0) {
        vec3 offset = rec.normal + RandomUnitVec3();
        direction = mix(offset, direction, mat.metalness);
    }

    return normalize(direction + mat.roughness * RandomInUnitSphere());
}

bool Scatter(inout Ray ray, in Record rec, inout vec3 contribution, inout vec3 light) {
    Material mat = materials[rec.material];

    if (length(mat.emissive) > 0.0) {
        if (rec.front_face) {
            light += contribution * mat.emissive;
        }
        return false;
    }

    contribution *= mat.diffuse;

    vec3 direction = get_direction(ray, rec);

    ray.origin = rec.p;
    ray.direction = direction;

    return true;
}
