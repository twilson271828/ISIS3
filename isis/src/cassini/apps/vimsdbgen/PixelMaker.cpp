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

#include <geos_c.h>
#include "Distance.h"
#include "FileName.h"
#include "IException.h"
#include "iTime.h"
#include "Latitude.h"
#include "Longitude.h"
#include "PixelMaker.h"
#include "Projection.h"
#include "SerialNumber.h"
#include "SpecialPixel.h"
#include "Statistics.h"
#include "SurfacePoint.h"

using namespace std;

namespace Isis {


bool PixelMaker::_isInitialized = false;

void PixelMaker::GEOSInit() {
  if (!_isInitialized) {
    initGEOS(PixelMaker::notice, PixelMaker::error);
    _isInitialized = true;
  }
}

void PixelMaker::GEOSFinish() {
  if (_isInitialized) {
    finishGEOS();
    _isInitialized = false;
  }
}

void PixelMaker::notice(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char buffer[1024];
  vsprintf(buffer, fmt, ap);
  va_end(ap);
  IException(IException::Programmer, buffer, _FILEINFO_);
  return;
}

void PixelMaker::error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char buffer[1024];
  vsprintf(buffer, fmt, ap);
  va_end(ap);
  throw IException(IException::Programmer, buffer, _FILEINFO_);
  return;
}

PixelDBConfig &PixelMaker::config() const throw (IException &) {
  if (!_config) {
     throw IException(IException::Programmer, 
                         "FATAL - Config constraints object not set",
                          _FILEINFO_);
  }
  return (*_config);
}

Camera *PixelMaker::Init(Cube &cube, File &vfile) const {
  FileName cfile(cube.fileName());
  vfile.set_name(cfile.name().toStdString());
  vfile.set_path(cfile.path().toStdString());
  vfile.set_serialnumber(SerialNumber::Compose(cube).toStdString());
  vfile.set_lines(cube.lineCount());
  vfile.set_samples(cube.sampleCount());
  vfile.set_bands(cube.bandCount());
  return (config().Validate(cube));
}

bool PixelMaker::Process(Camera &camera, Buffer &spectrum, File &vfile) const {

  _file = vfile.name();
  double line(spectrum.Line());
  double sample(spectrum.Sample());

//  Compute spectral data
  Pixel pixel;
  pixel.set_line(line);
  pixel.set_sample(sample);
  camera.SetBand(spectrum.Band());

  //  Extract spectral averages.  If fails, we are done
  if (!ExtractSpectrum(spectrum, pixel)) return (false);

  //  Set up Geometry for pixel (center point)
  if (!camera.SetImage(sample, line)) return (config().LogError(PDBE::NOCENTERPT));

  // Time of (pixel) acquisition
  pixel.set_ettime(camera.time().Et());

  //  Compute geometries
  if (!ComputeGeometry(line, sample, camera, pixel)) return (false);

  //  If the pixel is valid, add it to the file
  if (config().Validate(pixel)) {
    *vfile.add_pixels() = pixel;
  }
  return (true);
}

bool PixelMaker::Finalize(File &vfile) const {
  buildFileGeometry(vfile);
  return (config().Validate(vfile, false));
}

bool PixelMaker::ExtractSpectrum(Buffer &buffer, Pixel &pixel) const {
  for (int i = 0 ; i < buffer.size() ; i++) {
    pixel.add_spectrum(buffer[i]);
  }

  return (true);
}

