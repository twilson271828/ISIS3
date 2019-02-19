#if !defined(PixelDBErrorHandler_h)
#define PixelDBErrorHandler_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $ 
 * $Date: 2010/02/24 09:54:18 $ 
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

#include <iostream>
#include <sstream>                                                                      
#include <ostream>
#include <iomanip>

#include "PvlContainer.h"
#include "CollectorMap.h"

namespace Isis {

  /**                                                                       
   * @brief Error handler for PixelDB processing
   *                                                                        
   * @author  2010-07-22 Kris Becker
   * 
   */                                                                       
  class PixelDBErrorHandler {
    public:
      enum PDBE_ErrorType {
        BADTARGET = 0,
        BADINSTRUMENT,
        NOCAMERA,
        NOFILENAME,
        NOSERIALNUMBER,
        NOLINES,
        NOSAMPLES,
        NOBANDS,
        NOPIXELCOORDS,
        NULLSPECTRUM,
        BADAVGCOUNT,
        BADSAMPRES,
        BADLINERES,
        BADRESRATIO,
        NOETTIME,
        NOFOOTPRINT,
        NOVECTORS,
        NOSCPOS,
        NOSUNPOS,
        NOCENTERPT,
        NOULEFTPT,
        NOURIGHTPT,
        NOLRIGHTPT,
        NOLLEFTPT,
        NOSPECBANDS,
        BADSPECBANDS,
        NOSPECAVG,
        BADSPECAVG,
        BADSPECNAME,
        BADGEOMCOORDS,
        MAXEMISSION,
        MAXINCIDENCE,
        MAXDISTANCE,
        GEOSEXCEPTION,
        THEGEOMFAILED,
        BADTHEGEOM,
        PARTIALGEOM,
        FILEDUPLICATE,
        UNKNOWN
      };

      PixelDBErrorHandler() { init(); }

      //! Destructor
      virtual ~PixelDBErrorHandler() { }

      bool LogError(const PDBE_ErrorType &etype);
      bool LogError(const PDBE_ErrorType &etype, const std::string &errmsg);
      size_t ErrorCount() const;

      void getSummary(PvlContainer &pvl) const;
      std::ostream &logger(std::ostream &o) const;

    private:
      struct PDBEErrMsg {
        PDBEErrMsg() : eType(UNKNOWN), eName("UNKNOWN"),
                       eDescr("Unknown error"), eList(), eCount(0) { }
        PDBEErrMsg(PDBE_ErrorType etype, const std::string &name, 
                   const std::string &descr) :  eType(etype), eName(name),
                                                eDescr(descr), eList(), 
                                                eCount(0) { }
        virtual ~PDBEErrMsg() { }

        inline void HitMe() { eCount++; }
        inline void HitMe(const std::string &msg) { 
          eCount++; 
          eList.push_back(msg);
        }
        inline size_t Count() const { return (eCount); }
        inline int MsgSize() const { return (eList.size()); }

        PDBE_ErrorType           eType;
        std::string              eName;
        std::string              eDescr;
        std::vector<std::string> eList;
        size_t                   eCount;
      };

      typedef CollectorMap<int, PDBEErrMsg, SimpleCompare> ErrorList;
      ErrorList _errors;

      void init();
  };

}

#endif

