
#define MAX_TRIANGLES 200
#define MAX_SPHERES 10
#define MAX_LIGHTS 10

#define ERROR .000001

// -----------------------------------------------
// Rendering quality controls

#define RECURSIVE_SPECULAR 1
#define TRANSLUCENCY 1
#define ATTENUATION 1
#define ANTI_ALIASING 1
#define AA_LEVEL 2

// ---------------------------------------------------------



// Note: Higher resolutions settings cancel out lower
#define REALHIGHREZ 0
#define HIREZ 0
#define LOREZ 0
#define ULTRALOREZ 0

// -----------------------------------------------


//you may want to make these smaller for debugging purposes
#if NDEBUG
#if REALHIGHREZ
#define WIDTH 1920
#define HEIGHT 1080
#elif HIREZ
#define WIDTH 1280
#define HEIGHT 960
#elif LOREZ
#define WIDTH 320
#define HEIGHT 240
#elif ULTRALOREZ
#define WIDTH 160
#define HEIGHT 120
#else
#define WIDTH 640
#define HEIGHT 480
#endif
#else
#define WIDTH 320//160
#define HEIGHT 240//120

#define ANTI_ALIASING 0
#endif

//the field of view of the camera
#define fov 60.0
