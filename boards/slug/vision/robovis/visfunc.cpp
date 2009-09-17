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
#if !defined(__arm__)
#define breakpoint() __asm__("int $3")
#else
#define breakpoint()
#endif
#endif

#define template_size 5
#define sobel_size 5
#define image_depth IPL_DEPTH_8U

/* Bounds for what edge pixel values are accepted into a line being followed -
 * a line will only _start_ when a 'high' pixel is found, and will continue 
 * until it drops below 'low'. This lets us eliminate a lot of noise without
 * thresholding out faint outlines. If both values are the same, equivalent
 * of thresholding */

#define line_hysteresis_high 5
#define line_hysteresis_low 5

#define BLOB_MIN_MASS 400

static unsigned char bias[template_size][template_size];

#if sobel_size == 3
static int sobel_x[sobel_size][sobel_size] = {{1,0,-1},{2,0,-2},{1,0,-1}};
static int sobel_y[sobel_size][sobel_size] = {{1,2,1},{0,0,0},{-1,-2,-1}};
#else
static int sobel_x[sobel_size][sobel_size];
static int sobel_y[sobel_size][sobel_size];
#endif

static struct blob_position blobs[MAX_BLOBS+1];

static int num_blobs = 0;

#define SPANS 32
static blob_position spans_a[SPANS+1];
static blob_position spans_b[SPANS+1];
static blob_position *spans, *ospans;
static int span, ospan;

static int
fact(int num)
{
	int i, accumulator;

	accumulator = 1;
	for (i = 1; i <= num; i++)
		accumulator *= i;

	return accumulator;
}

static int
pascal(int width, int pos)
{

	return (fact(width)) / (fact(width - pos) * fact(pos));
}

