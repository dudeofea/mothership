#ifndef _EFFECTS_H_
#define _EFFECTS_H_
#include <fftw3.h>
#include <math.h>

#define BUFFER_LEN 1024

int run_effects(float* in, float* out, fftw_complex* fft_out);

#endif
