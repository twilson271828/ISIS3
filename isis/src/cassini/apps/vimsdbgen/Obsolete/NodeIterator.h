#ifndef NodeIterator_h
#define NodeIterator_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.6 $                                                             
 * $Date: 2009/12/22 02:09:54 $                                                                 
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

#include "PvlObject.h"
#include "iException.h"


#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>


namespace Isis {

/**
 * @brief Node iterator virtual interface 
 *  
 * This Node iterator is an implementation of the GOF Iterator pattern.  It is 
 * intended to be used to process a series of Google Protobufs.
 *  
 * @author Kris Becker 
 * @internal 
 *   @history 2010-08-11 Kris Becker Original Version 
 * 
 */
template <class NODE>  
class NodeIterator {
  public:
    /** Constructor */
    virtual ~NodeIterator() { }

    virtual int Count() const = 0;
    virtual bool Begin() = 0;
    virtual bool Next() = 0;
    virtual bool IsDone() const = 0;
    virtual const NODE &CurrentNode() const = 0;
    virtual NODE &CurrentNode() = 0;
    virtual NODE &Last() = 0;

  protected:
    NodeIterator() { }
};


} // namespace Isis

#endif

