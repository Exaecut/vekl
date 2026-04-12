#pragma once

struct FrameParams {
	uint out_pitch;
	uint in_pitch;
	uint dest_pitch;
	uint width;
	uint height;
	float progress;
	uint bpp;
	uint pixel_layout; // 0=RGBA, 1=BGRA, 2=VUYA601, 3=VUYA709
};
