#include <QColor>
#include <QRect>

#include "ffmpeg_av_helper.h"

using std::vector;

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
}

bool PacketIsKeyFrame(const AVPacket* packet) {
  return packet->flags & AV_PKT_FLAG_KEY;
}

void GetRectYfromAVFrame(std::vector<uint16_t>& v,
                      const AVFrame* frame, const QRect& pixel_rect) {
  v.reserve(frame->width * frame->height);

  uint8_t* plane_y = frame->data[0] +
      pixel_rect.top() * frame->linesize[0] + pixel_rect.left();
  for (uint16_t row = 0; row < pixel_rect.height(); ++row) {
    std::copy(plane_y, plane_y + pixel_rect.width(), std::back_inserter(v));
    plane_y += frame->linesize[0];
  }
}

QColor RGBfromY(uint8_t y)
{
  return QColor(y, y, y);
}

QColor RGBfromU(uint8_t u)
{

}

QColor RGBfromV(uint8_t v)
{

}
