#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  MainWindow window;
  window.resize(800, 600);
  window.show();
  window.setWindowTitle("Browser");

  return app.exec();
}
