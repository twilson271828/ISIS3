/**                                                                       
 * @file                                                                  
 * $Revision: 1.5 $
 * $Date: 2007/11/13 20:54:42 $
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

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <QDebug>
#include <QString>

using namespace std;

#include "DatabaseFactory.h"
#include "DbProfile.h"
#include "DbPublisher.h"
#include "SqlQuery.h"
#include "IString.h"
#include "IException.h"

namespace Isis {

DbPublisher::DbPublisher() : m_name(), m_null("NULL"), 
                             m_db(), m_status(-1), m_lastError("") { }

DbPublisher::DbPublisher(const std::string &config,
                         const std::string &profile) : m_name(),
                            m_null("NULL"), m_db(), m_status(-1), 
                            m_lastError("") {
  m_db = QSharedPointer<Database> (initializeDB(config, profile));
}


/** Writes to entire cube data for the vimspixel view*/
int DbPublisher::publish(const File &vfile, const DbProfile &keys) {
  return (insert(vfile, keys));
}

/** Writes to database the data for the datafile table content */
int DbPublisher::insert(const File &vfile, const DbProfile &keys) {
  IString sql;

  sql += sqlQuote(vfile.name());
  sql += sqlQuote(vfile.serialnumber());
  sql += sqlQuote(getValidValue("MissionPhaseName", keys));
  sql += sqlQuote(getValidValue("SequenceId", keys));
  sql += sqlQuote(getValidValue("SequenceTitle", keys));
  sql += sqlQuote(getValidValue("ObservationId", keys));
  sql += sqlQuote(getValidValue("InstrumentId", keys));
  sql += sqlQuote(getValidValue("ProductId", keys));
  sql += sqlQuote(getValidValue("Channel", keys));
  sql += sqlQuote(IString(vfile.lines()));
  sql += sqlQuote(IString(vfile.samples()));
  sql += sqlQuote(IString(vfile.bands()));
  sql += sqlQuote(getValidValue("SpacecraftClockStartCount", keys));
  sql += sqlQuote(getValidValue("SpacecraftClockStopCount", keys));
  sql += sqlQuote(getValidValue("NativeStartTime", keys));
  sql += sqlQuote(getValidValue("NativeStopTime", keys));
  sql += sqlQuote(getValidValue("UTCStartTime", keys));
  sql += sqlQuote(getValidValue("UTCStopTime", keys));
  sql += sqlQuote(getValidValue("TargetName", keys));
  sql += sqlQuote(getValidValue("InstrumentModeId", keys));
  sql += sqlQuote(getValidValue("GainMode", keys));
  sql += sqlQuote(getValidValue("CompressorId", keys));
  sql += sqlQuote(getValidValue("InterlineDelayDuration", keys));
  sql += sqlQuote(getValidValue("ExposureDuration", keys));
  sql += sqlQuote(getValidValue("Xoffset", keys));
  sql += sqlQuote(getValidValue("Zoffset", keys));
  sql += sqlQuote(getValidValue("SwathWidth", keys));
  sql += sqlQuote(getValidValue("SwathLength", keys));
  sql += sqlQuote(getValidValue("PowerStateFlag", keys));
  sql += sqlQuote(getValidValue("SpectralEditingFlag", keys));
  sql += sqlQuote(getValidValue("SpectralSummingFlag", keys));
  sql += sqlQuote(getValidValue("StarTracking", keys));
  sql += sqlQuote(getValidValue("SnapshotMode", keys));
  sql += sqlQuote(vfile.geom().thegeom(), "");

  sql = "SELECT merge_file(" + sql + ")";


  executeSql(sql);

  //  Now process each pixel
  int ngood = 0;
  for (int i = 0 ; i < vfile.pixels_size() ; i++) {
    ngood += insert(vfile, vfile.pixels(i));
  }

  return (ngood);
}

int DbPublisher::insert(const File &vfile, const Pixel &pixel) {
  IString sql;



  sql += sqlQuote(vfile.name());
  sql += sqlQuote(vfile.serialnumber());
  sql += sqlQuote(IString(pixel.line()));
  sql += sqlQuote(IString(pixel.sample()));
  sql += sqlQuote(IString(pixel.ettime()));
  sql += sqlQuote(IString(pixel.points().center().latitude()));
  sql += sqlQuote(IString(pixel.points().center().longitude()));
  sql += sqlQuote(IString(pixel.points().center().phase()));
  sql += sqlQuote(IString(pixel.points().center().emission()));
  sql += sqlQuote(IString(pixel.points().center().incidence()));

  sql += sqlQuote(IString(pixel.sampres()));
  sql += sqlQuote(IString(pixel.lineres()));

  sql += sqlQuote(IString(pixel.points().center().distance()));

  sql += sqlQuote(IString(pixel.points().center().locale().scpos().x()));
  sql += sqlQuote(IString(pixel.points().center().locale().scpos().y()));
  sql += sqlQuote(IString(pixel.points().center().locale().scpos().z()));

  sql += sqlQuote(IString(pixel.points().center().locale().sunpos().x()));
  sql += sqlQuote(IString(pixel.points().center().locale().sunpos().y()));
  sql += sqlQuote(IString(pixel.points().center().locale().sunpos().z()));

  IString bands("'{");
  for (int i = 0 ; i < pixel.spectrum_size() ; i++) {
    bands += sqlNoQuote(IString(pixel.spectrum(i), 6));
  }

  // Replace the last comma with the array terminator
  bands.replace(bands.end()-1, bands.end(), "}',");
  sql += bands;

  // Finally add the pixel Footprint
  sql += sqlQuote(pixel.footprint(), "");

  sql = "SELECT merge_pixel(" + sql + ")";
  bool good = executeSql(sql);

  //  Now add each corner point
  insert(vfile, pixel, "UPPERLEFT",  pixel.points().upperleft());
  insert(vfile, pixel, "UPPERRIGHT", pixel.points().upperright());
  insert(vfile, pixel, "LOWERRIGHT", pixel.points().lowerright());
  insert(vfile, pixel, "LOWERLEFT",  pixel.points().lowerleft());

  return ((good) ? 1 : 0);
}

