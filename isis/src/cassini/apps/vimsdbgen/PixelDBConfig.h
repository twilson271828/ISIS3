#if !defined(PixelDBConfig_h)
#define PixelDBConfig_h
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
#include <QString>

#include "IString.h"
#include "Camera.h"
#include "DbProfile.h"
#include "SpecialPixel.h"
#include "PixelDBErrorHandler.h"
#include "PixelDB.pb.h"

namespace Isis {

  class PvlObject;
  class Cube;
  class Camera;
  namespace google {
    namespace protobuf {
      class File;
//      class SpecAvgs;
      class Pixel;
      class Points;
    }
  }

  /**                                                                       
   * @brief An implementation for the PixelDBConfig configuration parameters
   *                                                                        
   * @author  2010-05-03 Kris Becker 
   *  
   * @history 2012-09-20 Kris Becker Added m_maxTargetDistance parameter 
   * 
   */                                                                       
  class PixelDBConfig {
    public:
      PixelDBConfig();

      /**
       * @brief Create Vims DB Config instance
       * 
       */
      PixelDBConfig (PvlObject &pvl);

      PixelDBConfig(const std::string &conf);

      //! Destructor
      virtual ~PixelDBConfig() {}


      void Read(const std::string &db);
      void setConfig(const std::string &conf);
      const PvlObject &getConfig() const { return (m_pvl); }

      virtual Camera *Validate(Cube &cube);
      virtual bool Validate(const File &vfile, const bool recurse = true);
      virtual bool Validate(const Pixel &vpixel, const bool recurse = true);
      virtual bool Validate(const Points &vpoints, const bool recurse = true);
      virtual bool Validate(const Geometry &geom, const bool recurse = true);

      double GetFootprintIncrement() const {  return (m_footinc); }

      bool LogError(const PixelDBErrorHandler::PDBE_ErrorType &etype);
      bool LogError(const PixelDBErrorHandler::PDBE_ErrorType &etype, 
                    const std::string &errmsg);

      void getErrorSummary(PvlContainer &pvl) const {
        m_errHandler.getSummary(pvl);
        return;
      }

      std::ostream &dumpErrorLog(std::ostream &o) const {
        return (m_errHandler.logger(o));
      }

    private:
      typedef PixelDBErrorHandler PDBE;

      PvlObject        m_pvl;
      std::string      m_instrument;
      std::string      m_target;
      int              m_nbands;
      double           m_footinc;
      double           m_maxEmission;
      double           m_maxIncidence;
      double           m_maxLineRes;
      double           m_maxSampRes;
      double           m_maxLineSampRatio;
      double           m_minLineSampRatio;
      double           m_maxTargetDistance;
      std::vector<DbProfile>  m_profiles;
      PixelDBErrorHandler     m_errHandler;

      void init(const PvlObject &pvl);
      std::string GetInstrument(Cube &cube) const;

      /**
       * @brief Helper method to initialize parameters
       *  
       * This method will check the existance of a keyword and extract the value 
       * if it exists to the passed parameter (type).  If it doesn't exist, the 
       * default values is returned. 
       * 
       * @param T Templated variable type
       * @param conf Parameter profile container
       * @param keyname Name of keyword to get a value from
       * @param defval Default value it keyword/value doesn't exist
       * @param index Optional index of the value for keyword arrays
       * 
       * @return T Return type
       */
      template <typename T> 
        T ConfKey(const DbProfile &conf, const std::string &keyname, 
                  const T &defval,int index = 0) const {
        if (!conf.exists(QString(keyname.c_str()))) { return (defval); }
        if (conf.count(QString(keyname.c_str()) ) < index) { return (defval); }
        IString iValue(conf.value(QString(keyname.c_str()), index));
        T value = iValue;  // This makes it work with a string?
        return (value);
      }

      /** 
       * @brief Helper function to convert values to doubles
       * 
       * @param T Type of value to convert
       * @param value Value to convert
       * 
       * @return double Converted value
       */
      template <typename T> double ToDouble(const T &value) const {
          return (IString(value).Trim(" \r\t\n").ToDouble());
      }

      /** 
      * @brief Helper function to convert values to Integers
      * 
      * @param T Type of value to convert
      * @param value Value to convert
      * 
      * @return int Converted value
      */
     template <typename T> int ToInteger(const T &value) const {
         return (IString(value).Trim(" \r\t\n").ToInteger());
     }

  };

}

#endif

