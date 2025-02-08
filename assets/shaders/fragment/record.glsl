#version 450 core

#include "common.incl"

void Record_SetNormal(inout Record self, in Ray ray, in vec3 outward_normal) {
    self.front_face = dot(ray.direction, outward_normal) < 0.0;
    self.normal = self.front_face ? outward_normal : -outward_normal;
}
