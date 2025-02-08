#version 450 core
#include "common.incl"

layout (binding = 1, std430) readonly buffer MaterialBuffer {
    Material materials[];
};

bool refract(in vec3 v, in vec3 n, in float ni_over_nt, out vec3 refracted) {
    vec3 uv = normalize(v);
    float dt = dot(uv, n);
    float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1.0 - dt * dt);
    if (discriminant > 0) {
        refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
        return true;
    } else {
        return false;
    }
}

float schlick(in float cosine, in float ref_idx) {
    float r0 = (1.0 - ref_idx) / (1.0 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow((1.0 - cosine), 5.0);
}

bool Scatter(inout Ray ray, in Record rec, inout vec3 contribution, inout vec3 light) {
    Material mat = materials[rec.material];

    if (length(mat.emissive) > 0.0) {
        if (rec.front_face) light += contribution * mat.emissive;
        return false;
    }

    contribution *= mat.diffuse;

    vec3 reflected = reflect(ray.direction, rec.normal);

    vec3 direction;
    if (mat.transparency > 0.0) {
        float ni_over_nt = rec.front_face ? 1.0 / mat.ir : mat.ir;
        float cosine = -dot(ray.direction, rec.normal) / length(ray.direction);

        vec3 refracted;
        if (refract(ray.direction, rec.normal, ni_over_nt, refracted)) {
            float reflect_prob = schlick(cosine, mat.ir);
            if (Random() < reflect_prob) {
                direction = reflected;
            } else {
                direction = refracted;
            }
        } else {
            direction = reflected;
        }
    }
    else {
        if (mat.metalness < 1.0) {
            vec3 diffuse = rec.normal + RandomUnitVec3();
            direction = mix(diffuse, reflected, mat.metalness);
        }
        else {
            direction = reflected;
        }

        direction = normalize(direction + mat.roughness * RandomInUnitSphere());
    }

    ray.origin = rec.p;
    ray.direction = direction;

    return true;
}