bool PixelMaker::ComputeGeometry(const double line, const double sample, 
                                 Camera &camera, Pixel &pixel) const {


  // Compute and validate each point
  Geometry center;
  if (!ComputePoint(line, sample, camera, center)) 
    return (config().LogError(PDBE::NOCENTERPT));
  if (!config().Validate(center)) return (false);
  *pixel.mutable_points()->mutable_center() = center;


  Geometry uleft;
  if (!ComputePoint(line-0.5, sample-0.5, camera, uleft)) 
    return (config().LogError(PDBE::NOULEFTPT));
  if (!config().Validate(uleft)) return (false);
  *pixel.mutable_points()->mutable_upperleft() = uleft;

  Geometry uright;
  if (!ComputePoint(line-0.5, sample+0.5, camera, uright)) 
    return (config().LogError(PDBE::NOURIGHTPT));
  if (!config().Validate(uright)) return (false);
  *pixel.mutable_points()->mutable_upperright() = uright;

  Geometry lright;
  if (!ComputePoint(line+0.5, sample+0.5, camera, lright)) 
    return (config().LogError(PDBE::NOLRIGHTPT));
  if (!config().Validate(lright)) return (false);
  *pixel.mutable_points()->mutable_lowerright() = lright;

  Geometry lleft;
  if (!ComputePoint(line+0.5, sample-0.5, camera, lleft)) 
    return (config().LogError(PDBE::NOLLEFTPT));
  if (!config().Validate(lleft)) return (false);
  *pixel.mutable_points()->mutable_lowerleft() = lleft;

  // Resolutions
  double top, bottom, left, right;
  if (!ComputeResolution(uleft, uright, top)) return (false);
  if (!ComputeResolution(lleft, lright, bottom)) return (false);
  if (!ComputeResolution(uleft, lleft, left)) return (false);
  if (!ComputeResolution(uright, lright, right)) return (false);

  pixel.set_sampres((top+bottom)/2.0);
  pixel.set_lineres((left+right)/2.0);

  //  Compute the footprint of the pixel
  bool success(false);
  try {
    success = ComputeFootprint(camera, pixel);
  } catch (IException &ie) {
    ostringstream mess;
    mess << "GISError: " << _file << "[" << center.sample() << "," 
         << center.line() << "] - " << ie.toString() << "\n";
    config().LogError(PDBE::GEOSEXCEPTION, mess.str());
    success = false;
  } catch (...) {
    success = false;
  }

  return (success);
}


bool PixelMaker::ComputePoint(const double line, const double sample, 
                              Camera &camera, Geometry &point) const {

  if (!camera.SetImage(sample, line)) return (false);
  point.set_line(line);
  point.set_sample(sample);
  point.set_latitude(camera.UniversalLatitude());
  point.set_longitude(camera.UniversalLongitude());
  point.set_resolution(camera.PixelResolution());
  point.set_incidence(camera.IncidenceAngle());
  point.set_emission(camera.EmissionAngle());
  point.set_phase(camera.PhaseAngle());
  point.set_radius(camera.LocalRadius().meters());
  point.set_distance(camera.targetCenterDistance());  // in km

  //  Get sun, spacecraft positions and time of acquisition
  double v[3];
  camera.sunPosition(v);
  point.mutable_locale()->mutable_sunpos()->set_x(v[0]);
  point.mutable_locale()->mutable_sunpos()->set_y(v[1]);
  point.mutable_locale()->mutable_sunpos()->set_z(v[2]);

  camera.instrumentPosition(v);
  point.mutable_locale()->mutable_scpos()->set_x(v[0]);
  point.mutable_locale()->mutable_scpos()->set_y(v[1]);
  point.mutable_locale()->mutable_scpos()->set_z(v[2]);

  return (true);

}

bool PixelMaker::ComputeResolution(const Geometry &p1, const Geometry &p2,
                                   double &res) const {
#if 0
  res = Camera::Distance(p1.latitude(), p1.longitude(), 
                         p2.latitude(), p2.longitude(),
                         ((p1.radius()+p2.radius())/2.0));
#else
  SurfacePoint p1Point(Latitude(p1.latitude(), Angle::Degrees),
                       Longitude(p1.longitude(), Angle::Degrees),
                       Distance(p1.radius(), Distance::Meters));
  SurfacePoint p2Point(Latitude(p2.latitude(), Angle::Degrees),
                       Longitude(p2.longitude(), Angle::Degrees),
                       Distance(p2.radius(), Distance::Meters));
  Distance avgD = Distance((p1.radius()+p2.radius())/2.0, Distance::Meters);

  res = p1Point.GetDistanceToPoint(p2Point, avgD).meters();
#endif
  return (true);

}



/**
 * @brief PixelMaker::To360Domain
 * This function was originally part of the ISIS3 projection class, but it was deprecated
 * It's included here because methods within this class make calls to it.
 *
 * @param lon
 * @return  lon Longitude converted to the [0,360] domain
 */

