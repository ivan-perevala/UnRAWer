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
#ifndef _UNRAWER_IMAGEIO_HPP
#define _UNRAWER_IMAGEIO_HPP

#include <iomanip>
#include <iostream>
#include <math.h>
#include <string>

#include "unrawer/timer.hpp"
#include "unrawer/ui.hpp"

#include <Imath/half.h>
#include <OpenImageIO/half.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/imagebufalgo_util.h>
#include <OpenImageIO/imagecache.h>
#include <OpenImageIO/imageio.h>
#include <libraw/libraw.h>

using namespace OIIO;

// void pbar_color_rand(QProgressBar* progressBar);
void pbar_color_rand(MainWindow *mainWindow);
bool m_progress_callback(void *opaque_data, float portion_done);

TypeDesc getTypeDesc(int bit_depth);
std::string formatText(TypeDesc format);
void formatFromBuff(ImageBuf &buf);

// bool img_load(ImageBuf& outBuf, const std::string& inputFileName, QProgressBar* progressBar, MainWindow* mainWindow);
std::pair<bool, std::pair<std::shared_ptr<ImageBuf>, TypeDesc>>
img_load(const std::string &inputFileName, QProgressBar *progressBar, MainWindow *mainWindow);

bool img_write(std::shared_ptr<ImageBuf> out_buf,
               const std::string &outputFileName,
               TypeDesc out_format,
               TypeDesc orig_format,
               QProgressBar *progressBar,
               MainWindow *mainWindow);

bool makePath(const std::string &out_path);

bool thumb_load(ImageBuf &outBuf, const std::string inputFileName, MainWindow *mainWindow);

void debugImageBufWrite(const ImageBuf &buf, const std::string &filename);

#endif // !_UNRAWER_IMAGEIO_HPP
