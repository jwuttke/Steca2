// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      core/data/dataset.h
//! @brief     Defines class Dataset
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#ifndef DATASET_H
#define DATASET_H

#include "core/calc/lens.h"
#include "core/calc/reflection.h"
#include "core/calc/reflection_info.h"
#include "core/data/angle_map.h"
#include "core/data/rawfile.h"
#include "core/data/experiment.h"
#include "core/data/image.h"
#include "core/typ/async.h"
#include "core/typ/cache.h"
#include <QSharedPointer> // no auto rm

//! Loaded Rawfile|s. Does not include the correction file.

class Dataset final {
public:
    // Modifying methods:
    void clear();
    bool addGivenFiles(const QStringList& filePaths) THROWS;
    void removeFile(int i);
    void setHighlight(const Rawfile*);
    void assembleExperiment(const vec<int>, const int);

    // Const methods:
    int count() const { return files_.count(); }
    const Rawfile* file(int i) const { return files_[i].data(); }
    int offset(const Rawfile* file) const { return mapOffset_[file]; }
    int highlight() const { return highlight_; }
    QJsonArray to_json() const;

    vec<int> const& filesSelection() const { return filesSelection_; }

    Experiment experiment_; // cluster collected ...

private:
    QVector<QSharedPointer<const Rawfile>> files_; //!< data files
    QMap<const Rawfile*,int> mapIndex_; //!< index in files_
    QMap<const Rawfile*,int> mapOffset_; //!< first index in total list of Measurement|s

    int highlight_ {0}; //!< index of highlighted file
    vec<int> filesSelection_; // from these files

    void updateCache();

    bool hasFile(rcstr fileName) const;
};

#endif // DATASET_H
