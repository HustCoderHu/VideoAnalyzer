#include <QOpenGLFunctions_3_3_Core>
#include <QMouseEvent>
#include "roi_rect_gl_helper.h"
#include "mylog.h"

RoiRectGLHelper::RoiRectGLHelper(QOpenGLFunctions_3_3_Core* glf,
                                 QWidget* parent)
    : glf_(glf) {

}

RoiRectGLHelper::~RoiRectGLHelper()
{
  if (! gl_inited)
    return;
  glf_->glDeleteBuffers(1, &VBO);
  glf_->glDeleteVertexArrays(1, &VAO);
}

void RoiRectGLHelper::OnInitializeGL()
{
  glf_->glGenVertexArrays(1, &VAO);
  glf_->glBindVertexArray(VAO);
  glf_->glGenBuffers(1, &VBO);
  glf_->glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glf_->glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vertices_),
                     vertices_,
                     GL_DYNAMIC_DRAW);
  glf_->glVertexAttribPointer(index_, 2, GL_FLOAT, GL_FALSE,
                              2 * sizeof(GL_FLOAT), nullptr);
  glf_->glEnableVertexAttribArray(index_);

  glf_->glBindBuffer(GL_ARRAY_BUFFER, 0);
  glf_->glBindVertexArray(0);
  gl_inited = true;
}

void RoiRectGLHelper::OnPaintGL()
{
  if (! gl_inited)
    OnInitializeGL();

  glf_->glBindVertexArray(VAO);

  glf_->glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glf_->glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vertices_),
                     vertices_,
                     GL_DYNAMIC_DRAW);
  glf_->glBindBuffer(GL_ARRAY_BUFFER, 0);

  glf_->glDrawArrays(GL_LINE_LOOP, 0, 4);

  glf_->glBindVertexArray(0);
}

/**
 * @brief 根据鼠标点击的位置计算 opengl 画框的区域 和 要显示像素值的区域
 * @param
 *
 */
void RoiRectGLHelper::mousePressEvent(QMouseEvent* event, const QSize& frame_size,
                                      const QSizeF& zoom_size)
{
  calcMousePixelPosition(event->pos(), frame_size, zoom_size);
  LOG << "mouse_pixel_pos: " << mouse_pixel_pos_;
  updateYUVrect(rect_pixel_width_, rect_pixel_height_);
  updateGLVertices(frame_size);
}

void RoiRectGLHelper::calcMousePixelPosition(const QPoint& mouse_pos,
                                                const QSize& frame_size,
                                                const QSizeF& zoom_size)
{
  int x = mouse_pos.x() * frame_size.width() / zoom_size.width();
  // opengl 渲染时显示在左上角
  int y= mouse_pos.y() * frame_size.height() / zoom_size.height();
  mouse_pixel_pos_.setX(x);
  mouse_pixel_pos_.setY(y);
}

/**
 * @brief RoiRectGLHelper::updateYUVrect
 * @param pixel_w
 * @param pixel_h
 * @note 左下 left bottom, 右上 right top, y 轴增长方向 top -> bottom
 */
void RoiRectGLHelper::updateYUVrect(uint32_t pixel_w, uint32_t pixel_h)
{
  int left = mouse_pixel_pos_.x() - pixel_w / 2;
  if (left < 0)
    left = 0;
  yuv_rect_.setLeft(left);
  yuv_rect_.setWidth(pixel_w);

  int top = mouse_pixel_pos_.y() - pixel_h / 2;
  if (top < 0)
    top = 0;
  yuv_rect_.setTop(top);
  yuv_rect_.setHeight(pixel_h);
  yuv_rect_.width();
  yuv_rect_.height();
  yuv_rect_.size();
}

void RoiRectGLHelper::updateGLVertices(const QSize& frame_size)
{
  GLfloat left = yuv_rect_.left() * 2.0 / frame_size.width() - 1;
  GLfloat right = (yuv_rect_.left()+yuv_rect_.width())
                      * 2.0 / frame_size.width() - 1;
  gl_ndc_rect_.setLeft(left);
  gl_ndc_rect_.setRight(right);
  // opengl y 轴向上是正, 和 qt 方向相反
  GLfloat bottom = (frame_size.height() - yuv_rect_.top()) * 2.0 / frame_size.height()
                -1;
  GLfloat top = (frame_size.height() - yuv_rect_.top() - yuv_rect_.height())
                       * 2.0 / frame_size.height()
                -1;
  // y 轴增长方向 top -> bottom

  gl_ndc_rect_.setTop(top);
  gl_ndc_rect_.setBottom(bottom);

//  vertices_[0] = left;  vertices_[1] = top;
//  vertices_[2] = right; vertices_[3] = top;
//  vertices_[4] = right; vertices_[5] = bottom;
//  vertices_[6] = left;  vertices_[7] = bottom;

  vertices_[0] = vertices_[6] = left;
  vertices_[1] = vertices_[3] = top;
  vertices_[2] = vertices_[4] = right;
  vertices_[5] = vertices_[7] = bottom;
}
