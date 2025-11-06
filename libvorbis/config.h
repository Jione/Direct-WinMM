/* config.h */

/* define Forced SSE2 acceleration on x86 */
#ifdef _WIN64
#ifdef _OS_H
#ifndef _CONFIG_H
#define _CONFIG_H
#  define VORBIS_FPU_CONTROL

typedef ogg_int16_t vorbis_fpu_control;

#include <emmintrin.h>
static __inline int vorbis_ftoi(double f){
        return _mm_cvtsd_si32(_mm_load_sd(&f));
}

static __inline void vorbis_fpu_setround(vorbis_fpu_control *fpu){
  (void)fpu;
}

static __inline void vorbis_fpu_restore(vorbis_fpu_control fpu){
  (void)fpu;
}
#endif
#endif
#endif
