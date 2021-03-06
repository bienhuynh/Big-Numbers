#include "stdafx.h"
#include <Math.h>
#include <float.h>
#include <Wintools.h>
#include <PixRect.h>

// ------------------------------------ PolygonCurve ------------------------------

void PolygonCurve::move(const Point2D &dp) {
  for(int i = 0; i < m_points.size(); i++) {
    m_points[i] += dp;
  }
}

Rectangle2D PolygonCurve::getBoundingBox() const {
  return m_points.getBoundingBox();
}

String PolygonCurve::toString() const {
  String result;
  switch(m_type) {
  case TT_PRIM_LINE   : result = "line   :"; break;
  case TT_PRIM_QSPLINE: result = "qspline:"; break;
  case TT_PRIM_CSPLINE: result = "cspline:"; break;
  default             : result = format(_T("unknown type:%d:"), m_type); break;
  }

  const char *delim = "";
  for(int i = 0; i < m_points.size(); i++, delim = "        ") {
    result += format(_T("%s%s\n"), delim, m_points[i].toString().cstr());
  }
  return result;
}

String PolygonCurve::toXML() {
  String type;
  switch(m_type) {
  case TT_PRIM_LINE   : type = "line"   ; break;
  case TT_PRIM_QSPLINE: type = "qspline"; break;
  case TT_PRIM_CSPLINE: type = "cspline"; break;
  }
  String result = "<polygoncurve>\n";
  result += "  <type>" + type + "</type>\n";

  for(int i = 0; i < m_points.size(); i++) {
    result += "  " + m_points[i].toXML();
  }
  result += "</polygoncurve>\n";
  return result;
}

// ------------------------------------ GlyphPolygon ------------------------------

Point2DArray GlyphPolygon::getAllPoints() const {
  Point2DArray result;
  result.add(m_start);
  for(int i = 0; i < m_polygonCurveArray.size(); i++) {
    result.addAll(m_polygonCurveArray[i].getAllPoints());
  }
  return result;
}

Rectangle2D GlyphPolygon::getBoundingBox() const {
  return getAllPoints().getBoundingBox();
}

void GlyphPolygon::move(const Point2D &dp) {
  m_start += dp;
  for(int i = 0; i < m_polygonCurveArray.size(); i++) {
    m_polygonCurveArray[i].move(dp);
  }
}

String GlyphPolygon::toString() const {
  String result = format(_T("start:%s\n"), m_start.toString().cstr());
  for(int p = 0; p < m_polygonCurveArray.size(); p++) {
    result += m_polygonCurveArray[p].toString() + "\n";
  }
  return result;
}

String GlyphPolygon::toXML() {
  String result = "<glyphpolygon>\n";
  result += "<start>" + m_start.toXML() + "</start>";
  for(int p = 0; p < m_polygonCurveArray.size(); p++) {
    result += m_polygonCurveArray[p].toXML();
  }
  result += "</glyphpolygon>\n";
  return result;
}

// ------------------------------------ GlyphData ------------------------------

GlyphData::GlyphData(HDC hdc, unsigned char ch, const MAT2 &m) : m_glyphCurveData(hdc, ch, m) {
  m_pixRect = NULL;
  m_ch      = ch;
  DWORD buffersize = GetGlyphOutline(hdc,ch, GGO_BITMAP,&m_metrics,0,NULL,&m);
  if(buffersize == GDI_ERROR) {
    return;
  }

  DWORD *buffer = new DWORD[buffersize/sizeof(DWORD)];
  GetGlyphOutline(hdc,ch, GGO_BITMAP,&m_metrics,buffersize,buffer,&m);
  const int width  = m_metrics.gmBlackBoxX;
  const int height = m_metrics.gmBlackBoxY;
  m_pixRect = new PixRect(width,height);
  m_pixRect->fillRect(0,0,width,height,BLACK);

//  FILE *logFile = FOPEN("c:\\temp\\glyph.txt", "a");
//  fprintf(logFile,"letter %x '%c'\n",ch,ch);
  PixelAccessor *pa = m_pixRect->getPixelAccessor();
  for(int y = 0; y < height; y++) {
    int startIndex = y * ((width+31)/32);
    char *rowBits = (char*)&buffer[startIndex];
    for(int x = 0; x < width; rowBits++) {
      for(unsigned char mask = 0x80; mask && x < width; x++, mask >>= 1) {
        if(*rowBits&mask) {
          pa->setPixel(x,y,WHITE);
//          fprintf(logFile,"#");
        }
//      else {
//          fprintf(logFile," ");
//        }
      }
    }
//    fprintf(logFile,"\n");
  }
//  fprintf(logFile,"\n\n");
//  fclose(logFile);
  delete pa;
  delete[] buffer;
}


