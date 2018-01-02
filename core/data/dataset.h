// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      core/data/dataset.h
//! @brief     Defines classes [One]Dataset[s]
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2017
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef DATASET_H
#define DATASET_H

#include "typ/angles.h"
#include "typ/curve.h"
#include "data/image.h"
#include "typ/variant.h"

class Session;
class Metadata;
class OneDataset;
class Dataset;
class Datasets;

typedef QSharedPointer<OneDataset const> shp_OneDataset;
typedef QSharedPointer<Dataset> shp_Dataset;

//! Metadata + Image, for calculation always accessed through its owning Dataset

class OneDataset final {

public:
    OneDataset(Metadata const&, size2d const&, inten_vec const&);
    OneDataset(OneDataset const&) = delete;

    QSharedPointer<Metadata const> metadata() const;

    deg midTth() const;

    qreal monitorCount() const;
    qreal deltaMonitorCount() const;
    qreal deltaTime() const;

    deg omg() const;
    deg phi() const;
    deg chi() const;

    Range rgeGma(Session const&) const;
    Range rgeGmaFull(Session const&) const;
    Range rgeTth(Session const&) const;

    Range rgeInten() const;

    shp_Image image() const { return image_; }
    size2d imageSize() const;

    void collectIntens(
        Session const&, Image const* intensCorr, inten_vec&, uint_vec&, Range const&,
        deg minTth, deg deltaTth) const;

private:
    QSharedPointer<Metadata const> md_;
    shp_Image image_;
};


//! One or more OneDataset(s)

class Dataset final : public vec<shp_OneDataset> {
private:
    friend class Datasets;

public:
    Dataset();

    QSharedPointer<Metadata const> metadata() const;
    Datasets const& datasets() const;

    deg omg() const;
    deg phi() const;
    deg chi() const;

    Range rgeGma(Session const&) const;
    Range rgeGmaFull(Session const&) const;
    Range rgeTth(Session const&) const;

    Range rgeInten() const;

    qreal avgMonitorCount() const;
    qreal avgDeltaMonitorCount() const;
    qreal avgDeltaTime() const;

    inten_vec collectIntens(Session const&, Image const* intensCorr, Range const&) const;
    void calculateAlphaBeta(deg tth, deg gma, deg& alpha, deg& beta) const;

    size2d imageSize() const;

private:
    Datasets* datasets_; // here it belongs (or can be nullptr)
    QSharedPointer<Metadata const> md_; // on demand, compute once
};

//! Collection of (Dataset)s

class Datasets final : public vec<shp_Dataset> {
public:
    Datasets();

    void appendHere(shp_Dataset);

    size2d imageSize() const;

    qreal avgMonitorCount() const;
    qreal avgDeltaMonitorCount() const;
    qreal avgDeltaTime() const;

    Range const& rgeGma(Session const&) const;
    Range const& rgeFixedInten(Session const&, bool trans, bool cut) const;

    Curve avgCurve(Session const&) const;

    void invalidateAvgMutables() const;

private:
    shp_Dataset combineAll() const;
    qreal calcAvgMutable(qreal (Dataset::*avgMth)() const) const;

    // computed on demand (NaNs or emptiness indicate yet unknown values)
    mutable qreal avgMonitorCount_, avgDeltaMonitorCount_, avgDeltaTime_;
    mutable Range rgeFixedInten_;
    mutable Range rgeGma_;
    mutable Curve avgCurve_;
};

Q_DECLARE_METATYPE(shp_Dataset)

#endif // DATASET_H
