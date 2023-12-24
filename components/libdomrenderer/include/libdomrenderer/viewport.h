#ifndef LIBDOMRENDERER_VIEWPORT_H
#define LIBDOMRENDERER_VIEWPORT_H

#include "libdom/node.h"
#include <cstddef>
#include <cstdint>
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
  virtual ~Viewport();

  virtual size_t getWidth();
  virtual size_t getHeight();
  virtual Color getPixel(size_t x, size_t y);
  virtual void setPixel(size_t x, size_t y, Color color);
  /**
  @deprecated Do not use if you are dealing with sub-viewports.
  */
  uint8_t *getBuffer();

  virtual Viewport *getSubViewport(size_t x, size_t y, size_t width,
                                   size_t height);

private:
  size_t m_width;
  size_t m_height;
  Color *m_buffer;
};

class SubViewport : public Viewport {
public:
  SubViewport(size_t x, size_t y, size_t width, size_t height,
              Viewport *parent);

  Color getPixel(size_t x, size_t y);
  void setPixel(size_t x, size_t y, Color color);

private:
  size_t m_x;
  size_t m_y;
  Viewport *m_parent;
};

} // namespace LibDOMRenderer

#endif // LIBDOMRENDERER_VIEWPORT_H
