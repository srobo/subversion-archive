#ifndef _BOARDS_SLUG_VISION_ROBOVIS_VISFUNC_H_
#define _BOARDS_SLUG_VISION_ROBOVIS_VISFUNC_H_

#include <cv.h>

IplImage *vis_do_smooth(IplImage *src);
IplImage *vis_do_roberts_edge_detection(IplImage *src);
IplImage *vis_do_sobel_edge_detection(IplImage *src);
IplImage *vis_normalize_plane(IplImage *src);
IplImage *vis_threshold(IplImage *src, unsigned char low, unsigned char high);


#endif /* _BOARDS_SLUG_VISION_ROBOVIS_VISFUNC_H_ */
