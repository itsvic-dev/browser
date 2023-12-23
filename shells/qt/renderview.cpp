#include "renderview.h"
#include "libdom/document.h"
#include "libdomrenderer/viewport.h"
#include "libhtml/parser.h"
#include "qimage.h"
#include "qpainter.h"
#include "qwidget.h"
#include <exception>
#include <memory>

RenderView::RenderView(QWidget *parent) : QWidget(parent) {}
void RenderView::setHtmlData(QString data) {
  if (m_htmlData == data)
    return;
  m_htmlData = data;

  // create a fresh Document for the parser
  m_parser.document = std::make_shared<LibDOM::Document>();
  m_parser.reset();

  // parse the document
  const QByteArray stringData = m_htmlData.toUtf8();
  const wchar_t eof[] = {EOF};
  try {
    m_parser.parse(stringData.constData(), stringData.length());
    m_parser.parse(eof, 1);
  } catch (std::exception &exc) {
    setHtmlData(QString("<p>Failed to parse website: ") + exc.what() +
                "</p><p>Check the console for details.</p>");
    return;
  }
  update();
}

void RenderView::paintEvent(QPaintEvent *) {
  // get a clean viewport
  m_viewport = std::make_shared<LibDOMRenderer::Viewport>(width(), height());

  // render to it
  m_renderer.renderToViewport(m_parser.document, m_viewport);

  // get a QImage from the renderer viewport
  QImage img(reinterpret_cast<const uchar *>(m_viewport->getBuffer()),
             m_viewport->getWidth(), m_viewport->getHeight(),
             m_viewport->getWidth() * 3, QImage::Format_RGB888);

  // paint it
  QPainter painter(this);
  painter.drawImage(painter.viewport(), img);
}
