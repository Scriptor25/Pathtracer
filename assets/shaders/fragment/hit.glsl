#version 450 core
#include "common.incl"

vec3 lambertian(in Ray ray, in Record rec, in Material mat) {
    return mix(reflect(ray.direction, rec.normal), normalize(rec.normal + RandomInUnitSphere()), mat.roughness);
}

vec3 metallic(in Ray ray, in Record rec, in Material mat) {
    return reflect(ray.direction, rec.normal) + mat.roughness * normalize(RandomVec3(-1.0, 1.0));
}

void Hit(in Scene scene, inout Ray ray, in Record rec, inout vec3 contribution, inout vec3 light) {
    Material mat = scene.materials[rec.material];

    contribution *= mat.albedo;
    light += mat.emission;

    ray.origin = rec.p;
    ray.direction = mix(lambertian(ray, rec, mat), metallic(ray, rec, mat), mat.metallic);
}
