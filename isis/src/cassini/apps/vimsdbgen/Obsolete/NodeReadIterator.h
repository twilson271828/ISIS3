#ifndef NodeReadIterator_h
#define NodeReadIterator_h
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

#include "iException.h"
#include "NodeIterator.h"
#include "PixelNode.h"


#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>


namespace Isis {

/**
 * @brief Google Protocol Message Volume Node reader
 *  
 * @author Kris Becker 
 * @internal 
 *   @history 2010-08-12 Kris Becker Original Version 
 * 
 */
  class NodeReadIterator : public NodeIterator<PixelDB> {
  public:

    /** Constructor */
    NodeReadIterator(const PBVolume &volume, std::istream *inp, 
                     BigInt startByte) : _nmap(), _input(inp), 
                     _byte0(startByte), _node(),  _current(-1) { 
      init(volume);
    }
    virtual ~NodeReadIterator() { delete _input; }

    virtual int Count() const { return (_nmap.size()); }
    virtual bool Begin() { 
      return (Load(0));
    }

    virtual bool Next() {
      return (Load(_current+1));
    }

    virtual bool IsDone() const {
      return (!isvalid(_current));
    }

    virtual const PixelDB &CurrentNode() const {
      return (fetch());
    }
    virtual PixelDB &CurrentNode() {
      return (fetch());
    }

    virtual PixelDB &Last() {
      Load(Count()-1);
      return (fetch());
    }


  private:
    std::vector<PBPartition> _nmap;
    std::istream *_input;
    BigInt       _byte0;
    PixelNode    _node;
    int          _current;

    bool isvalid(const int index) const {
      return ( (index >= 0) && (index < Count()));
    }

    void init(const PBVolume &volume) {
      _nmap.clear();
      for ( int i = 0 ; i < volume.maps_size() ; i++ ) {
        _nmap.push_back(volume.maps(i));
      }
      return;
    }

    int serialbytes(int index) const {
      return (_nmap[index].nbytes());
    }

    BigInt offset(int index) const {
      BigInt nbytes(_byte0);
      for ( int i = 0 ; i < index ; i++ ) {
         nbytes += serialbytes(i);
      }
      return (nbytes);
    }


    bool Load(int index) {
      if ( (index != _current) ) {
        _node.Reset();
        if ( isvalid(index) ) {
          if ( !_input ) {
            throw IException(IException::Programmer, 
                             "Stream pointer not established for PixelDB I/O",
                                      _FILEINFO_);
          }
  
          BigInt fpos = offset(index);
           // Position file pointer and read indicated bytes from stream
          _input->seekg(fpos, std::ios::beg);
          std::cout << "Reading node[" << index << "] at filepos " 
                    << (BigInt) _input->tellg() << "\n";
          _node.read(*_input, serialbytes(index));
        }
      }
      _current = index;
      return (true);
    }

    PixelDB &fetch() {
      if ( !IsDone() ) {
        return (_node.Get());
      }
      else {
        throw IException(IException::Programmer, 
                                  "PixelDB iterator out of bounds",
                                  _FILEINFO_);
      }
    }
    const PixelDB &fetch() const {
      if ( !IsDone() ) {
        return (_node.Get());
      }
      else {
        throw IException(IException::Programmer, 
                                  "PixelDB iterator out of bounds",
                                  _FILEINFO_);
      }
    }
};


} // namespace Isis

#endif

