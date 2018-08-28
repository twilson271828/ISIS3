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
#include <string>
#include <vector>
#include <numeric>
#include <iostream>
#include <sstream>

#include "tnt/tnt_array1d.h"
#include "IException.h"
#include "PixelDBErrorHandler.h"
#include "Statistics.h"

using namespace std;

namespace Isis {


bool PixelDBErrorHandler::LogError(const PixelDBErrorHandler::PDBE_ErrorType &etype) { 
  _errors.get(etype).HitMe();
 return (false);
}

bool PixelDBErrorHandler::LogError(const PixelDBErrorHandler::PDBE_ErrorType &etype, 
                                   const std::string &errmsg) {
 _errors.get(etype).HitMe(errmsg);
 return (false);
}

size_t  PixelDBErrorHandler::ErrorCount() const {
  ErrorList::CollectorConstIter e = _errors.begin();
  size_t n(0);
  for (  ; e != _errors.end() ; ++e ) {
    n += e->second.eCount;
  }
  return (n);
}

void  PixelDBErrorHandler::getSummary(PvlContainer &pvl) const {
  ErrorList::CollectorConstIter e;
  for ( e = _errors.begin() ; e != _errors.end() ; ++e ) {
    if ( e->second.Count() > 0 ) {
      //pvl += PvlKeyword(e->second.eName, (BigInt) e->second.Count());
        pvl += PvlKeyword(QString(e->second.eName.c_str()));
    }
  }
  return;
}


std::ostream &PixelDBErrorHandler::logger(std::ostream &out) const {
  ErrorList::CollectorConstIter e;
  for ( e = _errors.begin() ; e != _errors.end() ; ++e ) {
    if ( e->second.Count() > 0 ) {
      const PDBEErrMsg &elog = e->second;
      out << elog.eName << ": (" << elog.eCount << ") [" << elog.eDescr
          << "] => Messages: " << elog.eList.size()  << endl;
      for ( unsigned int i = 0 ; i < elog.eList.size() ; i++ ) {
        out << "  " << elog.eList[i] << endl;
      }
    }
  }

  return (out);
}

void PixelDBErrorHandler::init() {
  _errors = ErrorList();
  _errors.add(BADTARGET,PDBEErrMsg(BADTARGET,"BadTarget","Planet/Target does not match config")); 
  _errors.add(BADINSTRUMENT,PDBEErrMsg(BADINSTRUMENT,"BadInstrument","Instrument does not match config"));
  _errors.add(NOCAMERA, PDBEErrMsg(NOCAMERA,"NoCamera","No camera model provided in cube"));
  _errors.add(NOFILENAME,PDBEErrMsg(NOFILENAME,"NoFilename","No filename set in DB(File)"));
  _errors.add(NOSERIALNUMBER,PDBEErrMsg(NOSERIALNUMBER,"NoSerialNumber","No serialnumber set in DB(File)"));
  _errors.add(NOLINES,PDBEErrMsg(NOLINES,"NoLines","No cube lines set in DB(File)"));
  _errors.add(NOSAMPLES,PDBEErrMsg(NOSAMPLES,"NoSamples","No cube samples set in DB(File)"));
  _errors.add(NOBANDS,PDBEErrMsg(NOBANDS,"NoBands","No cube bands set in DB(File)"));
  _errors.add(NOPIXELCOORDS,PDBEErrMsg(NOPIXELCOORDS,"NoPixelCoords","Pixel line/sample not set in DB(Pixel)"));
  _errors.add(NULLSPECTRUM,PDBEErrMsg(NULLSPECTRUM,"NullSpectrum","Spectrum contains all NULLS - no valid data"));
  _errors.add(BADAVGCOUNT,PDBEErrMsg(BADAVGCOUNT,"BadVeAvgCount","Spectrum averages not complete in DB(SpecAvgs)"));
  _errors.add(BADSAMPRES,PDBEErrMsg(BADSAMPRES,"BadSampRes","Sample resolution exceeds MAX DB(Pixel)"));
  _errors.add(BADLINERES,PDBEErrMsg(BADLINERES,"BadLineRes","Line resolution exceeds MAX DB(Pixel)"));
  _errors.add(BADRESRATIO,PDBEErrMsg(BADRESRATIO,"BadResRatio","SampRes/LineRes exceeds limits in DB(Pixel)"));
  _errors.add(NOETTIME,PDBEErrMsg(NOETTIME,"NoETime","Ephemeris time not set in Db(Pixel)"));
  _errors.add(NOFOOTPRINT,PDBEErrMsg(NOFOOTPRINT,"NoFootprint","GIS Footprint no set in DB(Pixel)"));
  _errors.add(NOVECTORS,PDBEErrMsg(NOVECTORS,"NoVectors","No S/C,SunPos vectors set in DB(Pixel)"));
  _errors.add(NOSCPOS,PDBEErrMsg(NOSCPOS,"NoSCPos","No S/C postion set in DB(Pixel)"));
  _errors.add(NOSUNPOS,PDBEErrMsg(NOSUNPOS,"NoSunPos","No Sun Position set in DB(Pixel)"));
  _errors.add(NOCENTERPT,PDBEErrMsg(NOCENTERPT,"NoCenterPt","Center pixel point not set in DB(Pixel)"));
  _errors.add(NOULEFTPT,PDBEErrMsg(NOULEFTPT,"NoUpperLeftPt","Upper Left pixel point not set in DB(Pixel)"));
  _errors.add(NOURIGHTPT,PDBEErrMsg(NOURIGHTPT,"NoUpperRightPt","Upper Right pixel point not set in DB(Pixel)"));
  _errors.add(NOLRIGHTPT,PDBEErrMsg(NOLRIGHTPT,"NoLowerRightPt","Lower Right pixel point not set in DB(Pixel)"));
  _errors.add(NOLLEFTPT,PDBEErrMsg(NOLLEFTPT,"NoLowerLeftPt","Lower Left pixel point not set in DB(Pixel)"));
  _errors.add(NOSPECBANDS,PDBEErrMsg(NOSPECBANDS,"NoSpectralBands","No Spectral bands set in DB(Pixel)"));
  _errors.add(BADSPECBANDS,PDBEErrMsg(BADSPECBANDS,"BadSpectralBands","Invalid spectral bands in DB(SpecAvgs)"));
  _errors.add(NOSPECAVG,PDBEErrMsg(NOSPECAVG,"NoSpectralAvg","Spectral value not set in Db(SpevAvgs)"));
  _errors.add(BADSPECAVG,PDBEErrMsg(BADSPECAVG,"BadSpectralAvg","Invalid/missing spectral value in DB(SpecAvgs)"));
  _errors.add(BADSPECNAME,PDBEErrMsg(BADSPECNAME,"BadSpectralName", "Invalid/missing spectral average name in DB(SpecAgs)"));
  _errors.add(BADGEOMCOORDS,PDBEErrMsg(BADGEOMCOORDS,"BadGeometryCoords","Missing line/sample coords in DB(Geometry)"));
  _errors.add(MAXEMISSION,PDBEErrMsg(MAXEMISSION,"MaxEmission","Emission angle exceeds limits in DB(Geometry)"));
  _errors.add(MAXINCIDENCE,PDBEErrMsg(MAXINCIDENCE,"MaxIncidence","Incidence angle exceeds limits in DB(Geometry)"));
  _errors.add(MAXDISTANCE,PDBEErrMsg(MAXDISTANCE,"MaxDistance","Maximum distance to target exceeded in DB(Pixel)"));
  _errors.add(GEOSEXCEPTION,PDBEErrMsg(GEOSEXCEPTION,"GeosException","GIS GEOS exception (error) for pixel"));
  _errors.add(THEGEOMFAILED,PDBEErrMsg(THEGEOMFAILED,"TheGeomFailed","Creation of file pixel geom union failed in DB(File)"));
  _errors.add(BADTHEGEOM,PDBEErrMsg(BADTHEGEOM,"BadTheGeom","Composite Pixel GEOM union not set in DB(File)"));
  _errors.add(PARTIALGEOM,PDBEErrMsg(PARTIALGEOM,"PartialGeom","Composite Pixel GEOM union incomplete in DB(File)"));
  _errors.add(FILEDUPLICATE,PDBEErrMsg(FILEDUPLICATE,"FileDuplicate","File already exists in database"));
  _errors.add(UNKNOWN,PDBEErrMsg(UNKNOWN,"Unknown","Count of unknown errors during processing"));
  return;
}

} // namespace ISIS
