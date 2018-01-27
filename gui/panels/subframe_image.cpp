// ************************************************************************** //
//
//  Steca: stress and texture calculator
//
//! @file      gui/panels/subframe_image.cpp
//! @brief     Implements class SubframeImage
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016-2018
//! @authors   Scientific Computing Group at MLZ (see CITATION, MAINTAINER)
//
// ************************************************************************** //

#include "subframe_image.h"
#include "core/session.h"
#include "gui/cfg/colors.h"
#include "gui/thehub.h"
#include "gui/base/various_widgets.h"
#include <qmath.h>
#include <QPainter>

// ************************************************************************** //
//  local class ImageWidget
// ************************************************************************** //

class ImageWidget final : public QWidget {
public:
    ImageWidget();

    void setPixmap(QPixmap const&);
    void setScale();

private:
    void resizeEvent(QResizeEvent*);
    void paintEvent(QPaintEvent*);

    qreal scale_;
    QPixmap original_, scaled_;
};


ImageWidget::ImageWidget() : scale_(0) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(gHub->toggle_showOverlay, &QAction::toggled, [this](bool /*unused*/) { update(); });
    connect(gHub->toggle_stepScale, &QAction::toggled, [this](bool /*unused*/) { setScale(); });
}

void ImageWidget::setPixmap(QPixmap const& pixmap) {
    original_ = pixmap;
    setScale();
}

void ImageWidget::setScale() {
    if (original_.isNull()) {
        scale_ = 0;
    } else {
        const QSize& sz = size();
        const QSize& os = original_.size();
        scale_ = qMin(qreal(sz.width() - 2) / os.width(), qreal(sz.height() - 2) / os.height());
    }

    if (gHub->toggle_stepScale->isChecked() && scale_ > 0)
        scale_ = (scale_ >= 1) ? qFloor(scale_) : 1.0 / qCeil(1.0 / scale_);

    if (original_.isNull() || !(scale_ > 0))
        scaled_ = QPixmap();
    else
        scaled_ = original_.scaled(original_.size() * scale_);

    update();
}

void ImageWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    setScale();
}

void ImageWidget::paintEvent(QPaintEvent*) {
    // paint centered
    const QSize margin = (size() - scaled_.size()) / 2;
    const QRect rect(QPoint(margin.width(), margin.height()), scaled_.size());

    QPainter p(this);

    // image
    p.drawPixmap(rect.left(), rect.top(), scaled_);

    // overlay
    if (gHub->toggle_showOverlay->isChecked()) {
        p.setPen(Qt::lightGray);

        // cut
        const ImageCut& cut = gSession->imageCut();
        const QRect r = rect.adjusted(-1, -1, 0, 0)
                      .adjusted(
                          qRound(scale_ * cut.left), qRound(scale_ * cut.top),
                          -qRound(scale_ * cut.right), -qRound(scale_ * cut.bottom));
        p.drawRect(r);

        const QPoint rc = r.center();
        const int rcx = rc.x(), rcy = rc.y();

        int rl, rt, rr, rb;
        r.getCoords(&rl, &rt, &rr, &rb);
        const int rw = rr - rl;

        // cross
        const IJ& off = gSession->geometry().midPixOffset;
        const int x = qRound(rcx + scale_ * off.i);
        const int y = qRound(rcy + scale_ * off.j);
        p.drawLine(x, rt, x, rb);
        p.drawLine(rl, y, rr, y);

        // text
        const QFontMetrics& fm = fontMetrics();
        QPoint pos(rr - rw / 5, rcy);
        p.setPen(Qt::cyan);
        p.drawText(pos, "γ=0");
    }

    // frame
    p.setPen(Qt::black);
    p.drawRect(rect.adjusted(-1, -1, 0, 0));
}


// ************************************************************************** //
//  local base class ImageTab
// ************************************************************************** //

class ImageTab : public QWidget {
public:
    ImageTab();
    virtual void render() = 0;
    virtual void onHighlight() = 0;
protected:
    QBoxLayout* controls_;
    QPixmap makePixmap(shp_Image);
    QPixmap makePixmap(class Measurement const&, const Range&, const Range&);
    QPixmap makeBlankPixmap();
    QImage makeImage(shp_Image, bool curvedScale);
    ImageWidget* imageView_;
private:
    QBoxLayout* box_;
};

