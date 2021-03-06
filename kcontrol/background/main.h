/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef MAIN_H
#define MAIN_H

#include <kcmodule.h>
#include <kconfig.h>

class BGDialog;

/**
 * The Desktop/Background tab in kcontrol.
 */
class KBackground: public KCModule
{
    Q_OBJECT

public:
    KBackground(QWidget *parent, const QVariantList &);
    ~KBackground();

    virtual void load();
    virtual void save();
    virtual void defaults();

private:
    BGDialog      *m_base;
    KSharedConfigPtr m_pConfig;
};

#endif // MAIN_H
