/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>
Copyright (c) 2002 John Firebaugh <jfirebaugh@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __tasklmbmenu_h__
#define __tasklmbmenu_h__

#include <QMenu>
#include <QTimer>
#include <QAction>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QList>

#include "taskmanager/taskmanager.h"

#ifdef __GNUC__
#warning "Need custom menu item support, which isn't there in Qt4!"
#endif
#if 0
class TaskMenuItem : public QCustomMenuItem
{
public:
    TaskMenuItem(const QString &text,
                 bool active, bool minimized, bool attention);
    ~TaskMenuItem();

    void paint(QPainter*, const QColorGroup&, bool, bool, int, int, int, int);
    QSize sizeHint();
    void setAttentionState(bool state) { m_attentionState = state; }

private:
    QString m_text;
    bool m_isActive;
    bool m_isMinimized;
    bool m_demandsAttention;
    bool m_attentionState;
};
#endif

typedef QAction TaskMenuItem;


/*****************************************************************************/

class KDE_EXPORT TaskLMBMenu : public QMenu
{
    Q_OBJECT

public:
    explicit TaskLMBMenu(const QList<TaskManager::Task*> list, QWidget *parent = 0, const char *name = 0);

protected Q_SLOTS:
    void dragSwitch();
    void attentionTimeout();

protected:
    void dragEnterEvent(QDragEnterEvent*);
    void dragLeaveEvent(QDragLeaveEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);

private:
    void fillMenu();

    QList<TaskManager::Task*> m_tasks;
    int        m_lastDragId;
    bool       m_attentionState;
    QTimer*    m_attentionTimer;
    QTimer*    m_dragSwitchTimer;
    QPoint     m_dragStartPos;
    QList<TaskMenuItem*> m_attentionMap;
};

#endif