static void
add_blob(int minx, int miny, int maxx, int maxy)
{
	int w, h;

	if (!(num_blobs < MAX_BLOBS)) {
		fprintf(stderr, "add_blob, ran out of blob records...\n");
		return;
	}

	w = maxx - minx;
	h = maxy - miny;
	if (w * h < BLOB_MIN_MASS)
		return;

	blobs[num_blobs].x1 = minx;
	blobs[num_blobs].x2 = maxx;
	blobs[num_blobs].y1 = miny;
	blobs[num_blobs].y2 = maxy;
	num_blobs++;

	return;
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

#if sobel_size == 5
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
#elif sobel_size == 3

	/* It's initialized above */
	max = 2;

#else
#error bad sobel size
#endif

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
	for (j = 0; j < dst->height - template_border; j++) {
		for (i = 0; i < dst->width - template_border; i++) {
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
		return 0;
	else if (x == 0 && y < 0)
		return 128;
	else if (x > 0 && y == 0)
		return 64;
	else if (x < 0 && y == 0)
		return 192;
	else if (x == 0 && y == 0)
		return 0;

	if (x >= 0 && y >= 0)
		ret = atanf(fx/fy) * 360 / (2*M_PI);
	else if (x < 0 && y > 0)
		ret = 360 - (atanf(fx/fy) * 360 / (2*M_PI));
	else if (x > 0 && y < 0)
		ret = 180 - (atanf(fx/fy) * 360 / (2*M_PI));
	else
		ret = 180 + (atanf(fx/fy) * 360 / (2*M_PI));

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

			for (k = 0; k < sobel_size; k++) {
				for (l = 0; l < sobel_size; l++) {
					accuml_x += get(i+k,j+l)*sobel_x[k][l];
					accuml_y += get(i+k,j+l)*sobel_y[k][l];
				}
			}

#if sobel_size == 5
			/* This still needs to be fixed, however, for a five
			 * spaced sobel operator dividing by 1000 is roughly
			 * a correct drop in accuracy to fit it back in the
			 * range of 0-255. Fixme! */
			accuml_x >>= 10;
			accuml_y >>= 10;
#elif sobel_size == 3
			accuml_x >>= 8;
			accuml_y >>= 8;
#else
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
	unsigned char val, val_low;

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

/* Supression - arguments are x/y locations of the center and then two outer
 * points, the second outer point being the next pixel clockwise from the first.
 * "Ang" is the lower 5 bits of the angle byte, and describes how far the
 * edge-normal is between the two points. 0 would be the anti-clockwise-most
 * point, 1F almost at the next point. So, calculate the combination of those
 * appropriately weighted points, it's effectively scaled by 32x due to the
 * multiplication, so shift right 5 (shifts are free on ARM!) and compare to
 * the original */

#define interpolate_value(a1,b1,a2,b2,ang)\
		((get((a1),(b1)) * (32-(ang)) + (get((a2),(b2)) * ang)) >> 5)
#define supress(a,b,a1,b1,a2,b2,ang) (interpolate_value(a1,b1,a2,b2,ang)\
				< get((a),(b)))

/* So as well as the above, we get some situations where we get several
 * equal maximum values in a line - say, 40 -> 40 -> 40. In that case, the
 * "supress" check isn't triggered, so no maximum point is written. So, add a
 * second check to see if we're at the edge of such a series. */
#define check_diff_edge(a,b,a1,b1,a2,b2,ang)\
	(abs(interpolate_value(a1,b1,a2,b2,ang) - get(a,b)) == 0)

			val = getdir(i,j);
			val_low = val & 0x1F;

			/* Drop to three bits of accuracy, this gives us a
			 * general idea of where we were pointing. */

			/* Like this:
			 *         -----------------------
			 *	  | \         |         / |
			 *	  |   \       |       /   |
			 *	  |     \  7  | 0   /     |
			 *	  |       \   |   /       |
			 *	  |     6   \ | /   1     |
			 *	  |-----------.---------- |
			 *	  |     5   / | \   2     |
			 *	  |       /   |   \       |
			 *	  |     /   4 | 3   \     |
			 *	  |   /       |       \   |
			 *	  | /         |         \ |
			 *	   -----------------------
			 * Then the lower 5 bits can be used to interpolate the 
			 * surrounding square or pixels
			 */


			/* Note to self - this is the direction of the edge.
			 * We want the normal, dammit! */
			switch (val >> 5) {
			case 2: /* Right -> bottom-right and left -> top-left */
			case 6:
				if (supress(i,j, i,j-1, i+1,j-1, val_low) &&
				    supress(i,j, i,j+1, i-1,j+1, val_low))
					put(i,j) = get(i,j);
				else if (get(i,j) == get(i,j-1) &&
					 supress(i,j, i,j+1, i-1,j+1, val_low))
					put(i,j) = get(i,j);
				else
					put(i,j) = 0;
				break;

			case 3: /* bottom-right -> bottom, top-left -> top */
			case 7:
				if (supress(i,j, i+1,j-1, i+1,j, val_low) &&
				    supress(i,j, i-1,j+1, i-1,j, val_low))
					put(i,j) = get(i,j);
				else if (check_diff_edge(i,j,i+1,j-1,i-1,j+1,
								val_low) ||
					check_diff_edge(i,j,i-1,j+1,i-1,j,
								val_low))
					put(i,j) = get(i,j);
				else
					put(i,j) = 0;
				break;

			case 0: /* Edge is in top -> top-right and in	*/
			case 4: /* Bottom -> bottom-left pixels		*/
				if (supress(i,j, i+1,j, i+1,j+1, val_low) &&
				    supress(i,j, i-1,j, i-1,j-1, val_low))
					put(i,j) = get(i,j);
				else if (get(i,j) == get(i-1,j) &&
					 supress(i,j, i+1,j, i+1,j+1, val_low))
					put(i,j) = get(i,j);
				else
					put(i,j) = 0;
				break;

			case 1: /* Edge in top-right -> right and in	*/
			case 5: /* bottom-right -> right		*/
				if (supress(i,j, i+1,j+1, i,j+1, val_low) &&
				    supress(i,j, i-1,j-1, i,j-1, val_low))
					put(i,j) = get(i,j);
/*insert pain comment here*/	else if (abs(get(i,j) -
					interpolate_value(i-1,j-1,i,j-1,val_low)
					) <= 1 &&
					 supress(i,j, i+1,j+1, i,j+1, val_low))
					put(i,j) = get(i,j);
				else
					put(i,j) = 0;
				break;
			default:
				fprintf(stderr, "vis_nonmaximal_supression: "
						"reality error, please "
						"re-install the universe and "
						"try again\n");
				exit(1);
			}
		}
	}
