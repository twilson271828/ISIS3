#ifndef AbstractFilter_H
#define AbstractFilter_H

#include <QWidget>

#include "ControlNet"

class QBoxLayout;
class QButtonGroup;
class QCheckBox;
template< typename T > class QFlags;


namespace Isis {
  class AbstractFilterSelector;
  class ControlPoint;
  class ControlMeasure;
  class QString;

  /**
   * @brief Base class for control net filters
   *
   * This class is the base class that all other filters derive from. It
   * encompasses both the widget and the filter functionality itself. See the
   * cneteditor architecture document for further information about the
   * filtering system.
   *
   * @author ????-??-?? Eric Hyer
   *
   * @internal
   *   @history 2012-09-28 Kimberly Oyama - Changed member variables to be prefixed with "m_".
   *   @history 2017-07-25 Summer Stapleton - Removed the CnetViz namespace. Fixes #5054.
   *   @history 2018-06-01 Jesse Mapel - Changed ControlCubeGraphNode to image serial number.
   *                           References #5434.
   */
  class AbstractFilter : public QWidget {
      Q_OBJECT

    public:
      enum FilterEffectiveness {
        Images = 1,
        Points = 2,
        Measures = 4
      };
      Q_DECLARE_FLAGS(FilterEffectivenessFlag, FilterEffectiveness)


    public:
      AbstractFilter(FilterEffectivenessFlag, ControlNet *network, int minimumForSuccess = -1);
      AbstractFilter(const AbstractFilter &other);
      virtual ~AbstractFilter();

      virtual bool canFilterImages() const;
      virtual bool canFilterPoints() const;
      virtual bool canFilterMeasures() const;

      virtual bool evaluate(const QString *) const = 0;
      virtual bool evaluate(const ControlPoint *) const = 0;
      virtual bool evaluate(const ControlMeasure *) const = 0;

      virtual AbstractFilter *clone() const = 0;

      virtual QString getImageDescription() const;
      virtual QString getPointDescription() const;
      virtual QString getMeasureDescription() const;


    signals:
      void filterChanged();


    protected:
      bool inclusive() const;
      int getMinForSuccess() const {
        return m_minForSuccess;
      }
      AbstractFilter::FilterEffectivenessFlag *getEffectivenessFlags() const;
      QBoxLayout *getMainLayout() const;
      QBoxLayout *getInclusiveExclusiveLayout() const;

      bool evaluateImageFromPointFilter(const QString *) const;
      bool evaluateImageFromMeasureFilter(const QString *) const;
      bool evaluatePointFromMeasureFilter(const ControlPoint *) const;

      virtual bool evaluate(const ControlPoint *,
          bool (ControlPoint:: *)() const) const;
      virtual bool evaluate(const ControlMeasure *,
          bool (ControlMeasure:: *)() const) const;


    private:
      void createWidget();
      bool evaluateFromCount(QList< ControlMeasure * >, bool) const;
      void nullify();


    private slots:
      void updateEffectiveness();
      void updateMinForSuccess(int);


    private:
      QCheckBox *createEffectivenessCheckBox(QString);


    private:
      QBoxLayout *m_mainLayout;
      QBoxLayout *m_inclusiveExclusiveLayout;
      QButtonGroup *m_inclusiveExclusiveGroup;
      QButtonGroup *m_effectivenessGroup;
      QWidget *m_minWidget;


    private:
      int m_minForSuccess;
      FilterEffectivenessFlag *m_effectivenessFlags;
      QFont *m_smallFont;
      ControlNet *m_controlNet;
  };

  Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractFilter::FilterEffectivenessFlag)
}

#endif
