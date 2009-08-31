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

#ifndef __GNUC__
#define INLINE
#define breakpoint() exit(1)
#else
#define INLINE inline
#define breakpoint() __asm__("int $3")
#endif

#define template_size 5
#define sobel_size 5
#define image_depth IPL_DEPTH_8U

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

	sz.width = src->width;
	sz.height = src->height;

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

	out += (template_border * out_stride) + template_border;

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

	/* Clobber border */
	/* If we just dump 0 in the borders we trigger edge detection later,
	 * so instead fill the border with the adjacent pixel. Not efficient,
	 * but the intention is that the version on the slug will do everything
	 * in one pass, which means this can be ditched */

	in = (unsigned char*) dst->imageData;
	out = (unsigned char*) dst->imageData;
	for (i = 0; i < dst->width; i++) {
		for (j = 0; j < template_border; j++)
			put(i,j) = get(i,template_border);
		for (j = 0; j < template_border; j++)
			put(i,dst->height - template_border + j - 1) = 
				get(i,dst->height-template_border-1);
	}

	for (j = 0; j < dst->height; j++) {
		for (i = 0; i < template_border; i++)
			put(i,j) = get(template_border,j);
		for (i = 0; i < template_border; i++)
			put(dst->width - template_border + i - 1,j) = 
				get(dst->width-template_border-1,i);
	}

	return dst;
#undef get
#undef put
}

INLINE unsigned char
vis_find_angle(signed char x, signed char y)
{
	float fx, fy, ret;

	fx = (float) abs(x);
	fy = (float) abs(y);

	/* 0 degrees means edge direction is pointing upwards, 90 is right etc
	 * However becuase we have to deal in bytes here, everything is shrunk
	 * to a scale of 0-255. 90 degrees is now 64, 180 is 128, 270 is 192
	 * and so forth. */

	if (x == 0 && y > 0)
		return 128;
	else if (x == 0 && y < 0)
		return 0;
	else if (x > 0 && y == 0)
		return 64;
	else if (x < 0 && y == 0)
		return 192;
	else if (x == 0 && y == 0)
		return 0;

	if (x >= 0 && y >= 0)
		ret = atan(fx/fy) * 360 / (2*M_PI);
	else if (x < 0 && y > 0)
		ret = 360 - (atan(fx/fy) * 360 / (2*M_PI));
	else if (x > 0 && y < 0)
		ret = 180 - (atan(fx/fy) * 360 / (2*M_PI));
	else
		ret = 180 + (atan(fx/fy) * 360 / (2*M_PI));

	ret /= (360.0/256.0);

	return (unsigned char) ret;
}

IplImage *
vis_do_roberts_edge_detection(IplImage *src, IplImage **direction)
{
#define get(x, y) *(in + ((y) * in_stride) + (x))
#define put(x, y) *(out + ((y) * out_stride) + (x))
#define putdir(x, y) *(dir + ((y) * out_stride) + (x))
	CvSize sz;
	IplImage *dst;                  
	unsigned char *in;
	unsigned char *out;
	unsigned char *dir;
	int i, j, in_stride, out_stride;
	int diff1, diff2;

	vis_start();

	if (src->nChannels != 1) {
		fprintf(stderr, "vis_do_roberts_edge_detection: bad chans %d\n",
						src->nChannels);
                exit(1);
        }

	sz.width = src->width;
	sz.height = src->height;

	dst = cvCreateImage(sz, image_depth, 1);
	if (direction)
		*direction = cvCreateImage(sz, image_depth, 1);

	if (!dst || (direction && !*direction)) {     
		fprintf(stderr, "vis_do_roberts_edget_detection: "
				"can't create image\n");
		exit(1);                
	}

	in = (unsigned char*) src->imageData;
	out = (unsigned char*) dst->imageData;
	dir = (unsigned char*) (*direction)->imageData;
	in_stride = src->widthStep;
	out_stride = dst->widthStep;

	out += out_stride + 1;
	dir += out_stride + 1;

	for (j = 0; j < dst->height; j++) {
		for (i = 0; i < dst->width; i++) {

			diff1 = get(i, j) - get(i+1, j+1);
			diff2 = get(i+1, j) - get(i, j+1);

			if (direction)
				putdir(i,j) = vis_find_angle(diff1, diff2);

			diff1 = abs(diff1);
			diff2 = abs(diff2);

			put(i,j) = MAX(diff1, diff2);
		}
	}

	for (i = 0; i < dst->width; i++) {
		put(i,0) = 0;
		put(i,dst->height - 1) = 0;
		if (direction) {
			putdir(i,0) = 0;
			putdir(i,dst->height - 1) = 0;
		}
	}

	for (j = 0; j < dst->height; j++) {
		put(0,j) = 0;
		put(dst->width - 1,j) = 0;
		if (direction) {
			putdir(0,j) = 0;
			putdir(dst->width - 1,j) = 0;
		}
	}

	return dst;
#undef get
#undef put
}

