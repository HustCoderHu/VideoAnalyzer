
#pragma once

#include <vector>
#include <map>
#include <QWidget>
#include <QGridLayout>

#include "decoder_thread.h"
#include "image_glwidget.h"
#include "preview_glwidget.h"

using std::vector;
using std::map;

struct AVFrame;

class ImagesPanel : public QWidget
{
  Q_OBJECT
public:
  explicit ImagesPanel(QWidget *parent = nullptr);
  ~ImagesPanel();

  void InitGLWidgets();

  void PutLayout();

signals:
  void signal_require_frames(uint32_t n);
  void signal_frame_consumed(uint32_t frame_id, AVFrame* frame);

private slots:
  void on_frame_decoded(uint32_t frame_id, AVFrame* avframe, AVRational time_base);
  void on_timer();

public:
  map<uint32_t, AVFrame*> frames_map; // frame_id -> AVFrame*
  vector<ImageGLWidget*> image_glwidgets_; // size = 2^x
  // AVFrame record at [ frame_id % glwidgets_.size() ]

  // layout 第一个 widget 的 id
  // 滚动的时候此 id 会跟随变大 or 变小
  // layout 中的 widget 也会滚动
  // i.e. 当前显示 glwidgets_[1],[2],glwidgets_[3],4, 向后滚动 2 个, 就展示 3,4,5,6

  vector<uint32_t> displayed_frame_ids;

  QGridLayout *layout_ = nullptr;
  uint32_t layout_row_ = 3;
  uint32_t layout_col_ = 7;

  PreviewGLWidget* preview_bar_glwidget;

  DecoderThread *dec_obj_ = nullptr;
  QThread *dec_thread_;
  QTimer *timer_ = nullptr;

  int64_t last_frame_play_ms = -1;
  int32_t last_gl_index = -1;
};

