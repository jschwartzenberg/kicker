/*
    Copyright (C) 2000 Oswald Buddenhagen <ossi@kde.org>
    Based on several other files.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __KDMCONV_H__
#define __KDMCONV_H__

#include <QWidget>

class K3ListView;
class Q3ListViewItem;
class KComboBox;
class QCheckBox;
class QGroupBox;
class QLabel;
class QRadioButton;

class KDMConvenienceWidget : public QWidget {
	Q_OBJECT

  public:
	KDMConvenienceWidget( QWidget *parent = 0 );

	void load();
	void save();
	void defaults();
	void makeReadOnly();

  public Q_SLOTS:
	void slotClearUsers();
	void slotAddUsers( const QMap<QString,int> & );
	void slotDelUsers( const QMap<QString,int> & );


  Q_SIGNALS:
	void changed();

  private Q_SLOTS:
	void slotPresChanged();
	void slotSetAutoUser( const QString &user );
	void slotSetPreselUser( const QString &user );
	void slotUpdateNoPassUser( Q3ListViewItem *item );

  private:
	QGroupBox *alGroup, *puGroup, *npGroup, *btGroup;
	QCheckBox *cbarlen, *cbjumppw, *autoLockCheck;
	QRadioButton *npRadio, *ppRadio, *spRadio;
	KComboBox *userlb, *puserlb;
	K3ListView *npuserlv;
	QLabel *u_label, *pu_label, *w_label, *n_label, *pl_label;
	QString autoUser, preselUser;
	QStringList noPassUsers;
};

#endif


