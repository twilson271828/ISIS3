#ifndef PixelSearch_h
#define PixelSearch_h
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

#include <istream>
#include <ostream>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include "PvlObject.h"
#include "iException.h"

#include "PixelDB.pb.h"
#include "PixelDBQuery.h"
#include "PixelDBConfig.h"
#include "PixelMaker.h"
#include "Filename.h"
#include "TextFile.h"
#include <geos_c.h>

namespace Isis {

/**
 * @brief Search the database for pixels with multiple criteria
 * 
 * This class will iterate through a pixel database applying geometry polygon
 * and creation criteria.  Output is performed in realtime as containment
 * constraints will apply.
 * 
 */
class PixelSearch : public PixelDBQuery {
  public:
    PixelSearch() : PixelDBQuery(),  _ostrm(new std::ofstream("/dev/null")), 
                                     _reader(0), _geom(0), 
                                     _config(), _nfiles(0), _npixels(0), 
                                     _doWKB(false) { }
    PixelSearch(const std::string &ofile, const std::string &geomfile = "") :
                PixelDBQuery(), _ostrm(0),  _reader(0), _geom(0), 
                _config(), _nfiles(0), _npixels(0), _doWKB(false) { 
      std::string of("/dev/null");
      if (!ofile.empty()) of = Filename(ofile).Expanded();
      std::ofstream *ofstrm = new std::ofstream;
      ofstrm->open(of.c_str(), std::ios::out | std::ios::trunc);
      if (!ofstrm->is_open()) {
        delete ofstrm;
        throw iException::Message(iException::User, 
                                  "Cannot open/create file " + ofile, 
                                  _FILEINFO_);

      }
      _ostrm = ofstrm;
      allocateReader();
      setGeomWKT(geomfile);
    }

    virtual ~PixelSearch() { 
      delete _ostrm; 
 //     delete _config;
      deallocateReader();
      if (!_geom) GEOSGeom_destroy(_geom);
    }


    BigInt FileCount() const { return (_nfiles); }
    BigInt PixelCount() const { return (_npixels); }

    void setGeomWKT(const std::string &geomWKT) {
      GEOSGeometry *geom(0);
      if (!geomWKT.empty()) {
        TextFile geomf(geomWKT);
        std::string pline;
        GEOSWKTReader *wktReader = GEOSWKTReader_create();
        while (geomf.GetLine(pline)) {
          GEOSGeometry *poly = GEOSWKTReader_read(wktReader, pline.c_str());
          if (!geom) {  geom = poly; }
          else {
            GEOSGeometry *multi = GEOSUnion(geom, poly);
            GEOSGeom_destroy(geom);
            GEOSGeom_destroy(poly);
            geom = multi;
          }
        }
        GEOSWKTReader_destroy(wktReader);
      }
      GEOSGeom_destroy(_geom);
      _geom = geom;
    }

    void setConfig(const PixelDBConfig &config) {
      _config = config;
    }

    void setWKBOutput(bool doit = true) { _doWKB = doit; }

    void Reset() {
      _nfiles = _npixels = 0;
    }

    virtual void Execute(File &vfile) {
      if (vfile.geom().ngeoms() <= 0) return;
//      if (!_config.Validate(vfile, false)) return;  // Fails on incomplete file geom!

#if 0
      if (_geom) {
        if ((vfile.geom().ngeoms() == vfile.pixels_size()) && (vfile.has_geom())) {
          std::cout << "File: " << vfile.name() << "\n";
          std::string footprint = vfile.geom().thegeom();
          std::cout << "Got footprint of length=" << footprint.length() << "...converting\n";
          std::cout << "WKB: " << footprint << "\n";
          GEOSGeometry *thegeom = GEOSWKBReader_readHEX(_reader,
                      reinterpret_cast<const unsigned char *> (footprint.c_str()),
                                                        footprint.length());
          std::cout << "Got the geom...checking for intersection...\n";
          if (!GEOSIntersects(thegeom, _geom)) {
            std::cout << "Doesn't intersect!\n";
            GEOSGeom_destroy(thegeom);
            return;
          }
          GEOSGeom_destroy(thegeom);
        }
      }
#endif
      // Conditional tests have all passed - scrutinize all pixels
      BigInt current(_npixels);
      PixelIterator(vfile);
      if (current < _npixels) _nfiles++;
      return;
    }

