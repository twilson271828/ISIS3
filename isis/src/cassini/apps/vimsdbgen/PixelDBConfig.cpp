/**                                                                       
 * @file                                                                  
 * $Revision: 1.3 $                                                             
 * $Date: 2010/02/25 18:39:05 $                                                                 
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
#include <memory>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>


#include "IString.h"
#include "FileName.h"
#include "Cube.h"
#include "Camera.h"
#include "DbProfile.h"
#include "PvlObject.h"
#include "PixelDB.pb.h"
#include "PixelDBConfig.h"
#include "IException.h"
#include "Target.h"

using namespace std;
using namespace google::protobuf;

namespace Isis {


  /**
   * @brief Default constructor
   * 
   * @author Kris Becker - 2/21/2010
   */
  PixelDBConfig::PixelDBConfig () : m_instrument("VIMS"), m_target("Titan"),
                             m_nbands(256), m_footinc(0.2),
                             m_maxEmission(90.0), m_maxIncidence(120.0), 
                             m_maxLineRes(10000.0), m_maxSampRes(10000.),
                             m_maxLineSampRatio(5.0), m_minLineSampRatio(0.2),
                             m_maxTargetDistance(DBL_MAX), m_profiles() {
    return;
  }

  /**
   * @brief Construct from PVL object
   * 
   * @author Kris Becker - 2/21/2010
   * 
   * @param pvl Vims Configuration parameter files
   * @param cube Input cube file
   */
  PixelDBConfig::PixelDBConfig (PvlObject &pvl) {
    init(pvl);
    return;
  }

  /**
   * @brief Construct from PVL object
   * 
   * @author Kris Becker - 2/21/2010
   * 
   * @param pvl Vims Configuration parameter files
   * @param cube Input cube file
   */
  PixelDBConfig::PixelDBConfig(const std::string &conf) {
    Pvl pconf(QString(conf.c_str() ));
    init (pconf);
    return;
  }

  void PixelDBConfig::Read(const std::string &dbfile) {
    Pvl db(QString(dbfile.c_str()) );
    init(db.findObject("PixelDBConfig", Pvl::Traverse));
    return;
  }

  void PixelDBConfig::setConfig(const std::string &conf) {
    Pvl pconf(QString(conf.c_str()));
    init(pconf);
    return;
  }

  /**
   * @brief Checks a cube against constraints for db inclusion 
   *  
   * This method performs checks to ensure the cube file satisfies conditions 
   * set forth in the configuration file.  If sucessful, it will return a valid 
   * pointer to its Camera object (since this is required for processing and it 
   * may be used for validation and therefore effective/efficient utilization). 
   *  
   * In general, we do not allow projected images and they must have a valid 
   * camera model properly initialized (i.e., spiceinit'ed).  A check will also 
   * be performed on the target and instrument to ensure they match as we do not 
   * want to mix these criteria. 
   *  
   * Finally, since the bands extracted from a spectrum is absolute band 
   * numbers, it is required that the number of bands match the specfied count 
   * given in the config file. 
   * 
   * @param cube      Cube to check
   * 
   * @return Camera* Valid pointer is returned if cube is valid for processing, 
   *         otherwise returns a NULL pointer.
   */
  Camera *PixelDBConfig::Validate(Cube &cube) {
    Camera *camera(0);
    bool killit(false);
    QString file = FileName(cube.fileName()).name();
    if (!cube.isProjected()) {
      try {
        camera = cube.camera();
        if (!IString::Equal(camera->target()->name().toStdString(), m_target)) {
          ostringstream mess;
          mess << "File: " << file.toStdString() << " does not have the expected ( "
               << m_target << ") target (" << camera->target()->name() << ")";
          m_errHandler.LogError(PDBE::BADTARGET);
          killit = true;
        }
        if (!IString::Equal(GetInstrument(cube), m_instrument)) {
          ostringstream mess;
          mess << "File: " << file.toStdString() << " does not have the expected ( "
               << m_instrument << ") instrument (" << GetInstrument(cube) << ")";
          m_errHandler.LogError(PDBE::BADINSTRUMENT);
          killit = true;
        }
        if (cube.bandCount() != m_nbands) {
          ostringstream mess;
          mess << "File: " << file.toStdString() << " does not have the expected ( "
               << m_nbands << ") number bands (" << cube.bandCount() << ")";
          m_errHandler.LogError(PDBE::NOBANDS, mess.str());
          killit = true;
        }
      } catch (IException &ie) {
        ostringstream mess;
        mess << "File: " << file.toStdString() << " camera model failed to initialize ("
             << ie.toString() << ")";
        m_errHandler.LogError(PDBE::NOCAMERA, mess.str() );
        killit = true;
      }
    }
    else {
      QString mess = file + " is projected which is not supported/valid in the db";
      m_errHandler.LogError(PDBE::NOCAMERA, mess.toStdString());
    }

    // If any errors exist, delete the camera
    if (killit) {
      delete camera;
      camera = 0;
    }

    return (camera);
  }

  /**
   * @brief Validate a populated VIMS ISIS cube file class
   * 
   * @author kbecker (6/7/2010)
   * 
   * @param vfile Populated VIMS file to validated
   * 
   * @return bool True if the contents checks out, false if errors
   */
  bool PixelDBConfig::Validate(const File &vfile, const bool recurse)  {
    if (vfile.name().empty()) return (m_errHandler.LogError(PDBE::NOFILENAME));
    if (vfile.serialnumber().empty()) return (m_errHandler.LogError(PDBE::NOSERIALNUMBER)); 
    if (vfile.lines() <= 0) return (m_errHandler.LogError(PDBE::NOLINES)); 
    if (vfile.samples() <= 0) return (m_errHandler.LogError(PDBE::NOSAMPLES));
    if (vfile.bands() <= 0) return (m_errHandler.LogError(PDBE::NOBANDS));
    if (recurse) {
      for (int i = 0 ; i < vfile.pixels_size() ; i++) {
        if (!Validate(vfile.pixels(i), recurse)) return (false);
      }
    }

    if (!vfile.has_geom()) return (m_errHandler.LogError(PDBE::BADTHEGEOM));
    if (vfile.geom().ngeoms() != vfile.pixels_size()) {
      return (m_errHandler.LogError(PDBE::PARTIALGEOM));
    }

    return (true);
  }

  /**
   * @brief Validates a populated pixel PB message
   * 
   * @author kbecker (6/7/2010)
   * 
   * @param vpixel PB pixel to validated
   * 
   * @return bool True if valid, false if errors or incomplete
   */
  bool PixelDBConfig::Validate(const Pixel &vpixel, const bool recurse) {
    if (!vpixel.has_line())  return (m_errHandler.LogError(PDBE::NOLINES));
    if (!vpixel.has_sample())  return (m_errHandler.LogError(PDBE::NOSAMPLES));
    if (vpixel.spectrum_size() != m_nbands) return (m_errHandler.LogError(PDBE::BADAVGCOUNT)); 

    if (vpixel.has_sampres() && (vpixel.sampres() > m_maxSampRes)) 
      return (m_errHandler.LogError(PDBE::BADSAMPRES));
    if (vpixel.has_lineres() && (vpixel.lineres() > m_maxLineRes)) 
      return (m_errHandler.LogError(PDBE::BADLINERES));

    if (vpixel.has_sampres() && vpixel.has_lineres()) {
      if ((vpixel.sampres() != 0.0) && (vpixel.lineres() != 0.0)) {
        double ratio = vpixel.sampres()/vpixel.lineres();
        if (ratio > m_maxLineSampRatio) 
          return (m_errHandler.LogError(PDBE::BADRESRATIO));
        if (ratio < m_minLineSampRatio)
          return (m_errHandler.LogError(PDBE::BADRESRATIO));
      }
    }

    int ngood(0);
    for (int i = 0 ; i < vpixel.spectrum_size() ; i++) {
      if (!IsSpecial(vpixel.spectrum(i))) ngood++;
    }
    if (ngood == 0) return (m_errHandler.LogError(PDBE::NULLSPECTRUM));

    if (!vpixel.has_ettime()) return (m_errHandler.LogError(PDBE::NOETTIME));
    if (!vpixel.has_footprint()) return (m_errHandler.LogError(PDBE::NOFOOTPRINT));

    if (recurse) {
      if (!Validate(vpixel.points(), recurse)) return (false);
    }

    if (vpixel.points().center().distance() > m_maxTargetDistance) {
      return (m_errHandler.LogError(PDBE::MAXDISTANCE));
    }

    return (true);
  }

  bool PixelDBConfig::Validate(const Points &vpoint, const bool recurse)  {
    if (!vpoint.has_center()) return (m_errHandler.LogError(PDBE::NOCENTERPT));
    if (!vpoint.has_upperleft()) return (m_errHandler.LogError(PDBE::NOULEFTPT));
    if (!vpoint.has_upperright()) return (m_errHandler.LogError(PDBE::NOURIGHTPT));
    if (!vpoint.has_lowerright()) return (m_errHandler.LogError(PDBE::NOLRIGHTPT));
    if (!vpoint.has_lowerleft()) return (m_errHandler.LogError(PDBE::NOLLEFTPT)); 
    if (recurse) {
      if (!Validate(vpoint.center(), recurse)) return (false);
      if (!Validate(vpoint.upperleft(), recurse)) return (false);
      if (!Validate(vpoint.upperright(), recurse)) return (false);
      if (!Validate(vpoint.lowerright(), recurse)) return (false);
      if (!Validate(vpoint.lowerleft(), recurse)) return (false);
    }
    return (true);
  }

