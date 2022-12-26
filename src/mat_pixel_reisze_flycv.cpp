// adopting FlyCV's resize implementation here
// replace ncnn one
// looks like ncnn resize is a little slow
#include "mat.h"

#include <limits.h>
#include <math.h>
#if __ARM_NEON
#include <arm_neon.h>
#endif // __ARM_NEON
#include "platform.h"

namespace sim {

namespace mcv {


// Resize bilinear
void resize_bilinear_c1(const unsigned char *src, int srcw, int srch,
                        unsigned char *dst, int w, int h) {
  return resize_bilinear_c1(src, srcw, srch, srcw, dst, w, h, w);
}

void resize_bilinear_c2(const unsigned char *src, int srcw, int srch,
                        unsigned char *dst, int w, int h) {
  return resize_bilinear_c2(src, srcw, srch, srcw * 2, dst, w, h, w * 2);
}

void resize_bilinear_c3(const unsigned char *src, int srcw, int srch,
                        unsigned char *dst, int w, int h) {
  return resize_bilinear_c3(src, srcw, srch, srcw * 3, dst, w, h, w * 3);
}

void resize_bilinear_c4(const unsigned char *src, int srcw, int srch,
                        unsigned char *dst, int w, int h) {
  return resize_bilinear_c4(src, srcw, srch, srcw * 4, dst, w, h, w * 4);
}

// implementation here
void resize_bilinear_c1(const unsigned char *src, int srcw, int srch,
                        int srcstride, unsigned char *dst, int w, int h,
                        int stride) {}

void resize_bilinear_c2(const unsigned char *src, int srcw, int srch,
                        int srcstride, unsigned char *dst, int w, int h,
                        int stride) {}
void resize_bilinear_c3(const unsigned char *src, int srcw, int srch,
                        int srcstride, unsigned char *dst, int w, int h,
                        int stride) {}
void resize_bilinear_c4(const unsigned char *src, int srcw, int srch,
                        int srcstride, unsigned char *dst, int w, int h,
                        int stride) {}

// Resize nearest

} // namespace mcv

} // namespace sim
