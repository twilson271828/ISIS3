#if !defined(DbColumn_h)
#define DbColumn_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.1 $
 * $Date: 2007/03/30 17:19:48 $
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
#include <iostream>
#include <stack>
#include <functional>  

#include "Publisher.h"
#include "PixelDB.pb.h"

namespace Isis {

class iException;

class DbColumn {
  public:
  DbColumn() : _name(), _value() { }
  DbColumn(const std::string &colname) : _name(colname), _value(), _key() { }
  DbColumn(const std::string &colname, const std::string &val) : 
           _name(colname),_value(val),_key() { }
  DbColumn(const std::string &colname, const std::string &val, 
           const std::string &key) : _name(colname), _value(val), _key() { }

  bool isValid() const { return (!_name.empty()); }
  bool isMissing() const { return (_value.empty()); }

  std::string name() const { return (_name); }
  std::string value() const { return (_value); }
  std::string key() const { return (_key); }
  void setKey(const std::string &kpath) { _kpath = kpath; }
  void setValue(const std::string &value) { _value = value; }

  std::string bind() const { 
    return(bind(name()));
  }

  static std::string bind(const std::string &name) {
    return(std::string(":" + name));
  }

  private:
    std::string _name;
    std::string _value;
    std::string _key;
};

}  // namespace Isis
#endif

