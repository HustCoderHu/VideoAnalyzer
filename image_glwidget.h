
#pragma once

#include "glwidget.h"
#include "roi_rect_gl_helper.h"
#include "yuv_grid_viewer.h"

class ImageGLWidget : public GLWidget
{
  Q_OBJECT
public:
  ImageGLWidget();
  ~ImageGLWidget();

  void wheelEvent(QWheelEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;

protected:
  void initializeGL() override;
  void paintGL() override;

  RoiRectGLHelper *roi_rect_gl_helper_;
  YUVGridViewer *yuv_grid_;
};

