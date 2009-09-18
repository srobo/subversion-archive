#ifndef _BOARDS_SLUG_VISION_ROBOVIS_VISFUNC_H_
#define _BOARDS_SLUG_VISION_ROBOVIS_VISFUNC_H_

#include <cv.h>

#define MAX_BLOBS 32

struct blob_position {  
	int x1;
	int y1;
	int x2; 
	int y2;
	int minx, maxx;
	int miny, maxy;
	int colour;
#define NOTHING 0
#define RED 1
#define BLUE 2
#define GREEN 3
};

IplImage *vis_do_smooth(IplImage *src);
IplImage *vis_do_roberts_edge_detection(IplImage *src, IplImage **direction);
IplImage *vis_do_sobel_edge_detection(IplImage *src, IplImage **direction);
IplImage *vis_normalize_plane(IplImage *src);
IplImage *vis_threshold(IplImage *src, unsigned char low, unsigned char high);
IplImage *vis_clip(IplImage *src, IplImage *clip, unsigned char low,
						unsigned char high);
IplImage *vis_nonmaximal_supression(IplImage *src, IplImage *direction);
void vis_follow_edge(IplImage *src, IplImage *direction, int x, int y);
void vis_follow_edge_backwards(IplImage *src, IplImage *direction, int x, int y,
				int minx, int miny, int maxx, int maxy);
struct blob_position *vis_search_for_blobs(IplImage *img, IplImage *dir,
							int spacing);
struct blob_position *vis_find_blobs_through_scanlines(IplImage *hue,
					IplImage *sat);

#endif /* _BOARDS_SLUG_VISION_ROBOVIS_VISFUNC_H_ */