#if 0
  bool PixelDBConfig::Validate(const SpecAvgs &vspec, const bool recurse) {
    if (!vspec.has_band0()) return (m_errHandler.LogError(PDBE::NOSPECBANDS));
    if (!vspec.has_band1()) return (m_errHandler.LogError(PDBE::NOSPECBANDS));
    if (vspec.band0() > vspec.band1()) return (m_errHandler.LogError(PDBE::BADSPECBANDS));

    if (!vspec.has_avgdn()) return (m_errHandler.LogError(PDBE::NOSPECAVG));
    if (IsSpecial(vspec.avgdn())) return (m_errHandler.LogError(PDBE::BADSPECAVG));

    if (!vspec.has_name()) return (m_errHandler.LogError(PDBE::BADSPECNAME));
    if (vspec.name().empty()) return (m_errHandler.LogError(PDBE::BADSPECNAME));
    return (true);
  }
#endif

  bool PixelDBConfig::Validate(const Geometry &geom, const bool recurse) {
    if (!geom.has_line()) return (m_errHandler.LogError(PDBE::BADGEOMCOORDS));
    if (!geom.has_sample()) return (m_errHandler.LogError(PDBE::BADGEOMCOORDS));

    if (geom.incidence() > m_maxIncidence) return (m_errHandler.LogError(PDBE::MAXINCIDENCE));
    if (geom.emission() > m_maxEmission) return (m_errHandler.LogError(PDBE::MAXEMISSION));

    if (!geom.has_locale()) return (m_errHandler.LogError(PDBE::NOVECTORS));
    if (!geom.locale().has_scpos()) return (m_errHandler.LogError(PDBE::NOSCPOS));
    if (!geom.locale().has_sunpos()) return (m_errHandler.LogError(PDBE::NOSUNPOS)); 
    return (true);
  }

