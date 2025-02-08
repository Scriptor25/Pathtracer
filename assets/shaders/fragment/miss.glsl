#version 450 core

#include "common.incl"

const vec3 sun_direction = normalize(vec3(0.3, 0.9, 0.4));
const vec3 sun_color = vec3(1.0, 0.9, 0.8);

vec3 Miss(in Ray ray) {
    #ifdef SKY
    float dt = dot(sun_direction, normalize(ray.direction));
    if (dt > 0.995) {
        return sun_color * 10.0;
    }

    const float a = ray.direction.y * 0.5 + 0.5;
    return (1.0 - a) + a * vec3(0.5, 0.7, 1.0);
    #else
    return vec3(0.0);
    #endif
}
