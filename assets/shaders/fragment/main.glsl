#version 450 core
#include "common.incl"

layout (location = 0) in vec2 Sample;
layout (location = 0) out vec4 Color;

uniform layout (binding = 0, rgba32f) image2D Accumulation;

uniform uint SampleCount;
uniform uint MaxSampleCount = 2000u;
uniform vec3 Origin;
uniform mat4 CameraToWorld;
uniform mat4 ScreenToCamera;

void main() {

    Scene scene;
    scene.spheres[scene.sphere_count++] = Sphere(vec3(0.0, 0.5, 0.0), 0.5, 1);
    scene.spheres[scene.sphere_count++] = Sphere(vec3(-1.0, 0.5, 0.0), 0.5, 2);
    scene.spheres[scene.sphere_count++] = Sphere(vec3(1.0, 0.5, 0.0), 0.5, 3);

    float s = 10.0;
    scene.triangles[scene.triangle_count++] = Triangle(vec3(-s, 0.0, -s), vec3(-s, 0.0, s), vec3(s, 0.0, s), 0);
    scene.triangles[scene.triangle_count++] = Triangle(vec3(s, 0.0, s), vec3(s, 0.0, -s), vec3(-s, 0.0, -s), 0);

    scene.materials[scene.material_count++] = Material(vec3(0.8, 0.8, 0.0), vec3(0.0), 1.0, 0.0);
    scene.materials[scene.material_count++] = Material(vec3(0.1, 0.2, 0.5), vec3(0.0), 1.0, 0.0);
    scene.materials[scene.material_count++] = Material(vec3(0.8, 0.6, 0.2), vec3(0.0), 1.0, 1.0);
    scene.materials[scene.material_count++] = Material(vec3(0.8, 0.8, 0.8), vec3(0.0), 0.3, 1.0);

    ivec2 pixel_coord = ivec2(gl_FragCoord.xy);
    vec3 accum = imageLoad(Accumulation, pixel_coord).rgb;

    if (SampleCount >= MaxSampleCount) {
        Color = vec4(accum / MaxSampleCount, 1.0);
    }
    else {
        vec2 pixel_delta = 1.0 / vec2(imageSize(Accumulation));
        Seed(uint(pixel_coord.x + pixel_coord.y / pixel_delta.x) * SampleCount);

        vec4 target = ScreenToCamera * vec4(Sample + RandomVec2(-1.0, 1.0) * pixel_delta, 1.0, 1.0);
        vec3 ray_direction = mat3(CameraToWorld) * (normalize(target.xyz) / target.w);
        Ray ray = Ray(Origin, normalize(ray_direction));

        accum += SendRay(scene, ray);
        imageStore(Accumulation, pixel_coord, vec4(accum, 1.0));

        Color = vec4(accum / SampleCount, 1.0);
    }
}
