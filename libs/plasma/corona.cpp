/*
 *   Copyright (C) 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
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

#include "corona.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QGraphicsView>
#include <QStringList>

#include <KAuthorized>
#include <KDebug>
#include <KLocale>
#include <KMenu>
#include <KMimeType>
#include <KRun>
#include <KWindowSystem>

#include "applet.h"
#include "dataengine.h"
#include "karambamanager.h"
#include "phase.h"
#include "widgets/vboxlayout.h"
#include "widgets/icon.h"

using namespace Plasma;

namespace Plasma
{

class Corona::Private
{
public:
    Private()
        : formFactor(Planar),
          location(Floating),
          layout(0),
          engineExplorerAction(0)
    {
    }

    ~Private()
    {
        delete layout;
        qDeleteAll(applets);
    }

    bool immutable;
    Applet::List applets;
    FormFactor formFactor;
    Location location;
    Layout* layout;
    QAction *engineExplorerAction;
};

Corona::Corona(QObject * parent)
    : QGraphicsScene(parent),
      d(new Private)
{
    init();
}

Corona::Corona(const QRectF & sceneRect, QObject * parent )
    : QGraphicsScene(sceneRect, parent),
      d(new Private)
{
    init();
}

Corona::Corona(qreal x, qreal y, qreal width, qreal height, QObject * parent)
    : QGraphicsScene(x, y, width, height, parent),
      d(new Private)
{
    init();
}

void Corona::init()
{
/*    setBackgroundMode(Qt::NoBackground);*/

/*    QPalette pal = palette();
    pal.setBrush(QPalette::Base, Qt::transparent);
    setPalette(pal);*/
    //setViewport(new QGLWidget  ( QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)   ));

    /*
    KPluginInfo::List applets = Applet::knownApplets();
    kDebug() << "======= Applets =========" << endl;
    foreach (KPluginInfo* info, applets) {
        kDebug() << info->pluginName() << ": " << info->name() << endl;
    }
    kDebug() << "=========================" << endl;
    */

//    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(displayContextMenu(QPoint)));
    d->engineExplorerAction = new QAction(i18n("Engine Explorer"), this);
    connect(d->engineExplorerAction, SIGNAL(triggered(bool)), this, SLOT(launchExplorer()));
    d->immutable = false;
//    setContextMenuPolicy(Qt::CustomContextMenu);
}

Corona::~Corona()
{
    delete d;
}

Location Corona::location() const
{
    return d->location;
}

void Corona::setLocation(Location location)
{
    if (d->location == location) {
        return;
    }

    d->location = location;

    foreach (Applet* applet, d->applets) {
        applet->constraintsUpdated();
    }
}

FormFactor Corona::formFactor() const
{
    return d->formFactor;
}

void Corona::setFormFactor(FormFactor formFactor)
{
    if (d->formFactor == formFactor) {
        return;
    }

    //kDebug() << "switching FF to " << formFactor << endl;
    d->formFactor = formFactor;
    delete d->layout;
    d->layout = 0;

    switch (d->formFactor) {
        case Planar:
            break;
        case Horizontal:
            //d->layout = new HBoxLayout;
            break;
        case Vertical:
            d->layout = new VBoxLayout;
            break;
        case MediaCenter:
            break;
        default:
            kDebug() << "This can't be happening!" << endl;
            break;
    }

    foreach (Applet* applet, d->applets) {
        applet->constraintsUpdated();
    }
}

QRectF Corona::maxSizeHint() const
{
    //FIXME: this is a bit of a naive implementation, do you think? =)
    //       we should factor in how much space we actually have left!
    return sceneRect();
}

Applet* Corona::addPlasmoid(const QString& name, const QStringList& args)
{
    Applet* applet = Applet::loadApplet(name, 0, args);
    if (applet) {
        addItem(applet);
        //applet->constraintsUpdated();
        d->applets << applet;
        connect(applet, SIGNAL(destroyed(QObject*)),
                this, SLOT(appletDestroyed(QObject*)));
        Phase::self()->animate(applet, Phase::Appear);
    } else {
        kDebug() << "Plasmoid " << name << " could not be loaded." << endl;
    }

    return applet;
}

void Corona::addKaramba(const KUrl& path)
{
    QGraphicsItemGroup* karamba = KarambaManager::loadKaramba(path, this);
    if (karamba) {
        addItem(karamba);
        Phase::self()->animate(karamba, Phase::Appear);
    } else {
        kDebug() << "Karamba " << path << " could not be loaded." << endl;
    }
}

