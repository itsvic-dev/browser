#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "renderview.h"
// no fuck off
#include "qboxlayout.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qmainwindow.h"
#include "qnetworkaccessmanager.h"
#include "qtmetamacros.h"
#include "qwidget.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);

public slots:
  void onUrlChanged();

private:
  QWidget *m_centralWidget;
  QLineEdit *m_urlBar;
  QNetworkAccessManager *m_accessManager;
  QNetworkRequest m_request;

  RenderView *m_renderView;
};

#endif // MAINWINDOW_H