double PixelMaker::To360Domain(const double lon) {

  double mylon = lon;
  while (mylon < 0.0) mylon += 360.0;
  while (mylon > 360.0) mylon -= 360.0;
  return mylon;


}
bool PixelMaker::ComputeFootprint(Camera &camera, Pixel &pixel) const {

  GEOSInit();
//  double footinc(config().GetFootprintIncrement());
  CoordList pts(getLineSamp(pixel)), geom(getLatLon(pixel));

  GEOSGeometry *ptsgeom(GetPolygon(pts));
  GEOSGeometry *geometry(GetPolygon(geom));

  vector<GEOSGeometry *> polys;

  // Check for poles in pixel coordinate geometry
  bool hasNPole = HasPoint(camera, ptsgeom,  90.0, 0.0);
  bool hasSPole = HasPoint(camera, ptsgeom, -90.0, 0.0);

#if defined(GIS_DEBUG)
  if (hasNPole) cout << "  Got North Pole!\n";
  if (hasSPole) cout << "  Got South Pole!\n";
#endif

  // Test for a normal boundary crosser and adjust the longitudes to extend
  // beyond the 360 bound to create a valid solid geometry polygon
  bool bcross(false);
  if (!(hasNPole || hasSPole) && straddleBoundary(camera, geometry, ptsgeom) ) {
    bcross = true;
    // Convert all longitudes to 360+ data
#if defined(GIS_DEBUG)
    cout << "Got a boundary crosser at line " << pixel.points().center().line()
         << ", sample " << pixel.points().center().sample() << "\n";
#endif
    for (unsigned int i = 0 ; i < geom.size() ; i++) {
      if (geom[i].x < 180.0) {
        geom[i].x += 360.0;
      }
    }

    GEOSGeom_destroy(geometry);
    geometry = GetPolygon(geom);

    //  Now get all polygon intersections
    GEOSGeometry *part = GetPolygon(getWesternHalf());
    // Get western half first
    GEOSGeometry *p = GEOSIntersection(geometry, part);
    if (!GEOSisEmpty(p)) {
      polys.push_back(GeomCleaner(p));
    } else GEOSGeom_destroy(p);

    //  Get eastern half
    GEOSGeom_destroy(part);
    part = GetPolygon(getEasternHalf());
    p = GEOSIntersection(geometry, part);
    if (!GEOSisEmpty(p)) {
      polys.push_back(GeomCleaner(p));
    } else GEOSGeom_destroy(p);

    // Get what remains (typically 0 boundary crosser)
    GEOSGeom_destroy(part);
    part = GetPolygon(getWholePlanet());
    p = GEOSDifference(geometry, part);
    //  For this one, ensure the longitudes are within the 360 range.
    // NOTE!  It is important to ensure 360 longitudes map to 0!!!
    if (!GEOSisEmpty(p)) {
      CoordList coords = GetCoordinate(p);
      for (unsigned int i = 0 ; i < coords.size() ; i++) {
        if (coords[i].x >= 360.0) {
          coords[i].x -= 360.0;         
          coords[i].x = this->To360Domain(coords[i].x);
        }
      }

      // Get the new geometry and add to list
      GEOSGeom_destroy(p);
      p = GetPolygon(coords);
      polys.push_back(GeomCleaner(p));
    } else GEOSGeom_destroy(p);
  }
  else {
    polys.push_back(GeomCleaner(geometry,false));
  }

  // No longer need originals
  GEOSGeom_destroy(ptsgeom);
  GEOSGeom_destroy(geometry);


  //  Got all polygons, create the multipolygon and convert to WKT hex.
  GEOSGeometry *polygon = GetMultiPolygon(polys);
  GEOSWKBWriter *writer = GEOSWKBWriter_create();
  size_t length;
  unsigned char *wkb_h = GEOSWKBWriter_writeHEX(writer, polygon, &length); 
  pixel.set_footprint((const char *) wkb_h, length);

  // cout << "Footprint: " << pixel.footprint() << "\n";

#if defined(GIS_DEBUG)
  // DEBUG print if boundary crosser
  if (bcross || hasNPole || hasSPole) {
    char *ptr = GEOSGeomToWKT(geometry);
    cout << "Pole/Crosser MultiPolygon: " << ptr << "\n";
    GEOSFree(ptr);
  }
#endif

  // Final cleanup and exit
  GEOSFree(wkb_h);
  GEOSWKBWriter_destroy(writer);
  GEOSGeom_destroy(polygon);

  return (true);
}