IplImage *
vis_do_sobel_edge_detection(IplImage *src, IplImage **direction)
{
#define get(x, y) *(in + ((y) * in_stride) + (x))
#define put(x, y) *(out + ((y) * out_stride) + (x))
#define putdir(x, y) *(dir + ((y) * out_stride) + (x))
	CvSize sz;
	IplImage *dst;  
	unsigned char *in;
	unsigned char *out;
	unsigned char *dir;
	int i, j, k, l, in_stride, out_stride, border_size;
	int accuml_x, accuml_y;
 
	vis_start(); 

	if (src->nChannels != 1) {
		fprintf(stderr, "vis_do_sobel_edge_detection: bad chans %d\n",
							src->nChannels);
		exit(1);
	}

	border_size = (sobel_size-1)/2;
	sz.width = src->width;
	sz.height = src->height;

	dst = cvCreateImage(sz, image_depth, 1);
	if (direction)
		*direction = cvCreateImage(sz, image_depth, 1);

	if (!dst || (direction && !*direction)) {     
		fprintf(stderr, "vis_do_sobel_edge_detection: "
				"can't create image\n");
		exit(1);                
	}

	in = (unsigned char*) src->imageData;
	out = (unsigned char*) dst->imageData;
	dir = (unsigned char*) (*direction)->imageData;
	in_stride = src->widthStep;
	out_stride = dst->widthStep;

	out += (border_size * out_stride) + border_size;
	dir += (border_size * out_stride) + border_size;

	for (j = 0; j < dst->height; j++) {
		for (i = 0; i < dst->width; i++) {
			accuml_x = accuml_y = 0;

			for (k = 0; k < template_size; k++) {
				for (l = 0; l < template_size; l++) {
					accuml_x += get(i+k,j+l)*sobel_x[k][l];
					accuml_y += get(i+k,j+l)*sobel_y[k][l];
				}
			}

			/* This still needs to be fixed, however, for a five
			 * spaced sobel operator dividing by 1000 is roughly
			 * a correct drop in accuracy to fit it back in the
			 * range of 0-255. Fixme! */
			accuml_x >>= 10;
			accuml_y >>= 10;
#if sobel_size != 5
#error Re-calculate division approximation for sobel operator
#endif

			if (direction)
				putdir(i,j) = vis_find_angle(accuml_x, accuml_y);

			accuml_x = abs(accuml_x);
			accuml_y = abs(accuml_y);

			put(i, j) = MAX(accuml_x, accuml_y);
		}
	}

	out = (unsigned char*) dst->imageData;
	for (i = 0; i < dst->width; i++) {
		for (j = 0; j < border_size; j++) {
			put(i,j) = 0;
			if (direction)
				putdir(i,j) = 0;
		}
		for (j = 0; j < border_size; j++) {
			put(i,dst->height - border_size + j - 1) = 0;
			if (direction)
				putdir(i,j) = 0;
		}	
	}

	for (j = 0; j < dst->height; j++) {
		for (i = 0; i < border_size; i++) {
			put(i,j) = 0;
			if (direction)
				putdir(i,j) = 0;
		}
		for (i = 0; i < border_size; i++) {
			put(dst->width - border_size + i - 1,j) = 0;
			if (direction)
				putdir(i,j) = 0;
		}
	}

	return dst;
#undef get
#undef put
}

