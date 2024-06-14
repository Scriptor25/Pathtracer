#version 450 core
#include "common.incl"

layout (binding = 0, std430) readonly buffer TriangleBuffer {
    Triangle triangles[];
};

bool scene_hit(in Ray ray, in Interval ray_t, inout Record rec) {
    Record tmp_rec;
    bool hit = false;

    for (int i = 0; i < triangles.length(); ++i) {
        if (Triangle_Hit(triangles[i], ray, ray_t, tmp_rec)) {
            hit = true;
            ray_t.max = tmp_rec.t;
            rec = tmp_rec;
        }
    }

    return hit;
}

const vec3 sun_direction = normalize(vec3(0.1, 1.0, 0.4));
const vec3 sun_color = vec3(0.90, 0.87, 0.80);

vec3 SendRay(in Ray ray) {

    vec3 light = vec3(0.0);
    vec3 contribution = vec3(1.0);

    Record rec;
    for (int depth = 0; depth < 50; ++depth) {
        if (!scene_hit(ray, Interval(0.001, 1000.0), rec)) {
            // light += contribution * Miss(ray);
            break;
        }

        if (!Scatter(ray, rec, contribution, light)) {
            break;
        }

        Ray sun_ray = Ray(rec.p, sun_direction);
        if (!scene_hit(sun_ray, Interval(0.001, 1000.0), rec)) {
            light += contribution * sun_color * max(dot(sun_direction, rec.normal), 0.0);
        }
    }

    return light;
}
