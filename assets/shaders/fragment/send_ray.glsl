#version 450 core
#include "common.incl"

vec3 SendRay(in Scene scene, in Ray ray) {

    vec3 light = vec3(0.0);
    vec3 contribution = vec3(1.0);

    Record rec;
    for (int depth = 0; depth < 5; ++depth) {
        if (!Scene_Hit(scene, ray, Interval(0.001, 1000.0), rec)) {
            light += contribution * Miss(ray);
            break;
        }

        contribution *= Hit(ray, rec);

        ray.origin = rec.p;
        ray.direction = normalize(rec.normal + RandomInUnitSphere());
    }

    return light;
}