ImageTab::ImageTab() {
    box_ = newQ::BoxLayout(Qt::Vertical);
    setLayout(box_);

    controls_ = newQ::HBoxLayout();
    box_->addLayout(controls_);
    controls_->addWidget(newQ::IconButton(gHub->toggle_fixedIntenImage));
    controls_->addWidget(newQ::IconButton(gHub->toggle_stepScale));
    controls_->addWidget(newQ::IconButton(gHub->toggle_showOverlay));

    imageView_ = new ImageWidget();
    box_->addWidget(imageView_);

    connect(gHub->toggle_enableCorr, &QAction::toggled, [this](bool /*unused*/) { render(); });
    connect(gHub->toggle_showBins, &QAction::toggled, [this](bool /*unused*/) { render(); });

    connect(gHub, &TheHub::sigDisplayChanged, this, &ImageTab::render);
    connect(gSession, &Session::sigDetector, this, &ImageTab::render);
    connect(gSession, &Session::sigNorm, this, &ImageTab::render);
    connect(gSession, &Session::sigHighlight, this, &ImageTab::onHighlight);
}

QPixmap ImageTab::makePixmap(shp_Image image) {
    return QPixmap::fromImage(makeImage(image, !gHub->isFixedIntenImageScale()));
}

QPixmap ImageTab::makePixmap(
    Measurement const& cluster, const Range& rgeGma, const Range& rgeTth) {
    QImage im = makeImage(cluster.image(), !gHub->isFixedIntenImageScale());
    shp_AngleMap angleMap = gSession->angleMap(cluster);

    const QSize& size = im.size();
    for_ij (size.width(), size.height()) {
        ScatterDirection const& a = angleMap->at(i, j);
        QColor color = im.pixel(i, j);
        if (rgeGma.contains(a.gma)) {
            if (rgeTth.contains(a.tth)) {
                color = Qt::yellow;
            } else {
                color.setGreen(qFloor(color.green() * .3 + 255 * .7));
            }
        } else if (rgeTth.contains(a.tth)) {
            color.setGreen(qFloor(color.green() * .3 + 255 * .7));
        }
        im.setPixel(i, j, color.rgb());
    }

    return QPixmap::fromImage(im);
}

QPixmap ImageTab::makeBlankPixmap() {
    const size2d size = gSession->imageSize();
    QPixmap pixmap(size.w, size.h);
    pixmap.fill(QColor(0, 0, 0, 0));
    return pixmap;
}

QImage ImageTab::makeImage(shp_Image image, bool curvedScale) {
    if (!image)
        return {};

    shp_ImageLens imageLens = gSession->imageLens(*image, true, false);
    const size2d size = imageLens->size();
    if (size.isEmpty())
        return {};

    QImage ret(QSize(size.w, size.h), QImage::Format_RGB32);

    const Range rgeInten = imageLens->rgeInten(gHub->isFixedIntenImageScale());
    inten_t maxInten = inten_t(rgeInten.max);

    for_ij (size.w, size.h)
        ret.setPixel(i, j,
                    colormap::intenImage(imageLens->imageInten(i, j), maxInten, curvedScale));
    return ret;
}

// ************************************************************************** //
//  local class DataImageTab
// ************************************************************************** //

class DataImageTab : public ImageTab {
public:
    DataImageTab();
    void render() final;
    void onHighlight() final;
private:
    const Cluster* cluster_ {nullptr};

    QSpinBox* spinN_;
    QSpinBox *numSlices_, *numSlice_, *numBin_;
    QDoubleSpinBox *minGamma_, *maxGamma_;
};

