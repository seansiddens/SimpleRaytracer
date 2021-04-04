//
// Created by sean on 3/13/21.
//
#include "ray_math.h"

#ifndef ANOTHERRAYTRACER_SCENE_H
#define ANOTHERRAYTRACER_SCENE_H

typedef struct {
    vec3 color;
    double specular;
    double reflectivity;
} material;

typedef struct {
    vec3 center;
    double radius;
    material material;
} sphere;

typedef struct {
    vec3 normal;
    double D;
    material material;
} plane;

typedef struct {
    double intensity;
} ambient_light;

typedef struct {
    double intensity;
    vec3 position;
} point_light;

typedef struct {
    unsigned int number_of_planes;
    plane* planes;

    unsigned int number_of_spheres;
    sphere* spheres;

    ambient_light ambient_light;

    unsigned int number_of_point_lights;
    point_light* point_lights;
} world;

typedef struct {
    double t;
    vec3 hit_point;
    vec3 normal;
    material material;
} hit_rec;

#endif //ANOTHERRAYTRACER_SCENE_H
