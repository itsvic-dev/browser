#ifndef LIBDOMRENDERER_RENDERER_H
#define LIBDOMRENDERER_RENDERER_H

#include "libdom/element.h"
#include "libdom/node.h"
#include "libdom/text.h"
#include "libdomrenderer/viewport.h"
#include <fontconfig/fontconfig.h>
#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace LibDOMRenderer {

class Renderer {
public:
  Renderer();
  ~Renderer();

  void renderToViewport(std::shared_ptr<LibDOM::Document> document,
                        std::shared_ptr<Viewport> viewport);

private:
  /** renderX functions take in a node, a viewport, and X and Y offsets, and
   * return an offset of their main axis */
  size_t renderElement(std::shared_ptr<LibDOM::Element> element,
                       std::shared_ptr<Viewport> viewport, size_t x, size_t y);
  size_t renderText(std::shared_ptr<LibDOM::Text> text,
                    std::shared_ptr<Viewport> viewport, size_t x, size_t y);

  void putBitmap(FT_Bitmap *bitmap, std::shared_ptr<Viewport> viewport, long x,
                 long y);

  std::string findFontFile(std::string family);

  FT_Library m_freetype;
  FT_Face m_timesNewRoman; // should be changeable at some point lol
  FcConfig *m_fontConfig;
};

} // namespace LibDOMRenderer

#endif
