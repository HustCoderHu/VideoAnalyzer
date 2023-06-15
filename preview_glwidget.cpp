
extern "C" {
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

#include "preview_glwidget.h"
#include "mylog.h"

PreviewGLWidget::PreviewGLWidget()
{

}

PreviewGLWidget::~PreviewGLWidget()
{
  if (sws_ctx_ != NULL)
    sws_freeContext(sws_ctx_);
  if (concated_frame_ != NULL)
    av_frame_free(&concated_frame_);
}

void PreviewGLWidget::OnAVFrame(AVFrame* frame)
{
  ScaleFrameAndCache(frame);
}

void PreviewGLWidget::ScaleFrameAndCache(AVFrame* frame)
{
  AVFrame* new_frame = NULL;
  if (scaled_frames_.size() >= kMaxPreviewFrames) {
    new_frame = scaled_frames_.front();
    scaled_frames_.pop_front();
  }
  new_frame = mayReallocFrame(new_frame,
                              target_frame_size.x(),
                              target_frame_size.y(),
                              AV_PIX_FMT_YUV420P);


  sws_ctx_ = allocSwsContext(sws_ctx_, frame->width, frame->height,
                             (AVPixelFormat)frame->format,
                             new_frame->width, new_frame->height,
                             (AVPixelFormat)new_frame->format);
  int ret = sws_scale(sws_ctx_,
                      frame->data, frame->linesize, 0, frame->height,
                      new_frame->data, new_frame->linesize);
  char av_err_str[AV_ERROR_MAX_STRING_SIZE];
  if (ret < 0) {
    av_make_error_string(av_err_str, AV_ERROR_MAX_STRING_SIZE, ret);
    LOG << "sws_scale ERROR: " << av_err_str;
    // return to front
    av_frame_free(&new_frame);
  } else {
    scaled_frames_.push_back(new_frame);
  }
}

void PreviewGLWidget::ConcatFrames()
{
  if (scaled_frames_.empty())
    return;

  concated_frame_ = mayReallocFrame(concated_frame_,
                                    target_frame_size.x() * scaled_frames_.size(),
                                    target_frame_size.y(),
                                    AV_PIX_FMT_YUV420P);

  uint16_t frame_i = 0;
  uint8_t* dst_y = concated_frame_->data[0];
  int y_linesize = concated_frame_->linesize[0];
  uint8_t* dst_u = concated_frame_->data[1];
  uint8_t* dst_v = concated_frame_->data[2];
  int uv_linesize = concated_frame_->linesize[1];

  for(AVFrame *frame : scaled_frames_) {
    // concate
    concatY(dst_y, y_linesize, frame, frame_i);
    dst_y += frame->width;
    concatUV(dst_u, dst_v, uv_linesize, frame, frame_i);
    dst_u += (frame->width >> 1);
    dst_v += (frame->width >> 1);
  }
}

void PreviewGLWidget::PaintFrame()
{
  repaint(concated_frame_);
}

void PreviewGLWidget::concatY(uint8_t* dst, int linesize,
                              AVFrame* src_frame, uint32_t frame_idx)
{
  uint8_t* src = src_frame->data[0];
  for (uint32_t row = 0; row < src_frame->height; ++row) {
    memcpy(dst, src, src_frame->width);
    dst += linesize;
    src += src_frame->linesize[0];
  }
}

void PreviewGLWidget::concatUV(uint8_t* dst_u, uint8_t* dst_v,
                               int linesize, AVFrame* src_frame,
                               uint32_t frame_idx)
{
  uint8_t* src_u = src_frame->data[1];
  uint8_t* src_v = src_frame->data[2];
  uint32_t uv_width = src_frame->width >> 1;
  uint32_t src_h = src_frame->height >> 1;
  for (uint32_t row = 0; row < src_h; ++row) {
    memcpy(dst_u, src_u, uv_width);
    memcpy(dst_v, src_v, uv_width);

    dst_u += linesize;
    dst_v += linesize;
    src_u += src_frame->linesize[1];
    src_v += src_frame->linesize[2];
  }
}

AVFrame* PreviewGLWidget::mayReallocFrame(AVFrame* frame,
                                             int w, int h, AVPixelFormat format)
{
  if (frame != NULL) {
    if (frame->width == w && frame->height == h && frame->format == format) {
      return frame;
    }
    av_frame_unref(frame);
    av_frame_free(&frame);
  }

  frame = av_frame_alloc();
  if (NULL == frame) {
    LOG << "ERROR: av_frame_alloc return NULL";
    return NULL;
  }

  frame->width = w;
  frame->height = h;
  frame->format = format;
  int ret = av_frame_get_buffer(frame, 0);
  if (ret < 0) {
    char av_err_str[AV_ERROR_MAX_STRING_SIZE];
    av_make_error_string(av_err_str, AV_ERROR_MAX_STRING_SIZE, ret);
    LOG << "av_frame_get_buffer ERROR: " << av_err_str;
    av_frame_unref(frame);
    av_frame_free(&frame);
    return NULL;
  }
  return frame;
}

SwsContext* PreviewGLWidget::allocSwsContext(SwsContext* sws_ctx,
                                             int src_w, int src_h,
                                             AVPixelFormat src_fmt,
                                             int dst_w, int dst_h,
                                             AVPixelFormat dst_fmt)
{
  int sws_flags = SWS_FAST_BILINEAR; // SWS_BICUBIC;
  sws_ctx = sws_getCachedContext(sws_ctx_,
                                  src_w, src_h, src_fmt,
                                  dst_w, dst_h, dst_fmt,
                                  sws_flags, NULL, NULL, NULL);
  return sws_ctx;
}
