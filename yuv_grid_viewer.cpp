#include <QColor>

#include "yuv_grid_viewer.h"
#include "ffmpeg_av_helper.h"
//#include <QPainter>
//#include <QPen>

YUVGridViewer::YUVGridViewer(QWidget* parent)
    : QTableWidget(parent) { }

void YUVGridViewer::setRowColAndGridWidth(uint16_t rows, uint16_t cols, uint16_t grid_width)
{
  setRowCount(rows);
  setColumnCount(cols);
  for (uint16_t i = 0; i < rows; ++i)
    setRowHeight(i, grid_width);
  for (uint16_t i = 0; i < cols; ++i)
    setColumnWidth(i, grid_width);
}

/**
 * @brief 把 y(or u v) 转换到 RGB 并填充为 QTableWidgetItem 的背景色,
 * 数值也以数字显示在 item 里, 最终呈现为 frame_width * heigth 的表格
 * @param plane: y or u or v
 * @param frame_width
 */
void YUVGridViewer::UpdateTableItem(vector<uint16_t> plane, uint16_t frame_width)
{
  QColor color_white(0xFF, 0xFF, 0xFF, 0x80);
  QColor color_black(0, 0, 0, 0x80);
  uint16_t row = 0;
  for (uint16_t i = 0; i < plane.size(); ++i) {
    row = i / frame_width;
    uint16_t col = i - row * frame_width;
    QTableWidgetItem *item = this->item(row, col);
    if (nullptr == item) {
      item = new QTableWidgetItem;
      setItem(row, col, item);
    }

    uint16_t pixel = plane.at(i);
    if (pixel < 128)
      item->setData(Qt::ForegroundRole, color_white); // 白字黑底
    else
      item->setData(Qt::ForegroundRole, color_black); // 黑字白底

    item->setData(Qt::BackgroundRole, RGBfromY(pixel)); // 背景色
    item->setText( QString::number(pixel) ); // 显示数值
  }
  resizeColumnsToContents();
}

//void YUVGridViewer::paintEvent(QPaintEvent* event)
//{
//  QPainter painter(this);
//  QPen pen;
//  pen.setColor(QColor(0, 0, 0xFF));
//  pen.setWidth(2);
//  painter.setPen(pen);
//  painter.drawRect(1, 1, 4, 4);
//}
