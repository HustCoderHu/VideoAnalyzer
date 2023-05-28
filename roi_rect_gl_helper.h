#pragma once

#include <QWidget>

class QOpenGLFunctions_3_3_Core;

class RoiRectGLHelper : public QWidget
{
  Q_OBJECT
public:
  explicit RoiRectGLHelper(QOpenGLFunctions_3_3_Core *glf,
                           QWidget *parent = nullptr);
  ~RoiRectGLHelper();
  void OnInitializeGL();
  void OnPaintGL();
  void mousePressEvent(QMouseEvent* event, const QSize& frame_size,
                       const QSizeF& zoom_size);
  const QRect& GetYUVRect() { return yuv_rect_; }

  void setRectWidth(uint16_t w) { rect_pixel_width_ = w; }
  void setRectHeight(uint16_t h) { rect_pixel_height_ = h; }

private:
  void calcMousePixelPosition(const QPoint &mouse_pos, const QSize& frame_size,
                                 const QSizeF& zoom_size);
  void updateYUVrect(uint32_t pixel_w, uint32_t pixel_h);
  void updateGLVertices(const QSize& frame_size);

  QOpenGLFunctions_3_3_Core *glf_;

  bool gl_inited = false;

  uint16_t rect_pixel_width_ = 16;
  uint16_t rect_pixel_height_ = 16;

  GLuint VAO;
  GLuint VBO;
  GLuint index_ = 0;
  GLint size_ = 2 * 4; // rect

  QPoint mouse_pixel_pos_;
  QRect yuv_rect_;  // 选中方框的像素在 YUV plane 中的位置, qt (0, 0) 在左上
  // x 表示列, y 表示行
  QRectF gl_ndc_rect_;  // 选中方框在 openggl 中的位置
  // 即 NDC (Normalized Device Coordinate), 每维的数值都在 (-1, 1) 之间
  GLfloat vertices_[2 * 4] = {
      -0.5, -0.5,
       0.5, -0.5,
       0.5, 0.5,
      -0.5, 0.5
  };

signals:

};

