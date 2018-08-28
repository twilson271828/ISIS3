#if !defined(PixelDBConstraints_h)
#define PixelDBConstraints_h
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

#include "IString.h"
#include "Camera.h"
#include "DbProfile.h"
#include "SpecialPixel.h"
#include "PixelDBErrorHandler.h"

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
   * @brief An implementation for the PixelDBConstraints configuration parameters
   *                                                                        
   * @author  2010-05-03 Kris Becker
   * 
   */                                                                       
  class PixelDBConstraints {
    public:
      PixelDBConstraints();

      /**
       * @brief Create Vims DB Config instance
       * 
       */
      PixelDBConstraints (PvlObject &pvl);

      PixelDBConstraints(const std::string &conf);

      //! Destructor
      virtual ~PixelDBConstraints() {}


      void Read(const std::string &db);
      const PvlObject &get() const { return (_pvl); }

      virtual Camera *Validate(Cube &cube);
      virtual bool Validate(const File &vfile, const bool recurse = true);
      virtual bool Validate(const Pixel &vpixel, const bool recurse = true);
      virtual bool Validate(const Points &vpoints, const bool recurse = true);
      virtual bool Validate(const Geometry &geom, const bool recurse = true);

      bool LogError(const PixelDBErrorHandler::PDBE_ErrorType &etype);
      bool LogError(const PixelDBErrorHandler::PDBE_ErrorType &etype, 
                    const std::string &errmsg);

      void getErrorSummary(PvlContainer &pvl) const {
        _errHandler.getSummary(pvl);
        return;
      }

      std::ostream &dumpErrorLog(std::ostream &o) const {
        return (_errHandler.logger(o));
      }

    protected:

    private:
      typedef PixelDBErrorHandler PDBE;

      PvlGroup         _pvl;
      std::string      _instrument;
      std::string      _target;
      double           _maxEmission;
      double           _maxIncidence;
      double           _maxLineRes;
      double           _maxSampRes;
      double           _maxLineSampRatio;
      double           _minLineSampRatio;
      PixelDBErrorHandler     _errHandler;

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
        if (!conf.exists(keyname)) { return (defval); }
        if (conf.count(keyname) < index) { return (defval); }
        IString iValue(conf.value(keyname, index));
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

