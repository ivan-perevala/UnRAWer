/*
 * UnRAWer - camera raw batch processor on top of OpenImageIO
 * Copyright (c) 2023 Erium Vladlen.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <string>
#include <vector>
#include "ui.h"
#include "LOG.H"

#ifndef SETTINGS_H
#define SETTINGS_H

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

struct Settings {
	bool conEnable, useSbFldr;
	uint rangeMode;
	int lutMode, sharp_mode;
	int fileFormat, defFormat;
	int bitDepth, defBDepth;
	int rawRot;
	uint rawSpace, numThreads;
	int dDemosaic;
	float mltThreads;
	uint verbosity;

	std::vector<std::string> out_formats = { "tif", "exr", "png", "jpg", "jp2", "ppm" };
	std::string ocioConfig, dLutPreset;
	std::map<std::string, std::string> lut_Preset;
	const std::string sharp_kerns[13] = {"gaussian", "sharp-gaussian", "box", "triangle",
		"blackman-harris", "mitchell", "b-spline", "catmull-rom", "lanczos3",
		"disk", "binomial", "laplacian", "median" };
	uint sharp_kernel;		// index of the kernel in sharp_kerns[] array
	float sharp_width;
	float sharp_contrast;
	float sharp_tresh;

	const int raw_rot[5] = { -1, 0, 3, 5, 6 }; // -1 - Auto EXIF, 0 - Unrotated/Horisontal, 3 - 180 Horisontal, 5 - 90 CW Vertical, 6 - 90 CCW Vertical
	const uint rngConv[4] = { 0, 1, 2, 3}; // 0 - unsigned, 1 - signed, 2 - unsigned -> signed, 3 - signed -> unsigned
	const std::string rawCspace[11] = { "Raw", "sRGB", "sRGB-linear", "Adobe", "Wide", "ProPhoto", "ProPhoto-linear", "XYZ", "ACES", "DCI-P3", "Rec2020" };
	const std::string demosaic[15] = { "raw data", "none", "linear", "VNG", "PPG", "AHD", "DCB", "", "", "", "", "", "", "DHT", "AAHD"};

	struct rawparms {
		int use_camera_wb;
		int use_camera_matrix;
		int highlight;
		float aber[2];
		float exp_correc;
		int half_size;
		float denoise_thr;
	} rawParms;

	Settings() {
		reSettings();
	}

	// reset settings to default
	void reSettings() {
		conEnable = true;	// Console enabled/disabled
		useSbFldr = false;	// Use subfolder for output
		verbosity = 3;		// Verbosity level: 0 - none, 1 - errors, 2 - warnings, 3 - info, 4 - debug, 5 - trace
		lutMode = 0;		// LUT mode: -1 - disabled, 0 - Smart, 1 - Force
		dLutPreset = "";	// Default LUT preset, top one

		numThreads = 5;		// Number of threads: 0 - auto, >0 - number of threads
		rangeMode = 0;		// Float type: 0 - unsigned, 1 - signed, 2 - unsigned -> signed, 3 - signed -> unsigned
		fileFormat = -1;	// File format: -1 - original, 0 - TIFF, 1 - OpenEXR, 2 - PNG, 3 - JPEG, 4 - JPEG-2000, 5 - PPM
		defFormat = 0;		// Default file format = TIFF
		bitDepth = -1;		// Bit depth: -1 - Original, 0 - uint8, 1 - uint16, 2 - uint32, 3 - uint64, 4 - half, 5 - float, 6 - double
		defBDepth = 1;		// Default bit depth = uint16
		
		rawRot = -1;		// Raw rotation: -1 - Auto EXIF, 0 - Unrotated/Horisontal, 3 - 180 Horisontal, 5 - 90 CW Vertical, 6 - 90 CCW Vertical
		rawSpace = 1;
		dDemosaic = 3;
		
		ocioConfig = "";

		sharp_mode = 1;		// Sharpening mode: -1 - disabled, 0 - Smart, 1 - Force
		sharp_kernel = 0;
		sharp_width = 3.0f;
		sharp_contrast = 0.5f;
		sharp_tresh = 0.125f;

		rawParms.use_camera_wb = 1;		// If possible, use the white balance from the camera. 
		rawParms.use_camera_matrix = 1;
		// 0: do not use embedded color profile
		// 1 (default) : use embedded color profile(if present) for DNG files(always); for other files only if use_camera_wb is set;
		// 3: use embedded color data(if present) regardless of white balance setting.
		rawParms.highlight = 0;			// 0-9: Highlight mode (0=clip, 1=unclip, 2=blend, 3+=rebuild).
		rawParms.aber[0] = 1.0f;
		rawParms.aber[1] = 1.0f;
		rawParms.exp_correc = 1.0f;
		rawParms.half_size = 0;			// Half-size raw image (1=yes). For some formats, it affects RAW data reading.
		rawParms.denoise_thr = 0.0f;	// Threshold for wavelet denoising
	}

	// get bit depth in bytes
	int getBitDepth() {
		switch (bitDepth) {
		case 0:
			return 1;
		case 1:
			return 2;
		case 2:
			return 4;
		case 3:
			return 8;
		case 4:
			return 2;
		case 5:
			return 4;
		case 6:
			return 8;
		}
		return 0;
	}
};

extern Settings settings;

bool loadSettings(Settings& settings, const std::string& filename);
void printSettings(Settings& settings);
#endif