int DbPublisher::insert(const File &vfile, const Pixel &pixel, 
                        const std::string &position, const Geometry &point) { 
  IString sql;

  sql += sqlQuote(vfile.name());
  sql += sqlQuote(vfile.serialnumber());

  sql += sqlQuote(IString((int) pixel.line()));
  sql += sqlQuote(IString((int) pixel.sample()));

  sql += sqlQuote(position);

  sql += sqlQuote(IString(point.line()));
  sql += sqlQuote(IString(point.sample()));

  sql += sqlQuote(IString(point.latitude()));
  sql += sqlQuote(IString(point.longitude()));
  sql += sqlQuote(IString(point.phase()));
  sql += sqlQuote(IString(point.emission()));
  sql += sqlQuote(IString(point.incidence()));

  sql += sqlQuote(IString(point.resolution()));

  sql += sqlQuote(IString(point.locale().scpos().x()));
  sql += sqlQuote(IString(point.locale().scpos().y()));
  sql += sqlQuote(IString(point.locale().scpos().z()));

  sql += sqlQuote(IString(point.locale().sunpos().x()));
  sql += sqlQuote(IString(point.locale().sunpos().y()));
  sql += sqlQuote(IString(point.locale().sunpos().z()), "");

  sql = "SELECT merge_point(" + sql + ")";
  executeSql(sql);

  return (0);
}

Database *DbPublisher::initializeDB(const std::string &config, 
                                    const std::string &profile) {

#if defined(DEBUG)
  DatabaseFactory *factory = DatabaseFactory::getInstance();

  vector<string> drivers = factory->available();
  cout << "--- Database Drivers Available ---\n";
  copy(drivers.begin(), drivers.end(), ostream_iterator<std::string>(cout, "\n"));
#endif
#if 1
  Database::addAccessConfig(QString(config.c_str()) );
  DbProfile dbprof = Database::getProfile(QString(profile.c_str() ) );

  return (new Database(dbprof,Database::Connect)); 
#else 
  return (0);
#endif
}


std::string DbPublisher::sqlNoQuote(const std::string &value, 
                                  const std::string &separator) const {

  //  Check for empty value - needs a NULL
  string dbvalue(value);
  if (dbvalue.empty()) dbvalue = null();

  // If its NULL or inserting a geometry text value just return it unmodified
  if (IString::Equal(dbvalue, "NULL")) return (dbvalue+separator);
  if (IString::DownCase(dbvalue).find("st_geomfromtext") != string::npos) 
       return (dbvalue+separator);

  //  Must change single quotes to two quotes
  for (string::iterator quote = dbvalue.begin() ; quote != dbvalue.end() ; 
        ++quote) {
    if (*quote == '\'') {
      quote = dbvalue.insert(quote+1,'\'');
    }
  }

  return (string(dbvalue + separator));
}

std::string DbPublisher::sqlQuote(const std::string &value, 
                                  const std::string &separator) const {

  //  Check for empty value - needs a NULL
  string dbvalue(value);
  if (dbvalue.empty()) dbvalue = null();

  // If its NULL or inserting a geometry text value just return it unmodified
  if (IString::Equal(dbvalue, "NULL")) return (dbvalue+separator);
  if (IString::DownCase(dbvalue).find("st_geomfromtext") != string::npos) 
       return (dbvalue+separator);

  //  Must change single quotes to two quotes
  for (string::iterator quote = dbvalue.begin() ; quote != dbvalue.end() ; 
        ++quote) {
    if (*quote == '\'') {
      quote = dbvalue.insert(quote+1,'\'');
    }
  }

  return (string("'" + dbvalue + "'" + separator));
}

std::string DbPublisher::sqlQuoteArray(const std::string &value,
                                       const std::string &separator) const {
  return (string("{" + value + "}"));
}


std::string DbPublisher::getValidValue(const std::string &name,
                                       const DbProfile &keys, 
                                       const std::string &defval) const {
  QString value(defval.c_str());

  // Check for existance - if not return default
  if (keys.exists(QString(name.c_str()))) {
    if (keys.count(QString(name.c_str())) > 0) {
      value = keys.value(QString(name.c_str()) );
    }
  }

  return (value.toStdString() );
}

bool DbPublisher::executeSql(const std::string &statement,
                             const bool &useTransactions)  {


  m_status = -1;
  m_lastError = "";
  if (!m_db.isNull()) {
    //  Set up a transaction for error recovery
    if (useTransactions) m_db->transaction();

    //  Set up the inserter query
    SqlQuery inserter(*m_db);
    inserter.setThrowOnFailure();

    // Execute
    try {
      inserter.exec(statement);
      if (useTransactions) m_db->commit();   // Commit the update
      m_status = 0;
    } 
    catch (IException &ie) {
      //  Failed, rollback and report
      if (useTransactions) m_db->rollback();
      m_status = 1;
      m_lastError = ie.toString().toStdString();
    }
  }
  else {
    cout << "SQL: " << statement << "\n";
  }

  return (m_status == 0);
}

} // namespace Isis
