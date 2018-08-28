#ifndef PixelDBQuery_h
#define PixelDBQuery_h
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
#include <vector>
#include "PvlObject.h"
#include "iException.h"

#include "PixelDB.pb.h"

namespace Isis {


class PixelDBQuery {
  public:
    virtual ~PixelDBQuery() { }

    virtual PixelDBQuery &Visit(PixelDBQuery &query) { return (query); }
    virtual const PixelDBQuery &Visit(const PixelDBQuery &query) const { return (query); }

    virtual void Execute(PixelDB &db) {
      FileIterator(db);
      return;
    }

    virtual void Execute(const PixelDB &db) const {
      FileIterator(db);
      return;
    }

    virtual void Execute(File &vfile) {
      PixelIterator(vfile);
      return;
    }

    virtual void Execute(const File &vfile) const {
      PixelIterator(vfile);
      return;
    }

    virtual void Execute(Pixel &pixel, File &vfile) {
      return;
    }

    virtual void Execute(const Pixel &pixel, const File &vfile)  const {
      return;
    }

  protected:
    PixelDBQuery() { }

    void FileIterator(PixelDB &db) {
      for (int i = 0 ; i < db.files_size() ; i++) {
        Execute(*db.mutable_files(i));
      }
      return;
    }

    void FileIterator(const PixelDB &db) const {
      for (int i = 0 ; i < db.files_size() ; i++) {
        Execute(db.files(i));
      }
      return;
    }

    void PixelIterator(File &vfile) {
      for (int i = 0 ; i < vfile.pixels_size() ; i++) {
        Execute(*vfile.mutable_pixels(i), vfile);
      }
      return;
    }

    void PixelIterator(const File &vfile) const {
      for (int i = 0 ; i < vfile.pixels_size() ; i++) {
        Execute(vfile.pixels(i), vfile);
      }
      return;
    }

    virtual void throwError(const std::string &source, const char *f, int l) 
                            const { 
      std::string mess = "Query function for " + source + " not implemented!";
      throw IException(IException::Programmer, mess, f, l);
      return;
    }

  private:
};


/** This class is a simple file and pixel counter */
class VQCounter : public PixelDBQuery {
  public:
    VQCounter() : PixelDBQuery(), _nFiles(0), _nPixels(0) { }
    virtual ~VQCounter() { }
    virtual PixelDBQuery &Visit(PixelDBQuery &query) { return (query); }
    int Files() const { return (_nFiles); }
    int Pixels() const { return (_nPixels); }

    void Clear() { _nFiles = _nPixels = 0; }
    void Execute(File &vfile) {
      _nFiles++;
      PixelIterator(vfile);
      return;
    }

    void Execute(const File &vfile) const {
      _nFiles++;
      PixelIterator(vfile);
      return;
    }

    void Execute(Pixel &pixel, File &vfile) { _nPixels++; }
    void Execute(const Pixel &pixel, const File &vfile)  const { _nPixels++; }

  private:
    mutable int _nFiles;
    mutable int _nPixels;
};

/** This class is a IsisId finder */
class VQFindFileByIsisId : public PixelDBQuery {
  public:
    VQFindFileByIsisId() : PixelDBQuery(), _isisid(""), _files() { }
    VQFindFileByIsisId(const std::string &id, PixelDB &pdb) : PixelDBQuery(),
                       _isisid(id), _files() {
      FileIterator(pdb);
    } 
    virtual ~VQFindFileByIsisId() { }
    virtual PixelDBQuery &Visit(PixelDBQuery &query) { return (query); }

    void find(const std::string &id) { _isisid = id; }
    std::string id() const { return (_isisid); }
    int size() const { return (_files.size()); }

    File *getFile(const int &index = 0) const {
      if ((index < 0) || (index >= size())) return (0);
      return (_files[index]); 
    }

    void Execute(File &vfile) {
      if (vfile.has_serialnumber()) {
        if (vfile.serialnumber() == _isisid) {
          _files.push_back(&vfile);
        }
      }
      return;
    }

    void Clear() { _files.clear(); }

  private:
    std::string _isisid;
    std::vector<File *> _files;
};

} // Isis namespace

#endif
