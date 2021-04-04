#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image/stb_image_write.h"

#include "ray_math.h"
#include "scene.h"


const int image_width = 1280;
const int image_height = 720;
const double aspect_ratio = (double)image_width / (double)image_height;
const double viewport_height = 2.0;
const double viewport_width = aspect_ratio * viewport_height;
const double focal_length = 1.0;


// Writes a normalized RGB color to specified pixel in image buffer
void put_pixel(char* img, int x, int y, double i) {
    int offset = 3 * (y * image_width + x);

    char r_final = i * 255.;
    char g_final = i * 255.;
    char b_final = i * 255.;

    img[offset] = r_final;
    img[offset+1] = g_final;
    img[offset+2] = b_final;
}

// For a given (x, y) on the image canvas, return the corresponding point on the viewport plane
vec3 canvas_to_viewport(int x, int y) {
    // Camera space is oriented so it is looking toward the -Z direction, with
    double horizontal = viewport_width;
    double vertical = viewport_height;
    vec3 lower_left_corner = {-viewport_width/2.0, -vertical/2.0, -1.0};

    double u = (double)x / (double)(image_width-1);
    double v = (double)y / (double)(image_height-1);
    v = 1.0 - v;

    vec3 p = {lower_left_corner.x + u * horizontal,
              lower_left_corner.y + v * vertical,
              lower_left_corner.z};
    return p;
}

hit_rec intersect_ray_sphere(sphere sphere, vec3 ro, vec3 rd, double t_min, double t_max) {
    hit_rec hit;
    hit.t = INFINITY;
    double r = sphere.radius;
    vec3 p = sphere.center;
    vec3 co = {ro.x - p.x, ro.y - p.y, ro.z - p.z};

    double a = dot(rd, rd);
    double b = 2 * dot(co, rd);
    double c = dot(co, co) - r * r;

    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return hit;
    }

    double t1 = (-b + sqrt(discriminant)) / 2 * a;
    double t2 = (-b - sqrt(discriminant)) / 2 * a;

    if (t1 < hit.t) {
        hit.t = t1;
    }
    if (t2 < hit.t) {
        hit.t = t2;
    }
    if ((hit.t > t_min) && (hit.t < t_max)) {
        vec3 p = {ro.x + hit.t * rd.x,
                  ro.y + hit.t * rd.y,
                  ro.z + hit.t * rd.z};
        vec3 c = sphere.center;
        vec3 n = {p.x - c.x, p.y - c.y, p.z - c.z};
        hit.hit_point = p;
        hit.normal = normalize(n);
        hit.material = sphere.material;
        return hit;
    }
    else {
        hit.t = INFINITY;
        return hit;
    }

}

hit_rec intersect_ray_plane(plane plane, vec3 ro, vec3 rd, double t_min, double t_max) {
    hit_rec hit;

    double D = plane.D;
    vec3 N = plane.normal;
    double tolerance = 0.0001f;
    double denominator = dot(N, rd);
    if (denominator != 0) {
        hit.t = (-D - dot(N, ro)) / denominator;
    }
    else {
        hit.t = INFINITY;
        return hit;
    }

    if ((hit.t > t_min) && (hit.t < t_max)) {
        hit.normal = N;
        hit.material = plane.material;
        vec3 p = {ro.x + hit.t * rd.x,
                  ro.y + hit.t * rd.y,
                  ro.z + hit.t * rd.z};
        hit.hit_point = p;
        return hit;
    }
    else {
        hit.t = INFINITY;
        return hit;
    }
}

