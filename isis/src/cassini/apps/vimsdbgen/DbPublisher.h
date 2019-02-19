#if !defined(DbPublisher_h)
#define DbPublisher_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.1 $
 * $Date: 2007/03/30 17:19:48 $
 * 
 *   Unless noted otherwise, the portions of Isis written by the USGS are 
 *   public domain. See individual third-party library and package descriptions 
 *   for intellectual property information, user agreements, and related  
 *   information.                                                         
 *                                                                        
 *   Although Isis has been used by the USGS, no warranty, expressed or   
 *   implied, is made by the USGS as to the accuracy and functioning of such 
 *   software and related material nor shall the fact of distribution     
 *   constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.                                        
 *                                                                        
 *   For additional information, launch                                   
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html                
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */                                                                       

#include <string>
#include <iostream>
#include <stack>
#include <functional>
  
#include <QSharedPointer>  

#include "Database.h"
#include "Publisher.h"
#include "PixelDB.pb.h"

namespace Isis {

class DbProfile;

class DbPublisher : public Publisher {
  public:

    DbPublisher();
    DbPublisher(const std::string &config, const std::string &profile = "");
    virtual ~DbPublisher() {}

    std::string getName() const { return (m_name); }

    void setNull(const std::string &nullv = "NULL") { m_null = nullv; }
    std::string null() const { return (m_null); }

    int getStatus() const { return (m_status); }
    std::string getLastError() const { return (m_lastError); }

    virtual int publish(const File &vfile, const DbProfile &keys);

  private:
    std::string                 m_name;
    std::string                 m_null;
    QSharedPointer<Database>    m_db;
    int                         m_status;
    std::string                 m_lastError;


    Database *initializeDB(const std::string &config,
                           const std::string &profile = "");

    std::string sqlNoQuote(const std::string &value, 
                         const std::string &separator = ",")  const;
    std::string sqlQuote(const std::string &value, 
                         const std::string &separator = ",")  const;
    std::string sqlQuoteArray(const std::string &value, 
                         const std::string &separator = ",") const;
    std::string getValidValue(const std::string &name, const DbProfile &keys, 
                             const std::string &defval = "NULL") const;

    int insert(const File &vfile, const DbProfile &keys);
    int insert(const File &vfile, const Pixel &pixel);
    int insert(const File &vfile, const Pixel &pixel, 
               const std::string &position, const Geometry &point);

    bool executeSql(const std::string &statement,
                    const bool &useTransactions = true);
};

}  // namespace Isis
#endif

