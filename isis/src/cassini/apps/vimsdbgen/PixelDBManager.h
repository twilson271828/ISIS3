#ifndef PixelDBManager_h
#define PixelDBManager_h
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

#include <QSharedPointer>

#include "CubeAttribute.h"
#include "PvlObject.h"
#include "CollectorMap.h"
#include "IException.h"

#include "PixelDBConfig.h"
#include "PixelMaker.h"
#include "PixelDB.pb.h"

#include "Publisher.h"

namespace Isis {


class PixelDBManager {
  public:
    PixelDBManager();
    PixelDBManager(const std::string &dbfile);
    virtual ~PixelDBManager () { }

    void setConfig(const std::string &config) throw (IException &);
    void setPublisher(Publisher *publisher);

    void ProcessBySpectra(const std::string &ifile);

    BigInt getProcessed()  const { return (m_processed); }

    BigInt getTotalValid() const { return (m_totalValid); }


    void getErrorSummary(PvlContainer &pvl) const {
      m_config.getErrorSummary(pvl);
      return;
    }

    std::ostream &dumpErrorLog(std::ostream &o) const {
      return (m_config.dumpErrorLog(o));
    }

  private:
    typedef PixelDBErrorHandler PDBE;

    PixelDBConfig                m_config;
    PixelMaker                   m_maker;
    QSharedPointer<Publisher>    m_publisher;
    BigInt                       m_processed;
    BigInt                       m_totalValid;

};

} // namespace Isis

#endif

