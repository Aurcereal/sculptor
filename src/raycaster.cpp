#include "marching_cubes_manager.h"
#include <iostream>

// class Raycaster {
//         struct Intersection {
//             vec3 pos;
//             vec3 norm;
//         };

//         public:
//         Intersection RayFieldIntersect(vec3 origin, vec3 direction);  
//     };

MarchingCubes::Raycaster::Intersection MarchingCubes::Raycaster::RayFieldIntersect(vec3 origin, vec3 direction) {
    return {vec3(0.0f), vec3(0.0f)};
}