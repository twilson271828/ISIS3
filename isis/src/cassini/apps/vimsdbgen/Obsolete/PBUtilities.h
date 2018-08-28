#ifndef PBUtilities_h
#define PBUtilities_h
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
#include "IException.h"

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include "tnt/tnt_array1d.h"

namespace Isis {

/**
 * @brief Google Protocol buffer I/O policy 
 *  
 * Usage of this class requires one to include the Google Protocol Buffer 
 * include file T.pb.h so that certain methods are available to perform I/O 
 * related activities 
 *  
 * @author Kris Becker 
 * @internal 
 *   @history 2010-04-21 Kris Becker Original Version 
 * 
 */
template <class PB>  class PBIO {
  public:
    /** Constructor */
    PBIO() { }
    /** Destructor */
    virtual ~PBIO() { }

    /**
     * @brief Returns the name of the Pvl object describing message 
     *  
     * This method returns the name of the PvlObject that will contain all the 
     * data describing the contents of the Protocol Buffer message. 
     * 
     * @author kbecker (6/9/2010)
     * 
     * @return std::string Name of PvlObject used to contain message description
     */
    std::string DataObjectName() const {
      return (std::string("ProtocolBuffer"));
    }

  /**
   * @brief Create a ISIS Pvl PB data object 
   *  
   * This method creates an ISIS Pvl data object that describes the location and 
   * size of a Google Protocol Buffer (PB) message within a file.  The PB object 
   * is used to determine the size of the message when serialized.  This will be 
   * the amount of byte data written to the file for the current state of the 
   * PB object and all its contents. 
   *  
   * The Name keyword is optional but may be important if more than one PB data 
   * object is to be written. 
   *  
   * The StartByte keyword specifies the position in the file where the PBData 
   * is to be written or read from.  This position is 1-based, in that StartByte 
   * = 1 is the first byte in the file.  Note that this offset can be adjusted 
   * freely before writing the data to the file if needed. 
   *  
   * The Bytes keyword specifies the size of the PB data as determined by the 
   * Google Protocol Buffer ByteSize() method. 
   * 
   * @param pb        Protocol Buffer class to describe
   * @param startbyte Postion in file (stream) to write the PB data
   * @param name      [Optional] Name of the PB data object
   * 
   * @return PvlObject Pvl object that describes the PB data placement in the 
   *         output stream
   */
  PvlObject DataObject(const PB &pb, const BigInt &startbyte = 1, 
                       const std::string &name = "") {
    PvlObject pvl(DataObjectName());
    const ::google::protobuf::Descriptor *desc = pb.GetDescriptor();
    const ::google::protobuf::FileDescriptor *fdesc = desc->file();
    std::string messName = desc->name();
    std::string protoFile = fdesc->name();
    std::string pbName = (name.empty() ? messName : name);
    pvl += PvlKeyword("Name", pbName);
    pvl += PvlKeyword("MessageName", messName);
    pvl += PvlKeyword("ProtoFile", protoFile);
    pvl += PvlKeyword("StartByte", startbyte);
    pvl += PvlKeyword("Bytes", pb.ByteSize());
   return (pvl);
  }

  /**
   * @brief Returns PBData location parameters from a Pvl container 
   *  
   * This method will return location and size parameters of a PDData object as 
   * specified in a Pvl container object, typically PBData if utilized within 
   * this class 
   *  
   * The PvlContainer must have Name, StartByte and Bytes keywords in order to 
   * satisfy the minimum requirements.  Note that it does not require the name 
   * of the container to be "PBData" as specified in this class.  This 
   * generalizes its use to other implementations that read and write Google 
   * Protocol Buffers. 
   * 
   * @param pvl       Pvl container of required keywords
   * @param name      Returns the value of the Name keyword
   * @param startByte Returns the value of the StartByte keyword
   * @param bytes     Returns the value of the Bytes keyword 
   *  
   * @return bool     Returns true if all keywords are found, false otherwise
   */
  bool GetParameters(const PvlContainer &pvl, std::string &name, BigInt &startByte,
                     BigInt &bytes) const {
    name = "";
    startByte = bytes = 0;
    bool isGood = false;
    if ( pvl.HasKeyword("Name") ) {
      name = (std::string) pvl["Name"];
      if ( pvl.HasKeyword("StartByte") ) {
        startByte = (BigInt) pvl["StartByte"];
        if ( pvl.HasKeyword("Bytes") ) {
          bytes = (BigInt) pvl["Bytes"];
          isGood = true;
        }
      }
    }
    return (isGood);
  }

  /**
   * @brief Writes the contents of a Google Protocol buffer to a stream 
   *  
   * This method writes a Protocol Buffer Message to a stream after 
   * serialization of the contents of the PB object. 
   * 
   * @param pb     Protocol Buffer message to serialize and write
   * @param output Stream to write the data to
   * 
   * @return int Number of bytes written to stream
   */
  int PBWrite(const PB &pb, std::ostream &output) throw (IException &) {
    if ( !pb.SerializeToOstream(&output) ) {
      std::string mess = "Failed to write Protocol Buffer to output stream";
      throw IException(IException::Programmer, mess, _FILEINFO_);
    }
    return (pb.ByteSize());
  }

  /**
   * @brief Write Protocol Buffer Message to file at specified location
   *  
   * This method writes the serialized Protocol Buffer Messsage, pb, to the 
   * stream after seeking to the specified byte postion in the stream.  Note 
   * that startByte is a 1-based designation of the position in the stream, 
   * typically a file. 
   *  
   * Note that some streams cannot be position to absolute locations.  Callers 
   * should use the alternative method that does not specify the starting byte 
   * location and ensure the stream (pointer) is in the proper state. 
   * 
   * @param pb         Protocol Buffer Message to serialize/write
   * @param output     Stream to write PB data to
   * @param startByte  Starting byte of the stream to write data
   * @param name       Name of Data object Pvl descriptor
   * 
   * @return PvlObject Returns the PvlObject describing the location and size of 
   *         the PB message data
   */
  PvlObject PBWrite(const PB &pb, std::ostream &output, const BigInt startByte,
                    const std::string &name = "") throw (IException &) {

    // Position file pointer to specified location in stream to write the data
    output.seekp(std::max(startByte, (BigInt) 0), std::ios::beg);
    if ( !output.good() ) {
      std::ostringstream mess;
      mess << "Failed to postion stream pointer to byte position " 
           << startByte << " for writing";
      throw IException(IException::Programmer, mess.str(), _FILEINFO_);
    }

    // Write the buffer
    PBWrite(pb, output);
    return (DataObject(pb, startByte, name));
  }


  /**
   * @brief Reads a Protocol Buffer message from an input stream
   *  
   * This method reads a serialized Protocol Buffer message from a stream.  The 
   * PB message is assumed to be a PB type protocal buffer.  Errors are likely 
   * if the type of message does not match with PB class. 
   *  
   * Note that the stream is assumed to be properly positioned and will consume 
   * all data in the stream until and EOF is reached. 
   * 
   * @param pb    Protocol Buffer class to read from the stream
   * @param input Input stream to read the PB message from
   * 
   * @return int Returns the size of the resulting PB class data object which is 
   *         the number of bytes read
   */
  int PBRead(PB &pb, std::istream &input) throw (IException &) {
    if ( !pb.ParseFromIstream(&input) ) {
      std::string mess = "Failed to read Protocol Buffer from input stream";
      throw IException(IException::Programmer, mess, _FILEINFO_);
    }   
    return (pb.ByteSize());
  }


  /**
   * @brief Read a Protocol buffer message to specified location 
   *  
   * Thus method reads a Protocol Buffer message at the specified location 
   * in the stream of a specified size.  This method is best applied when 
   * reading a PB from an arbitrary location in the stream and the exact size 
   * of the message is known. 
   * 
   * @author Kris Becker - 4/23/2010
   * 
   * @param pb        Protocol Buffer to read
   * @param input     Input stream to read from 
   * @param startByte Starting location of the PB
   * @param bytes     Size in bytes of the PB to read
   * 
   * @return int Number of bytes read from the stream
   */
  int PBRead(PB &pb, std::istream &input, const size_t startByte, 
             const size_t bytes)  throw (IException &) {

    TNT::Array1D<char> pbData((int) bytes);

    // Position file pointer and read indicated bytes from stream
    input.seekg(std::max(startByte, (size_t) 0), std::ios::beg);
    input.read(static_cast<char *> (&pbData[0]), bytes);
    if ( !input.good() ) {
      std::ostringstream mess;
      mess << "Failed to read " << bytes << " from input stream";
      throw IException(IException::Programmer, mess.str(), _FILEINFO_);
    }


    // Now parse the protocol message from the string
    if ( !pb.ParseFromArray(static_cast<char *> (&pbData[0]), bytes) ) {
      std::string mess = "Failed to parse Protocol Buffer from input string stream";
      throw IException(IException::Programmer, mess, _FILEINFO_);
    }
    return (pb.ByteSize());
  }


  /**
   * @brief Reads a Protocol Buffer message from a stream 
   *  
   * This method is used to read a PB message from a stream given the Pvl 
   * description.  This Pvl description must adhere to the content as defined 
   * in the DataBuffer() method in this class. 
   * 
   * @author Kris Becker - 4/23/2010
   * 
   * @param pb    Protocol buffer to read the contents into
   * @param input Input stream to read PB from 
   * @param pvl   Pvl container of Name, StartByte and Bytes keyword that 
   *              describes the read parameters
   * 
   * @return int  Number of actual bytes read
   */
  int PBRead(PB &pb, std::istream &input, const PvlObject &pvl) 
                                     throw (IException &) {
    std::string name;
    BigInt startByte, bytes;
    bool gottem(false);
    if (iString::Equal(pvl.Name(), DataObjectName())) 
      gottem =  GetParameters(pvl, name, startByte, bytes);
    else {
      gottem = GetParameters(pvl.FindObject(DataObjectName(), PvlObject::Traverse), 
                             name, startByte, bytes);
    }

    if ( !gottem ) {
      std::ostringstream mess;
      mess << "Improperly configured Protocol Buffer object in "
           << pvl.Name() 
           << " PVl container (needs Name, StartByte and Bytes keywords)";
      throw IException(IException::Programmer, mess.str(), _FILEINFO_);
    }
    return (PBRead(pb, input, startByte, bytes));
  }

  private:

};



} // namespace Isis

#endif

