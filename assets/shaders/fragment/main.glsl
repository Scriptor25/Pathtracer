#version 450 core

#include "common.incl"

layout (location = 0) in vec2 Sample;
layout (location = 0) out vec4 Color;

uniform layout (binding = 0, rgba32f) image2D Accumulation;

uniform uint SampleCount;
uniform uint MaxSampleCount = 20000u;
uniform vec3 Origin;
uniform mat4 CameraToWorld;
uniform mat4 ScreenToCamera;

void main() {

    ivec2 pixel_coord = ivec2(gl_FragCoord.xy);
    vec3 accum = imageLoad(Accumulation, pixel_coord).rgb;

    if (SampleCount >= MaxSampleCount) {
        Color = vec4(accum / MaxSampleCount, 1.0);
        return;
    }

    vec2 pixel_delta = 1.0 / vec2(imageSize(Accumulation));
    Seed(uint(pixel_coord.x + pixel_coord.y / pixel_delta.x) * SampleCount);

    uint sqrt_max_samples = uint(sqrt(float(MaxSampleCount)));
    float inv_sqrt_max_samples = 1.0 / float(sqrt_max_samples - 1);
    float sample_i = float(SampleCount % sqrt_max_samples) * inv_sqrt_max_samples - 0.5;
    float sample_j = float(SampleCount / sqrt_max_samples) * inv_sqrt_max_samples - 0.5;

    vec4 target = ScreenToCamera * vec4(Sample + vec2(sample_i, sample_j) * pixel_delta, 1.0, 1.0);
    vec3 ray_direction = mat3(CameraToWorld) * (normalize(target.xyz) / target.w);
    Ray ray = Ray(Origin, normalize(ray_direction));

    vec3 ray_color = SendRay(ray);
    if (ray_color.r < 0.0 || ray_color.r != ray_color.r) ray_color.r = 0.0;
    if (ray_color.g < 0.0 || ray_color.g != ray_color.g) ray_color.g = 0.0;
    if (ray_color.b < 0.0 || ray_color.b != ray_color.b) ray_color.b = 0.0;
    accum += ray_color;
    imageStore(Accumulation, pixel_coord, vec4(accum, 1.0));

    Color = vec4(accum / SampleCount, 1.0);
}