GlyphData::~GlyphData() {
  delete m_pixRect;
}

// ------------------------------------ GlyphCurveData ------------------------------

GlyphCurveData::GlyphCurveData() {
}

GlyphCurveData::GlyphCurveData(HDC hdc, unsigned char ch, const MAT2 &m) {
  GLYPHMETRICS m_metrics;
  DWORD buffersize = GetGlyphOutline(hdc,ch, GGO_BEZIER,&m_metrics,0,NULL,&m);
  if(buffersize == GDI_ERROR) {
    return;
  }

  char *buffer = new char[buffersize];
  GetGlyphOutline(hdc,ch, GGO_BEZIER,&m_metrics,buffersize,buffer,&m);

  for(int index = 0;index < (int)buffersize;) {
    TTPOLYGONHEADER *header = (TTPOLYGONHEADER*)&buffer[index];
    GlyphPolygon polygon(header->pfxStart);
    TTPOLYCURVE *c;
    for(int h = sizeof(TTPOLYGONHEADER); h < (int)header->cb; h += sizeof(TTPOLYCURVE)+(c->cpfx-1)*sizeof(POINTFX)) {
      c = (TTPOLYCURVE*)&buffer[index+h];
      PolygonCurve curve(c->wType);
      for(int i = 0; i < c->cpfx; i++) {
        curve.addPoint(c->apfx[i]);
      }
      polygon.addCurve(curve);
    }
    addPolygon(polygon);
    index += h;
  }

//  String s = dumphex(buffer+index,buffersize-index);
  String s = toString();
  delete[] buffer;
}

Point2DArray GlyphCurveData::getAllPoints() const {
  Point2DArray result;
  for(int i = 0; i < m_glyphPolygonArray.size(); i++) {
    result.addAll(m_glyphPolygonArray[i].getAllPoints());
  }
  return result;
}

Point2DArray GlyphCurveData::getLinePoints() const {
  PointCollector collector;
  applyToGlyph(*this,collector);
  return collector.m_result;
}

Rectangle2D GlyphCurveData::getBoundingBox() const {
  return getAllPoints().getBoundingBox();
}

void GlyphCurveData::addLine(const Point2D &p1, const Point2D &p2) {
  GlyphPolygon polygon(p1);
  PolygonCurve curve(TT_PRIM_LINE);
  curve.addPoint(p2);
  polygon.addCurve(curve);
  addPolygon(polygon);
}

void GlyphCurveData::move(const Point2D &dp) {
  for(int i = 0; i < m_glyphPolygonArray.size(); i++) {
    m_glyphPolygonArray[i].move(dp);
  }
}

String GlyphCurveData::toString() const {
  String result;
  for(int i = 0; i < m_glyphPolygonArray.size(); i++) {
    result += m_glyphPolygonArray[i].toString();
  }
  return result;
}

String GlyphCurveData::toXML() {
  String result = "<glyphcurvedata>";
  for(int i = 0; i < m_glyphPolygonArray.size(); i++) {
    result += m_glyphPolygonArray[i].toXML();
  }
  result += "</glyphcurvedata>";
  return result;
}

// ------------------------------------ PixRectFont ------------------------------

PixRectFont::PixRectFont() {
  m_font.CreateFont( 10, 8, 0, 0, 400, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                     CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                     DEFAULT_PITCH | FF_MODERN,
                     _T("Times new roman")
                   );
  initGlyphData(0);
}

PixRectFont::PixRectFont(const LOGFONT &logfont, float orientation) {
  m_font.CreateFontIndirect(&logfont);
  initGlyphData(orientation);
}

PixRectFont::~PixRectFont() {
  m_font.DeleteObject();
  for(int i = 0; i < ARRAYSIZE(m_glyphData); i++) {
    delete m_glyphData[i];
  }
}

void PixRectFont::getLogFont(LOGFONT &logfont) {
  m_font.GetLogFont(&logfont);
}