    virtual void Execute(Pixel &pixel, File &vfile) {
      if (!_config.Validate(pixel)) return;

      if (_geom) {
        GEOSGeometry *thegeom = convert(pixel.footprint());
        if (!GEOSContains(_geom, thegeom)) {
          GEOSGeom_destroy(thegeom);
          return;
        }
      }

      //  If we reach here, we have a valid pixel, so write it to stream
      writePixel(*_ostrm, vfile, pixel);
      _npixels++;   // Order is important here so header will be written
      return;
    }

    virtual PixelDBQuery &Visit(PixelDBQuery &query) { return (query); }

    void getErrorSummary(PvlContainer &pvl) const {
      _config.getErrorSummary(pvl);
      return;
    }

  private:
    PixelSearch operator=(const PixelSearch &other);

    std::ostream   *_ostrm;
    GEOSWKBReader  *_reader;
    GEOSGeometry   *_geom;
    PixelDBConfig  _config;
    BigInt         _nfiles;
    BigInt         _npixels;
    bool           _doWKB;

    void allocateReader() {
      if (!_reader) {
        try {
          PixelMaker::GEOSInit();
          _reader = GEOSWKBReader_create();
        } catch (iException &ie) {
          deallocateReader();
          throw; 
        }
      }
    }

    void deallocateReader() {
      if (_reader) GEOSWKBReader_destroy(_reader);
      _reader = 0;
    }

    virtual std::ostream &writeHeader(std::ostream &os, const File &vfile,
                                      const Pixel &pixel) const {
      os << "Filename:Line:Sample:Latitude:Longitude:Incidence:Emission:"
         << "Phase:Radius:LineRes:SampRes:ObsTime:SCPosX:SCPosY:SCPosZ:"
         << "SunPosX:SunPosY:SunPosZ";
      for (int i = 0 ; i < pixel.values_size() ; i++) {
        os << ":" << pixel.values(i).name();
      }
      if (_doWKB) os << ":WKTGeometry";
      os << "\n";
      return (os);
    }

    virtual std::ostream &writePixel(std::ostream &os, const File &vfile,
                                     const Pixel &pixel) const {
      if (_npixels == 0) writeHeader(os, vfile, pixel);
      const Geometry &center = pixel.points().center();
      std::ios::fmtflags oldFlags = os.flags();
      os.setf(std::ios::fixed);
      os << vfile.name() << ":" 
         << std::setprecision(2) << center.line() << ":" 
         << std::setprecision(2) << center.sample() << ":"
         << std::setprecision(12) << center.latitude() << ":" 
         << std::setprecision(12) << center.longitude() << ":"
         << std::setprecision(12) << center.incidence() << ":" 
         << std::setprecision(12) << center.emission() << ":" 
         << std::setprecision(12) << center.phase() << ":" 
         << std::setprecision(12) << center.radius()  << ":"
         << std::setprecision(8) << pixel.lineres() << ":" 
         << std::setprecision(8) << pixel.sampres() << ":" 
         << std::setprecision(16) << pixel.ettime() << ":"
         << std::setprecision(16) << pixel.locale().scpos().x() << ":" 
         << std::setprecision(16) << pixel.locale().scpos().y() << ":" 
         << std::setprecision(16) << pixel.locale().scpos().z() << ":" 
         << std::setprecision(16) << pixel.locale().sunpos().x() << ":" 
         << std::setprecision(16) << pixel.locale().sunpos().y() << ":" 
         << std::setprecision(16) << pixel.locale().sunpos().z();
       
      for (int i = 0 ; i < pixel.values_size() ; i++) {
        os << ":" << std::setprecision(6) << pixel.values(i).avgdn();
      }

      // WKB output if requested
      if (_doWKB) {
        GEOSGeometry *thegeom = convert(pixel.footprint());
        char *ptr = GEOSGeomToWKT(thegeom);
        os << ":" << vfile.name() << "[" << (int) pixel.line() << "," 
           << (int) pixel.sample() << "](" << ptr << ")";
        GEOSGeom_destroy(thegeom);
        GEOSFree(ptr);
      }
      os << "\n";
      os.setf(oldFlags);
      return (os);
    }


    GEOSGeometry *convert(const std::string &wkb) const {
      GEOSGeometry *thegeom = GEOSWKBReader_readHEX(_reader, 
                  reinterpret_cast<const unsigned char *> (wkb.c_str()),
                                                    wkb.length());
      return (thegeom);
    }

};

} // Isis namespace

#endif
