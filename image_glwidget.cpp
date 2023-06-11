#include <QWheelEvent>
#include "image_glwidget.h"
#include "ffmpeg_av_helper.h"

ImageGLWidget::ImageGLWidget()
{
  roi_rect_gl_helper_ = new RoiRectGLHelper(this);
  yuv_grid_ = new YUVGridViewer;
}

ImageGLWidget::~ImageGLWidget()
{
  yuv_grid_->close();
}

void ImageGLWidget::wheelEvent(QWheelEvent* event)
{
  return;
  //  if (gl_viewport_rect_.isEmpty())
  //    return;

  //  QPoint numDegrees;                  // 定义指针类型参数numDegrees用于获取滚轮转角
  //  numDegrees = event->angleDelta();   // 获取滚轮转角
  //  int step = 0;                       // 设置中间参数step用于将获取的数值转换成整数型
  //  if (!numDegrees.isNull())           // 判断滚轮是否转动
  //  {
  //    step = numDegrees.y();            // 将滚轮转动数值传给中间参数step
  //  }
  //  int new_width = 0;
  //  if (step > 0)
  //    new_width = gl_viewport_rect_.width() * 107 / 100;
  //  else if (step < 0)
  //    new_width = gl_viewport_rect_.width() * 93 / 100;

  //  int new_height = new_width * m_zoomSize.height() / m_zoomSize.width();
  //  QPointF mouse_pos = event->position();
  //  int x = mouse_pos.x() - new_width / 2;
  //  // qt widget 鼠标 Y 坐标和 opengl 相反
  //  // qt 左上角 (0, 0), opengl 左下角 (0, 0)
  //  int y = rect().height() - mouse_pos.y() - new_height / 2;
  //  gl_viewport_rect_.setX(x);
  //  gl_viewport_rect_.setY(y);
  //  gl_viewport_rect_.setWidth(new_width);
  //  gl_viewport_rect_.setHeight(new_height);
  //  LOG << gl_viewport_rect_;
  //  update();
}

void ImageGLWidget::mousePressEvent(QMouseEvent* event)
{
  uint32_t width = 96;
  roi_rect_gl_helper_->setRectWidth(width);
  roi_rect_gl_helper_->setRectHeight(width);
  if (event->button() == Qt::LeftButton) {
    QPoint pos = event->pos();
    roi_rect_gl_helper_->mousePressEvent(event, frame_size_, m_zoomSize);
    vector<uint16_t> y_plane;
    GetRectYfromAVFrame(y_plane, avframe_, roi_rect_gl_helper_->GetYUVRect());
    yuv_grid_->setRowColAndGridWidth(y_plane.size() / width + 1, width, 10);
    yuv_grid_->UpdateTableItem(y_plane, width);
  } else if (event->button() == Qt::RightButton) {
  }
  update();

  if (!yuv_grid_->isVisible())
    yuv_grid_->show();
  else
    yuv_grid_->update();
}

void ImageGLWidget::initializeGL()
{
  GLWidget::initializeGL();
  roi_rect_gl_helper_->OnInitializeGL();
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 指定颜色缓冲区的清除值(背景色)
}

void ImageGLWidget::paintGL()
{
  GLWidget::paintGL();
  roi_rect_gl_helper_->OnPaintGL();
}
