#include "libdomrenderer/viewport.h"
#include <cstring>

namespace LibDOMRenderer {

Viewport::Viewport(size_t width, size_t height)
    : m_width(width), m_height(height), m_buffer(new Color[width * height]) {
  memset(m_buffer, 255, width * height * sizeof(Color));
}

Viewport::Viewport(size_t width, size_t height, Color *buffer)
    : m_width(width), m_height(height), m_buffer(buffer) {
  if (m_buffer == nullptr)
    return;
  memset(m_buffer, 255, width * height * sizeof(Color));
}

Viewport::~Viewport() { delete[] m_buffer; }

size_t Viewport::getWidth() { return m_width; }
size_t Viewport::getHeight() { return m_height; }
Color Viewport::getPixel(size_t x, size_t y) {
  return m_buffer[y * m_width + x];
}
void Viewport::setPixel(size_t x, size_t y, Color c) {
  m_buffer[y * m_width + x] = c;
}

Viewport *Viewport::getSubViewport(size_t x, size_t y, size_t width,
                                   size_t height) {
  return new SubViewport(x, y, width, height, this);
}

uint8_t *Viewport::getBuffer() { return reinterpret_cast<uint8_t *>(m_buffer); }

SubViewport::SubViewport(size_t x, size_t y, size_t width, size_t height,
                         Viewport *parent)
    : Viewport(width, height, nullptr), m_x(x), m_y(y), m_parent(parent) {}

Color SubViewport::getPixel(size_t x, size_t y) {
  return m_parent->getPixel(x + m_x, y + m_y);
}
void SubViewport::setPixel(size_t x, size_t y, Color color) {
  m_parent->setPixel(x + m_x, y + m_y, color);
}

} // namespace LibDOMRenderer
