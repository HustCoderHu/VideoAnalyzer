#include <QTimer>
#include "images_panel.h"
#include "mylog.h"

ImagesPanel::ImagesPanel(QWidget *parent)
    : QWidget{parent}
{
  const char* file = "D:/docs/movie/.unknown/6798973.mp4";
  file = "F:/docs/test_videos/Wildlife.wmv";
  file = "F:/docs/test_videos/20230511092644_origin.mp4";
//  file = "Z:/coding/github/steins_gate_cut.mkv";
//  file = "z:/coding/github/VideoAnalyzer/cut.mkv";
//  file = "Z:/download/DownKyi-1.5.9/Media/鬼泣5维吉尔出场全cg动画/1-鬼泣5维吉尔出场全cg动画-1080P 高清-AVC.mp4";
  dec_obj_ = new DecoderThread;
  dec_obj_->Init(file);
  dec_thread_ = new QThread;
  dec_obj_->moveToThread(dec_thread_);
  dec_thread_->start();

  connect(dec_thread_, &QThread::finished, dec_obj_, &QObject::deleteLater);
  connect(dec_thread_, &QThread::finished, dec_thread_, &QObject::deleteLater);
  connect(this, &ImagesPanel::signal_require_frames,
          dec_obj_, &DecoderThread::on_require_frames);
  connect(dec_obj_, &DecoderThread::signal_frame_decoded,
          this, &ImagesPanel::on_frame_decoded);
  connect(this, &ImagesPanel::signal_frame_consumed,
          dec_obj_, &DecoderThread::on_frame_consumed);

  timer_ = new QTimer(this);
  connect(timer_, &QTimer::timeout, dec_obj_, &DecoderThread::on_timer);
  connect(timer_, &QTimer::timeout, this, &ImagesPanel::on_timer);
  timer_->start(1000);
}

ImagesPanel::~ImagesPanel()
{
  dec_thread_->quit();
  dec_thread_->wait();
}

void ImagesPanel::InitGLWidgets()
{
  if (!layout_)
    layout_ = new QGridLayout;

//  uint32_t glw_idx = 0;
  for (uint32_t row = 0; row < layout_row_; ++row) {
    for (uint32_t col = 0; col < layout_col_; ++col) {
      ImageGLWidget *glw = new ImageGLWidget;
      image_glwidgets_.push_back(glw);
      layout_->addWidget(glw, row, col);
    }
    layout_->setRowMinimumHeight(row, 1080 / layout_row_ * 2 / 5);
  }
  preview_bar_glwidget = new PreviewGLWidget;
  layout_->addWidget(preview_bar_glwidget, layout_row_, 0, 1, layout_col_);
//  layout_->setRowStretch(layout_row_, 0.5);
//  layout_->setRowMinimumHeight(layout_row_, 1080 / 40);
  setLayout(layout_);
}

void ImagesPanel::PutLayout()
{

}

void ImagesPanel::on_frame_decoded(uint32_t frame_id, AVFrame* avframe,
                                   AVRational time_base)
{
//  timer_->stop();
  LOG << "fid: " << frame_id
      << " key:" << avframe->key_frame
      << " pts:" << avframe->pkt_pos
      << " " << avframe->width << "*" << avframe->height
      << " " << avframe;
//  frames_map.emplace(frame_id, avframe);

  if (last_frame_play_ms < 0) {
    image_glwidgets_[0]->repaint(avframe);
    preview_bar_glwidget->OnAVFrame(avframe);
    last_frame_play_ms = avframe->pts * av_q2d(time_base) * 1000;
//    avframe->pkt_dts
    last_gl_index = 0;
  } else {
    int64_t play_ms = avframe->pts * av_q2d(time_base) * 1000;
    if (play_ms >= last_frame_play_ms + 1000) {
      ++last_gl_index;
//      uint32_t glw_idx = frame_id % image_glwidgets_.size();
      image_glwidgets_[last_gl_index % image_glwidgets_.size()]->repaint(avframe);
//      preview_bar_glwidget->OnAVFrame(avframe);
//      last_frame_play_ms = play_ms;
//      if (last_gl_index == 6) {
//        timer_->stop();
//        preview_bar_glwidget->ConcatFrames();
//        preview_bar_glwidget->PaintFrame();
//      }
    }
  }
  emit signal_frame_consumed(frame_id, avframe);
}

void ImagesPanel::on_timer()
{
  emit signal_require_frames(30 * 5); //layout_row_ * layout_col_);
}
