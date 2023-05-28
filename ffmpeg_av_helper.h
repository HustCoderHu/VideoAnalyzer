#pragma once

#include <vector>
#include <stdint.h>

class QColor;
class AVFrame;

/**
 * @brief GetRectYfromAVFrame
 * @param v
 * @param frame
 * @param pixel_rect 希望获取像素的方框坐标
 */
void GetRectYfromAVFrame(std::vector<uint16_t>& v, const AVFrame* frame,
                      const QRect& pixel_rect);

// @todo nv12, nv21

QColor RGBfromY(uint8_t y);
QColor RGBfromU(uint8_t u);
QColor RGBfromV(uint8_t v);
