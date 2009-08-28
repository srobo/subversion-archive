/* This file contains eye supliments designed to strengthen the surface of your
 * eyeballs against pokeage, which will save you a lot of pain in the future.
 * This is because looking at robovis code tends to make your eyeballs swell
 * to the size where forks and other sharp cutlery is attracted to your face
 * by gravity.
 *
 * In case of mis-use, consult your doctor
 */

#include <stdio.h>
#include <math.h>

#include <cv.h>

#include "visfunc.h"

/* Ensure that "MAX" is what we expect it to be */
#if defined(MAX)
#undef MAX
#endif
#define        MAX(a,b)        (((a)>(b))?(a):(b))

#define template_size 5
#define image_depth 8

unsigned char bias[template_size][template_size];

void
vis_start()
{
	static int started = 0;
	int x, y, i, j;
	double calc;

	if (started)
		return;

	started = 1;

	x =  -((template_size-1)/2);
	y =  -((template_size-1)/2);
	for (i = 0; i < template_size; i++, x++) {
		for (j = 0; j < template_size; j++, y++) {
			calc = (x * x) + (y * y);
			calc /= 2;
			calc = exp(-calc);
			bias[i][j] = (unsigned char) (calc * 255);
		}
		y =  -((template_size-1)/2);
	}

	return;
}

IplImage *
vis_do_smooth(IplImage *src)
{
#define get(x, y) *(in + ((y) * in_stride) + (x))
#define put(x, y) *(out + ((y) * out_stride) + (x))
	CvSize sz;
	IplImage *dst;
	unsigned char *in;
	unsigned char *out;
	int i, j, k, l, x, y, in_stride, out_stride;
	int template_border;
	int accumul;

	vis_start();

	if (src->nChannels != 1) {
		fprintf(stderr, "vis_do_smooth: bad channels %d\n",
					src->nChannels);
		exit(1);
	}

	sz.width = src->width - (template_size-1);
	sz.height = src->height - (template_size-1);

	dst = cvCreateImage(sz, image_depth, 1);
	if (!dst) {
		fprintf(stderr, "vis_do_smooth: can't create image\n");
		exit(1);
	}

	in = (unsigned char*) src->imageData;
	out = (unsigned char*) dst->imageData;
	in_stride = src->widthStep;
	out_stride = dst->widthStep;
	template_border = (template_size-1)/2;

	/* For cache efficiency, run loop going widthways first */
	for (j = 0; j < dst->height; j++) {
		for (i = 0; i < dst->width; i++) {
			/* For a particular pixel, take an average of the
			 * surrounding area. Efficiency might be reached by
			 * caching previous values... but then bias dies */

			x = i - template_border;
			y = j - template_border;
			accumul = 0;
			for (k = 0; k < template_size; k++) {
				for (l = 0; l < template_size; l++) {
					accumul += get(x+k,y+l) * bias[k][l];
				}
			}

			/* Gaussian filter works thus: Coefficient is scaled to
			 * an integer value between 0 and 255, then used to
			 * multiply part of the template. This leads to a 16 bit
			 * integer, of which we discard the lower 8 bits as
			 * acceptable loss of accuracy. This all involves no
			 * floating point logic. */

			/* Drop some accuracy and then divide by number of
			 * samples. Drop accuracy _before_ because microcoded
			 * divide instructions are going to take longer on
			 * larger integers... */

			accumul >>= 8;
			accumul /= (template_size*template_size);

			/* We have a sample. */

			put(i,j) = (unsigned char) accumul;
		}
	}

	return dst;
#undef get
#undef put
}

IplImage *
vis_do_roberts_edge_detection(IplImage *src)
{
#define get(x, y) *(in + ((y) * in_stride) + (x))
#define put(x, y) *(out + ((y) * out_stride) + (x))
	CvSize sz;
	IplImage *dst;                  
	unsigned char *in;
	unsigned char *out;
	int i, j, x, y, in_stride, out_stride;
	int diff1, diff2;

	vis_start();

	if (src->nChannels != 1) {
		fprintf(stderr, "vis_do_roberts_edge_detection: bad chans %d\n",
						src->nChannels);
                exit(1);
        }

	sz.width = src->width - 2;
	sz.height = src->height - 2;

	dst = cvCreateImage(sz, image_depth, 1);
	if (!dst) {     
		fprintf(stderr, "vis_do_roberts_edget_detection: "
				"can't create image\n");
		exit(1);                
	}

	in = (unsigned char*) src->imageData;
	out = (unsigned char*) dst->imageData;
	in_stride = src->widthStep;
	out_stride = dst->widthStep;

	for (j = 0; j < dst->height; j++) {
		for (i = 0; i < dst->width; i++) {
			x = i;
			y = j;

			diff1 = get(x, y) - get(x+1, y+1);
			diff2 = get(x+1, y) - get(x, y+1);

			diff1 = abs(diff1);
			diff2 = abs(diff2);

			put(x,y) = MAX(diff1, diff2);
		}
	}

	return dst;
#undef get
#undef put
}

