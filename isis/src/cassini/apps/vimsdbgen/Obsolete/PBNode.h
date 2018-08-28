#ifndef PBNode_h
#define PBNode_h
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


#include "IException.h"
#include "PvlGroup.h"
#include "tnt/tnt_array1d.h"

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>


namespace Isis {

/**
 * @brief DefaultSupport provides an interface for vectors
 * 
 */
template <typename T, typename U>
  struct DefaultSupport {
    private:
  // These methods must be customized for each Protocol Buffer message
  // repeated field
      void Initialize(T &buffer) { buffer.clear(); }
      bool Insert(T &buffer, const U &message) {
         buffer.push_back(message); 
         return (true);
      }
      int Count(const T &buffer) const { return (buffer.size());  }

};

/**
 * @brief Google Protocol Message Volume Node container
 *  
 * This class provides control over the size of any Google Protocol Buffer (PB) 
 * that contains a repeated parameter.  There are not too many instances where 
 * this becomes a problem, but the PB will be large, but I have come across one.
 *  
 * The user of this class must provide the insert method as that will be 
 * dependant on the name of the repeated field.  It is declare as a private 
 * method so that inserts can be better managed through the add(U) method. 
 * Failure (add(U) == false) indicates the node buffer is full.  Outright 
 * failures are indicated through exceptions. 
 *  
 * @author Kris Becker 
 * @internal 
 *   @history 2010-07-29 Kris Becker Original Version 
 * 
 */
template < class T, class U,
           template <class, class> class SupportPolicy = DefaultSupport
         >  
  class PBNode : public SupportPolicy<T, U> {
  public:
    /** Constructor */
    PBNode() : _maxsize(64*1024*1024), _data(), _name(""), 
               _size(_data.ByteSize()) { 
      Initialize(_data);
      _name = getName(_data);
      _size = SerializedSize();
    }

    PBNode(const int &maxsize) : _maxsize(maxsize), _data(), 
                                 _name(""), _size(_data.ByteSize()) {
      Initialize(_data);
      _name = getName(_data);
      _size = SerializedSize();
    }

    PBNode(const T &data, const BigInt &maxsize) : _maxsize(maxsize), 
                          _data(data), _name(getName(_data)), 
                          _size(data.ByteSize()) { }

    PBNode(std::istream &inp, int nbytes) : _maxsize(0), _data(), 
                                            _name(""), _size(0) {
      read(inp, nbytes);
      _name = getName(_data);
      _maxsize = size();
    }

    PBNode(const char *pbdata, int nbytes) : _maxsize(nbytes), _data(), 
                                             _name(""), _size(nbytes) {
      read(pbdata, nbytes);
      _name = getName(_data);
      _maxsize = size();
    }

    /** Destructor */
    virtual ~PBNode() { }

    int size() const { return (_size); }
    int size(const int &bytes) const { return (_size+bytes); }
    int count() const { return (Count(_data)); }
    std::string name() const { return (_name);  }

    T &Data()            { return (_data); }
    const T Data() const { return (_data); }

    bool HasSpaceFor(const int &bytes) const {
      return (size(bytes) <= _maxsize);
    }

    int SerializedSize() const { return (_data.ByteSize());  }

    bool add(const U &field) {
      int nbytes = field.ByteSize();
      if ( !HasSpaceFor(nbytes)  ) { return (false); }
      if ( !Insert(_data, field) ) { return (false); }
      adjust(nbytes);
      return (true);
    }

    void adjust(const int nbytes) {
      _size += nbytes;
    }

    std::ostream &write(std::ostream &out)  const { 
      if ( !_data.SerializeToOstream(&out) ) {
        std::string mess = "Failed to write PB to output stream";
        throw IException(IException::User, mess, _FILEINFO_);
      }
      //_size = SerializedSize();
      return (out);
    }

    std::istream &read(std::istream &inp) { 
      if ( !_data.ParseFromIstream(inp) ) {
        std::string mess = "Failed to parse PB from input stream";
        throw IException(IException::User, mess, _FILEINFO_);
      }
      _size = SerializedSize();
      return (inp);
    }

    std::istream &read(std::istream &input, int nbytes) { 
      TNT::Array1D<char> pbData(nbytes);
      std::cout << "Reading a buffer of " << nbytes << " size\n";

      // Position file pointer and read indicated bytes from stream
      input.read(&pbData[0], nbytes);
      if ( !input.good() ) {
        std::ostringstream mess;
        mess << "Failed to read " << nbytes << " from input stream";
        throw IException(IException::Programmer, mess.str(), _FILEINFO_);
      }

      // Parse from the buffer
      read(&pbData[0], nbytes);
      return (input);
    }

    void read(const char *pbdata, int nbytes) { 
      //  Got to set up a coded input stream to work around limits
      ::google::protobuf::io::ArrayInputStream data(pbdata, nbytes);
      ::google::protobuf::io::ZeroCopyInputStream *zbuff = &data;
      ::google::protobuf::io::CodedInputStream ips(zbuff);
      ips.SetTotalBytesLimit(nbytes, -1);

      // Now parse the protocol message from the string
      if ( !_data.ParseFromCodedStream(&ips) ) {
        std::string mess = "Failed to parse Protocol Buffer from input stream";
        throw IException(IException::Programmer, mess, _FILEINFO_);
      }
      _size = nbytes;
    }

    PvlGroup ToPvl(const std::string &name = "") const {
      const ::google::protobuf::Descriptor *desc = _data.GetDescriptor();
      const ::google::protobuf::FileDescriptor *fdesc = desc->file();
      std::string messName = desc->name();
      std::string protoFile = fdesc->name();
      std::string pbName = (name.empty() ? messName : name);

      PvlGroup pvl("PBNode");
      pvl += PvlKeyword("Name", pbName);
      pvl += PvlKeyword("MessageName", messName);
      pvl += PvlKeyword("ProtoFile", protoFile);
      pvl += PvlKeyword("Size", SerializedSize());
     return (pvl);
    }

  private:
    int        _maxsize;
    T           _data;
    std::string _name;
    mutable BigInt _size;

    std::string getName(const T &data) const {
      const ::google::protobuf::Descriptor *desc = data.GetDescriptor();      
      return (desc->name());
    }


};


} // namespace Isis

#endif

