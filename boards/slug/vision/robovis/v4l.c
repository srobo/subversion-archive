#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#ifdef __NetBSD__
#include <sys/videoio.h>
#elif defined(__linux__)
#include <linux/videodev2.h>
#else
#error What OS is this? Where am I? Are those my feet?
#endif
#include <sys/mman.h>

#define DEBUG

#ifdef DEBUG
#define DFPRINTF(x) fprintf x
#else
#define DFPRINTF(x)
#endif

#define BUFFERS 1

static int fd;

struct {
	uint8_t *start;
	size_t length;
} *buffers;

int
open_webcam(unsigned int desired_width, unsigned int desired_height)
{
	unsigned int i;
	int ret, type;
	struct v4l2_format format;
	struct v4l2_capability cap;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer buffer;
	struct v4l2_control control;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	fd = open("/dev/video0", O_RDWR, 0);
	if (fd == -1) {
		perror("Couldn't open /dev/video0");
		return 1;
	}

	memset(&format, 0, sizeof(format));
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_G_FMT, &format);
	if (ret < 0) {
		perror("Couldn't get format");
		return 1;
	}

	DFPRINTF((stderr, "buf type %d\n", format.type));
	DFPRINTF((stderr, "pix data %d %d\n", format.fmt.pix.width,
				format.fmt.pix.height));
#define pix format.fmt.pix.pixelformat
	DFPRINTF((stderr, "%c%c%c%c\n", pix & 0xFF, (pix >> 8) & 0xFF,
				(pix >> 16) & 0xFF, pix >> 24));
#undef pix
	DFPRINTF((stderr, "field is %d\n", format.fmt.pix.field));
	DFPRINTF((stderr, "bytesperline is %d\n", format.fmt.pix.bytesperline));

	memset(&format, 0, sizeof(format));
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = desired_width;
	format.fmt.pix.height = desired_height;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	format.fmt.pix.bytesperline = 2 * desired_width;
	format.fmt.pix.sizeimage = 2 * desired_width * desired_height;

	ret = ioctl(fd, VIDIOC_S_FMT, &format);
	if (ret < 0) {
		perror("Couldn't set format");
		return 1;
	}

	printf("Set format worked\n");

	memset(&cap, 0, sizeof(cap));
	ioctl(fd, VIDIOC_QUERYCAP, &cap);
	DFPRINTF((stderr,
		"Capability driver is %s, card %s, version %X, cap %X\n",
		cap.driver, cap.card, cap.version, cap.capabilities));

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf(stderr, "Oh noes, no streaming capability\n");
		return 1;
	}

	memset(&control, 0, sizeof(control));
	control.id = V4L2_CID_AUTO_WHITE_BALANCE;
	control.value = 1;
	if (ioctl(fd, VIDIOC_S_CTRL, &control) < 0) {
		perror("Couldn't set whitebalancing on");
	}

	memset(&reqbuf, 0, sizeof(reqbuf));
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count = BUFFERS;
	ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
	if (ret < 0) {
		perror("Couldn't reqbufs");
		return 1;
	}

	DFPRINTF((stderr, "%d buffers aquired\n", reqbuf.count));

	buffers = calloc(reqbuf.count, sizeof(*buffers));
	if (buffers == NULL) {
		printf("Couldn't allocate buffer ptrs\n");
		return 1;
	}

	for (i = 0; i < reqbuf.count; i++) {
		memset(&buffer, 0, sizeof(buffer));
		buffer.type = reqbuf.type;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = i;

		ret = ioctl(fd, VIDIOC_QUERYBUF, &buffer);
		if (ret < 0) {
			perror("Vidioc querybuf req failure\n");
			return 1;
		}

		buffers[i].length = buffer.length;
		buffers[i].start = mmap(NULL, buffer.length,
					PROT_READ | PROT_WRITE, MAP_SHARED,
					fd, buffer.m.offset);

		if (buffers[i].start == MAP_FAILED) {
			perror("mmap failed");
			return 1;
		}

		memset(buffers[i].start, 0x41, buffers[i].length);
	}

	ret = ioctl(fd, VIDIOC_STREAMON, &type);
	if (ret < 0) {
		perror("Starting stream failed");
		return 1;
	}

	return 0;
}

uint8_t *
get_v4l_frame()
{
	struct v4l2_buffer buffer;
	int ret;

	memset(&buffer, 0, sizeof(buffer));
	buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer.memory = V4L2_MEMORY_MMAP;
	buffer.index = 0;

	/* Queue it */
	ret = ioctl(fd, VIDIOC_QBUF, &buffer);
	if (ret < 0) {
		perror("queueing buffer failed");
		return NULL;
	}

	/* Now dequeue it */
	ret = ioctl(fd, VIDIOC_DQBUF, &buffer);
	if (ret < 0) {
		perror("couldn't dequeue buffer");
		return NULL;
	}
	DFPRINTF((stderr, "dequeued buffer is size %d?\n", buffer.length));

	return buffers[0].start;
}

void
close_webcam()
{
	int i, type, ret;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_STREAMOFF, &type);
	if (ret < 0) {
		perror("Stopping stream failed.");
	}

	for (i = 0; i < BUFFERS; i++)
		munmap(buffers[i].start, buffers[i].length);

	free(buffers);
	close(fd);

	/* As far as I can tell, we don't free device buffers, as they don't
	 * belong to us, they belong to the device */
	return;
}