IplImage *
vis_nonmaximal_supression(IplImage *src, IplImage *direction)
{
#define get(x, y) *(in + ((y) * in_stride) + (x))
#define getdir(x, y) *(dir + ((y) * in_stride) + (x))
#define put(x, y) *(out + ((y) * out_stride) + (x))
	CvSize sz;
	IplImage *dst;
	unsigned char *in, *dir, *out;
	int i, j, in_stride, out_stride;
	unsigned char val;

	if (src->width != direction->width || src->height != direction->height){
		fprintf(stderr, "vis_nonmaximal_supression - image mismatch\n");
		exit(1);
	}

	sz.width = src->width;
	sz.height = src->height;

	dst = cvCreateImage(sz, image_depth, 1);
	if (!dst) {
		fprintf(stderr, "vis_nonmaximal_supression: "
				"can't create image\n");
		exit(1);
	}

	in = (unsigned char *)src->imageData;
	dir = (unsigned char *)direction->imageData;
	out = (unsigned char *)dst->imageData;

	in_stride = src->widthStep;
	out_stride = dst->widthStep;

	in += (in_stride) + 1;
	dir += (in_stride) + 1;
	out += (out_stride) + 1;

	for (i = 0; i < src->width-1; i++) {
		for (j = 0; j < src->height-1; j++) {
			val = getdir(i,j);

			/* Drop to four bits of accuracy, this gives us a
			 * general idea of where we were pointing. */
			switch (val >> 4) {
			case 3: /* Right */
			case 4:
			case 11: /* Left */
			case 12:
				if (get(i,j) > get(i,j-1) &&
							get(i,j) > get(i,j+1))
					put(i,j) = get(i,j);
				else if (get(i,j) == get(i,j-1) &&
							get(i,j) > get(i,j+1))
					put(i,j) = get(i,j);
				else
					put(i,j) = 0;
				break;

			case 5: /* Bottom right */
			case 6:
			case 13: /*Top right */
			case 14:
				if (get(i,j) > get(i+1,j-1) && 
							get(i,j) > get(i-1,j+1))
					put(i,j) = get(i,j);
				else if (get(i,j) == get(i+1,j-1) &&
							get(i,j) > get(i-1,j+1))
					put(i,j) = get(i,j);
				else
					put(i,j) = 0;
				break;

			case 0: /* Top */
			case 15: 
			case 7: /* Bottom */
			case 8:
				if (get(i,j) > get(i-1,j) && 
							get(i,j) > get(i+1,j))
					put(i,j) = get(i,j);
				else if (get(i,j) == get(i-1,j) &&
							get(i,j) > get(i+1,j))
					put(i,j) = get(i,j);
				else
					put(i,j) = 0;
				break;

			case 1: /* Top right */
			case 2:
			case 9: /* Bottom Left */
			case 10:
				if (get(i,j) > get(i-1,j-1) && 
							get(i,j) > get(i+1,j+1))
					put(i,j) = get(i,j);
				else if (get(i,j) == get(i-1,j-1) &&
							get(i,j) > get(i+1,j+1))
					put(i,j) = get(i,j);
				else
					put(i,j) = 0;
				break;
			}
		}
	}

	/* Clober border */

	for (i = 0; i < dst->width; i++) {
		put(i,0) = 0;
		put(i,dst->height-1) = 0;
	}

	for (j = 0; j < dst->height; j++) {
		put(0,j) = 0;
		put(dst->width-1,j) = 0;
	}

	return dst;
}

IplImage *
vis_normalize_plane(IplImage *src)
{
	CvSize sz;
	IplImage *dst;
	unsigned char *in, *out;
	int i, i_scale;
	float scale;
	unsigned char max;

	sz.width = src->width;
	sz.height = src->height;

	dst = cvCreateImage(sz, image_depth, 1);
	if (!dst) {
		fprintf(stderr, "vis_normalize_plane: "
				"can't create image\n");
		exit(1);
	}

	max = 0;
	in = (unsigned char *)src->imageData;
	out = (unsigned char *)dst->imageData;

	for (i = 0; i < src->imageSize; i++)
		max = MAX(max, *(in+i));

	/* Perform some fixed point scaling - we don't want to make everything
	 * floating, but we _do_ want to scale everything up. So, fixed point
	 * arithmatic */

	scale = 255.0 / max;
	i_scale = (int) scale * 100;
	for (i = 0; i < src->imageSize; i++)
		*(out + i) = ((*(in + i)) * i_scale) >> 8;

	return dst;
}

IplImage *
vis_threshold(IplImage *src, unsigned char low, unsigned char high)
{
	CvSize sz;
	IplImage *dst;
	unsigned char *in, *out;
	int i;

	sz.width = src->width;
	sz.height = src->height;

	dst = cvCreateImage(sz, image_depth, 1);
	if (!dst) {
		fprintf(stderr, "vis_threshold: "
			"can't create image\n");
		exit(1);
	}

	in = (unsigned char *)src->imageData;
	out = (unsigned char *)dst->imageData;

	for (i = 0; i < src->imageSize; i++)
		*(out+i) = ((*(in+i) >= low) && (*(in+i) <= high)) ? 255 : 0;

	return dst;
}

