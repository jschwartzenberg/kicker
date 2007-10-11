/*
 *   Copyright (C) 2007 Robert Knight <robertknight@gmail.com>
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

#include "tasksengine.h"

#include <KDebug>
#include <KLocale>

#include <plasma/datacontainer.h>

using namespace Plasma;

TasksEngine::TasksEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent)
{
}
void TasksEngine::connectTask( Task::TaskPtr task )
{
        connect( task.constData() , SIGNAL(changed()) , this , SLOT(taskChanged()) );
}
void TasksEngine::init()
{
    foreach( Task::TaskPtr task , TaskManager::self()->tasks().values() ) {
        connectTask(task);
        setDataForTask(task);
    }

    connect( TaskManager::self() , SIGNAL(taskAdded(Task::TaskPtr)) , this , SLOT(taskAdded(Task::TaskPtr)) );
    connect( TaskManager::self() , SIGNAL(taskRemoved(Task::TaskPtr)) , this , SLOT(taskRemoved(Task::TaskPtr)) );

}

void TasksEngine::taskAdded(Task::TaskPtr task)
{
    connectTask(task);
    setDataForTask(task);
}
void TasksEngine::taskRemoved(Task::TaskPtr task)
{
    removeSource( QString::number(task->window()) );
}
void TasksEngine::taskChanged()
{
    Task* task = qobject_cast<Task*>(sender());

    Q_ASSERT(task);

    setDataForTask( Task::TaskPtr(task) );
}

void TasksEngine::setDataForTask(Task::TaskPtr task)
{
    Q_ASSERT( task );

    QString name = QString::number(task->window());

    const QMetaObject* metaObject = task->metaObject();

    for ( int i = 0 ; i < metaObject->propertyCount() ; i++ ) {
        QMetaProperty property = metaObject->property(i);

        setData(name,property.name(),property.read(task.constData()));
    }
}

#include "tasksengine.moc"