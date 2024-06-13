#version 450 core

#include "common.incl"

vec3 Miss(in Ray ray) {
    const float a = ray.direction.y * 0.5 + 0.5;
    return (1.0 - a) * vec3(1.0) + a * vec3(0.5, 0.7, 1.0);
}