IplImage *
vis_clip(IplImage *src, IplImage *clip, unsigned char low, unsigned char high)
{
	CvSize sz;
	IplImage *dst;
	unsigned char *in, *other, *out;
	int i;

	if (src->width != clip->width || src->height != clip->height) {
		fprintf(stderr, "vis_clip, mismatching images\n");
		exit(1);
	}

	sz.width = src->width;
	sz.height = src->height;

	dst = cvCreateImage(sz, image_depth, 1);
	if (!dst) {
		fprintf(stderr, "vis_threshold: "
			"can't create image\n");
		exit(1);
	}

	in = (unsigned char *)src->imageData;
	other = (unsigned char *)clip->imageData;
	out = (unsigned char *)dst->imageData;

	for (i = 0; i < src->imageSize; i++) 
		*(out+i) = ((*(other+i) >= low) && (*(other+i) <= high))
				? *(in+i) : 0;

	return dst;
}

void
vis_follow_edge(IplImage *src, IplImage *direction, int x, int y)
{
#define get(x, y) *(in + ((y) * in_stride) + (x))
#define getdir(x, y) *(dir + ((y) * in_stride) + (x))
#define put(x, y) *(out + ((y) * out_stride) + (x))
	CvSize sz;
	IplImage *dst;
	unsigned char *in, *dir, *out;
	int dx, dy, in_stride, out_stride;
	int maxx, minx, maxy, miny, pixel_count;
	unsigned char val, have_moved;

	if (src->width != direction->width || src->height != direction->height){
		fprintf(stderr, "vis_find_edge - image mismatch\n");
		exit(1);
	}

	sz.width = src->width;
	sz.height = src->height;

	dst = cvCreateImage(sz, image_depth, 1);
	if (!dst) {
		fprintf(stderr, "vis_find_edge: " "can't create image\n");
		exit(1);
	}

	in = (unsigned char *)src->imageData;
	dir = (unsigned char *)direction->imageData;
	out = (unsigned char *)dst->imageData;

	in_stride = src->widthStep;
	out_stride = dst->widthStep;

	minx = miny = 0x7FFFFFFF;
	maxx = maxy = -1;
	dx = x;
	dy = y;
	pixel_count = 0;/* How many pixels have we covered since starting */

	/* So - we have the co-ordinates of what's apparently the corner of an
	 * edge. For each pixel, take the direction, and see if there's an edge
	 * pixel in that direction. If many, take the highest edge-change.
	 * Store max/min x/y values, if we come back to somewhere in the
	 * viscinity of the starting point, presume we have completed. */
	while (1) {
		val = getdir(dx, dy);

		/* As with non-maximal supression, drop accuracy, get a general
		 * idea of where we were pointing */
		switch(val >> 5) {

#define advance(a,b) 	do {\
				if (get(dx+(a),dy+(b))) {\
					dx += (a);\
					dy += (b);\
					goto breakout;\
				}\
			} while (0);

		case 0: /* Top */
		case 7:
			advance(-1,-1);
			advance(0,-1);
			advance(1,-1);
			goto bees;
		case 1: /* Rightwards */
		case 2:
			advance(1,-1);
			advance(1,0);
			advance(1,1);
			goto bees;

		case 3: /* Downwards */
		case 4:
			advance(1,1);
			advance(0,1);
			advance(-1,1);
			goto bees;

		case 5: /* Leftwards */
		case 6: 
			advance(-1,1);
			advance(-1,0);
			advance(-1,-1);
			goto bees;
		default:
			fprintf(stderr, "vis_follow_edge reality error; please "
					"re-install the universe and try again"
					"\n");
			exit(1);
#undef advance
		}

		bees:
		printf("It'd appear that the edge point %d %d you gave me "
			"doesn't form a loop\n", x, y);
		return;

		breakout:
		/* Reached if we have actually progressed */
		pixel_count++;
		maxx = MAX(maxx, dx);
		minx = MIN(minx, dx);
		maxy = MAX(maxy, dy);
		miny = MIN(miny, dy);

		/* We now need to guess if we've reached the starting point
		 * again. It _is_ possible to skip it, if we're on a diagonal
		 * and the direction makes us move past it, like this:
		 *       XX
		 *        XP
		 *         XX
		 * where P is the start point. So, use fuzzy checking logic.
		 * Also, only trigger once we've gone more than 10 pixels */
#define fuzz 2
		if (abs(dx - x) <= fuzz && abs(dy - y) <= fuzz &&
					pixel_count > 10) {
			printf("Congratulations, we appear to have a loop, "
				"with bounds %d,%d to %d %d\n", minx, miny,
				maxx, maxy);
			return;
		}
#undef fuzz

		/* If not, move along */
	};

	/* Should never get out of this while(1)... */
	fprintf(stderr, "Loop fail in vis_find_edge\n");
	exit(1);
	return;
}
