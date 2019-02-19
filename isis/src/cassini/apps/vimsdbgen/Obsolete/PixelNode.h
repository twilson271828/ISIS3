#ifndef PixelNode_h
#define PixelNode_h
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

#include "PvlObject.h"
#include "CollectorMap.h"
#include "IException.h"

#include "PixelDB.pb.h"
#include "PBUtilities.h"

namespace Isis {

class PixelNode {
  public:
    PixelNode() : _node(0) { }
    PixelNode(PixelDB *p) : _node(p) { }
    PixelNode(std::istream &inp, int nbytes) : _node(0) {
        read(inp, nbytes);
    }
    virtual ~PixelNode () { delete _node; }

    PixelDB &Get() {
      if ( !_node ) {
         throw IException(IException::Programmer,
                                    "No PixelNode defined",
                                   _FILEINFO_);
      }
      return (*_node);
    }

    const PixelDB &Get() const {
      if ( !_node ) {
         throw IException(IException::Programmer,
                                    "No PixelNode defined",
                                   _FILEINFO_);
      }
      return (*_node);
    }

    std::istream &read(std::istream &inp, int nbytes) {
      delete _node;
      _node = new PixelDB;

      TNT::Array1D<char> pbdata(nbytes);
      std::cout << "Reading a buffer of " << nbytes << " size\n";

      // Position file pointer and read indicated bytes from stream
      inp.read(&pbdata[0], nbytes);
      if ( !inp.good() ) {
        std::ostringstream mess;
        mess << "Failed to read " << nbytes << " from input stream";
        throw IException(IException::Programmer, mess.str(), _FILEINFO_);
      }

      //  Got to set up a coded input stream to work around limits
      ::google::protobuf::io::ArrayInputStream data(&pbdata[0], nbytes);
      ::google::protobuf::io::ZeroCopyInputStream *zbuff = &data;
      ::google::protobuf::io::CodedInputStream ips(zbuff);
      ips.SetTotalBytesLimit(nbytes, -1);

      // Now parse the protocol message from the string
      if ( !_node->ParseFromCodedStream(&ips) ) {
        std::string mess = "Failed to parse Protocol Buffer from input stream";
        throw IException(IException::Programmer, mess, _FILEINFO_);
      }

      return (inp);
    }

    std::ostream &write(std::ostream &out)  const { 
      if ( !_node->SerializeToOstream(&out) ) {
        std::string mess = "Failed to write PB to output stream";
        throw IException(IException::User, mess, _FILEINFO_);
      }
      return (out);
    }

    void Reset(PixelDB *node = 0) {
      delete _node;
      _node = node;
    }

  private:
    PixelDB   *_node;

};

} // namespace Isis

#endif

