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

#ifndef __taskcontainer_h__
#define __taskcontainer_h__

#include <QPixmap>
#include <QTimer>
#include <QToolButton>
#include <QDragLeaveEvent>
#include <QPaintEvent>
#include <QEvent>
#include <QList>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>

#include <taskmanager/startup.h>
#include <taskmanager/taskitem.h>

#include "kickertip.h"

class TaskBar;

typedef QList<QPixmap*> PixmapList;

class TaskContainer : public QToolButton, public KickerTip::Client
{
    Q_OBJECT

public:
    typedef QList<TaskContainer*> List;
    typedef QList<TaskContainer*>::iterator Iterator;

    TaskContainer(TaskManager::Task*, TaskBar*, QWidget *parent = 0);
    TaskContainer(TaskManager::Startup*, PixmapList&, TaskBar*,
                  QWidget *parent = 0);
    virtual ~TaskContainer();

    void setArrowType( Qt::ArrowType at );

    void init();

    void add(TaskManager::Task*);
    void remove(TaskManager::Task*);
    void remove(TaskManager::Startup*);

    bool contains(TaskManager::Task*);
    bool contains(TaskManager::Startup*);
    bool contains(WId);

    bool isEmpty();
    bool onCurrentDesktop();
    bool isIconified();
    bool isOnScreen();

    QString id();
    int desktop();
    QString name();

    virtual QSizePolicy sizePolicy () const;

    void publishIconGeometry( QPoint );
    void desktopChanged( int );
    void windowChanged(TaskManager::Task*);
    void settingsChanged();
    bool eventFilter( QObject *o, QEvent *e );

    int taskCount() const { return tasks.count(); }
    int filteredTaskCount() const { return m_filteredTasks.count(); }

    bool activateNextTask( bool forward, bool& forcenext );

    void updateTipData(KickerTip::Data&);

Q_SIGNALS:
    void showMe(TaskContainer*);

protected:
    void paintEvent(QPaintEvent*);
    void drawButton(QPainter*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void dragEnterEvent(QDragEnterEvent*);
    void dragLeaveEvent(QDragLeaveEvent*);
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    bool startDrag(const QPoint& pos);
    void stopTimers();

    void performAction(int);
    void popupMenu(int);

    void updateFilteredTaskList();

protected Q_SLOTS:
    void animationTimerFired();
    void attentionTimerFired();
    void dragSwitch();
    void iconChanged();
    void setLastActivated();
    void taskChanged();
    void showMe();

private:
    void checkAttention(const TaskManager::Task *changed_task = NULL);
    QString                     sid;
    QTimer                      animationTimer;
    QTimer                      dragSwitchTimer;
    QTimer                      attentionTimer;
    int                         currentFrame;
    PixmapList                  frames;
    int                         attentionState;
    QRect                       iconRect;
    QPixmap                     animBg;
    QList<TaskManager::Task*>   tasks;
    QList<TaskManager::Task*>   m_filteredTasks;
    TaskManager::Task*          lastActivated;
    QMenu*                      m_menu;
    TaskManager::Startup*       m_startup;
    Qt::ArrowType               arrowType;
    TaskBar*                    taskBar;
    bool                        discardNextMouseEvent;
    bool                        aboutToActivate;
    bool                        m_mouseOver;
    enum                        { ATTENTION_BLINK_TIMEOUT = 4 };
    static QImage               blendGradient;
    QPoint                      m_dragStartPos;
};

#endif