bool PixelMaker::buildFileGeometry(File &vfile) const {
  GEOSInit();
  GISGeometry filegeom;
  filegeom.set_ngeoms(0);
  filegeom.set_thegeom("");
  *vfile.mutable_geom() = filegeom;  // Make initial assignment

  // Test for no pixels...
  if (vfile.pixels_size() == 0) return (true);

#if defined(GIS_DEBUG)
  cout << "Composing file geom...\n";
#endif

  GEOSGeometry *thegeom(0);
  GEOSWKBReader *reader(0);
  try {
//    thegeom = GEOSGeom_createEmptyPolygon(); // Not present in older versions
    reader = GEOSWKBReader_create();
  } catch (IException &ie) {
    if (thegeom) GEOSGeom_destroy(thegeom);
    if (reader) GEOSWKBReader_destroy(reader);
    ostringstream mess;
    mess << "TheGeomError: " << _file << " -> " << ie.toString() << "\n";
    return (config().LogError(PDBE::THEGEOMFAILED, mess.str()));
  }

#if defined(GIS_DEBUG)
  cout << "Union " << vfile.pixels_size()  << " pixel geometries.\n";
#endif
  int ngeoms(0);
  for (int i = 0 ; i < vfile.pixels_size() ; i++) {
    const Pixel &pixel = vfile.pixels(i);
    string footprint = pixel.footprint();

    GEOSGeometry *pixelgeom(0);
    GEOSGeometry *composite(0);
    try {
      pixelgeom = GEOSWKBReader_readHEX(reader,
                    reinterpret_cast<const unsigned char *> (footprint.c_str()),
                                        footprint.length());

      // Check for first polygon (GEOS version dependant - see above)
      if (!thegeom) {
        thegeom = pixelgeom;
      }
      else {
        composite = GEOSUnion(thegeom, pixelgeom);
        GEOSGeom_destroy(thegeom);
        GEOSGeom_destroy(pixelgeom);
        thegeom = composite;
      }
      ngeoms++;

    } catch (IException &ie) {
      if (pixelgeom) GEOSGeom_destroy(pixelgeom);
      if (composite) GEOSGeom_destroy(composite);
      ostringstream mess;
      mess << "TheGeomError: " << _file << "[" 
           << pixel.points().center().sample() << "," 
           << pixel.points().center().line() << "] - " 
           << ie.toString() << "\n";
      config().LogError(PDBE::GEOSEXCEPTION, mess.str());
    }
  }

#if defined(GIS_DEBUG)
  cout << "Unioned " << ngeoms << " geometries.\n";
#endif
  // OK, test for case where none where successfully processed
  if (ngeoms == 0) {
    GEOSWKBReader_destroy(reader);
    if (thegeom) GEOSGeom_destroy(thegeom);
    return (false);
  }

  // Put a convex hull around each individual polygon
  int ngs = GEOSGetNumGeometries(thegeom);
  vector<GEOSGeometry *> thehulls;
  // BEWARE!  Cannot use for anything but a Collection up to  GEOS 3.1.0!
  if (ngs > 1) {
    for (int nh = 0 ; nh < ngs ; nh++) {
      const GEOSGeometry *g = GEOSGetGeometryN(thegeom, nh);
      thehulls.push_back(GeomCleaner(const_cast<GEOSGeometry *> (g), false));
    }
  }
  else {
    thehulls.push_back(GeomCleaner(thegeom, false));
  }

#if defined(GIS_DEBUG)
  cout << "Got " << ngs << " geometries.\n";
#endif

  GEOSGeom_destroy(thegeom);
  thegeom = GetMultiPolygon(thehulls);

#if defined(GIS_DEBUG0)
  {
    cout << "Suceeded creating multipolygon.\n";
    char *ptr = GEOSGeomToWKT(thegeom);
    cout << "Union Polygon: " << ptr << "\n";
    GEOSFree(ptr);
  }
#endif

  // Format the polygon to hex and store to File 
  GEOSWKBWriter *writer(0);
  unsigned char *wkb_h(0);
  try {
    size_t length;
    writer = GEOSWKBWriter_create();
    wkb_h = GEOSWKBWriter_writeHEX(writer, thegeom, &length); 
    // cout << "WKB: " << wkb_h << "\n";
    filegeom.set_thegeom(reinterpret_cast<const char *> (wkb_h), length);
    filegeom.set_ngeoms(ngeoms);
    *vfile.mutable_geom() = filegeom;
  } catch (IException &ie) {
    GEOSWKBReader_destroy(reader);
    GEOSGeom_destroy(thegeom);
    if (writer) GEOSWKBWriter_destroy(writer);
    if (wkb_h) GEOSFree(wkb_h);
    ostringstream mess;
    mess << "TheGeomError: " << _file << "-> " << ie.toString() << "\n";
    return (config().LogError(PDBE::THEGEOMFAILED, mess.str()));
  }


  // Final cleanup and exit
  GEOSWKBReader_destroy(reader);
  GEOSWKBWriter_destroy(writer);
  GEOSGeom_destroy(thegeom);
  GEOSFree(wkb_h);
  return (true);
}

