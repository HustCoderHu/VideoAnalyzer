#pragma once

#include <QTableWidget>
#include <vector>

using std::vector;

class YUVGridViewer : public QTableWidget
{
public:
  YUVGridViewer(QWidget *parent = nullptr);

  void setRowColAndGridWidth(uint16_t rows, uint16_t cols,
                             uint16_t grid_width);
  void UpdateTableItem(vector<uint16_t> plane, uint16_t frame_width);

protected:
//  void resizeEvent(QResizeEvent *event) override;

private:
//  QTableView *frozenTableView;
//  void init();
//  void updateFrozenTableGeometry();
};
