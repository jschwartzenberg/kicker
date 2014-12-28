/*
 *  advancedDialog.cpp
 *
 *  Copyright (c) 2002 Aaron J. Seigo <aseigo@olympusproject.org>
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
 */

#include <QButtonGroup>
#include <QCheckBox>
#include <QLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>

#include <kcolorbutton.h>
#include <klocale.h>

#include "advancedDialog.h"
#include "advancedOptions.h"
#include "main.h"

advancedDialog::advancedDialog(QWidget* parent, const char* name)
    : KDialog( parent )
{
    setCaption( i18n("Advanced Options") );
    setButtons( Ok | Apply |Cancel );
    setDefaultButton( Cancel );
    setObjectName( name );
    showButtonSeparator( false );

    connect(this, SIGNAL(applyClicked()),
            this, SLOT(save()));
    connect(this, SIGNAL(okClicked()),
            this, SLOT(save()));

    QFrame* page = new QFrame( this );
    setMainWidget( page );
    QVBoxLayout* layout = new QVBoxLayout(page);
    m_advancedWidget = new advancedKickerOptions(page);
    layout->addWidget(m_advancedWidget);
    layout->addStretch();

    setMinimumSize( sizeHint() );

    connect(m_advancedWidget->handles, SIGNAL(clicked(int)),
            this, SLOT(changed()));
    connect(m_advancedWidget->hideButtonSize, SIGNAL(valueChanged(int)),
            this, SLOT(changed()));
    connect(m_advancedWidget->tintColorB, SIGNAL(clicked()),
            this, SLOT(changed()));
    connect(m_advancedWidget->tintSlider, SIGNAL(valueChanged(int)),
            this, SLOT(changed()));
    connect(m_advancedWidget->menubarPanelTransparent, SIGNAL(clicked()),
            this, SLOT(changed()));
    load();
}

advancedDialog::~advancedDialog()
{
}

void advancedDialog::load()
{
    KConfig kc(KickerConfig::the()->configName());
    KConfigGroup c(&kc, "General");

    bool fadedOut = c.readEntry("FadeOutAppletHandles", false);
    bool hideHandles = c.readEntry("HideAppletHandles", false);
    if (hideHandles)
        m_advancedWidget->hideHandles->setChecked(true);
    else if (fadedOut)
        m_advancedWidget->fadeOutHandles->setChecked(true);
    else
        m_advancedWidget->visibleHandles->setChecked(true);

    int defaultHideButtonSize = c.readEntry("HideButtonSize", 14);
    m_advancedWidget->hideButtonSize->setValue(defaultHideButtonSize);
    QColor color = c.readEntry( "TintColor", colorGroup().mid() );
    m_advancedWidget->tintColorB->setColor( color );
    int tintValue = c.readEntry( "TintValue", 0 );
    m_advancedWidget->tintSlider->setValue( tintValue );
    
    bool transparentMenubarPanel = c.readEntry("MenubarPanelTransparent", false);
    m_advancedWidget->menubarPanelTransparent->setChecked( transparentMenubarPanel );

    enableButtonApply(false);
}

void advancedDialog::save()
{
    KConfig kc(KickerConfig::the()->configName());

    KConfigGroup c(&kc, "General");
    c.writeEntry("FadeOutAppletHandles",
                 m_advancedWidget->fadeOutHandles->isChecked());
    c.writeEntry("HideAppletHandles",
                 m_advancedWidget->hideHandles->isChecked());
    c.writeEntry("HideButtonSize",
                 m_advancedWidget->hideButtonSize->value());
    c.writeEntry("TintColor",
                 m_advancedWidget->tintColorB->color());
    c.writeEntry("TintValue",
                 m_advancedWidget->tintSlider->value());
    c.writeEntry("MenubarPanelTransparent",
                 m_advancedWidget->menubarPanelTransparent->isChecked());

    QStringList elist = c.readEntry("Extensions2", QStringList());
    for (QStringList::Iterator it = elist.begin(); it != elist.end(); ++it)
    {
        // extension id
        QString group(*it);

        // is there a config group for this extension?
        if(!c.hasGroup(group) ||
           group.contains("Extension"))
        {
            continue;
        }

        // set config group
        KConfigGroup cg(&c, group);
        KConfig extConfigkc(cg.readEntry("ConfigFile"));
        KConfigGroup extConfig(&extConfigkc, "General");
        extConfig.writeEntry("FadeOutAppletHandles",
                             m_advancedWidget->fadeOutHandles->isChecked());
        extConfig.writeEntry("HideAppletHandles",
                             m_advancedWidget->hideHandles->isChecked());
        extConfig.writeEntry("HideButtonSize",
                             m_advancedWidget->hideButtonSize->value());
        extConfig.writeEntry("TintColor",
                             m_advancedWidget->tintColorB->color());
        extConfig.writeEntry("TintValue",
                             m_advancedWidget->tintSlider->value());
        extConfig.writeEntry("MenubarPanelTransparent",
                             m_advancedWidget->menubarPanelTransparent->isChecked());

        extConfig.sync();
    }
   
    c.sync();

    KickerConfig::the()->notifyKicker();
    enableButtonApply(false);
}

void advancedDialog::changed()
{
    enableButtonApply(true);
}

#include "advancedDialog.moc"