bool PixelMaker::HasPoint(Camera &camera, const GEOSGeometry *gpts, 
                          const double pntlat, const double pntlon) const {
  // Check if geometry point is in the given pixel geometry
  bool isIn(false);
  if (camera.SetUniversalGround(pntlat, pntlon)) {
    GEOSGeometry *point = GetPoint(camera.Sample(), camera.Line());
    isIn = GEOSWithin(point, gpts);
    GEOSGeom_destroy(point);
  }
  return (isIn);
}

bool PixelMaker::HasPixel(Camera &camera, const GEOSGeometry *gpts, 
                          const double sample, const double line) const {
  // Check if geometry point is in the given pixel geometry
  bool isIn(false);
  if (camera.SetImage(sample, line)) {
    GEOSGeometry *point = GetPoint(camera.UniversalLongitude(), 
                                   camera.UniversalLatitude());
    isIn = GEOSContains(point, gpts);
    GEOSGeom_destroy(point);
  }
  return (isIn);
}


/**
 * @brief Checks geometry for 0 boundary crosser
 *  
 * The most straight forward way to handles this is to find minimum and maximum 
 * longitudes of the geometry and test it for exceeding 180 degrees.  The limits 
 * is currently set 270.0 degrees. 
 *  
 * The check is only invoked if the geometry has the 180 boundary which will 
 * occur if it truly does cross the 180 boundary and on unscrutinized polygons 
 * created in 360 longitude domain which cross the 0 boundary. 
 *  
 * Note that this routine does not check for the existance of the poles.  In 
 * this case, it will always return true.  If you wish to treat poles 
 * differently, they must be checked for prior to calling this routine or after 
 * depending upon if the polygon generator needs this info. 
 * 
 * @param camera   Camera object for the pixel - may be needed in the 
 *                 implementation
 * @param geometry Lat/Lon geometry to test
 * @param pntgeom  Sample/Line geometry of each point
 * 
 * @return bool    Returns true if it detects the 0 longitude boundary in the 
 *                 geometry.
 */
bool PixelMaker::straddleBoundary(Camera &camera, const GEOSGeometry *geometry, 
                                   const GEOSGeometry *pntgeom) const {

  bool crosses(false);
  GEOSGeometry *boundary = GetLineString(get180Boundary());
  if (GEOSIntersects(boundary, geometry)) {
    CoordList pts = GetCoordinate(geometry);
    Statistics lonstats;
    for (unsigned int i = 0 ; i < pts.size() ; i++) { 
      lonstats.AddData(pts[i].x);
    }
    crosses = (lonstats.Maximum()-lonstats.Minimum()) > 270.0;
  }
  GEOSGeom_destroy(boundary);
  return (crosses);
}


