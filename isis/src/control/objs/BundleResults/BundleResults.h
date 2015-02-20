#ifndef BundleResults_h
#define BundleResults_h

/**
 * @file
 * $Revision: 1.20 $
 * $Date: 2009/10/15 01:35:17 $
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

#include <QList>
#include <QPair>
#include <QObject>
#include <QString>
#include <QVector>

#include <boost/numeric/ublas/matrix_sparse.hpp>

#include <H5Cpp.h>
#include <hdf5_hl.h>
#include <hdf5.h>

#include "BundleSettings.h"
#include "Distance.h"
#include "MaximumLikelihoodWFunctions.h"
#include "PvlObject.h"
#include "Statistics.h" // ???
#include "XmlStackedHandler.h"

class QDataStream;
class QUuid;
class QXmlStreamWriter;

namespace Isis {
  class ControlNet;
  class CorrelationMatrix;
  class FileName;
  class Project;// ??? does xml stuff need project???
  class PvlObject;
  class SerialNumberList;
  class StatCumProbDistDynCalc;
  class XmlStackedHandlerReader;

  /**
   * A container class for statistical results from a BundleAdjust solution. 
   *  
   * @author 2014-07-01 Jeannie Backer
   *
   * @internal
   *   @history 2014-07-01 Jeannie Backer - Original version
   *   @history 2014-07-14 Kimberly Oyama - Added support for correlation matrix.
   *   @history 2014-07-16 Jeannie Backer - Changed pvlGroup() to pvlObject()
   *   @history 2014-07-23 Jeannie Backer - Added QDataStream operators (<< and >>) and read/write
   *                           methods. Initialize m_cumProRes in the constructor since this
   *                           variable is used regardless of whether maximum likelihood estimation
   *                           is used.
   */
  class BundleResults : public QObject {
    Q_OBJECT
    public:
      BundleResults(QObject *parent = 0);
      BundleResults(Project *project, XmlStackedHandlerReader *xmlReader, QObject *parent = 0);   // TODO: does xml stuff need project???
      BundleResults(const BundleResults &src);
      ~BundleResults();
      BundleResults &operator=(const BundleResults &src);
      void initialize();

      // mutators and computation methods
      void resizeSigmaStatisticsVectors(int numberImages);
      void setRmsImageResidualLists(QList<Statistics> rmsImageLineResiduals,
                                    QList<Statistics> rmsImageSampleResiduals,
                                    QList<Statistics> rmsImageResiduals);
      void setRmsImageResidualLists(QVector<Statistics> rmsImageLineResiduals,
                                    QVector<Statistics> rmsImageSampleResiduals,
                                    QVector<Statistics> rmsImageResiduals);
      void setSigmaLatitudeRange(Distance minLatDist, Distance maxLatDist,
                                 QString minLatPointId, QString maxLatPointId);
      void setSigmaLongitudeRange(Distance minLonDist, Distance maxLonDist,
                                  QString minLonPointId, QString maxLonPointId);
      void setSigmaRadiusRange(Distance minRadDist, Distance maxRadDist,
                               QString minRadPointId, QString maxRadPointId);
      void setRmsFromSigmaStatistics(double rmsFromSigmaLatStats,
                                     double rmsFromSigmaLonStats,
                                     double rmsFromSigmaRadStats);
      void maximumLikelihoodSetUp(
          QList< QPair< MaximumLikelihoodWFunctions::Model, double > > modelsWithQuantiles);
      void printMaximumLikelihoodTierInformation();
      void initializeResidualsProbabilityDistribution(unsigned int nodes = 20);
      void initializeProbabilityDistribution(unsigned int nodes = 20);
      void addResidualsProbabilityDistributionObservation(double obsValue);
      void addProbabilityDistributionObservation(double obsValue);
      void addProbabilityDistributionObservation(double obsValue, bool residuals);
      void incrementMaximumLikelihoodModelIndex();

      void incrementFixedPoints();
      int numberFixedPoints() const;
      void incrementHeldImages();
      int numberHeldImages() const;
      void incrementIgnoredPoints();
      int numberIgnoredPoints() const; // currently unused ???
#if 0
      double computeRejectionLimit(ControlNet *p_Cnet,
                                   double outlierRejectionMultiplier,
                                   int numObservations);
#endif
      void setRejectionLimit(double rejectionLimit);
#if 0
      double computeResiduals(ControlNet *m_pCnet,
                              std::vector< boost::numeric::ublas::bounded_vector< double, 3 > > pointWeights,
                              std::vector< boost::numeric::ublas::bounded_vector< double, 3 > > pointCorrections,
                              boost::numeric::ublas::vector< double > image_Corrections,
                              std::vector< double > imageParameterWeights,
                              int numImagePartials,
                              int rank);
#endif
      void setRmsXYResiduals(double rx, double ry, double rxy);
#if 0
      bool flagOutliers(ControlNet *m_pCnet);
#endif
      void setNumberRejectedObservations(int numberObservations);
      void setNumberObservations(int numberObservations);
      void setNumberImageParameters(int numberParameters); // ??? this is the same value an m_nRank
      void resetNumberConstrainedPointParameters();
      void incrementNumberConstrainedPointParameters(int incrementAmount);
      void resetNumberConstrainedImageParameters();
      void incrementNumberConstrainedImageParameters(int incrementAmount);
      void setNumberUnknownParameters(int numberParameters);
      void computeDegreesOfFreedom();
      void computeSigma0(double dvtpv, BundleSettings::ConvergenceCriteria criteria);
      void setDegreesOfFreedom(double degreesOfFreedom);
      void setSigma0(double sigma0);
      void setElapsedTime(double time);
      void setElapsedTimeErrorProp(double time);
      void setConverged(bool converged); // or initialze method
      
      // Accessors...
      QList<Statistics> rmsImageSampleResiduals() const;
      QList<Statistics> rmsImageLineResiduals() const;
      QList<Statistics> rmsImageResiduals() const;
      QVector<Statistics> rmsImageXSigmas() const;       // currently unused ???
      QVector<Statistics> rmsImageYSigmas() const;       // currently unused ???
      QVector<Statistics> rmsImageZSigmas() const;       // currently unused ???
      QVector<Statistics> rmsImageRASigmas() const;      // currently unused ???
      QVector<Statistics> rmsImageDECSigmas() const;     // currently unused ???
      QVector<Statistics> rmsImageTWISTSigmas() const;   // currently unused ???
      Distance minSigmaLatitudeDistance() const;
      Distance maxSigmaLatitudeDistance() const;
      Distance minSigmaLongitudeDistance() const;
      Distance maxSigmaLongitudeDistance() const;
      Distance minSigmaRadiusDistance() const;
      Distance maxSigmaRadiusDistance() const;
      QString maxSigmaLatitudePointId() const;
      QString minSigmaLatitudePointId() const;
      QString minSigmaLongitudePointId() const;
      QString maxSigmaLongitudePointId() const;
      QString minSigmaRadiusPointId() const;
      QString maxSigmaRadiusPointId() const;
      double sigmaLatitudeStatisticsRms() const;
      double sigmaLongitudeStatisticsRms() const;
      double sigmaRadiusStatisticsRms() const;
      double rmsRx() const;  // currently unused ???
      double rmsRy() const;  // currently unused ???
      double rmsRxy() const; // currently unused ???
      double rejectionLimit() const;
      int numberRejectedObservations() const;
      int numberObservations() const;

      int numberImageParameters() const; // ??? this is the same value an m_nRank
      int numberConstrainedPointParameters() const;
      int numberConstrainedImageParameters() const;
      int numberUnknownParameters() const;
      int degreesOfFreedom() const;
      double sigma0() const;
      double elapsedTime() const;
      double elapsedTimeErrorProp() const;
      bool converged() const; // or initialze method
      
      int numberMaximumLikelihoodModels() const;
      int maximumLikelihoodModelIndex() const;
      StatCumProbDistDynCalc cumulativeProbabilityDistribution() const;
      StatCumProbDistDynCalc residualsCumulativeProbabilityDistribution() const;
      double maximumLikelihoodMedianR2Residuals() const;
      MaximumLikelihoodWFunctions maximumLikelihoodModelWFunc(int modelIndex) const;
      double maximumLikelihoodModelQuantile(int modelIndex) const;

      QList< QPair< MaximumLikelihoodWFunctions, double > > maximumLikelihoodModels() const;

      bool setNumberHeldImages(SerialNumberList m_pHeldSnList,
                               SerialNumberList *m_pSnList);

      PvlObject pvlObject(QString name = "BundleResults") const;

      // Correlation Matrix accessors for cnetsuite and mutators for bundle adjust.
      CorrelationMatrix correlationMatrix() const;
      void setCorrMatCovFileName(FileName name);
      void setCorrMatImgsAndParams(QMap<QString, QStringList> imgsAndParams);

      void save(QXmlStreamWriter &stream, const Project *project) const;   // TODO: does xml stuff need project???
      
      QDataStream &write(QDataStream &stream) const;
      QDataStream &read(QDataStream &stream);

      void savehdf5(hid_t fileId, H5::Group settingsGroup) const;

    private:
      // Used for hdf5 method...
      // Struct that is used to pack the sigma table
      typedef struct sigma {
          char    type[64];
          char    pid[64];
          float   value;
      } m_sigmaTable;

      /**
       *
       * @author 2014-07-28 Jeannie Backer
       *
       * @internal
       */
      class XmlHandler : public XmlStackedHandler {
        public:
          XmlHandler(BundleResults *statistics, Project *project);   // TODO: does xml stuff need project???
          ~XmlHandler();
   
          virtual bool startElement(const QString &namespaceURI, const QString &localName,
                                    const QString &qName, const QXmlAttributes &atts);
          virtual bool characters(const QString &ch);
          virtual bool endElement(const QString &namespaceURI, const QString &localName,
                                    const QString &qName);
   
        private:
          Q_DISABLE_COPY(XmlHandler);
   
          BundleResults *m_xmlHandlerBundleResults;
          Project *m_xmlHandlerProject;   // TODO: does xml stuff need project???
          QString m_xmlHandlerCharacters;
          int m_xmlHandlerResidualsListSize;
          int m_xmlHandlerSampleResidualsListSize;
          int m_xmlHandlerLineResidualsListSize;
          int m_xmlHandlerXSigmasListSize;
          int m_xmlHandlerYSigmasListSize;
          int m_xmlHandlerZSigmasListSize;
          int m_xmlHandlerRASigmasListSize;
          int m_xmlHandlerDECSigmasListSize;
          int m_xmlHandlerTWISTSigmasListSize;
          QList<Statistics *> m_xmlHandlerStatisticsList;
          StatCumProbDistDynCalc *m_xmlHandlerCumProCalc;

          QString m_xmlHandlerCorrelationImageId;
          QStringList m_xmlHandlerCorrelationParameterList;
          QMap< QString, QStringList> m_xmlHandlerCorrelationMap;
      };

      /**
       * A unique ID for this object (useful for others to reference this object
       *   when saving to disk).
       */
      QUuid *m_id;
//???      QString m_instrumentId;               //!< The spacecraft instrument id for this observation.
      CorrelationMatrix *m_correlationMatrix;

// ???       Statistics m_statsx;                       //!<  x errors
// ???       Statistics m_statsy;                       //!<  y errors
// ???       Statistics m_statsrx;                      //!<  x residuals
// ???       Statistics m_statsry;                      //!<  y residuals
// ???       Statistics m_statsrxy;                     //!< xy residuals

      int m_numberFixedPoints;                //!< number of 'fixed' (ground) points (define)
      int m_numberIgnoredPoints;              //!< number of ignored points                  // currently set but unused ???
      int m_numberHeldImages;                 //!< number of 'held' images (define)          
      
      
      
            
      double m_rms_rx;  // set but unused ???                   //!< rms of x residuals
      double m_rms_ry;  // set but unused ???                   //!< rms of y residuals
      double m_rms_rxy; // set but unused ???                   //!< rms of all x and y residuals
      double m_rejectionLimit;                //!< current rejection limit
      // TODO:??? reorder read/write data stream, init, copy constructor, operator= 
      int m_numberObservations;               //!< number of image coordinate observations
      int m_numberRejectedObservations;       //!< number of rejected image coordinate observations
      int m_numberUnknownParameters;          //!< total number of parameters to solve for
      int m_numberImageParameters;            //!< number of image parameters
      int m_numberConstrainedImageParameters; //!< number of constrained image parameters
      int m_numberConstrainedPointParameters; //!< number of constrained point parameters
      int m_degreesOfFreedom;           //!< degrees of freedom
      double m_sigma0;                     //!< std deviation of unit weight
      double m_elapsedTime;                //!< elapsed time for bundle
      double m_elapsedTimeErrorProp;       //!< elapsed time for error propagation
      bool m_converged;


      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // variables set in computeBundleResults()
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      QList<Statistics> m_rmsImageSampleResiduals;// QList??? jigsaw apptest gives - ASSERT failure in QList<T>::operator[]: "index out of range",
      QList<Statistics> m_rmsImageLineResiduals;
      QList<Statistics> m_rmsImageResiduals;
      QVector<Statistics> m_rmsImageXSigmas;     // unset and unused ???
      QVector<Statistics> m_rmsImageYSigmas;     // unset and unused ???
      QVector<Statistics> m_rmsImageZSigmas;     // unset and unused ???
      QVector<Statistics> m_rmsImageRASigmas;    // unset and unused ???
      QVector<Statistics> m_rmsImageDECSigmas;   // unset and unused ???
      QVector<Statistics> m_rmsImageTWISTSigmas; // unset and unused ???

      Distance m_minSigmaLatitudeDistance;
      Distance m_maxSigmaLatitudeDistance;
      Distance m_minSigmaLongitudeDistance;
      Distance m_maxSigmaLongitudeDistance;
      Distance m_minSigmaRadiusDistance;
      Distance m_maxSigmaRadiusDistance;

      QString m_minSigmaLatitudePointId;
      QString m_maxSigmaLatitudePointId;
      QString m_minSigmaLongitudePointId;
      QString m_maxSigmaLongitudePointId;
      QString m_minSigmaRadiusPointId;
      QString m_maxSigmaRadiusPointId;

      double m_sigmaLatStatsRms;               //!< rms of adjusted Latitude sigmas
      double m_sigmaLonStatsRms;               //!< rms of adjusted Longitude sigmas
      double m_sigmaRadStatsRms;               //!< rms of adjusted Radius sigmas

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // variables for maximum likelihood estimation
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      QList< QPair< MaximumLikelihoodWFunctions, double > > m_maximumLikelihoodFunctions;

      /**< The number of maximum likelihood estimation 
      models. Up to three different models can be used
      in succession.*/
      /**< This class is used to reweight observations in 
           order to achieve more robust parameter 
           estimation, up to three different maximum 
           likelihood estimation models can be used in
           succession.*/
      /**< Quantiles of the |residual| distribution to be
           used for tweaking constants of the maximum 
           probability models.*/
      int m_maximumLikelihoodIndex;            /**< This count keeps track of which stadge of the
                                                    maximum likelihood adjustment the bundle is
                                                    currently on.*/
      StatCumProbDistDynCalc *m_cumPro;        /**< This class will be used to calculate the
                                                    cumulative probability distribution of
                                                    |R^2 residuals|, quantiles of this distribution
                                                    are used to adjust the maximum likelihood
                                                    functions dynamically iteration by iteration.*/
      StatCumProbDistDynCalc *m_cumProRes;     /**< This class keeps track of the cumulative 
                                                    probability distribution of residuals
                                                    (in unweighted pixels), this is used for 
                                                    reporting, and not for computation.*/
      double m_maximumLikelihoodMedianR2Residuals; /**< Median of R^2 residuals.*/ 
       
  };
  // operators to read/write BundleResults to/from binary disk file
  QDataStream &operator<<(QDataStream &stream, const BundleResults &bundleResults);
  QDataStream &operator>>(QDataStream &stream, BundleResults &bundleResults);
};
#endif // BundleResults_h