#undef supress

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
#undef put
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

int
vis_find_nearest_blob_distance(int x, int y)
{
	int i, nx, ny, within;

	nx = 0x7FFFFFFF;
	ny = 0x7FFFFFFF;
	within = 0;

	for (i = 0; i < num_blobs; i++) {
		nx = MIN(abs(x - blobs[i].x1), nx);
		nx = MIN(abs(x - blobs[i].x2), nx);
		ny = MIN(abs(y - blobs[i].y1), ny);
		ny = MIN(abs(y - blobs[i].y2), ny);
		if (x >= blobs[i].x1 && x <= blobs[i].x2 &&
		    y >= blobs[i].y1 && y <= blobs[i].y2)
			within = 1;
	}

	/* If there are no other blobs, we can travel max int pixels */

	return (within) ? -1 : MIN(nx, ny);
}

void
vis_follow_edge(IplImage *src, IplImage *direction, int x, int y)
{
#define get(x, y) *(in + ((y) * in_stride) + (x))
#define getdir(x, y) *(dir + ((y) * in_stride) + (x))
	CvSize sz;
	unsigned char *in, *dir;
	int dx, dy, in_stride;
	int maxx, minx, maxy, miny, pixel_count, pixel_check;
	unsigned char val, val_trimmed;

	if (src->width != direction->width || src->height != direction->height){
		fprintf(stderr, "vis_find_edge - image mismatch\n");
		exit(1);
	}

	sz.width = src->width;
	sz.height = src->height;

	in = (unsigned char *)src->imageData;
	dir = (unsigned char *)direction->imageData;

	in_stride = src->widthStep;
	minx = miny = 0x7FFFFFFF;
	maxx = maxy = -1;
	dx = x;
	dy = y;
	pixel_count = 0;/* How many pixels have we covered since starting */
	pixel_check = vis_find_nearest_blob_distance(x, y);

	if (pixel_check == -1) {
		fprintf(stderr, "Started following a line within existing "
				"blob\n");
		return;
	}

	/* So - we have the co-ordinates of what's apparently the corner of an
	 * edge. For each pixel, take the direction, and see if there's an edge
	 * pixel in that direction. If many, take the highest edge-change.
	 * Store max/min x/y values, if we come back to somewhere in the
	 * viscinity of the starting point, presume we have completed. */
	while (1) {
		val = getdir(dx, dy);
		val_trimmed = val >> 4;
#if 0
		if ((val & 15) >= 8)
			val_trimmed++;
#endif

		/* Trim to 0-15, if val is 255, that last add will overflow */
		val_trimmed &= 0xF;

		switch(val_trimmed) {

#define acceptable_point(p) (((p) >= line_hysteresis_low))

#define advance(a,b) 	do {\
				if (acceptable_point(get(dx+(a),dy+(b)))) {\
					dx += (a);\
					dy += (b);\
					goto breakout;\
				}\
			} while (0);

/* Policy of - try to advance in the direction we're pointing in first, then
 * try whichever direction follows our curve (outlines are always followed
 * anti-clockwise), then the other pixel adjacent to where we're pointing,
 * and finally the next pixel pixel going anticlockwise. Any more attempts
 * at advancing'll probably end up going backwards */
/* test - swapped so that we always tend anticlockwise, and the other adjacent
 * pixel is the last to be tried. */

		case 0: /* Top */
		case 15:
			advance(0,-1);
			advance(-1,-1);
			advance(-1,0);
			advance(1,-1);
			goto bees;
		case 1: /* Top Right */
		case 2:
			advance(1,-1);
			advance(0,-1);
			advance(-1,-1);
			advance(1,0);
			goto bees;

		case 3: /* Rightwards */
		case 4:
			advance(1,0);
			advance(1,-1);
			advance(0,-1);
			advance(1,1);
			goto bees;

		case 5: /* Bottom Rightwards */
		case 6: 
			advance(1,1);
			advance(1,0);
			advance(1,-1);
			advance(0,1);
			goto bees;

		case 7: /* Down */
		case 8:
			advance(0,1);
			advance(1,1);
			advance(1,0);
			advance(-1,1);
			goto bees;

		case 9: /* Bottom Leftwards */
		case 10:
			advance(-1,1);
			advance(0,1);
			advance(1,1);
			advance(-1,0);
			goto bees;

		case 11: /* Left */
		case 12:
			advance(-1,0);
			advance(-1,1);
			advance(0,1);
			advance(-1,-1);
			goto bees;

		case 13: /* Top left */
		case 14:
			advance(-1,-1);
			advance(-1,0);
			advance(-1,1);
			advance(0,-1);
			goto bees;

		default:
			fprintf(stderr, "vis_follow_edge reality error; please "
					"re-install the universe and try again"
					"\n");
			exit(1);
#undef advance
		}

		bees:
		pixel_check = vis_find_nearest_blob_distance(x, y);
		if (pixel_check == -1) {
			printf("ponies\n");
			breakpoint();
			return;
		}
		return;

		breakout:
		/* Reached if we have actually progressed */
		pixel_count++;

		if (pixel_count >= pixel_check) {
			pixel_check = vis_find_nearest_blob_distance(dx, dy);
			if (pixel_check == -1) {
				breakpoint();
				vis_follow_edge_backwards( src, direction, x, y,
						minx, miny, maxx, maxy);
				return;
			}
		}

		maxx = MAX(maxx, dx);
		minx = MIN(minx, dx);
		maxy = MAX(maxy, dy);
		miny = MIN(miny, dy);

if (pixel_count > 3000)
	return;

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
			add_blob(minx, miny, maxx, maxy);

			return;
		}
