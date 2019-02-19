#include "Isis.h"

#include <iostream> 
#include <vector>
#include <string>
#include <QString>

#include <sstream>
#include <fstream>
#include <iomanip> 

#include <algorithm>
#include <memory>

using namespace std;

//#include "UserInterface.h"
#include "SessionLog.h"
#include "IString.h"
#include "iTime.h"
#include "FileName.h"
#include "DbAccess.h"
#include "Database.h"
#include "DatabaseFactory.h"
#include "SqlQuery.h"
#include "SqlRecord.h"

using namespace Isis;

void IsisMain() {


  UserInterface &ui = Application::GetUserInterface();
  QString version("$Revision: 1.6 $");

  QString pvlSource = ui.GetFileName("FROM");

  QString dbMap("");
  if (ui.WasEntered("DBMAP")) {
    dbMap = ui.GetFileName("DBMAP");
  }

  QString dbConf("");
  if (ui.WasEntered("DBCONFIG")) dbConf = ui.GetFileName("DBCONFIG");

  QString dbProf("");
  if (ui.WasEntered("PROFILE")) dbProf = ui.GetString("PROFILE");

  QString dbTable("");
  if (ui.WasEntered("TABLES")) dbTable = ui.GetString("TABLES");

  QString dbMissing("EXCLUDE");
  dbMissing = ui.GetString("MISSING");

  QString dbMode("COMMIT");
  dbMode = ui.GetString("MODE");
  //bool commit = IString::Equal(dbMode, "COMMIT");
  bool commit = (dbMode=="COMMIT");

  QString dbVerbose("NO");
  dbVerbose = ui.GetString("VERBOSE");
  //bool verbose = IString::Equal(dbVerbose, "YES");
  bool verbose = (dbVerbose=="YES");

  QString dbShowSql("NO");
  dbShowSql = ui.GetString("SHOWSQL");
  //bool showsql = IString::Equal(dbShowSql, "YES");
  bool showsql = (dbShowSql=="YES");

  // If we are not commiting or want'n to see the SQL, turn verbose on
  if (!commit) verbose = true;
  if (showsql) verbose = true;



  DatabaseFactory *factory = DatabaseFactory::getInstance();
  vector<QString> drivers = factory->available();

  vector<string> driversStdStringRepresentation;

  for (auto qstr:  drivers) {

    driversStdStringRepresentation.push_back(qstr.toStdString() );

  }

  cout << "--- Database Drivers Available ---\n";
  copy(driversStdStringRepresentation.begin(), driversStdStringRepresentation.end(), ostream_iterator<string>(cout, "\n"));

  Database::addAccessConfig("vimsdb.prof");
  DbProfile profile = Database::getProfile(dbProf);

  //auto_ptr<Database> db = auto_ptr<Database> (new Database(profile,Database::Connect));
  unique_ptr<Database> db (new Database(profile,Database::Connect));
  SqlQuery query("SELECT * FROM pg_tables where schemaname = 'public'", *db.get());
  cout << "Number of rows returned: " << query.size() << "\n";
  while (query.next()) {
    SqlRecord record = query.getRecord();
    string comma("");
    for (int i = 0 ; i < record.size() ; i++) {
      cout << comma << record.getValue(i);
      comma = ",";
    }
    cout << "\n";
  }




  try {

//  First thing to do is add the access profile if provided by user
    if (!dbConf.isEmpty()) {
      if (!Database::addAccessConfig(dbConf)) {
        QString mess = "Unable to open/add DBCONF file " + dbConf;
        throw IException(IException::User, mess, _FILEINFO_);
      }
    }
// Get the default profile.
    DbProfile profile = Database::getProfile(dbProf);

// Instantiate the map object
    DbPvlMap pvlmap;

//  Add program and profile variables
    pvlmap.addVariable("PROGRAM", "pvltodb");
    pvlmap.addVariable("VERSION", version);
    pvlmap.addVariable("RUNDATE", iTime::CurrentLocalTime());
    pvlmap.addVariable("MISSING", dbMissing);
    pvlmap.addVariable("VERBOSE", dbVerbose);
    pvlmap.addVariable("SHOWSQL", dbShowSql);
    pvlmap.addVariable(profile);

//  Add PVL input source file (FROM)
    pvlmap.setPvlSource(pvlSource);
    pvlmap.addVariable("FROM", pvlSource);

// Determine the map file and add it
    if (dbMap.isEmpty()) {
      try {
        dbMap = profile("DBMAP");
      } catch (iException &ie) {
        ostringstream mess;
        mess << "DBMAP file not provided by user and not found in DBCONF ("
             << dbConf << ") file profile (" << profile.Name() << ") either."
             << endl;
        ie.Message(iException::User, mess.str(), _FILEINFO_);
        throw;
      }
    }

// Add the map file
    pvlmap.setDbMap(dbMap);
    pvlmap.addVariable("DBMAP", dbMap);

//  Insert map into database
    DbResult result;
    if (commit) {
      // This implementation is necessary for persistant connects when using
      // more than one table.  Pointers are required so the database connection
      // can be removed for more than one run (in GUI mode) as Qt will complain.
      Database *db = new Database(profile,Database::Connect);
      string dbName = db->Name();
      db->makePersistant();
      if (!dbTable.empty()) {
        result = pvlmap.Insert(dbTable, *db);
      } 
      else {
        result = pvlmap.Insert(*db);
      }
      delete db;
      Database::remove(dbName);
    }
    else {
      if (!dbTable.empty()) {
        result = pvlmap.Insert(dbTable);
      } 
      else {
        result = pvlmap.Insert();
      }
    }

// Report processing if requested
    int nerrors(0);
    for (int i = 0 ; i < result.size() ; i++) {
      const DbStatus &status = result.get(i);
      PvlGroup pResult("Result" + iString(i));
      pResult += PvlKeyword("Table", status.table);
      pResult += PvlKeyword("Columns", status.columns);
      pResult += PvlKeyword("Operation", status.operation);
      if (showsql) pResult += PvlKeyword("Query", status.sql);
      pResult += PvlKeyword("RowsAffected", status.rowsAffected);
      pResult += PvlKeyword("Status", status.status);
      pResult += PvlKeyword("Error", ((status.lastError.empty()) ? "None" : 
                                      status.lastError));
      if (verbose) {
        Application::Log(pResult);
      }
      else {
        SessionLog::TheLog().AddResults(pResult);
        SessionLog::TheLog().Write();
      }

      if (status.status > 0) {
        nerrors++;
        iException::Message(iException::Programmer, status.lastError, 
                            _FILEINFO_);
      }
    }

    if (nerrors > 0) {
      throw iException::Message(iException::Programmer,
                                "Database operations has errors",
                                 _FILEINFO_);
    }

  }
  catch (iException &ie) {
    ie.Message(iException::Programmer, "Error encountered in pvltodb processing",
               _FILEINFO_);
    throw;
  }
  catch (...) {
    // This is really unexpected
    cerr << "Unhandled exception" << endl;
    throw iException::Message(iException::Programmer, "Unhandled exception",
                               _FILEINFO_);
  }

  return;
}