PixelMaker::CoordList PixelMaker::getLineSamp(const Pixel &pixel) const {
  CoordList pts;
  pts.push_back(Coordinate(pixel.points().upperleft().sample(), 
                           pixel.points().upperleft().line()));
  pts.push_back(Coordinate(pixel.points().upperright().sample(), 
                           pixel.points().upperright().line()));
  pts.push_back(Coordinate(pixel.points().lowerright().sample(), 
                           pixel.points().lowerright().line()));
  pts.push_back(Coordinate(pixel.points().lowerleft().sample(), 
                           pixel.points().lowerleft().line()));
  pts.push_back(Coordinate(pixel.points().upperleft().sample(), 
                           pixel.points().upperleft().line()));
  return (pts);
}

PixelMaker::CoordList PixelMaker::getLatLon(const Pixel &pixel) const {
  CoordList geom;
  geom.push_back(Coordinate(pixel.points().upperleft().longitude(), 
                            pixel.points().upperleft().latitude()));
  geom.push_back(Coordinate(pixel.points().upperright().longitude(), 
                            pixel.points().upperright().latitude()));
  geom.push_back(Coordinate(pixel.points().lowerright().longitude(), 
                            pixel.points().lowerright().latitude()));
  geom.push_back(Coordinate(pixel.points().lowerleft().longitude(), 
                            pixel.points().lowerleft().latitude()));
  geom.push_back(Coordinate(pixel.points().upperleft().longitude(), 
                            pixel.points().upperleft().latitude()));
  return (geom);
}

PixelMaker::CoordList PixelMaker::getWesternHalf() const { 
  CoordList geom;
  geom.push_back(Coordinate(0.0, 90.0));
  geom.push_back(Coordinate(180.0, 90.0));
  geom.push_back(Coordinate(180.0, -90.0));
  geom.push_back(Coordinate(0.0, -90.0));
  geom.push_back(Coordinate(0.0, 90.0));
  return (geom);
}


PixelMaker::CoordList PixelMaker::getEasternHalf() const { 
  CoordList geom;
  geom.push_back(Coordinate(180.0, 90.0));
  geom.push_back(Coordinate(360.0, 90.0));
  geom.push_back(Coordinate(360.0, -90.0));
  geom.push_back(Coordinate(180.0, -90.0));
  geom.push_back(Coordinate(180.0, 90.0));
  return (geom);
}

PixelMaker::CoordList PixelMaker::getWholePlanet() const { 
  CoordList geom;
  geom.push_back(Coordinate(0.0, 90.0));
  geom.push_back(Coordinate(360.0, 90.0));
  geom.push_back(Coordinate(360.0, -90.0));
  geom.push_back(Coordinate(0.0, -90.0));
  geom.push_back(Coordinate(0.0, 90.0));
  return (geom);
}


PixelMaker::CoordList PixelMaker::get180Boundary() const {
  CoordList geom;
  geom.push_back(Coordinate(180.0, 90.0));
  geom.push_back(Coordinate(180.0, -90.0));
  return (geom);
}

PixelMaker::CoordList PixelMaker::get0Boundary() const {
  CoordList geom;
  geom.push_back(Coordinate(0.0, 90.0));
  geom.push_back(Coordinate(0.0, -90.0));
  return (geom);
}

PixelMaker::CoordList PixelMaker::get360Boundary() const {
  CoordList geom;
  geom.push_back(Coordinate(360.0, 90.0));
  geom.push_back(Coordinate(360.0, -90.0));
  return (geom);
}


GEOSGeometry *PixelMaker::GetMultiPolygon(const vector<PixelMaker::CoordList> &pts) 
                                          const {
  vector<GEOSGeometry *> geoms;
  for (unsigned int i = 0 ; i < pts.size() ; i++) {
    geoms.push_back(GEOSGeom_createLinearRing(GetGEOSSeqFromCoord(pts[i])));
  }
  return (GEOSGeom_createCollection(GEOS_MULTIPOLYGON, &geoms[0], geoms.size())); 
}


GEOSGeometry *PixelMaker::GetMultiPolygon(vector<GEOSGeometry *> &geoms) 
                                          const {
  return (GEOSGeom_createCollection(GEOS_MULTIPOLYGON, &geoms[0], geoms.size()));
}

