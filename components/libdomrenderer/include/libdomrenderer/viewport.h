#ifndef LIBDOMRENDERER_VIEWPORT_H
#define LIBDOMRENDERER_VIEWPORT_H

#include "libdom/node.h"
#include <cstddef>
#include <memory>

namespace LibDOMRenderer {

typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} __attribute__((packed)) Color;

class Viewport {
public:
  Viewport(size_t width, size_t height);
  Viewport(size_t width, size_t height, Color *buffer);
  ~Viewport();

  size_t getWidth();
  size_t getHeight();
  Color *getBuffer();

private:
  size_t m_width;
  size_t m_height;
  Color *m_buffer;
};

} // namespace LibDOMRenderer

#endif // LIBDOMRENDERER_VIEWPORT_H
