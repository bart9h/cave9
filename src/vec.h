#ifndef math_h_included
#define math_h_included

#include <SDL_opengl.h>
#include <GL/gl.h>

#ifndef __cplusplus
  #ifndef bool
    typedef unsigned char bool;
    #define false 0
    #define true  1
  #endif
#endif

#define EPSILON 0.000001

typedef GLfloat Vec3[3];

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))
#define CLAMP(x,y,z) ((x)<(y)?(y):((x)>(z)?(z):(x)))

// FIXME parenthize

#define RAND (rand()/(float)RAND_MAX)

#define COPY(a,b) \
	a[0]=b[0]; \
	a[1]=b[1]; \
	a[2]=b[2]; 

#define CROSS(a,b,c) \
	a[0]=b[1]*c[2]-b[2]*c[1]; \
	a[1]=b[2]*c[0]-b[0]*c[2]; \
	a[2]=b[0]*c[1]-b[1]*c[0];

#define DOT(a,b) \
	(a[0]*b[0]+a[1]*b[1]+a[2]*b[2])

#define ADD2(a,b,c) \
	a[0]=b[0]+c[0]; \
	a[1]=b[1]+c[1]; \
	a[2]=b[2]+c[2]; 

#define ADD(a,b) \
	a[0]+=b[0]; \
	a[1]+=b[1]; \
	a[2]+=b[2]; 

#define ADDSCALE(a,b,x) \
	a[0]+=b[0]*x; \
	a[1]+=b[1]*x; \
	a[2]+=b[2]*x; 

#define SUB2(a,b,c) \
	a[0]=b[0]-c[0]; \
	a[1]=b[1]-c[1]; \
	a[2]=b[2]-c[2]; 

#define SCALE(a,k) \
	a[0]*=k; \
	a[1]*=k; \
	a[2]*=k; 

#define SCALE2(a,b,k) \
	a[0]=b[0]*k; \
	a[1]=b[1]*k; \
	a[2]=b[2]*k; 

#define LEN(a) \
	sqrt(DOT(a,a))

#define SET(a,x,y,z) \
	a[0]=x; \
	a[1]=y; \
	a[2]=z; 

#if 1
#define NORM(a) { \
	float invlen = 1/LEN(a); \
	SCALE(a,invlen); }
#else
#define NORM(a) { \
	float x = DOT(a,a);
	float xhalf = 0.5f*x;
	int i = *(int*)&x;        // get bits for floating value
	i = 0x5f3759df - (i>>1);  // gives initial guess y0
	x = *(float*)&i;          // convert bits back to float
	x = x*(1.5f-xhalf*x*x);   // Newton step, repeating increases accuracy
	SCALE(a,x); }
#endif

#endif

