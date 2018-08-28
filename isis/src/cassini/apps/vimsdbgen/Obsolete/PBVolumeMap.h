#ifndef PBVolumeMap_h
#define PBVolumeMap_h
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
#include "PBVolume.pb.h"
#include "PBUtilities.h"


#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>


namespace Isis {


#if defined(NODE_ROTISARY)
template <class N> class PBNodeManager {
  public:
    PBNodeManager() { }

    void Load() { 
      LoadNode(&_node, _nodeFile, startByte, _nbytes);
      return;
    }

    void Load() { 
      LoadNode(&_node, _nodeFile, startByte, _nbytes);
      return;
    }

    void Unload() { 
      UnLoadNode(&_node);
      return;
    }

    N &Data() { 
      Load();
      return (*_node); 
    }

    const N &Data() const {
      Load();
      return (*_node);
    }

  private:
    mutable N   *_node;        // 0 implies its not loaded
    std::string  _nodefile;
    BigInt       _startByte;
    int          _nbytes;


    void LoadNode() const {

    }

    void UnLoadNode(N **node) const {
      if ((*node)) {
        delete *node;
        *node = 0;
      }
      return;
    }

}

#endif

/**
 * @brief Google Protocol Message Volume Mapper container
 *  
 * This class provides control over the size of any Google Protocol Buffer (PB) 
 * that contains a repeated parameter.  There are not too many instances where 
 * this becomes a problem, but if the potential exists for a size run away on a
 * PBI, this class helps manage the size of PB.  
 *  
 * The user of this class must provide the insert method as that will be 
 * dependant on the name of the repeated field.  It is declare as a private 
 * method so that inserts can be better managed through the add(U) method. 
 * Failure (add(U) == false) indicates the node bufer is full.  Outright 
 * failures are indicated through exceptions. 
 *  
 * @author Kris Becker 
 * @internal 
 *   @history 2010-07-29 Kris Becker Original Version 
 * 
 */
template <class NODE>  class PBVolumeMap {
  public:
    enum { MaxNodeSize = 64*1024*1024 };
    typedef typename std::vector<NODE *>::iterator NodeIterator;
    typedef typename std::vector<NODE *>::const_iterator ConstNodeIterator;

    /** Constructor */
    PBVolumeMap() : _maxnodesize(MaxNodeSize), _nodes() { }
    PBVolumeMap(const int maxnodesize) : _maxnodesize(maxnodesize), _nodes() { }
    virtual ~PBVolumeMap() { Destruct(); }

    BigInt size() const {  return (SizeOfVolume()); }
    int nodes() const { return (_nodes.size()); }

    void SetMaxNodeSize(const int &maxsize) {
      _maxnodesize = maxsize;
      return;
    }

    NODE *addNode() {
      _nodes.push_back(new NODE(_maxnodesize));
      return (_nodes.back());
    }

    NODE *last() { 
      if (nodes() <= 0) { return (addNode()); }
      return (_nodes.back()); 
    }
    const NODE *last() const { return (_nodes.back()); }
    NODE *operator()(const int index)             { return (_nodes[index]); }
    const NODE *operator()(const int index) const { return (_nodes[index]); }

    NodeIterator begin() { return (_nodes.begin()); }
    ConstNodeIterator begin() const { return (_nodes.begin()); }

    NodeIterator end() { return (_nodes.end()); }
    ConstNodeIterator end() const { return (_nodes.end()); }

    std::string LabelName() const { 
      return (std::string("ProtocolBufferVolume"));
    }
    PvlObject write(std::ostream &out) const;
    int read(std::istream &inp, PvlObject &pvl);

  private:
    int _maxnodesize;
    std::vector<NODE *> _nodes;

    BigInt SizeOfVolume() const {
      BigInt nbytes(0);
      for (ConstNodeIterator cni = begin()  ; cni != end() ; ++cni) {
        nbytes += (*cni)->size();
      }
      return (nbytes);
    }
    void Destruct() {
      for (unsigned int i = 0 ; i < _nodes.size() ; i++) {
        delete _nodes[i];
      }
      _nodes.clear();
    }
};


template <class NODE>  
  PvlObject PBVolumeMap<NODE>::write(std::ostream &out) const {
    PBVolume volume;
    BigInt totalSize(0);
    PvlKeyword nodesizes("NodeSizes");

    //  Initially, determine complete output size
    for (int i = 0 ; i < nodes() ; i++) {
      const NODE *node = _nodes[i];;
      PBPartition part;
      part.set_pbdescr(node->name());
      part.set_maxsize(_maxnodesize);
      part.set_number(i);
      part.set_nbytes(node->SerializedSize());

      *volume.add_maps() = part;
      totalSize += part.nbytes();
      nodesizes.AddValue(part.nbytes());
    }

    totalSize += volume.ByteSize();

    //  Complete the PVL description of the volume
    PvlObject pvol(LabelName());
    pvol += PvlKeyword("StartByte", (BigInt) out.tellp());
    pvol += PvlKeyword("VolumeMapSize", volume.ByteSize());
    pvol += PvlKeyword("MaximumNodeSize", _maxnodesize);
    pvol += PvlKeyword("Nodes", nodes());
    //pvol += nodesizes;
    pvol += PvlKeyword("VolumeSize", totalSize);

    //  Write the volume map to the output stream.  Note only the size of the
    //  volume map needs to be recorded in the PVL object to get the read
    //  going.
    if ( !volume.SerializeToOstream(&out) ) {
      std::string mess = "Failed to write Protocol Buffer Volume Map to output stream";
      throw IException(IException::Programmer, mess, _FILEINFO_);
    }

    // Now write each node
    for (int i = 0 ; i < nodes() ; i++) {
      const NODE *node = _nodes[i];
      node->write(out);
    }

    return(pvol);
  }

template <class NODE>  
  int PBVolumeMap<NODE>::read(std::istream &inp, PvlObject &pvl) {
    BigInt start = pvl["StartByte"];
    int nbytes = pvl["VolumeMapSize"];

    //  Move the stream to the proper location
    PBIO<PBVolume> vreader;
    PBVolume volume;
    vreader.PBRead(volume, inp, start, nbytes);

    // Clear the current map and reinitialize
    Destruct();
    for (int i = 0 ; i < volume.maps_size() ; i++) {
      _nodes.push_back(new NODE(inp, (int) volume.maps(i).nbytes()));
    }

    return (nodes());
}


} // namespace Isis

#endif

