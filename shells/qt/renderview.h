#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include "libdomrenderer/renderer.h"
#include "libdomrenderer/viewport.h"
#include "libhtml/parser.h"
#include "qevent.h"
#include "qtmetamacros.h"
#include "qwidget.h"
#include <memory>

class RenderView : public QWidget {
  Q_OBJECT

public:
  RenderView(QWidget *parent = nullptr);

public slots:
  void setHtmlData(QString data);

protected:
  void repaint();
  void paintEvent(QPaintEvent *event);

private:
  LibHTML::Parser m_parser;
  QString m_htmlData;
  std::shared_ptr<LibDOMRenderer::Viewport> m_viewport;
  LibDOMRenderer::Renderer m_renderer;
};

#endif
