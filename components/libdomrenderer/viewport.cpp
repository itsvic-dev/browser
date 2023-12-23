#include "libdomrenderer/viewport.h"
#include <cstring>

namespace LibDOMRenderer {

Viewport::Viewport(size_t width, size_t height)
    : m_width(width), m_height(height), m_buffer(new Color[width * height]) {
  memset(m_buffer, 255, width * height * sizeof(Color));
}

Viewport::Viewport(size_t width, size_t height, Color *buffer)
    : m_width(width), m_height(height), m_buffer(buffer) {
  memset(m_buffer, 255, width * height * sizeof(Color));
}

Viewport::~Viewport() { delete[] m_buffer; }

size_t Viewport::getWidth() { return m_width; }
size_t Viewport::getHeight() { return m_height; }
Color *Viewport::getBuffer() { return m_buffer; }

} // namespace LibDOMRenderer
