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
  void putBitmap(FT_Bitmap *bitmap, std::shared_ptr<Viewport> viewport, long x,
                 long y);

  std::string findFontFile(std::string family);

  FT_Library m_freetype;
  FT_Face m_timesNewRoman; // should be changeable at some point lol
  FcConfig *m_fontConfig;
};

} // namespace LibDOMRenderer

#endif
