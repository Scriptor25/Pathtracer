#version 450 core
#include "common.incl"

bool Sphere_Hit(in Sphere self, in Ray ray, in float ray_t_min, in float ray_t_max, inout Record rec) {

    vec3 oc = self.center - ray.origin;
    float a = dot(ray.direction, ray.direction);
    float b = dot(ray.direction, oc);
    float c = dot(oc, oc) - self.radius * self.radius;
    float discriminant = b * b - a * c;

    if (discriminant < 0.0) {
        return false;
    }

    float sqrtd = sqrt(discriminant);
    float inva = 1.0 / a;

    float root = (b - sqrtd) * inva;
    if (!(ray_t_min <= root && root <= ray_t_max)) {
        root = (b + sqrtd) * inva;
        if (!(ray_t_min <= root && root <= ray_t_max)) {
            return false;
        }
    }

    rec.t = root;
    rec.p = Ray_At(ray, root);
    vec3 outward_normal = (rec.p - self.center) / self.radius;
    Record_SetNormal(rec, ray, outward_normal);
    rec.material = self.material;

    return true;
}
