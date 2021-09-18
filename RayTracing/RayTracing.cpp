#include "header/color.h"
#include "header/myUtility.h"
#include "header/hittable_list.h"
#include "header/sphere.h"
#include "header/camera.h"
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
    if (world.hit(r, 0.001, infinity, rec))
    { 
        //true lambertian reflection
        point3 target = rec.p + random_in_hemisphere(rec.normal);
        return 0.5 * ray_color(ray(rec.p, target - rec.p), world, depth-1);
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}


int main()
{
    //Image
    ofstream out("diffuse sphere with hemisphere ditribution.ppm", ios::out);
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = 400;
    const int image_height = static_cast<int> (image_width / aspect_ratio);
    //For Multiple Sampling Anti-Aliasing
    int samples_per_pixel = 100;
    const int max_depth = 50;

    //World
    hittable_list world;
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

    //setting Camera
    camera cam;

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



