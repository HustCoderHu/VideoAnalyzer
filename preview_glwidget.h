
#pragma once

#include <QPoint>
#include <vector>
#include <deque>

extern "C" {
#include "libavutil/pixfmt.h"
}

#include "glwidget.h"

using std::vector;
using std::deque;

struct AVFrame;
struct SwsContext;

struct MyFrame
{
  uint8_t* data[4];
  int linesize[4];
};

class PreviewGLWidget : public GLWidget
{
  Q_OBJECT
public:
  PreviewGLWidget();
  ~PreviewGLWidget();

  void OnAVFrame(AVFrame* frame);
  void ScaleFrameAndCache(AVFrame* frame);
  void ConcatFrames();
  void PaintFrame();

  void concatY(uint8_t* dst, int linesize,
               AVFrame* src_frame, uint32_t frame_idx);
  void concatUV(uint8_t* dst_u, uint8_t* dst_v,
                int linesize, AVFrame* src_frame, uint32_t frame_idx);

  AVFrame* allocTmpScaledFrame(AVFrame* frame, int w, int h, AVPixelFormat format);
  AVFrame* mayReallocFrame(AVFrame* frame, int w, int h, AVPixelFormat format);
  SwsContext* allocSwsContext(SwsContext* sws_ctx, int src_w, int src_h,
                              AVPixelFormat src_fmt,
                              int dst_w, int dst_h, AVPixelFormat dst_fmt);

  static constexpr int kMaxPreviewFrames = 60;

  SwsContext* sws_ctx_ = NULL;

  // 单个 frame 的大小
  QPoint target_frame_size = QPoint(1920 / 20, 1080 / 20);

  // 多个 frame 水平合并成预览图
  deque<AVFrame*> scaled_frames_;

  AVFrame* tmp_scaled_frame_ = NULL;
  AVFrame* concated_frame_ = NULL;
};
