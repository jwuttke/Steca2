//  ***********************************************************************************************
//
//  Steca: stress and texture calculator
//
//! @file      gui/panels/controls_detector.cpp
//! @brief     Implements class ControlsDetector, and local classes
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
//  ***********************************************************************************************

#include "controls_detector.h"
#include "core/session.h"
#include "gui/actions/image_trafo_actions.h"
#include "gui/actions/toggles.h"
#include "gui/mainwin.h"
#include "qcr/engine/debug.h"
#include <QThread> // for sleep for debugging

//  ***********************************************************************************************
//! @class GeometryControls (local scope)

//! Control widgets that govern the detector geometry.

class GeometryControls : public QWidget {
public:
    GeometryControls();
private:
    QVBoxLayout vbox_;
    QGridLayout mmGrid_;
    QHBoxLayout trafoLayout_;
    QHBoxLayout offsetLayout_;

    QcrDoubleSpinBox detDistance_;
    QcrDoubleSpinBox detPixelSize_;
    QcrSpinBox beamOffsetI_;
    QcrSpinBox beamOffsetJ_;
};

GeometryControls::GeometryControls()
    : detDistance_  {"detDistance", &gSession->geometry().detectorDistance, 6}
    , detPixelSize_ {"detPixelSize", &gSession->geometry().pixSize, 3}
    , beamOffsetI_  {"beamOffsetI", &gSession->geometry().pixOffset[0], 3, true}
    , beamOffsetJ_  {"beamOffsetJ", &gSession->geometry().pixOffset[1], 3, true}
{
    // layout
    mmGrid_.addWidget(new QLabel("det. distance"), 0, 0);
    mmGrid_.addWidget(&detDistance_, 0, 1);
    mmGrid_.addWidget(new QLabel("mm"), 0, 2);
    mmGrid_.addWidget(new QLabel("pixel size"), 1, 0);
    mmGrid_.addWidget(&detPixelSize_, 1, 1);
    mmGrid_.addWidget(new QLabel("mm"), 1, 2);

    trafoLayout_.addWidget(new QLabel("image rotate"));
    trafoLayout_.addWidget(new QcrIconButton(&gGui->imageTrafoActions->rotateImage));
    trafoLayout_.addWidget(new QLabel("mirror"));
    trafoLayout_.addWidget(new QcrIconButton(&gGui->imageTrafoActions->mirrorImage));
    trafoLayout_.addStretch(1);

    offsetLayout_.addWidget(new QLabel("offset X"));
    offsetLayout_.addWidget(&beamOffsetI_);
    offsetLayout_.addWidget(new QLabel(" Y"));
    offsetLayout_.addWidget(&beamOffsetJ_);
    offsetLayout_.addWidget(new QLabel("pix"));
    offsetLayout_.addStretch(1);

    vbox_.addLayout(&mmGrid_);
    vbox_.addLayout(&trafoLayout_);
    vbox_.addLayout(&offsetLayout_);
    setLayout(&vbox_);
}

//  ***********************************************************************************************
//! @class CutControls (local scope)

//! Control widgets that govern the detector cuts.

class CutControls : public QFrame {
public:
    CutControls();
private:
    QGridLayout layout_;
    QcrIconButton link_;
    QcrSpinBox cutLeft_;
    QcrSpinBox cutTop_;
    QcrSpinBox cutRight_;
    QcrSpinBox cutBottom_;
};

CutControls::CutControls()
    : link_      {&gGui->toggles->linkCuts}
    , cutLeft_   {"cutLeft",   &gSession->imageCut().left,   3, false, 0}
    , cutTop_    {"cutTop",    &gSession->imageCut().top,    3, false, 0}
    , cutRight_  {"cutRight",  &gSession->imageCut().right,  3, false, 0}
    , cutBottom_ {"cutBottom", &gSession->imageCut().bottom, 3, false, 0}
{

    // layout
    layout_.addWidget(new QLabel("cut"), 1, 0);
    layout_.addWidget(&cutLeft_, 1, 2);
    layout_.addWidget(&link_, 1, 3, Qt::AlignHCenter);
    layout_.addWidget(&cutTop_, 0, 3);
    layout_.addWidget(&cutBottom_, 2, 3);
    layout_.addWidget(&cutRight_, 1, 4);
    layout_.setColumnStretch(5, 1);
    setLayout(&layout_);
}

//  ***********************************************************************************************
//! @class ActiveClustersControls (local scope)

//! Control widgets that govern the combination of Measurement|s into Cluster|s.

class ActiveClustersControls : public QWidget {
public:
    ActiveClustersControls();
private:
    void fromCore();

    QHBoxLayout layout_;
    QcrSpinBox    combineMeasurements_;
    QcrToggle     dropIncompleteAction_;
    QcrIconButton dropIncompleteButton_;
};

ActiveClustersControls::ActiveClustersControls()
    : combineMeasurements_ {
        "combineMeasurements", &gSession->dataset().binning, 3, false, 1, 999,
        "Combine this number of measurements into one group"}
    , dropIncompleteAction_ {
        "dropIncomplete", &gSession->dataset().dropIncomplete,
        "Drop measurement groups that do not have the full number of members",
        ":/icon/dropIncomplete" }
    , dropIncompleteButton_ { &dropIncompleteAction_ }
{
    // layout
    layout_.addWidget(new QLabel("combine"));
    layout_.addWidget(&combineMeasurements_);
    layout_.addWidget(new QLabel("measurements"));
    layout_.addWidget(&dropIncompleteButton_);
    layout_.addStretch(1);
    setLayout(&layout_);

    //initialization
    dropIncompleteAction_.setEnabled(false);
    fromCore();
}

void ActiveClustersControls::fromCore()
{
    dropIncompleteAction_.setEnabled(gSession->dataset().hasIncomplete());
}

//  ***********************************************************************************************
//! @class GammaControls (local scope)

//! Control widgets that govern the gamma slicing.

class GammaControls : public QWidget {
public:
    GammaControls();
private:
    QHBoxLayout layout_;
    QcrSpinBox* numSlices_;
};

GammaControls::GammaControls()
{
    numSlices_ = new QcrSpinBox {
        "numSlices", &gSession->gammaSelection().numSlices, 2, false, 0, INT_MAX,
        "Number of γ slices (0: no slicing, take entire image)" };

    // layout
    layout_.addWidget(new QLabel("number of γ slices"));
    layout_.addWidget(numSlices_);
    layout_.addStretch(1);
    setLayout(&layout_);
}

//  ***********************************************************************************************
//! @class ControlsDetector

ControlsDetector::ControlsDetector()
{
    auto* vbox = new QVBoxLayout;
    vbox->addWidget(new GeometryControls);
    vbox->addWidget(new CutControls);
    vbox->addWidget(new ActiveClustersControls);
    vbox->addWidget(new GammaControls);
    vbox->addStretch();
    setLayout(vbox);
}
