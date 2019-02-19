#ifndef GisTopologyOld_h
#define GisTopologyOld_h
/**                                                                       
 * @file                                                                  
 * $Revision $ 
 * $Date$ 
 *                                                                        
 *   Unless noted otherwise, the portions of Isis written by the USGS are public
 *   domain. See individual third-party library and package descriptions for 
 *   intellectual property information,user agreements, and related information.
 *                                                                        
 *   Although Isis has been used by the USGS, no warranty, expressed or implied,
 *   is made by the USGS as to the accuracy and functioning of such software 
 *   and related material nor shall the fact of distribution constitute any such 
 *   warranty, and no responsibility is assumed by the USGS in connection 
 *   therewith.                                                           
 *                                                                        
 *   For additional information, launch                                   
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html in a browser or see 
 *   the Privacy &amp; Disclaimers page on the Isis website,              
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */                                                                       
#include <cstdarg>
#include <istream>
#include <ostream>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include "IException.h"

#include <geos_c.h>

namespace Isis {

class GisTopologyOld {
  public:
    enum Disposition { DestroyGeometry, PreserveGeometry };

    static GisTopologyOld *getInstance();

    GEOSGeometry *getGeomFromWKB(const std::string &wkb);
    GEOSGeometry *getGeomFromWKT(const std::string &wkt);
    GEOSGeometry *clone(const GEOSGeometry *geom) const;
    const GEOSPreparedGeometry *getPreparedGeometry(const GEOSGeometry *geom) const;


    std::string getWKB(const GEOSGeometry *geom, 
                       const Disposition &disp = PreserveGeometry);
    std::string getWKT(const GEOSGeometry *geom, 
                       const Disposition &disp = PreserveGeometry);


    void destroy(GEOSGeometry *geom) const;
    void destroy(const GEOSGeometry *geom) const;
    void destroy(const GEOSPreparedGeometry *ppgeom) const;
    void destroy(const unsigned char *geos_text) const;
    void destroy(const char *geos_text) const;

  private:
    static GisTopologyOld *m_gisfactory;
    GisTopologyOld();
    ~GisTopologyOld();

    static void DieAtExit();

    GEOSWKTReader *m_WKTreader;
    GEOSWKTWriter *m_WKTwriter;
    GEOSWKBReader *m_WKBreader;
    GEOSWKBWriter *m_WKBwriter;

    void initialize();

    //  Initialize and shut down GEOS C API
    void GEOSInit();
    void GEOSFinish();

    static void notice(const char *fmt, ...);
    static void error(const char *fmt, ...);
     
    //  Reader/Writer allocations
    GEOSWKTReader *getWKTReader();
    GEOSWKTWriter *getWKTWriter();

    GEOSWKBReader *getWKBReader();
    GEOSWKBWriter *getWKBWriter();

};

}  // namespace Isis

#endif


