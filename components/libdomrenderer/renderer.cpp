#include "libdomrenderer/renderer.h"
#include "fontconfig/fontconfig.h"
#include "libdom/document.h"
#include "libdom/element.h"
#include "libdom/node.h"
#include "libdom/text.h"
#include <iostream>
#include <memory>

namespace LibDOMRenderer {

Renderer::Renderer() {
  m_fontConfig = FcInitLoadConfigAndFonts();
  if (FT_Init_FreeType(&m_freetype)) {
    std::cout << "FreeType failed to initialize\n";
    throw 0;
  }
  auto font = findFontFile("Times New Roman");
  if (FT_New_Face(m_freetype, font.c_str(), 0, &m_timesNewRoman)) {
    std::cout << "FreeType failed to load font \"" << font << "\"\n";
    throw 0;
  }
  FT_Set_Pixel_Sizes(m_timesNewRoman, 0, 16);
}
Renderer::~Renderer() { FcFini(); }

void Renderer::renderToViewport(std::shared_ptr<LibDOM::Document> document,
                                std::shared_ptr<Viewport> viewport) {
  if (document->body == nullptr)
    return;
  (void)viewport;
  // renderElement(document->body, viewport, 0, 0);
}

void Renderer::putBitmap(FT_Bitmap *bitmap, std::shared_ptr<Viewport> viewport,
                         long x, long y) {
  long vWidth = viewport->getWidth();
  long vHeight = viewport->getHeight();
  for (long j = 0; j < bitmap->rows; j++) {
    for (long i = 0; i < bitmap->width; i++) {
      if ((x + i) >= vWidth || (x + i) < 0 || (y + j) >= vHeight || (y + j) < 0)
        continue;
      auto bitmapIdx = j * bitmap->width + i;
      unsigned char clr = 255 - bitmap->buffer[bitmapIdx];
      viewport->setPixel(x + i, y + j, {clr, clr, clr});
    }
  }
}

std::string Renderer::findFontFile(std::string family) {
  FcPattern *pattern =
      FcNameParse(reinterpret_cast<const FcChar8 *>(family.c_str()));
  FcConfigSubstitute(m_fontConfig, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);

  std::string retVal;

  FcResult res = FcResultNoMatch;
  FcPattern *font = FcFontMatch(m_fontConfig, pattern, &res);
  if (font) {
    FcChar8 *file = NULL;
    if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
      retVal = reinterpret_cast<char *>(file);
    }
    FcPatternDestroy(font);
  }
  FcPatternDestroy(pattern);

  return retVal;
}

} // namespace LibDOMRenderer
