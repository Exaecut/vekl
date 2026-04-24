#pragma once

struct FrameParams {
	unsigned int out_pitch;
	unsigned int in_pitch;
	unsigned int dest_pitch;
	unsigned int width;
	unsigned int height;
	float        time;
	float        progress;
	unsigned int bpp;
	unsigned int pixel_layout;
};
