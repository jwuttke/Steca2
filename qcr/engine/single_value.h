//  ***********************************************************************************************
//
//  libqcr: capture and replay Qt widget actions
//
//! @file      qcr/engine/single_value.h
//! @brief     Defines and implements templated classes QcrControl
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2018-
//! @author    Joachim Wuttke
//
//  ***********************************************************************************************

#ifndef SINGLE_VALUE_H
#define SINGLE_VALUE_H

#include "qcr/base/debug.h"
#include "qcr/base/string_ops.h"
#include "qcr/engine/mixin.h"
#include "qcr/engine/cell.h"

template<class T>
class QcrCell;

//! Base class for all Qcr widgets that hold a single value.
template<class T>
class QcrControl : public QcrSettable {
public:
    QcrControl(QObject& object, const QString& name, QcrCell<T>* cell);
    QcrControl(QObject& object, const QString& name, const T val); // TODO get rid of this variant
    ~QcrControl();
    void programaticallySetValue(T val);
    T getValue() { remake(); return doGetValue(); } // TODO rather call Cell::val
    virtual T doGetValue() const = 0; //!< to be overriden by the widget-specific get function
    virtual void executeConsoleCommand(const QString& arg);
    QcrCell<T>* cell() { return cell_; } // TODO rm
protected:
    void initControl();
    void onChangedValue(bool hasFocus, T val);
    bool softwareCalling_ {false}; // make it private again ??
    QcrCell<T>* cell_ {nullptr};
    T reportedValue_;
private:
    virtual void doSetValue(T) = 0; //!< to be overriden by the widget-specific set function
    bool ownsItsCell_ {false};
};

//  ***********************************************************************************************
//  implementation of QcrControl<T>

//! Constructor that associates this QcrControl with an external QcrCell.
template<class T>
QcrControl<T>::QcrControl(QObject& object, const QString& name, QcrCell<T>* cell)
    : QcrSettable {object, name}
    , cell_ {cell}
{
    cell_->backlink_ = this;
}

//! Constructs a QcrControl that owns a QcrCell.
template<class T>
QcrControl<T>::QcrControl(QObject& object, const QString& name, const T val)
    : QcrSettable {object, name}
    , ownsItsCell_ {true}
{
    cell_ = new QcrCell<T>(val); // TODO RECONSIDER smart pointer
    cell_->backlink_ = this;
}

template<class T>
QcrControl<T>::~QcrControl()
{
    if (ownsItsCell_)
        delete cell_;
}

//! Ensures synchronization of this Control with its associated Cell.

//! Cannot be called from QcrControl constructors, because it indirectly calls the pure
//! virtual member function 'doSetValue' that has overrides that are not available in
//! the constructor of 'QcrControl'.

template<class T>
void QcrControl<T>::initControl()
{
    T givenValue = cell_->val();
    programaticallySetValue(givenValue);
    reportedValue_ = getValue();
    if (!ownsItsCell_ && reportedValue_ != givenValue)
        qWarning("Widget %s has changed value of cell from %s to %s",
               name().toLatin1().constData(),
               strOp::to_s(givenValue).toLatin1().constData(),
               strOp::to_s(reportedValue_).toLatin1().constData());
    doLog(false, "initControl "+name()+" "+strOp::to_s(reportedValue_));
}

//! Wraps a call to 'doSetValue', with flag softwareCalling_ = true.

template<class T>
void QcrControl<T>::programaticallySetValue(T val)
{
    qDebug() << "progSetValue" << name() << "repVal=" << reportedValue_ << "val=" << val;
    softwareCalling_ = true;
    doSetValue(val);
    reportedValue_ = val;
    softwareCalling_ = false;
}

template<class T>
void QcrControl<T>::executeConsoleCommand(const QString& arg)
{
    programaticallySetValue(strOp::from_s<T>(arg));
}

//! If value has changed, then transmit value to cell, and log.

//! Used by control widgets, typically through Qt signals that are emitted upon user actions.

template<class T>
void QcrControl<T>::onChangedValue(bool hasFocus, T val)
{
    qDebug() << "onChangedValue" << name() << "hasFocus=" << hasFocus << "swCalling=" << softwareCalling_ << "reportedVal=" << reportedValue_ << "val=" << val;
    if (val==reportedValue_)
        return; // nothing to do
    bool userCall = hasFocus || !softwareCalling_;
    qDebug() << "onChangedValue ctd, userCall=" << userCall;
    doLog(userCall, name()+" "+strOp::to_s(val));

    // not sure whether we want to get rid of hasFocus; perform some tests:
    if (hasFocus && softwareCalling_)
        qDebug() << "UNEXPECTED in "+name()+" hasFocus && softwareCalling_";

    reportedValue_ = val;
    cell_->guiSetsVal(val, userCall);
}

#endif // SINGLE_VALUE_H
