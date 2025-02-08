#version 450 core

#include "common.incl"

vec3 Ray_At(in Ray self, in float t) {
    return self.origin + t * self.direction;
}
