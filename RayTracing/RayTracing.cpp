#include "header/color.h"
#include "header/myUtility.h"
#include "header/hittable.h"
#include "header/hittable_list.h"
#include "header/sphere.h"
#include "header/camera.h"
#include "header/material.h"

#include <iostream>
#include <fstream>
using namespace std;

color ray_color(const ray& r, const hittable& world, int depth)
{
    hit_record rec;
    if (depth <= 0)
    {
        return color(0.0, 0.0, 0.0);
    }
    //avoid float number compare precision problem
    //这个操作能带来非常大的改善
    //因为在算bounce的光线时，ray的起点一定在world内物体上,所以理论上一定有一个精确解为0.0
    //但是因为浮点数之间的比较局限于精度原因，会得到不正确的t值
    //我们需要设置一个tolerance去摆脱这种浮点数精度导致的噪点
    if (world.hit(r, 0.0001, infinity, rec))
    { 
        //true lambertian reflection
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        {
            return attenuation * ray_color(scattered, world, depth - 1);
        }
        return color(0);
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

hittable_list random_scene() {
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return world;
}

int main()
{
    //Image
    ofstream out("ray tracing in one weekend.ppm", ios::out);
    const auto aspect_ratio = 3.0 / 2.0;
    const int image_width = 1200;
    const int image_height = static_cast<int> (image_width / aspect_ratio);
    //For Multiple Sampling Anti-Aliasing
    int samples_per_pixel = 500;
    const int max_depth = 50;

    //World
    hittable_list world = random_scene();

 

    //setting Camera
    point3 lookfrom(13, 2, 3);
    point3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;

    camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

    if (out.is_open())
    {
        out << "P3\n" << image_width << " " << image_height << "\n255\n";
        for (int j = image_height - 1; j >= 0; --j)
        {
            std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
            for (int i = 0; i < image_width; ++i)
            {
                color pixel_color(0, 0, 0);
                for (int s = 0; s < samples_per_pixel; ++s) {
                    auto u = (i + random_double()) / (image_width - 1);
                    auto v = (j + random_double()) / (image_height - 1);
                    ray r = cam.get_ray(u, v);
                    pixel_color += ray_color(r, world, max_depth);
                }
                write_color(out, pixel_color, samples_per_pixel);
            }
        }
        out.close();
    }
    std::cerr << "\nDone.\n";
    return 0;
}