void PixRectFont::initGlyphData(float orientation) {
  HDC screenDC = getScreenDC();
  SelectObject(screenDC,m_font.m_hObject);
  GetTextMetrics(screenDC,&m_textMetrics);
  MAT2 m = orientation == 0 ? getIdentity() : rotation(orientation);

  for(int i = 0; i < ARRAYSIZE(m_glyphData); i++) {
    m_glyphData[i] = new GlyphData(screenDC,i, m);
  }

  DeleteDC(screenDC);
}

const GlyphData *PixRectFont::getGlyphData(unsigned char index) const {
  return m_glyphData[index];
}

String dumphex(char *buffer, int count) {
  String res;
  for(int i = 0; i < count; i++) {
    res += format(_T("%02x "),(unsigned char)buffer[i]);
  }
  return res;
}

String dumphex(DWORD *buffer, int count) {
  String res;
  for(int i = 0; i < count; i++) {
    res += format(_T("%08lx "),buffer[i]);
  }
  return res;
}

void PixRect::text(const CPoint &p, const char *text, const PixRectFont &font, D3DCOLOR color, bool invert) {
  int x = p.x;
  int y = p.y;
  PixRect *csrc = new PixRect(font.getTextMetrics().tmMaxCharWidth,font.getTextMetrics().tmHeight,getPixelFormat());
  csrc->fillRect(0,0,csrc->getWidth(),csrc->getHeight(),color);
  for(int i = 0; text[i] != '\0'; i++) {
    unsigned char ch = text[i];
    const GlyphData *gd = font.getGlyphData(ch);
    if(gd->m_pixRect != NULL) {
      mask(x+gd->m_metrics.gmptGlyphOrigin.x,y-gd->m_metrics.gmptGlyphOrigin.y,gd->m_pixRect->getWidth(),gd->m_pixRect->getHeight()
          ,SRCCOPY,csrc,0,0,gd->m_pixRect);
      x += gd->m_metrics.gmCellIncX;
      y += gd->m_metrics.gmCellIncY;
    }
  }
  delete csrc;
}

void applyToGlyphPolygon(const GlyphPolygon &polygon, CurveOperator &op) {
  const Point2D *pp = &polygon.m_start;
  op.beginCurve();
  op.apply(*pp);
  for(int i = 0; i < polygon.m_polygonCurveArray.size(); i++) {
    const PolygonCurve &curve = polygon.m_polygonCurveArray[i];
    switch(curve.getType()) {
    case TT_PRIM_LINE   :
      { const Point2DArray &pa = curve.getAllPoints();
        for(int j = 0; j < pa.size(); j++) {
          const Point2D &np = pa[j];
          op.apply(np);
          pp = &np;
        }
      }
      break;
    case TT_PRIM_QSPLINE:
      { String str = curve.toString();
        int f = 1;
      }
      break;
    case TT_PRIM_CSPLINE:
      { const Point2DArray &pa = curve.getAllPoints();
        for(int j = 0; j < pa.size(); j+=3) {
          const Point2D &end = pa[j+2];
          applyToBezier(*pp,pa[j],pa[j+1],end, op,false);
          pp = &end;
        } 
      }
      break;
    }
  }
  op.apply(polygon.m_start);
  op.endCurve();
}

void applyToGlyph(const GlyphCurveData &glyphCurve, CurveOperator &op) {
  const Array<GlyphPolygon> &pa = glyphCurve.getPolygonArray();
  for(int i = 0; i < pa.size(); i++) {
    applyToGlyphPolygon(pa[i], op);
  }
}

void applyToText(const char *text, const PixRectFont &font, TextOperator &op) {
  Point2D chPos(0,0);
  for(int i = 0; text[i] != '\0'; i++) {
    unsigned char ch = text[i];
    const GlyphData *gd = font.getGlyphData(ch);
    op.beginGlyph(chPos);
    if(gd->m_pixRect != NULL) {
      applyToGlyph(gd->m_glyphCurveData,op);
      chPos.x += gd->m_metrics.gmCellIncX;
      chPos.y += gd->m_metrics.gmCellIncY;
    }
    op.endGlyph();
  }
}

void PixRect::drawGlyph(const CPoint &p, const GlyphCurveData &glyph, D3DCOLOR color, bool invert) {
  applyToGlyph(glyph,PixRectTextMaker(this,p,color,invert));
}

void PixRect::drawText(const CPoint &p, const char *text, const PixRectFont &font, D3DCOLOR color, bool invert) {
  applyToText(text,font,PixRectTextMaker(this,p,color,invert));
}

