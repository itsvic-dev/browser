#include "mainwindow.h"
#include "qboxlayout.h"
#include "qlabel.h"
#include "qlineedit.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  m_centralWidget = new QWidget;

  auto layout = new QVBoxLayout;

  auto urlLayout = new QHBoxLayout;
  urlLayout->addWidget(new QLabel("URL:"));
  urlLayout->addWidget(new QLineEdit("https://example.com"));
  layout->addLayout(urlLayout);

  layout->addWidget(new QWidget);

  m_centralWidget->setLayout(layout);
  setCentralWidget(m_centralWidget);
}
