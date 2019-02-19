/**                                                                       
 * @file                                                                  
 * $Revision$
 * $Date$
 * $Id$
 * 
 *   Unless noted otherwise, the portions of Isis written by the USGS are 
 *   public domain. See individual third-party library and package descriptions 
 *   for intellectual property information, user agreements, and related  
 *   information.                                                         
 *                                                                        
 *   Although Isis has been used by the USGS, no warranty, expressed or   
 *   implied, is made by the USGS as to the accuracy and functioning of such 
 *   software and related material nor shall the fact of distribution     
 *   constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.                                        
 *                                                                        
 *   For additional information, launch                                   
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html                
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */ 
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>
#include <numeric>
#include <iostream>
#include <sstream>

#include <QCoreApplication>

#include <geos_c.h>
#include "IString.h"
#include "IException.h"
#include "GisTopologyOld.h"


using namespace std;

namespace Isis {

GisTopologyOld *GisTopologyOld::m_gisfactory = 0;

GisTopologyOld::GisTopologyOld() : m_WKTreader(0), m_WKTwriter(0),
                             m_WKBreader(0), m_WKBwriter(0) {
    GEOSInit();
    //  This ensures this singleton is shut down when the application exists,
    //  so the GEOS system can be shut down cleanly.
   qAddPostRoutine(DieAtExit);
}

GisTopologyOld::~GisTopologyOld() {
  if (m_WKTreader) {  
    GEOSWKTReader_destroy(m_WKTreader);   
    m_WKTreader = 0;
  }

  if (m_WKTwriter) {  
    GEOSWKTWriter_destroy(m_WKTwriter);   
    m_WKTwriter = 0;
  }

  if (m_WKBreader) {  
    GEOSWKBReader_destroy(m_WKBreader);   
    m_WKBreader = 0;
  }

  if (m_WKBwriter) {  
    GEOSWKBWriter_destroy(m_WKBwriter);   
    m_WKBwriter = 0;
  }

  GEOSFinish();
}

GisTopologyOld *GisTopologyOld::getInstance() {
  if (!m_gisfactory) {
    m_gisfactory = new GisTopologyOld();
  }
  return (m_gisfactory);
}

GEOSGeometry *GisTopologyOld::getGeomFromWKB(const std::string &wkb) {
  const unsigned char *footy(reinterpret_cast<const unsigned char *> (wkb.c_str()));
  GEOSGeometry *geom = GEOSWKBReader_read(getWKBReader(), footy, wkb.size());
  if (!geom) {
    IString mess("Cannot convert WKB (" + wkb + ") to a GEOSGeometry");
    throw IException(IException::Programmer, mess, _FILEINFO_);

  }
  return (geom);
}

GEOSGeometry *GisTopologyOld::getGeomFromWKT(const std::string &wkt) {
  const char *footy(wkt.c_str());
  GEOSGeometry *geom = GEOSWKTReader_read(getWKTReader(), footy);
  if (!geom) {
    string mess = "Cannot convert WKT (" + wkt + ") to a GEOSGeometry";
    throw IException(IException::Programmer, mess, _FILEINFO_);

  }
  return (geom);
}

GEOSGeometry *GisTopologyOld::clone(const GEOSGeometry *geom) const {
  if (!geom) return (0);
  return (GEOSGeom_clone(geom));
}


const GEOSPreparedGeometry *GisTopologyOld::getPreparedGeometry(const GEOSGeometry *geom) const {
  const GEOSPreparedGeometry *ppgeom = GEOSPrepare(geom);
  if (!ppgeom) {
    throw IException(IException::Programmer, 
                     "Cannot convert a GEOSGeometry to a GEOSPreparedGeometry",
                     _FILEINFO_);

  }
  return (ppgeom);
}

std::string GisTopologyOld::getWKT(const GEOSGeometry *geom,
                                const GisTopologyOld::Disposition &disp) {
  size_t length;
  unsigned char *wkt_h = GEOSWKBWriter_writeHEX(getWKBWriter(), geom, &length);
  std::string thegeom(reinterpret_cast<const char *> (wkt_h), length);

  if (disp == DestroyGeometry) { destroy(geom); }
  destroy(wkt_h);

  return (thegeom);
}

std::string GisTopologyOld::getWKB(const GEOSGeometry *geom,
                                const GisTopologyOld::Disposition &disp) {
  size_t length;
  unsigned char *wkt_h = GEOSWKBWriter_writeHEX(getWKBWriter(), geom, &length);
  std::string thegeom(reinterpret_cast<const char *> (wkt_h), length);

  if (disp == DestroyGeometry) { destroy(geom); }
  destroy(wkt_h);

  return (thegeom);
}


void GisTopologyOld::destroy(GEOSGeometry *geom) const {
  if (geom) { GEOSGeom_destroy(geom); }
  return;
}

void GisTopologyOld::destroy(const GEOSGeometry *geom) const {
  if (geom) { destroy(const_cast<GEOSGeometry *> (geom)); }
  return;
}

void GisTopologyOld::destroy(const GEOSPreparedGeometry *geom) const {
  if (geom) { GEOSPreparedGeom_destroy(geom); }
  return;
}


void GisTopologyOld::destroy(const char *geos_text) const {
  if (geos_text) GEOSFree(const_cast<char *> (geos_text));
  return;
}

void GisTopologyOld::destroy(const unsigned char *geos_text) const {
  if (geos_text) GEOSFree(const_cast<unsigned char *> (geos_text));
  return;
}

void GisTopologyOld::GEOSInit() {
  initGEOS(GisTopologyOld::notice, GisTopologyOld::error);
}

void GisTopologyOld::GEOSFinish() {
  finishGEOS();
}

void GisTopologyOld::notice(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char buffer[1024];
  vsprintf(buffer, fmt, ap);
  va_end(ap);
  throw IException(IException::Programmer, buffer, _FILEINFO_);
  return;
}

void GisTopologyOld::error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char buffer[1024];
  vsprintf(buffer, fmt, ap);
  va_end(ap);
  throw IException(IException::Programmer, buffer, _FILEINFO_);
  return;
}

GEOSWKTReader *GisTopologyOld::getWKTReader() {
  if (!m_WKTreader) {
    m_WKTreader = GEOSWKTReader_create();
  }
  return (m_WKTreader);
}

GEOSWKTWriter *GisTopologyOld::getWKTWriter() {
  if (!m_WKTwriter) {
    m_WKTwriter = GEOSWKTWriter_create();
  }
  return (m_WKTwriter);
}

GEOSWKBReader *GisTopologyOld::getWKBReader() {
  if (!m_WKBreader) {
    m_WKBreader = GEOSWKBReader_create();
  }
  return (m_WKBreader);
}

GEOSWKBWriter *GisTopologyOld::getWKBWriter() {
  if (!m_WKBwriter) {
    m_WKBwriter = GEOSWKBWriter_create();
  }
  return (m_WKBwriter);
}

 /**
   * @brief Exit termination routine
   *
   * This (static) method ensure this object is destroyed when Qt exits.  
   *
   * Note that it is error to add this to the system _atexit() routine because
   * this object utilizes Qt classes.  At the time the atexit call stack is
   * executed, Qt is long gone resulting in Very Bad Things.  Fortunately, Qt has
   * an exit stack function as well.  This method is added to the Qt exit call
   * stack.
   */
  void GisTopologyOld::DieAtExit() {
    delete  m_gisfactory;
    m_gisfactory = 0;
    return;
  }


} // namespace Isis