GEOSGeometry *PixelMaker::GetPolygon(const PixelMaker::CoordList &pts) const {
  GEOSGeometry *lr = GEOSGeom_createLinearRing(GetGEOSSeqFromCoord(pts));
  return (GEOSGeom_createPolygon(lr, NULL, 0));
}

GEOSGeometry *PixelMaker::GetLineString(const PixelMaker::CoordList &pts) const {
  return (GEOSGeom_createLineString(GetGEOSSeqFromCoord(pts)));
}
         

GEOSGeometry *PixelMaker::GetLinearRing(const PixelMaker::CoordList &pts) const {
  return (GEOSGeom_createLinearRing(GetGEOSSeqFromCoord(pts)));
}


GEOSGeometry *PixelMaker::GetPoint(const PixelMaker::Coordinate &pt) const {
  CoordList pts;
  pts.push_back(pt);
  return (GEOSGeom_createPoint(GetGEOSSeqFromCoord(pts)));
}


GEOSGeometry *PixelMaker::GetPoint(const double &x, const double &y) const {
  return(GetPoint(Coordinate(x,y)));
}

PixelMaker::CoordList PixelMaker::GetCoordinate(const GEOSGeometry *geom, 
                                                const int &nth) const {
  int ngeoms = GEOSGetNumGeometries(geom);
  if ((nth < 0) || (nth >= ngeoms)) {
    string mess = "GEOSGeometry->Coordinate conversion of " + IString(nth) +
                  " invalid";
    throw IException(IException::Programmer, mess, _FILEINFO_);
  }

  // Convert based upon type
  const GEOSCoordSequence *cs(0);
  const GEOSGeometry *gtmp(geom);
  int gtype = GEOSGeomTypeId(geom);
  switch (gtype) {
    case GEOS_POINT:
    case GEOS_LINESTRING:
    case GEOS_LINEARRING:
      cs = GEOSGeom_getCoordSeq(geom);
      break;
    case GEOS_POLYGON:
      gtmp = GEOSGetExteriorRing(geom);
      cs = GEOSGeom_getCoordSeq(gtmp);
      break;

    case GEOS_MULTIPOINT:
    case GEOS_MULTILINESTRING:
    case GEOS_MULTIPOLYGON:
    case GEOS_GEOMETRYCOLLECTION:
        gtmp = GEOSGetGeometryN(geom, nth);
        cs = GEOSGeom_getCoordSeq(gtmp);
        break;
    default:
        string mess = "GEOSGeometry-Coordinate - unknown type " + IString(gtype);
        throw IException(IException::Programmer, mess, _FILEINFO_);
  }

  unsigned int npts;
  GEOSCoordSeq_getSize(cs, &npts);
  
  CoordList coords; 
  double x, y;
  for (unsigned int i = 0 ; i < npts ; i++) {
    GEOSCoordSeq_getX(cs, i, &x);
    GEOSCoordSeq_getY(cs, i, &y);
    coords.push_back(Coordinate(x, y));
  }
  return (coords);
}

GEOSCoordSequence *PixelMaker::GetGEOSSeqFromCoord(
                                    const PixelMaker::CoordList &c) const {
  GEOSCoordSequence *cs = GEOSCoordSeq_create(c.size(), 2);
  for (unsigned int i = 0 ; i < c.size() ; i++) {
    GEOSCoordSeq_setX(cs, i, c[i].x); 
    GEOSCoordSeq_setY(cs, i, c[i].y); 
  }
  return (cs);
}

GEOSGeometry *PixelMaker::GeomCleaner(GEOSGeometry *poly, bool free_poly) const {
  GEOSGeometry *geom;
  int gtype = GEOSGeomTypeId(poly);
  if (gtype != GEOS_POLYGON) {
   geom = GEOSConvexHull(poly);
  }
  else {
    const GEOSGeometry *gtmp = GEOSGetExteriorRing(poly);
    GEOSCoordSequence *cs = GEOSCoordSeq_clone(GEOSGeom_getCoordSeq(gtmp));
    GEOSGeometry *shell = GEOSGeom_createLinearRing(cs);
    geom = GEOSGeom_createPolygon(shell, 0, 0);
  }
  if (free_poly) GEOSGeom_destroy(poly);
  return (geom);
}

} // namespace ISIS
