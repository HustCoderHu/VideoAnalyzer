
#include <QApplication>
#include <QSurfaceFormat>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  QSurfaceFormat format;
  format.setSamples(4);
  QSurfaceFormat::setDefaultFormat(format);
  MainWindow w;
//  w.show();
//  w.showFullScreen();
  w.InitImgPanel();
  w.showMaximized();
//  w.showMaximized();
  return a.exec();
}
