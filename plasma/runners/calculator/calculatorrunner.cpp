/*
 *   Copyright (C) 2007 Barış Metin <baris@pardus.org.tr>
 *   Copyright (C) 2006 David Faure <faure@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "calculatorrunner.h"

#include <QAction>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

#include <KIcon>
#include <KLocale>
#include <kshell.h> //TODO: replace with KShell after 31/7/2007
#include <KStandardDirs>

CalculatorRunner::CalculatorRunner( QObject* parent, const QVariantList &args )
    : Plasma::AbstractRunner( parent ),
      m_options( 0 )
{
    Q_UNUSED(args)

    setObjectName( i18n( "Calculator" ) );
}

CalculatorRunner::~CalculatorRunner()
{
    delete m_options;
}

QAction* CalculatorRunner::accepts( const QString& term )
{
    QString cmd = term.trimmed();
    QAction *action = 0;

    if ( !cmd.isEmpty() && 
         ( cmd[0].isNumber() || ( cmd[0] == '(') ) &&
         ( QRegExp("[a-zA-Z\\]\\[]").indexIn(cmd) == -1 ) ) {
        QString result = calculate(cmd);

        if ( !result.isEmpty() ) {
            action = new QAction(KIcon("accessories-calculator"),
                                 QString("%1 = %2").arg(term, result),
                                 this);
            action->setEnabled( false );
        }
    }

    return action;
}

bool CalculatorRunner::exec(QAction* action, const QString& term)
{
    Q_UNUSED(action)
    Q_UNUSED(term)
    return true;
}

// function taken from kdesktop/minicli.cpp
QString CalculatorRunner::calculate( const QString& term )
{
    QString result, cmd;
    const QString bc = KStandardDirs::findExe( "bc" );
    if ( !bc.isEmpty() ) {
        cmd = QString( "echo %1 | %2" )
                     .arg(KShell::quoteArg(QString("scale=8; ") + term),
                          KShell::quoteArg(bc));
    }
    else {
        cmd = QString( "echo $((%1))" ).arg(term);
    }

    FILE *fs = popen(QFile::encodeName(cmd).data(), "r");
    if (fs)
    {
        { // scope for QTextStream
            QTextStream ts(fs, QIODevice::ReadOnly);
            result = ts.readAll().trimmed();
        }
        pclose(fs);
    }
    return result;
}

#include "calculatorrunner.moc"