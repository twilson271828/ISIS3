#include "Isis.h"
#include <cstdio>
#include <cmath>
#include <string>

#include "Constants.h"
#include "UserInterface.h"

#include "FileName.h"
#include "FileList.h"
#include "IException.h"
#include "Progress.h"
#include "Pvl.h"
#include "Process.h"
#include "iTime.h"
#include "IString.h"

#include "PixelDB.pb.h"
#include "PixelDBManager.h"

#include "DbPublisher.h"


using namespace std; 
using namespace Isis;
using namespace ::google::protobuf;

void IsisMain ()
{

  const string vimsdbgen_program = "vimsdbgen";
  const string vimsdbgen_version = "2.0";
  const string vimsdbgen_revision = "$Revision$";
  string vimsdbgen_runtime = iTime::CurrentGMT().toStdString();

  // Get input parameters
  UserInterface &ui = Application::GetUserInterface();
  string from  = ((ui.WasEntered("FROM")) ? ui.GetFileName("FROM").toStdString() : "");
  string fromlist = ((ui.WasEntered("FROMLIST")) ? ui.GetFileName("FROMLIST").toStdString() : "");

  string config = ((ui.WasEntered("CONFIG")) ? ui.GetFileName("CONFIG").toStdString()   : "");

  string dbconfig = ((ui.WasEntered("DBCONFIG")) ? ui.GetFileName("DBCONFIG").toStdString() : "");
  string profile = ((ui.WasEntered("PROFILE"))   ? ui.GetString("PROFILE").toStdString()   : "");

  FileList flist;
  if (!from.empty()) flist.push_back(QString(from.c_str()));
  if (!fromlist.empty()) flist.read(FileName(fromlist.c_str()));
  if (flist.size() < 1) {
    string msg = "FROM [" + from + "] and/or FROMLIST [" + fromlist +
    "] does not contain any ISIS cube filenames - need to provide at least one"; 
    throw IException(IException::User,msg,_FILEINFO_);
  }

  // Load/create the PixelDB
  PixelDBManager db;
  db.setConfig(config);

  // Set up database access
  db.setPublisher(new DbPublisher(dbconfig, profile));

  //  Set up generation of pixel database
  Progress progress;
  progress.SetMaximumSteps(flist.size());
  progress.CheckStatus();

  for (int i = 0 ; i < flist.size() ; i++) {
    cout << flist[i].toString() << "\n";
    db.ProcessBySpectra(flist[i].toString().toStdString());
    progress.CheckStatus();
  }

  cout << "\nTotal Processed: " << db.getProcessed() << "\n";
  cout << "Total Valid:     " << db.getTotalValid() << "\n\n";

  //  Output the result
//  if (!dbto.empty()) { db.Write(dbto);  }
  PixelMaker::GEOSFinish();

  //  Report errors
  PvlGroup errlog("PDBErrors");
  db.getErrorSummary(errlog);
  Application::Log(errlog);

  // Provide a detailed error log if requested
  if (ui.WasEntered("ERRLOG")) {
    string eFile = FileName(ui.GetFileName("ERRLOG")).expanded().toStdString();
    ofstream os;
    os.open(eFile.c_str(), ios::out|ios::trunc);
    if (!os) {
      string mess = "Failed to open/create output log file " + eFile;
      IException(IException::User, mess, _FILEINFO_);
    }
    db.dumpErrorLog(os);
    os.close();
  }

  return;
}


