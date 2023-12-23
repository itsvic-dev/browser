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
  renderElement(document->body, viewport, 0, 0);
}

size_t Renderer::renderElement(std::shared_ptr<LibDOM::Element> element,
                               std::shared_ptr<Viewport> viewport, size_t x,
                               size_t y) {
  // don't even attempt to draw if we're out of the viewport
  if (x >= viewport->getWidth() || y >= viewport->getHeight())
    return 0;

  size_t mainAxisOffset = 0; // our main axis is hardcoded to be vertical and rn
                             // nothing can change this
  for (const auto &child : element->childNodes) {
    switch (child->nodeType) {
      case LibDOM::Node::ELEMENT_NODE:
        mainAxisOffset +=
            renderElement(std::static_pointer_cast<LibDOM::Element>(child),
                          viewport, x, y + mainAxisOffset);
        break;
      case LibDOM::Node::TEXT_NODE:
        mainAxisOffset +=
            renderText(std::static_pointer_cast<LibDOM::Text>(child), viewport,
                       x, y + mainAxisOffset);
        break;
      case LibDOM::Node::COMMENT_NODE:
        break;
      default:
        std::wcout << "[LibDOMRenderer] unknown type " << child->nodeType
                   << "\n";
        throw 0;
    }
  }

  return mainAxisOffset;
}

size_t Renderer::renderText(std::shared_ptr<LibDOM::Text> text,
                            std::shared_ptr<Viewport> viewport, size_t x,
                            size_t y) {
  // don't even attempt to draw if we're out of the viewport
  if (x >= viewport->getWidth() || y >= viewport->getHeight())
    return 0;

  long xOffset = 0;
  long yOffset = 0;
  size_t mainAxisOffset = 16; // FIXME: shouldnt be hardcoded

  FT_GlyphSlot slot = m_timesNewRoman->glyph;

  for (size_t i = 0; i < text->data.size(); i++) {
    if (FT_Load_Char(m_timesNewRoman, text->data[i], FT_LOAD_RENDER))
      continue; // ignore errors

    if (xOffset + x + (slot->advance.x >> 6) >= viewport->getWidth()) {
      xOffset = 0;
      yOffset += 16; // FIXME: shouldnt be hardcoded
    }

    putBitmap(&slot->bitmap, viewport, x + xOffset + slot->bitmap_left,
              y + yOffset - slot->bitmap_top + 16);

    xOffset += slot->advance.x >> 6;
    yOffset += slot->advance.y >> 6;

    // size_t endY = slot->bitmap.rows + yOffset;
    // if (endY >= mainAxisOffset) {
    //   mainAxisOffset = endY;
    // }
  }

  return mainAxisOffset + yOffset;
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
      auto viewportIdx = (y + j) * vWidth + (x + i);
      unsigned char clr = 255 - bitmap->buffer[bitmapIdx];
      viewport->getBuffer()[viewportIdx] = {clr, clr, clr};
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
