//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      core/pars/detector.h
//! @brief     Defines classes Detector, ImageCut, ScatterDirection, ImageKey
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#ifndef DETECTOR_H
#define DETECTOR_H

#include "core/aux/angles.h"
#include "core/typ/range.h"
#include "core/typ/size2d.h"
#include "qcr/engine/param_wrapper.h"
#include <array>

// TODO after removal of ImageKey: delete copy c'tors

//! Detector detector.
class Detector {
public:
    static double const DEF_DETECTOR_DISTANCE;
    static double const DEF_DETECTOR_PIXEL_SIZE;

    Detector();
    ~Detector();

    void fromJson(const JsonObj& obj);

    QJsonObject toJson() const;

    QcrCell<double> detectorDistance {DEF_DETECTOR_DISTANCE};
    QcrCell<double> pixSize          {DEF_DETECTOR_PIXEL_SIZE};
    std::array<QcrCell<int>,2> pixOffset {{0, 0}};
};


//! Image cut (margins)
class ImageCut {
public:
    ImageCut();

    void fromJson(const JsonObj& obj);
    void clear();

    int horiz() const { return left.val() + right.val(); }
    int vertical() const { return top.val() + bottom.val(); }
    size2d marginSize() const;
    QJsonObject toJson() const;

    QcrCell<int> left {0};
    QcrCell<int> right {0};
    QcrCell<int> top {0};
    QcrCell<int> bottom {0};
};


//! A pair of angles (gamma, 2theta) that designate a scattering direction.
class ScatterDirection {
public:
    ScatterDirection();
    ScatterDirection(deg, deg);

    deg tth;
    deg gma;
};

#endif // DETECTOR_H
