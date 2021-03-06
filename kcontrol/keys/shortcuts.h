/*
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SHORTCUTS_MODULE_H
#define SHORTCUTS_MODULE_H

#include <QButtonGroup>
#include <QPushButton>
#include <QRadioButton>
#include <QTabWidget>
#include <kcombobox.h>
#include <kshortcutsdialog.h>

class ShortcutsModule : public QWidget
{
	Q_OBJECT
 public:
	explicit ShortcutsModule( QWidget *parent = 0, const char *name = 0 );
	~ShortcutsModule();

	void load();
	void save();
	void defaults();
	QString quickHelp() const;

 protected:
	void initGUI();
	void createActionsGeneral();
	void createActionsSequence();
	void readSchemeNames();
	void saveScheme();
	void resizeEvent(QResizeEvent *e);

 Q_SIGNALS:
	void changed( bool );

 protected Q_SLOTS:
	void slotSchemeCur();
	void slotKeyChange();
	void slotSelectScheme( int = 0 );
	void slotSaveSchemeAs();
	void slotRemoveScheme();

 private:
	QTabWidget* m_pTab;
	QRadioButton *m_prbPre, *m_prbNew;
	QComboBox* m_pcbSchemes;
	QPushButton* m_pbtnSave, * m_pbtnRemove;
	int m_nSysSchemes;
	QStringList m_rgsSchemeFiles;
	KActionCollection* m_actionsGeneral, * m_actionsSequence;//, m_actionsApplication;
	KActionCollection* m_listGeneral, * m_listSequence, * m_listApplication;

	KShortcutsEditor* m_pseGeneral, * m_pseSequence, * m_pseApplication;
};

#endif // __SHORTCUTS_MODULE_H
