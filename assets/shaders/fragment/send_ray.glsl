#version 450 core
#include "common.incl"

layout (binding = 2, std430) readonly buffer ModelBuffer {
    Model models[];
};

layout (binding = 3, std430) readonly buffer BVHBuffer {
    BVHNode nodes[];
};

bool models_hit(in Ray ray, in float ray_t_min, in float ray_t_max, inout Record rec) {

    Record tmp_rec;
    bool hit = false;

    uint stack[32];
    uint sptr = 0u;

    for (uint model_index = 0u; model_index < models.length(); ++model_index) {
        Model model = models[model_index];

        vec4 o = model.inverse_transform * vec4(ray.origin, 1.0);
        vec3 d = mat3(model.inverse_transform) * ray.direction;
        Ray tmp_ray = Ray(o.xyz / o.w, d);

        stack[sptr++] = model.root;

        while (sptr > 0u) {
            uint node_index = stack[--sptr];

            if (BVHNode_Hit(node_index, tmp_ray, ray_t_min, ray_t_max)) {
                BVHNode node = nodes[node_index];
                if (node.left == 0u) {
                    for (uint triangle_index = node.start; triangle_index < node.end; ++triangle_index) {
                        if (Triangle_Hit(triangle_index, tmp_ray, ray_t_min, ray_t_max, tmp_rec)) {
                            hit = true;
                            ray_t_max = tmp_rec.t;
                            rec = tmp_rec;
                            vec4 p = model.transform * vec4(rec.p, 1.0);
                            rec.p = p.xyz / p.w;
                        }
                    }
                } else {
                    stack[sptr++] = node.left;
                    stack[sptr++] = node.right;
                }
            }
        }
    }

    return hit;
}

vec3 SendRay(in Ray ray) {

    vec3 light = vec3(0.0);
    vec3 contribution = vec3(1.0);

    Record rec;
    for (int depth = 0; depth < 20; ++depth) {
        if (!models_hit(ray, 0.001, 1000.0, rec)) {
            // light += contribution * Miss(ray);
            break;
        }

        if (!Scatter(ray, rec, contribution, light)) {
            break;
        }
    }

    return light;
}
