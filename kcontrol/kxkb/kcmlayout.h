/*
 *  Copyright (C) 2003-2006 Andriy Rysin (rysin@kde.org)
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

#ifndef __KCM_LAYOUT_H__
#define __KCM_LAYOUT_H__


#include <kcmodule.h>

#include <QHash>

#include <QtGui/QListView>

#include "kxkbconfig.h"


class QWidget;
class OptionListItem;
class Q3ListViewItem;
class Ui_LayoutConfigWidget;
class XkbRules;


class SrcLayoutModel;
class DstLayoutModel;

class LayoutConfig : public KCModule
{
  Q_OBJECT

public:
  LayoutConfig(QWidget *parent, const QVariantList &args);
  virtual ~LayoutConfig();

  void load();
  void save();
  void defaults();
  void initUI();

protected:
  QString createOptionString();
  void updateIndicator();

protected slots:
  void moveUp();
  void moveDown();
  void variantChanged();
  void displayNameChanged(const QString& name);
//  void latinChanged();
  void layoutSelChanged();
  void loadRules();
  void refreshRulesUI();
  void updateLayoutCommand();
  void updateOptionsCommand();
  void add();
  void remove();

  void changed();

private:
  Ui_LayoutConfigWidget* widget;

  XkbRules *m_rules;
  KxkbConfig m_kxkbConfig;
  QHash<QString, OptionListItem*> m_optionGroups;
  SrcLayoutModel* m_srcModel;
  DstLayoutModel* m_dstModel;

  QWidget* makeOptionsTab();
  void updateStickyLimit();
  void updateAddButton();
  void updateDisplayName();
  void moveSelected(int shift);
  int getSelectedDstLayout();
};


#endif
