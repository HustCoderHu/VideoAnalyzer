#include <QtWidgets>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "images_panel.h"

#include "mylog.h"

Q_DECLARE_METATYPE(AVFrame)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  img_panel_ = new ImagesPanel(this);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::InitImgPanel()
{
  img_panel_->InitGLWidgets();
  img_panel_->PutLayout();
  img_panel_->setWindowTitle("image_panel");
  img_panel_->resize(1600, 900);

  setCentralWidget(img_panel_);
}

void MainWindow::slots_on_avframe(AVFrame* frame)
{
  static int frame_id = 0;
  LOG << "frame_id:" << frame_id
      << " key:" << frame->key_frame
      << " " << frame->width << '*' << frame->height
      << " dts:" << frame->pkt_dts << " :"
      << frame->pts;
  ++frame_id;
}