void Corona::dragEnterEvent( QGraphicsSceneDragDropEvent *event)
{
    kDebug() << "Corona::dragEnterEvent(QGraphicsSceneDragDropEvent* event)" << endl;
    if (event->mimeData()->hasFormat("text/x-plasmoidservicename") ||
        KUrl::List::canDecode(event->mimeData())) {
        event->acceptProposedAction();
        //TODO Create the applet, move to mouse position then send the 
        //     following event to lock it to the mouse
        //QMouseEvent event(QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, event->mouseButtons(), 0);
        //QApplication::sendEvent(this, &event);
    }
    //TODO Allow dragging an applet from another Corona into this one while
    //     keeping its settings etc.
}

void Corona::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
    kDebug() << "Corona::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)" << endl;
    //TODO If an established Applet is dragged out of the Corona, remove it and
    //     create a QDrag type thing to keep the Applet's settings
}

void Corona::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
    kDebug() << "Corona::dragMoveEvent(QDragMoveEvent* event)" << endl;
}

void Corona::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    kDebug() << "Corona::dropEvent(QDropEvent* event)" << endl;
    if (event->mimeData()->hasFormat("text/x-plasmoidservicename")) {
        //TODO This will pretty much move into dragEnterEvent()
        QString plasmoidName;
        plasmoidName = event->mimeData()->data("text/x-plasmoidservicename");
        addPlasmoid(plasmoidName);
        d->applets.last()->setPos(event->pos());

        event->acceptProposedAction();
    } else if (KUrl::List::canDecode(event->mimeData())) {
        KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());	
        foreach (const KUrl& url, urls) {
            QStringList args;
            args << url.url();
            Applet* button = addPlasmoid("url", args);
            if (button) {
                //button->setSize(128,128);
                button->setPos(event->scenePos() - QPoint(button->boundingRect().width()/2,
                               button->boundingRect().height()/2));
            }
            addItem(button);
        }
        event->acceptProposedAction();
    }
}

void Corona::contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent)
{
    if (!KAuthorized::authorizeKAction("desktop_contextmenu")) {
        return;
    }

    QPointF point = contextMenuEvent->scenePos();
    /*
     * example for displaying the SuperKaramba context menu
    QGraphicsItem *item = itemAt(point);
    if(item) {
        QObject *object = dynamic_cast<QObject*>(item->parentItem());
        if(object && object->objectName().startsWith("karamba")) {
            QContextMenuEvent event(QContextMenuEvent::Mouse, point);
            contextMenuEvent(&event);
            return;
        }
    }
    */
    contextMenuEvent->accept();
    QGraphicsItem* item = itemAt(point);
    Applet* applet = 0;

    while (item) {
        applet = qgraphicsitem_cast<Applet*>(item);
        if (applet) {
            break;
        }

        item = item->parentItem();
    }

    KMenu desktopMenu;
    //kDebug() << "context menu event " << d->immutable << endl;
    if (!applet) {
        if (d->immutable) {
            return;
        }

        desktopMenu.addTitle("Plasma");
        desktopMenu.addAction(d->engineExplorerAction);
    } else if (applet->immutable()) {
        return;
    } else {
        desktopMenu.addTitle(applet->name());
        desktopMenu.addSeparator();

            QAction* configureApplet = new QAction(i18n("Configure Plasmoid..."), this);
            connect(configureApplet, SIGNAL(triggered(bool)),
                    applet, SLOT(configureDialog())); //This isn't implemented in Applet yet...
            desktopMenu.addAction(configureApplet);
        if (!d->immutable) {
            QAction* closeApplet = new QAction(i18n("Close Plasmoid"), this);
            connect(closeApplet, SIGNAL(triggered(bool)),
                    applet, SLOT(deleteLater()));
            desktopMenu.addAction(closeApplet);
        }
    }
    desktopMenu.exec(point.toPoint());
}

void Corona::launchExplorer()
{
    KRun::run("plasmaengineexplorer", KUrl::List(), 0);
}

void Corona::appletDestroyed(QObject* object)
{
    // we do a static_cast here since it really isn't an Applet by this
    // point anymore since we are in the qobject dtor. we don't actually
    // try and do anything with it, we just need the value of the pointer
    // so this unsafe looking code is actually just fine.
    Applet* applet = static_cast<Plasma::Applet*>(object);
    int index = d->applets.indexOf(applet);

    if (index > -1) {
        d->applets.removeAt(index);
    }
}

bool Corona::isImmutable()
{
    return d->immutable;
}

void Corona::setImmutable(bool immutable)
{
    if (d->immutable == immutable) {
        return;
    }

    d->immutable = immutable;
    foreach (QGraphicsItem* item, items()) {
        QGraphicsItem::GraphicsItemFlags flags = item->flags();
        if (immutable) {
            flags ^= QGraphicsItem::ItemIsMovable;
        } else {
            flags |= QGraphicsItem::ItemIsMovable;
        }
        item->setFlags(flags);
    }
}

} // namespace Plasma

#include "corona.moc"
