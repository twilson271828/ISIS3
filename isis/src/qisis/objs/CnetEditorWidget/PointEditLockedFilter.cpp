#include "IsisDebug.h"

#include "PointEditLockedFilter.h"

#include <iostream>

#include <QHBoxLayout>

#include "ControlMeasure.h"
#include "ControlPoint.h"


namespace Isis {
  PointEditLockedFilter::PointEditLockedFilter(
    AbstractFilter::FilterEffectivenessFlag flag,
    ControlNet *network,
    int minimumForSuccess) : AbstractFilter(flag, network, minimumForSuccess) {
  }


  PointEditLockedFilter::PointEditLockedFilter(const AbstractFilter &other)
    : AbstractFilter(other) {
  }


  PointEditLockedFilter::~PointEditLockedFilter() {
  }


  bool PointEditLockedFilter::evaluate(const QString *imageSerial) const {
    return AbstractFilter::evaluateImageFromPointFilter(imageSerial);
  }


  bool PointEditLockedFilter::evaluate(const ControlPoint *point) const {
    return AbstractFilter::evaluate(point, &ControlPoint::IsEditLocked);
  }


  bool PointEditLockedFilter::evaluate(const ControlMeasure *measure) const {
    return true;
  }


  AbstractFilter *PointEditLockedFilter::clone() const {
    return new PointEditLockedFilter(*this);
  }


  QString PointEditLockedFilter::getImageDescription() const {
    QString description = AbstractFilter::getImageDescription();

    if (getMinForSuccess() == 1)
      description += "point that is ";
    else
      description += "points that are ";

    if (inclusive())
      description += "edit locked";
    else
      description += "not edit locked";

    return description;
  }


  QString PointEditLockedFilter::getPointDescription() const {
    QString description = "are ";

    if (inclusive())
      description += "edit locked";
    else
      description += "not edit locked";

    return description;
  }
}
