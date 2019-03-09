//  ***********************************************************************************************
//
//  libqcr: capture and replay Qt widget actions
//
//! @file      qcr/widgets/modal_dialogs.cpp
//! @brief     Implements classes QcrModal, QcrModalDialog, QcrFileDialog
//!
//! @homepage  https://github.com/scgmlz/Steca
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2018-
//! @author    Joachim Wuttke
//
//  ***********************************************************************************************

#include "qcr/widgets/modal_dialogs.h"
#include "qcr/base/debug.h"
#include "qcr/base/qcrexception.h"
#include "qcr/engine/console.h"


//  ***********************************************************************************************
//! @class QcrModal

QcrModal::QcrModal(const QString& name)
    : QcrCommandable {gConsole->learn("@push " + name,this)}
{}

QcrModal::~QcrModal()
{
    qDebug() << "/~QcrModal";
    preclose();
    qDebug() << "~QcrModal/";
}

void QcrModal::preclose()
{
    qDebug() << "/preclose";
    if (preclosed_)
        return;
    qDebug() << "preclose calling forget";
    gConsole->forget(name());
    qDebug() << "preclose calling cMD";
    gConsole->closeModalDialog(name());
    preclosed_ = true;
    qDebug() << "preclose/";
}


//  ***********************************************************************************************
//! @class QcrModalDialog

QcrModalDialog::QcrModalDialog(QWidget* parent, const QString& caption)
    : QcrModal{"modal"}
    , QDialog {parent}
{
    setWindowTitle(caption);
    setModal(true);
    setAttribute(Qt::WA_DeleteOnClose, true);
}

void QcrModalDialog::setFromCommand(const QString& arg)
{
    if (arg=="")
        throw QcrException{"Empty argument in Dialog command"};
    if (arg=="close") {
        accept();
        return;
    }
}


//  ***********************************************************************************************
//! @class QcrFileDialog

QcrFileDialog::QcrFileDialog(
    QWidget* parent, const QString& caption, const QString& directory, const QString& filter,
    std::function<void(const QStringList)> postprocess)
    : QcrModal{"fdia"}
    , QFileDialog{parent, caption, directory, filter}
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    connect(this, &QcrFileDialog::finished, this,
            [this,postprocess](){
                if (result()==Accepted) {
                qDebug() << "FileDialog calling /postprocess";
                    postprocess(this->selectedFiles());
                qDebug() << "FileDialog calling postprocess/";
                }
                qDebug() << "FileDialog calling /close";
                preclose();
                close();
                qDebug() << "FileDialog calling close/";
            });
}

QcrFileDialog::~QcrFileDialog()
{
    gConsole->log("fdia select "+selectedFiles().join(';'));
}

void QcrFileDialog::setFromCommand(const QString& arg)
{
    if (arg=="")
        throw QcrException{"Empty argument in FileDialog command"};
    if (arg=="close") {
        qDebug() << "FileDialog /close->accept";
        accept(); // will emit signal finished(), which triggers postprocess and close
        qDebug() << "FileDialog close->accept/";
        return;
    }
    QStringList args = arg.split(' ');
    if (args[0]!="select")
        throw QcrException{"Unexpected filedialog command"};
    if (args.size()<2)
        throw QcrException{"Missing argument to command 'select'"};
    QStringList list = args[1].split(';');
    QString tmp = '"' + list.join("\" \"") + '"';
    selectFile(tmp);
}