#if 0
  SpecAvgs PixelDBConfig::GetSpecAvgs(const int index) const {
    if ((index >= SpecAvgsCount()) || (index < 0)) {
      string mess = "Index " + IString(index) + " out of bounds";
      throw IException(IException::Programmer, mess, _FILEINFO_);
    }

    SpecAvgs savg;
    const DbProfile &dnProf = m_profiles[index];
    savg.set_name(dnProf.value("Name"));
    savg.set_band0(ToInteger(dnProf.value("StartingBand")));
    savg.set_band1(ToInteger(dnProf.value("EndingBand")));
    savg.set_avgdn(Null);
    return (savg);
  }
#endif

  bool PixelDBConfig::LogError(const PixelDBErrorHandler::PDBE_ErrorType &etype) {
     return (m_errHandler.LogError(etype));
  } 

  bool PixelDBConfig::LogError(const PixelDBErrorHandler::PDBE_ErrorType &etype, 
                                     const std::string &errmsg) {
    return (m_errHandler.LogError(etype,errmsg));
  }

  /**
   * @brief Initialize class from input PVL and Cube files 
   *  
   * This method is typically called at class instantiation time, 
   * but is reentrant.  It reads the parameter PVL file and 
   * extracts Photometric model and Normalization models from it. 
   * The cube is needed to match all potential profiles for each 
   * band. 
   * 
   * @param pvl  Input PVL parameter files
   * @param cube Input cube file to correct 
   *  
   * @author Kris Becker - 2/22/2010 
   * @history 2010-02-25 Kris Becker Added check for valid incidence angle 
   */
  void PixelDBConfig::init(const PvlObject &pvl) {

    //  Make it reentrant
    m_profiles.clear();

    //  Get DB constraints
    if (IString::Equal(pvl.name().toStdString(), "PixelDBConfig")) {
      m_pvl = pvl;
    }
    else {
      m_pvl = pvl.findObject("PixelDBConfig", PvlObject::Traverse);
    }
    m_instrument = QString(m_pvl["Instrument"]).toStdString();
    m_target = QString(m_pvl["Target"]).toStdString();
    m_nbands = QString(m_pvl["RequiredBands"]).toInt();

    DbProfile constraints(m_pvl.findGroup("Constraints",Pvl::Traverse));
    m_footinc = ConfKey(constraints, "FootprintIncrement", 0.2);
    m_maxIncidence = ConfKey(constraints, "MaximumIncidence", 120.0);
    m_maxEmission  = ConfKey(constraints, "MaximumEmission", 120.0);
    m_maxLineRes   = ConfKey(constraints, "MaxLineResolution", DBL_MAX);
    m_maxSampRes   = ConfKey(constraints, "MaxSampResolution", DBL_MAX);
    double lsratio1  = ConfKey(constraints, "MaxLineSampResRatio", DBL_MAX);
    double lsratio2 = 1.0/lsratio1;
    m_minLineSampRatio = std::min(lsratio1, lsratio2);
    m_maxLineSampRatio = std::max(lsratio1, lsratio2);
    m_maxTargetDistance = ConfKey(constraints, "MaximumTargetDistance", DBL_MAX);
    return;
  }


  std::string PixelDBConfig::GetInstrument(Cube &cube) const {
    //return (cube.group("Instrument")["InstrumentId"][0]);
      return QString(cube.group("Instrument")["InstrumentId"]).toStdString();
  }

} // namespace Isis