// Traces ray and returns color
double trace_ray(world* world, vec3 ro, vec3 rd, double t_min, double t_max) {
    // Set initial color to black
    double col = 0;

    // Iterate over every primitive and find closest hit
    hit_rec closest_hit;
    closest_hit.t = INFINITY;
    // Spheres
    for (int i = 0; i < world->number_of_spheres; i++) {
        sphere s = world->spheres[i];
        hit_rec hit = intersect_ray_sphere(s, ro, rd, t_min, t_max);
        if (hit.t < closest_hit.t) {
            closest_hit = hit;
        }
    }
    // Planes
    for (int i = 0; i < world->number_of_planes; i++) {
        plane p = world->planes[i];
        hit_rec hit = intersect_ray_plane(p, ro, rd, t_min, t_max);
        if (hit.t < closest_hit.t) {
            closest_hit = hit;
        }
    }
    // If don't hit anything, return background color
    if (closest_hit.t == INFINITY) {
        return col;
    }

    vec3 p = closest_hit.hit_point;   // Point of hit
    vec3 N = closest_hit.normal;      // Normal of where we hit
    // Vector from point we hit back to the ray origin
    vec3 view_ray = {ro.x - p.x, ro.y - p.y, ro.z - p.z};
    col = closest_hit.material.color.x; // Color of material where we hit

    // Calculate lighting at point
    double intensity = 0.0;

    // Add ambient light contribution
    intensity += world->ambient_light.intensity;

    // Compute point light contribution
    for (int i = 0; i < world->number_of_point_lights; i++) {
        point_light light = world->point_lights[i];
        // Vector from hit point to light position
        vec3 L = {light.position.x - p.x,
                  light.position.y - p.y,
                  light.position.z - p.z};
        // Diffuse
        double N_dot_L = dot(N, L);
        if (N_dot_L > 0) {
            intensity += light.intensity * N_dot_L / (length(N) * length(L));
        }
        // Specular
        if (closest_hit.material.specular != -1) {
            vec3 R = reflect_ray(L, N);
            double R_dot_view_ray = dot(R, view_ray);
            if (R_dot_view_ray > 0) {
                double specular_term = pow(R_dot_view_ray / (length(R) * length(view_ray)),
                                           closest_hit.material.specular);
                intensity += light.intensity * specular_term;
            }
        }
    }

    // Multiply color by clamped light intensity
    intensity = clamp(intensity, 0.0, 1.0);
    col *= intensity;
    return col;
}

int main() {
    char img[image_width * image_height * 3]; // Declare image buffer
    memset(img, 0, sizeof(img));

    // Scene setup
    material mat1 = {{0.8, 0.5, 0.5}, 500, 0.2};
    material mat2 = {{0.3,  0.4, 0.4}, 10, 0.4};

    world world;

    world.number_of_planes = 0;
    plane planes[world.number_of_planes];
    world.planes = planes;
    plane p = {{0, 0, 0}, 20, mat2};
    planes[0] = p;

    world.number_of_spheres = 1;
    sphere spheres[world.number_of_spheres];
    world.spheres = spheres;
    sphere s0 = {{0, 1, -3}, 1.0, mat1};
    sphere s1 = {{0, -1000, -14}, 1000.0, mat2};
    spheres[0] = s0;
    spheres[1] = s1;

    world.ambient_light.intensity = 0.09;

    world.number_of_point_lights = 1;
    point_light point_lights[world.number_of_point_lights];
    world.point_lights = point_lights;
    point_light pl1 = {0.6, {3, 5, 0}};
    point_lights[0] = pl1;

    vec3 camera_origin = {0, 0, 0};

    // Cast initial rays
    for (int y = 0; y < image_height; y++) {
        for (int x = 0; x < image_width; x++) {
            vec3 p = canvas_to_viewport(x, y);          // Point on viewport
            vec3 ray_dir = {p.x - camera_origin.x,
                            p.y - camera_origin.y,
                            p.z - camera_origin.z};
            ray_dir = normalize(ray_dir);

            double c = trace_ray(&world, camera_origin, ray_dir, 1, INFINITY); // Color from tracing ray

            put_pixel(img, x, y, c); // Write color to image
        }
    }

    // Write out image data to file
    if (stbi_write_bmp("out.bmp", image_width, image_height, 3, img) == 0) {
        printf("ERROR: Failed to write out to image file!");
    }

    return 0;
}
