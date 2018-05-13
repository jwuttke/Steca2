//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      core/calc/interpol_params.h
//! @brief     Defines InterpolParams
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#ifndef INTERPOL_PARAMS_H
#define INTERPOL_PARAMS_H

#include "core/typ/json.h"
#include "qcr/engine/cell.h"

//! Supports different ways of setting a gamma range.

class InterpolParams {
public:
    InterpolParams();

    void fromJson(const JsonObj& obj);
    void fromSettings();

    SingleValueCell<bool>   enabled     {false};
    SingleValueCell<double> stepAlpha   {5.};
    SingleValueCell<double> stepBeta    {5.};
    SingleValueCell<double> idwRadius   {10.};
    SingleValueCell<double> avgAlphaMax {5.};
    SingleValueCell<double> avgRadius   {5.};
    SingleValueCell<int>    threshold   {100};

    QJsonObject toJson() const;
    void toSettings() const;
};

#endif // INTERPOL_PARAMS_H
