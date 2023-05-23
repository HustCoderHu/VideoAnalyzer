
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include "decoder_thread.h"
//#include "glwidget.h"

#include <QMainWindow>
extern "C" {
#include <libavcodec/avcodec.h>
}


class ImagesPanel;
class QGridLayout;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void InitImgPanel();

//  void resizeEvent(QResizeEvent *);

public slots:
  void slots_on_avframe(AVFrame *frame);

private:
  Ui::MainWindow *ui;

  QGridLayout *layout_;
  ImagesPanel *img_panel_;
};

#endif // MAINWINDOW_H
