/*******************************************************************************

    This file is a part of Fahrplan for maemo 2009-2010

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*/

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QtGui>
#include "mainwindow.h"

class configDialog : public QDialog
{
    Q_OBJECT
public:
    explicit configDialog(QSettings *mainSettings, QWidget *mainParent);

signals:

public slots:
    void okButtonClicked();
    void storelastSearchToggled(int state);
    void orientationChanged();

private:
    QSettings          *settings;
    QMaemo5ListPickSelector *backendSelector;
    QCheckBox   *gpsBox;
    QCheckBox   *gpsBackendBox;
    QCheckBox   *storelastSearchBox;
    QCheckBox   *prefillSearchBox;
    QWidget *parent;
    QPushButton *okButton;
};

#endif // CONFIGDIALOG_H
