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

#include "BoxcarCachingAlgorithm.h"
#include "Brick.h"
#include "DbProfile.h"
#include "IException.h"
#include "iTime.h"
#include "PixelDBManager.h"
#include "Statistics.h"

using namespace std;

namespace Isis {


PixelDBManager::PixelDBManager() :  m_config(), m_maker(&m_config), 
                                    m_publisher(new Publisher()), 
                                    m_processed(0), m_totalValid(0) { }

PixelDBManager::PixelDBManager(const std::string &dbfile) :  
                               m_config(), m_maker(&m_config), 
                               m_publisher(new Publisher()),
                               m_processed(0), m_totalValid(0) { 

}


void PixelDBManager::setConfig(const std::string &config) throw (IException &) {
   m_config.Read(config);
   m_maker.setConfig(&m_config);
  return;
}

void PixelDBManager::setPublisher(Publisher *publisher) {
  m_publisher = QSharedPointer<Publisher> (publisher);
  return;
}

void PixelDBManager::ProcessBySpectra(const std::string &ifile) {
  Cube cube;
  cube.open(QString(ifile.c_str()));
  cube.addCachingAlgorithm(new BoxcarCachingAlgorithm());  //  Improved I/O

  //  Check to ensure this cube satisfies constraints
  Camera *camera(0);
  File vfile;    
  // If for some reason this doen't suceed, it has already been recorded
  if ((camera = m_maker.Init(cube, vfile)) != 0) {

    // Load the entire brick into memory
    Brick data(cube, cube.sampleCount(), cube.lineCount(),
               cube.bandCount());
    data.begin();
    cube.read(data);

    Brick spectra(cube, 1, 1, cube.bandCount());
    // Compute database
    spectra.begin();
    int nbands(cube.bandCount());
    int boffset(cube.sampleCount() * cube.lineCount());
    while (!spectra.end()) {
      // cube.Read(spectra);
      int sample = spectra.Sample();
      int line = spectra.Line();
      // cout << "Sample: " << sample << ", Line: " << line << "\n";
      int sindex = data.Index(sample, line, 1);
      for (int band = 0 ; band < nbands ; band++) {
        int index = sindex + (band * boffset);
       //  cout << "Index: " << index << "\n";
        spectra[band] = data[index];
      }

      //  Single pixel cartography and data procesing
      m_maker.Process(*camera, spectra, vfile);
      spectra.next();
      m_processed++;
    }

    // Generates the full file geometry
    m_maker.Finalize(vfile);

    // Extract keywords from Instrument and Archive groups
    DbProfile instrmt(cube.label()->findGroup("Instrument", Isis::Pvl::Traverse));
    DbProfile archive(cube.label()->findGroup("Archive", Isis::Pvl::Traverse));
    DbProfile keys(instrmt, archive, FileName(QString(ifile.c_str())).baseName());

    //  This conversion is required to put the UTC time from the label, which
    //  is of the form YYYY-DOYTHH:MM:SS.MM, into a form that PostgreSQL's
    //  TIMESTAMP supports (YYYY-MM-DDTHH:MM:SS.mmm).
    keys.add("UTCStartTime", iTime(keys("StartTime")).UTC());
    keys.add("UTCStopTime", iTime(keys("StopTime")).UTC());
    

    //  Add this file to the database
    m_totalValid += m_publisher->publish( vfile, keys );
  }
  return;
}

} // namespace ISIS
