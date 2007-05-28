/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Rivo Laks <rivolaks@hot.ee>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#ifndef KWIN_PRESENTWINDOWS_CONFIG_H
#define KWIN_PRESENTWINDOWS_CONFIG_H

#include <kcmodule.h>

class QComboBox;


namespace KWin
{

class PresentWindowsEffectConfig : public KCModule
    {
    Q_OBJECT
    public:
        PresentWindowsEffectConfig(QWidget* parent = 0, const QStringList& args = QStringList());
        ~PresentWindowsEffectConfig();

        virtual void save();
        virtual void load();
        virtual void defaults();

    protected:
        void addItems(QComboBox* combo);

    private:
        QComboBox* mActivateCombo;
        QComboBox* mActivateAllCombo;
    };

} // namespace

#endif