#undef fuzz

		/* If not, move along */
	};

	/* Should never get out of this while(1)... */
	fprintf(stderr, "Loop fail in vis_find_edge\n");
	exit(1);
	return;
#undef get
#undef getdir
}

void
vis_follow_edge_backwards(IplImage *src, IplImage *direction, int x, int y,
				int minx, int miny, int maxx, int maxy)
{
#define get(x, y) *(in + ((y) * in_stride) + (x))
#define getdir(x, y) *(dir + ((y) * in_stride) + (x))
	CvSize sz;
	unsigned char *in, *dir;
	int dx, dy, in_stride;
	int pixel_count, pixel_check;
	unsigned char val, val_trimmed;

	if (src->width != direction->width || src->height != direction->height){
		fprintf(stderr, "vis_find_edge_backwards - image mismatch\n");
		exit(1);
	}

	sz.width = src->width;
	sz.height = src->height;

	in = (unsigned char *)src->imageData;
	dir = (unsigned char *)direction->imageData;

	in_stride = src->widthStep;
	minx = miny = 0x7FFFFFFF;
	maxx = maxy = -1;
	dx = x;
	dy = y;
	pixel_count = 0;
	pixel_check = vis_find_nearest_blob_distance(x, y);

	if (pixel_check == -1) {
		fprintf(stderr, "Started following a line within existing "
				"blob\n");
		return;
	}

	while (1) {
		val = getdir(dx, dy);
		val_trimmed = val >> 4;
#if 0
		if ((val & 15) >= 8)
			val_trimmed++;

		val_trimmed &= 0xF;
#endif

		switch(val_trimmed) {

#define acceptable_point(p) (((p) >= line_hysteresis_low))

#define advance(a,b) 	do {\
				if (acceptable_point(get(dx+(a),dy+(b)))) {\
					dx += (a);\
					dy += (b);\
					goto breakout;\
				}\
			} while (0);

		case 7: /* Down */
		case 8:
			advance(0,-1);
			advance(-1,-1);
			advance(-1,0);
			advance(1,-1);
			goto bees;

		case 9: /* Bottom Leftwards */
		case 10:
			advance(1,-1);
			advance(0,-1);
			advance(-1,-1);
			advance(1,0);
			goto bees;

		case 11: /* Left */
		case 12:
			advance(1,0);
			advance(1,-1);
			advance(0,-1);
			advance(1,1);
			goto bees;

		case 13: /* Top left */
		case 14:
			advance(1,1);
			advance(1,0);
			advance(1,-1);
			advance(0,1);
			goto bees;

		case 0: /* Top */
		case 15:
			advance(0,1);
			advance(1,1);
			advance(1,0);
			advance(-1,1);
			goto bees;

		case 1: /* Top Right */
		case 2:
			advance(-1,1);
			advance(0,1);
			advance(1,1);
			advance(-1,0);
			goto bees;

		case 3: /* Rightwards */
		case 4:
			advance(-1,0);
			advance(-1,1);
			advance(0,1);
			advance(-1,-1);
			goto bees;

		case 5: /* Bottom Rightwards */
		case 6: 
			advance(-1,-1);
			advance(-1,0);
			advance(-1,1);
			advance(0,-1);
			goto bees;

		default:
			fprintf(stderr, "vis_follow_edge_backwards reality "
					"error; please re-install the universe "
					"and try again\n");
			exit(1);
#undef advance
		}

		bees:
		pixel_check = vis_find_nearest_blob_distance(x, y);
		if (pixel_check == -1) {
			printf("ponies\n");
			breakpoint();
			return;
		}
		return;

		breakout:
		pixel_count++;

		if (pixel_count >= pixel_check) {
			pixel_check = vis_find_nearest_blob_distance(dx, dy);
			if (pixel_check == -1) {
				breakpoint();
				add_blob(minx, miny, maxx, maxy);
				return;
			}
		}

		maxx = MAX(maxx, dx);
		minx = MIN(minx, dx);
		maxy = MAX(maxy, dy);
		miny = MIN(miny, dy);

if (pixel_count > 3000)
	return;

		/* We aren't really looking for a loop - this function should
		 * only be called from vis_follow_line. However there's the
		 * chance that the line we're moving along develops a tight
		 * loop later on. And thinking about it, there's nothing
		 * we can do about that anyway, so never mind about this
		 * clause I just deleted */

		/* Continue */

	};

	fprintf(stderr, "Loop fail in vis_find_edge\n");
	exit(1);
	return;
