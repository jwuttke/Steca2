// ************************************************************************** //
//
//  Steca2: stress and texture calculator
//
//! @file      gui/panels/tabs_images.h
//! @brief     Defines class TabsImages
//!
//! @homepage  https://github.com/scgmlz/Steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef TABS_IMAGES_H
#define TABS_IMAGES_H

#include "calc/lens.h"
#include <QTabWidget>

class Measurement;
class QDoubleSpinBox;
class QSpinBox;

class TabsImages : public QTabWidget {
public:
    TabsImages();

private:
    QPixmap makeBlankPixmap();

    QImage makeImage(QSharedPointer<Image>, bool curvedScale);
    QPixmap makePixmap(QSharedPointer<Image>);
    QPixmap makePixmap(Measurement const&, Range const&, Range const&);

    void setSuite(QSharedPointer<Suite>);
    void render();

    QSharedPointer<Suite> dataseq_;
    class ImageWidget *dataImageWidget_, *corrImageWidget_;

    QSpinBox* spinN_;
    QSpinBox *numSlices_, *numSlice_, *numBin_;
    QDoubleSpinBox *minGamma_, *maxGamma_;

    QSharedPointer<SequenceLens> lens_;
};

#endif // TABS_IMAGES_H