DataImageTab::DataImageTab() {
    controls_->addWidget((spinN_ = newQ::SpinBox(4, false, 1)));

    controls_->addStretch(1);

    controls_->addWidget(newQ::IconButton(gHub->toggle_showBins));
    controls_->addWidget(newQ::Label("γ count"));
    controls_->addWidget((numSlices_ = newQ::SpinBox(4, false, 0)));
    controls_->addWidget(newQ::Label("#"));
    controls_->addWidget((numSlice_ = newQ::SpinBox(4, false, 1)));

    controls_->addWidget(newQ::Label("min"));
    controls_->addWidget((minGamma_ = newQ::DoubleSpinBox(6)));
    controls_->addWidget(newQ::Label("max"));
    controls_->addWidget((maxGamma_ = newQ::DoubleSpinBox(6)));

    minGamma_->setReadOnly(true);
    maxGamma_->setReadOnly(true);

    controls_->addWidget(newQ::Label("bin#"));
    controls_->addWidget((numBin_ = newQ::SpinBox(4, false, 1)));

    connect(spinN_, _SLOT_(QSpinBox, valueChanged, int),
            [this](int /*unused*/) { render(); });
    connect(numSlices_, _SLOT_(QSpinBox, valueChanged, int),
            [this](int /*unused*/) { render(); });
    connect(numSlice_, _SLOT_(QSpinBox, valueChanged, int),
            [this](int /*unused*/) { render(); });
    connect(numBin_, _SLOT_(QSpinBox, valueChanged, int),
            [this](int /*unused*/) { render(); });
}

void DataImageTab::onHighlight() {
    cluster_ = gSession->dataset().highlightedCluster();
    render();
}

void DataImageTab::render() {
    QPixmap pixMap;

    const int nSlices = numSlices_->value();
    numSlice_->setMaximum(qMax(1, nSlices));
    numSlice_->setEnabled(nSlices > 0);

    if (cluster_) {
        // 1 - based
        const int by = gSession->dataset().binning();
        const int n = qBound(1, spinN_->value(), by);

        spinN_->setValue(n);
        spinN_->setEnabled(by > 1);

        shp_SequenceLens lens = gSession->highlightsLens();

        Range rge;
        if (nSlices > 0) {
            int nSlice = qMax(1, numSlice_->value());
            int iSlice = nSlice - 1;

            const Range rgeGma = lens->rgeGma();
            const qreal min = rgeGma.min;
            const qreal wn = rgeGma.width() / nSlices;

            rge = Range(min + iSlice * wn, min + (iSlice + 1) * wn);

            minGamma_->setValue(rge.min);
            maxGamma_->setValue(rge.max);
        } else {
            rge = Range::infinite();
            minGamma_->clear();
            maxGamma_->clear();
        }
        gSession->setGammaRange(rge);

        const Measurement* measurement = cluster_->at(n - 1);

        numBin_->setEnabled(true);
        if (gHub->toggle_showBins->isChecked()) {
            Range rgeTth = lens->rgeTth();
            int count = lens->makeCurve().count();
            numBin_->setMaximum(count - 1);
            int min = rgeTth.min, wdt = rgeTth.width();
            qreal num = qreal(numBin_->value());
            pixMap = makePixmap(
                *measurement, rge,
                Range(min + wdt * (num / count), min + wdt * ((num + 1) / count)));
        } else {
            pixMap = makePixmap(measurement->image());
        }
    } else {
        spinN_->setEnabled(false);
        numBin_->setMaximum(0);
        numBin_->setEnabled(false);

        pixMap = makeBlankPixmap();
    }

    imageView_->setPixmap(pixMap);
}

// ************************************************************************** //
//  local class CorrImageTab
// ************************************************************************** //

class CorrImageTab : public ImageTab {
public:
    CorrImageTab();
    void render() final;
    void onHighlight() final {}
};

CorrImageTab::CorrImageTab() {
    controls_->addStretch(1);
    connect(gSession, &Session::sigCorr, [this]() {
            setEnabled(gSession->corrset().isEnabled()); });
}

void CorrImageTab::render() {
    QPixmap pixMap = makePixmap(gSession->corrset().image());
    imageView_->setPixmap(pixMap);
}

// ************************************************************************** //
//  class SubframeImage
// ************************************************************************** //

SubframeImage::SubframeImage() {
    setTabPosition(QTabWidget::North);
    auto tabData = new DataImageTab(); addTab(tabData, "Data image");
    auto tabCorr = new CorrImageTab(); addTab(tabCorr, "Corr image");
}
