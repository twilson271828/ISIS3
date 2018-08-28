#ifndef PixelMaker_h
#define PixelMaker_h
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
#include "PvlObject.h"
#include "Statistics.h"
#include "IException.h"

#include "PixelDB.pb.h"
#include "PixelDBConfig.h"
#include <geos_c.h>

namespace Isis {

  class Camera;
  class Buffer;

class PixelMaker {
  public:
    PixelMaker() : _file(""), _config(0) { }
    PixelMaker(PixelDBConfig *config) :  _file(""), _config(config) { }
    virtual ~PixelMaker() { }

    void setConfig(PixelDBConfig *conf) { _config = conf; }
    Camera *Init(Cube &cube, File &vfile) const;
    bool Process(Camera &camera, Buffer &spectrum, File &vfile) const;
    bool Finalize(File &vfile) const;

    //  Initialize and shut down GEOS C API
    static void GEOSInit();
    static void GEOSFinish();
    static double To360Domain(const double lon);

  private:
    static bool _isInitialized;
    mutable std::string _file;
    struct Coordinate {
      Coordinate() : x(Null), y(Null) { }
      Coordinate(double xval, double yval) : x(xval), y(yval) { }
      ~Coordinate() { }
      double x;  // Longitude or sample
      double y;  // Latitude or line
    };

    typedef std::vector<Coordinate> CoordList;
    PixelDBConfig *_config;

    typedef PixelDBErrorHandler PDBE;
    PixelDBConfig &config() const throw (IException &);


    bool ExtractSpectrum(Buffer &buffer, Pixel &pixel) const;
    bool ComputeGeometry(const double line, const double sample, 
                         Camera &camera, Pixel &pixel) const;
    bool ComputePoint(const double line, const double sample, Camera &camera, 
                      Geometry &point) const;
    bool ComputeResolution(const Geometry &p1,const Geometry &p2,
                           double &res) const;
    bool ComputeFootprint(Camera &camera, Pixel &pixel) const;
    bool straddleBoundary(Camera &camera, const GEOSGeometry *geometry,
                          const GEOSGeometry *ptsgeom) const;
    bool buildFileGeometry(File &vfile) const;

    static void notice(const char *fmt, ...);
    static void error(const char *fmt, ...);

    CoordList getLineSamp(const Pixel &pixel) const;
    CoordList getLatLon(const Pixel &pixel) const;

    CoordList getWesternHalf() const;
    CoordList getEasternHalf() const;
    CoordList getWholePlanet() const;

    CoordList get180Boundary() const;
    CoordList get0Boundary() const;
    CoordList get360Boundary() const;

    bool HasPoint(Camera &camera, const GEOSGeometry *gpts, 
                  const double pntlat, const double pntlon) const;
    bool HasPixel(Camera &camera, const GEOSGeometry *gpts, 
                  const double sample, const double line) const;

    GEOSGeometry *GetMultiPolygon(const std::vector<CoordList> &geoms) const;
    GEOSGeometry *GetMultiPolygon(std::vector<GEOSGeometry *> &geoms) const; 
    GEOSGeometry *GetPolygon(const CoordList &pts) const;
    GEOSGeometry *GetLineString(const CoordList &pts) const;
    GEOSGeometry *GetLinearRing(const CoordList &pts) const;
    GEOSGeometry *GetPoint(const Coordinate &pt) const;
    GEOSGeometry *GetPoint(const double &x, const double &y) const;

    CoordList GetCoordinate(const GEOSGeometry *geom, const int &nth = 0) const; 
    CoordList GetCoordListFromGEOSGeom(const GEOSGeometry *geom) const;
    GEOSCoordSequence *GetGEOSSeqFromCoord(const CoordList &c) const;
    GEOSGeometry *GeomCleaner(GEOSGeometry *poly, bool free_poly = true) const;
};
} // Isis namespace

#endif
