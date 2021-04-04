//
// Created by sean on 3/13/21.
//

#ifndef ANOTHERRAYTRACER_RAY_MATH_H
#define ANOTHERRAYTRACER_RAY_MATH_H

// STRUCT DEFS
typedef struct {
    double x, y, z;
} vec3;

// FUNCTION DEFS
double clamp(double n, double min, double max) {
    if (n < min) {
        return min;
    }
    if (n > max) {
        return max;
    }
    return n;
}
double dot(vec3 u, vec3 v) {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

double length(vec3 u) {
    return sqrt(dot(u, u));
}

vec3 normalize(vec3 u) {
    double l = length(u);
    u.x /= l;
    u.y /= l;
    u.z /= l;
    return u;
}

// Takes a ray u and a normal n and a vector p
// reflected with respect to n
vec3 reflect_ray(vec3 r, vec3 n) {
    double w = dot(r, n);
    vec3 p = {2 * n.x * w - r.x,
              2 * n.y * w - r.y,
              2 * n.z * w - r.z};
    return p;
}


#endif //ANOTHERRAYTRACER_RAY_MATH_H
