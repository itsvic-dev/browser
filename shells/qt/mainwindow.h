#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qboxlayout.h"
#include "qlabel.h"
#include "qmainwindow.h"
#include "qwidget.h"

class MainWindow : public QMainWindow {
public:
  MainWindow(QWidget *parent = nullptr);

private:
  QWidget *m_centralWidget;
};

#endif // MAINWINDOW_H