#undef get
}

struct blob_position *
vis_search_for_blobs(IplImage *img, IplImage *dir, int spacing)
{
#define get(i, j) *(img->imageData + ((j) * img->widthStep) + (i))
	int x, y, i;

	/* Visualize this by placing a grid over the image, then searching
	 * along the grid lines for nonzero pixels, then following those alleged
	 * edges and checking if they're blobs. */

	num_blobs = 0;
	memset(blobs, 0, sizeof(blobs));

#define acceptable_start(p) ((p) >= line_hysteresis_high)

	for (x = 0; x < img->width; x += spacing) {
		for (y = 0; y < img->height; y++) {
			for (i = 0; i < num_blobs; i++)
				if (x >= blobs[i].x1 && x <= blobs[i].x2 &&
				    y >= blobs[i].y1 && y <= blobs[i].y2)
					goto skip;

			if (acceptable_start(get(x,y))) {
				vis_follow_edge(img, dir, x, y);
			}

			skip:
			continue;
		}
	}

	return blobs;
#undef get
}

struct blob_position *
vis_find_blobs_through_scanlines(IplImage *hue, IplImage *sat, IplImage *sat_edge)
{
#define getsat(a, b) *(s + ((b) * sat->widthStep) + (a))
#define gethue(a, b) *(h + ((b) * hue->widthStep) + (a))
#define getedge(a, b) *(e + ((b) * sat_edge->widthStep) + (a))
#define put(a, b) *(out->imageData + ((b) * out->widthStep) + (a))

#define line_cache_sz 5
#define red_min 0
#define red_max 21 * line_cache_sz
#define green_min 30 * line_cache_sz
#define green_max 90 * line_cache_sz
#define blue_min 91 * line_cache_sz
#define blue_max 149 * line_cache_sz
#define red2_min 150 * line_cache_sz
#define red2_max 185 * line_cache_sz

	CvSize sz;
	IplImage *out;
	unsigned char *h, *s, *e;
	void *tmp;
	int x, y, i, j, cache;

	h = (unsigned char *)hue->imageData;
	s = (unsigned char *)sat->imageData;
	e = (unsigned char *)sat_edge->imageData;

	if (hue->width != sat->width || hue->width != sat_edge->width ||
	    hue->height != sat->height || hue->height != sat_edge->height) {
		fprintf(stderr, "vis_find_blobs_through_scanlines, size "
				"mismatch\n");
		exit(1);
	}

        sz.width = hue->width;
        sz.height = hue->height;
        
        out = cvCreateImage(sz, image_depth, 1);
	if (!out) {
		fprintf(stderr, "Couldn't allocate in find_blobs_through_sl\n");
		exit(1);
	}

	memset(blobs, 0, sizeof(blobs));
	num_blobs = 0;
	spans = spans_a;
	ospans = spans_b;
	memset(spans, 0, sizeof(spans_a));
	memset(ospans, 0, sizeof(spans_a));
	span = 0;

	for (y = 0; y < hue->height; y++) {
		memset(ospans, 0, sizeof(spans_a));
		/* swap */
		tmp = ospans;
		ospans = spans;
		spans = (blob_position *)tmp;

		/* Swap sizes too */
		ospan = span;

		cache = 0;
		span = 0;

		for (x = 0; x < line_cache_sz - 1; x++)
			cache += gethue(x, y);

		for (x = line_cache_sz - 1; x < hue->width; x++) {
			cache += gethue(x, y);

			if (getsat(x, y) >= 60) {
				if (cache <= red_max && cache >= red_min)
					put(x, y) = RED;
				else if (cache <= blue_max && cache >= blue_min)
					put(x, y) = BLUE;
				else if (cache <= green_max &&
					cache >= green_min)
					put(x, y) = GREEN;
				else if (cache <= red2_max && cache >= red2_min)
					put(x, y) = RED;
				else
					put(x, y) = NOTHING;
			} else {
				put(x, y) = NOTHING;
			}

			cache -= gethue(x-line_cache_sz+1, y);

#warning fixme overflow
			if (put(x-1, y) != put(x, y)) { /* Start/end span */
				/* First, end the current span. Insert here
				 * some logic to make sure the first span slot
				 * isn't discarded each time */
				if (spans[span].y1 != y ||
				    spans[span].colour == NOTHING)
					goto trumpets;
#warning beans
				/* Reject anything not 10 in length */
				if (x - spans[span].x1 <= 10)
					goto trumpets;

				spans[span].x2 = x;
				spans[span].y2 = y;
				spans[span].miny = spans[span].maxy = y;
				spans[span].maxx = x;
				spans[span].minx = spans[span].x1;
				span++;
				trumpets:
				spans[span].x1 = x;
				spans[span].y1 = y;
				spans[span].colour = put(x, y);
			}

			if (span >= SPANS) {
				fprintf(stderr, "Out of spans storage\n");
				break;
			}
		}

		if (spans[span].colour != NOTHING)
			span++;

#warning won't work on blocks joining at either side

		/* After processing one scan line, see whether there are
		 * similar spans of colour on the higher scanline */

/* If we have one span above another, the endpoints of the span obviously won't
 * be exactly above each other, so we have to decide how close they need to
 * be to be acceptably next to each other. Let's try 10. */

#define span_match_fuzz 30
		for (i = 0; i < span; i++) {
			for (j = 0; j < ospan; j++) {
				if (abs(spans[i].x1 - ospans[j].x1) < span_match_fuzz && abs(spans[i].x2 - ospans[j].x2) < span_match_fuzz && ospans[j].colour == spans[i].colour)  {
					spans[i].maxx =
					       MAX(ospans[j].maxx, spans[i].x2);
					spans[i].minx =
					       MIN(ospans[j].minx, spans[i].x1);
					spans[i].maxy =
					       MAX(ospans[j].maxy, spans[i].y2);
					spans[i].miny = ospans[j].miny;
					/* Mark that ospan as having been used*/
					ospans[j].colour = NOTHING;
					break; /* Out of ospan loop */
				}
			}
		}

		for (j = 0; j < ospan; j++) {
			if (ospans[j].colour != NOTHING) {
				/* Not used, block is dead */
				/* Check if it's of any significance */
				if (ospans[j].maxx - ospans[j].minx > 20 &&
				    ospans[j].maxy - ospans[j].miny > 20) {
					add_blob(ospans[j].minx, ospans[j].miny,
						 ospans[j].maxx, ospans[j].maxy);
				}
			}
		}

	}

	cvReleaseImage(&out);
	return blobs;
#undef getsat
#undef gethue
#undef getedge
#undef put
}

