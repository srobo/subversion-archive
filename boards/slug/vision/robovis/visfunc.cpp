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

/* Ensure that "MAX" and "MIN" are what we expect them to be */
#if defined(MAX)
#undef MAX
#endif
#if defined(MIN)
#undef MIN
#endif
#define MIN(a,b)        (((a)<(b))?(a):(b))
#define        MAX(a,b)        (((a)>(b))?(a):(b))

#define template_size 5
#define sobel_size 5
#define image_depth 8

unsigned char bias[template_size][template_size];
int sobel_x[sobel_size][sobel_size];
int sobel_y[sobel_size][sobel_size];

int
fact(int num)
{
	int i, accumulator;

	accumulator = 1;
	for (i = 1; i <= num; i++)
		accumulator *= i;

	return accumulator;
}

int
pascal(int width, int pos)
{

	return (fact(width)) / (fact(width - pos) * fact(pos));
}
	
void
vis_start()
{
	double calc;
	static int started = 0;
	int x, y, i, j, max;
	float calcf;

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

	max = 0;
	for (i = 0; i < sobel_size; i++) {
		for (j = 0; j < sobel_size; j++) {
			sobel_y[i][j] = pascal(sobel_size-1, j) * 
			(pascal(sobel_size-2, i) - pascal(sobel_size-2, i-1));
			max = MAX(max, sobel_y[i][j]);
		}
	}

	for (i = 0; i < sobel_size; i++) {
		for (j = 0; j < sobel_size; j++) {
			sobel_x[i][j] = pascal(sobel_size-1, i) * 
			(pascal(sobel_size-2, j) - pascal(sobel_size-2, j-1));
		}
	}

	/* We have the maximum integer number used in the template, now scale
	 * all values to be in the range 0-255. Negative parts will scale to
	 * 0-(-255). */

	calcf = 255 / max;
	for (i = 0; i < sobel_size; i++) {
		for (j = 0; j < sobel_size; j++) {
			sobel_y[i][j] = (int) (sobel_y[i][j] * calcf);
		}
	}

	for (i = 0; i < sobel_size; i++) {
		for (j = 0; j < sobel_size; j++) {
			sobel_x[i][j] = (int) (sobel_x[i][j] * calcf);
		}
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
	int i, j, k, l, in_stride, out_stride;
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

			accumul = 0;
			for (k = 0; k < template_size; k++) {
				for (l = 0; l < template_size; l++) {
					accumul += get(i+k,j+l) * bias[k][l];
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
	int i, j, in_stride, out_stride;
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

			diff1 = get(i, j) - get(i+1, j+1);
			diff2 = get(i+1, j) - get(i, j+1);

			diff1 = abs(diff1);
			diff2 = abs(diff2);

			put(i,j) = MAX(diff1, diff2);
		}
	}

	return dst;
#undef get
#undef put
}

IplImage *
vis_do_sobel_edge_detection(IplImage *src)
{
#define get(x, y) *(in + ((y) * in_stride) + (x))
#define put(x, y) *(out + ((y) * out_stride) + (x))
	CvSize sz;
	IplImage *dst;  
	unsigned char *in;
	unsigned char *out;
	int i, j, k, l, in_stride, out_stride, border_size;
	int accuml_x, accuml_y;
 
	vis_start(); 

	if (src->nChannels != 1) {
		fprintf(stderr, "vis_do_sobel_edge_detection: bad chans %d\n",
							src->nChannels);
		exit(1);
	}

	border_size = (sobel_size-1)/2;
	sz.width = src->width - (sobel_size-1);
	sz.height = src->height - (sobel_size-1);

	dst = cvCreateImage(sz, image_depth, 1);
	if (!dst) {     
		fprintf(stderr, "vis_do_sobel_edge_detection: "
				"can't create image\n");
		exit(1);                
	}

	in = (unsigned char*) src->imageData;
	out = (unsigned char*) dst->imageData;
	in_stride = src->widthStep;
	out_stride = dst->widthStep;

	for (j = 0; j < dst->height; j++) {
		for (i = 0; i < dst->width; i++) {
			accuml_x = accuml_y = 0;

			for (k = 0; k < template_size; k++) {
				for (l = 0; l < template_size; l++) {
					accuml_x += get(i+k,j+l)*sobel_x[k][l];
					accuml_y += get(i+k,j+l)*sobel_y[k][l];
				}
			}

			accuml_x >>= 8;
			accuml_y >>= 8;
//			accuml_x /= (sobel_size*sobel_size);
//			accuml_y /= (sobel_size*sobel_size);
			accuml_x = abs(accuml_x);
			accuml_y = abs(accuml_y);

			put(i, j) = MAX(accuml_x, accuml_y);
		}
	}

	return dst;
#undef get
#undef put
}

