#version 450 core
#include "common.incl"

uint seed;

uint hash(const uint seed)
{
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

void Seed(in uint _seed) {
    seed = _seed;
}

float Random()
{
    seed = hash(seed);
    return float(seed) / float(0xffffffffu);
}

float Random(in float min, in float max) {
    return min + (max - min) * Random();
}

vec2 RandomVec2() {
    return vec2(Random(), Random());
}

vec2 RandomVec2(in float min, in float max) {
    return vec2(Random(min, max), Random(min, max));
}

vec3 RandomVec3() {
    return vec3(Random(), Random(), Random());
}

vec3 RandomVec3(in float min, in float max) {
    return vec3(Random(min, max), Random(min, max), Random(min, max));
}

vec3 RandomInUnitSphere() {
    vec3 v;
    for (int i = 0; i < 10; ++i) {
        v = RandomVec3(-1.0, 1.0);
        if (dot(v, v) > 1.0) {
            return v;
        }
    }
    return normalize(v);
}
