/* This file is released in public domain. */

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))
#define CLAMP(x,y,z) ((x)<(y)?(y):((x)>(z)?(z):(x)))

#define RAND (rand()/(float)RAND_MAX)

/* WARNING: returns a fixed buffer, not reentrant */
/* paths[] must be NULL-terminated */
const char* find_file (const char* basename, const char* paths[], bool required);

void arabic (char *buf, unsigned int n);
void roman  (char *buf, unsigned int n);

#define NUMBER_STR_MAX 80

// FIXME what is the license?
// ######################################################################
// http://ilab.usc.edu/wiki/index.php/HSV_And_H2SV_Color_Space
// T. Nathan Mundhenk
// mundhenk@usc.edu
// C/C++ Macro HSV to RGB
#define PIX_HSV_TO_RGB_COMMON(H,S,V,R,G,B)                          \
if( V == 0 )                                                        \
{ R = 0; G = 0; B = 0; }                                            \
else if( S == 0 )                                                   \
{                                                                   \
  R = V;                                                            \
  G = V;                                                            \
  B = V;                                                            \
}                                                                   \
else                                                                \
{                                                                   \
  const double hf = H / 60.0;                                       \
  const int    i  = (int) floor( hf );                              \
  const double f  = hf - i;                                         \
  const double pv  = V * ( 1 - S );                                 \
  const double qv  = V * ( 1 - S * f );                             \
  const double tv  = V * ( 1 - S * ( 1 - f ) );                     \
  switch( i )                                                       \
    {                                                               \
    case 0:                                                         \
      R = V;                                                        \
      G = tv;                                                       \
      B = pv;                                                       \
      break;                                                        \
    case 1:                                                         \
      R = qv;                                                       \
      G = V;                                                        \
      B = pv;                                                       \
      break;                                                        \
    case 2:                                                         \
      R = pv;                                                       \
      G = V;                                                        \
      B = tv;                                                       \
      break;                                                        \
    case 3:                                                         \
      R = pv;                                                       \
      G = qv;                                                       \
      B = V;                                                        \
      break;                                                        \
    case 4:                                                         \
      R = tv;                                                       \
      G = pv;                                                       \
      B = V;                                                        \
      break;                                                        \
    case 5:                                                         \
      R = V;                                                        \
      G = pv;                                                       \
      B = qv;                                                       \
      break;                                                        \
    case 6:                                                         \
      R = V;                                                        \
      G = tv;                                                       \
      B = pv;                                                       \
      break;                                                        \
    case -1:                                                        \
      R = V;                                                        \
      G = pv;                                                       \
      B = qv;                                                       \
      break;                                                        \
    default:                                                        \
      break;                                                        \
    }                                                               \
}


