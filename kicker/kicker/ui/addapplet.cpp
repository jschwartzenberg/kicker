/*****************************************************************

Copyright (c) 2007, 2006 Rafael Fernández López <ereslibre@kde.org>
Copyright (c) 2005 Marc Cramdal
Copyright (c) 2005 Aaron Seigo <aseigo@kde.org>

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

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPalette>
#include <QTimer>
#include <Qt3Support/q3tl.h>

#include <QMimeData>
#include <QMouseEvent>
#include <QPixmap>
#include <QVBoxLayout>
#include <QEvent>
#include <QCloseEvent>

#include <kicon.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <KStandardGuiItem>

#include "addapplet.h"
#include "addappletvisualfeedback.h"
#include "container_applet.h"
#include "container_extension.h"
#include "containerarea.h"
#include "kicker.h"
#include "kickerSettings.h"
#include "menuinfo.h"
#include "pluginmanager.h"

AddAppletDialog::AddAppletDialog(ContainerArea *cArea,
                                 QWidget *parent,
                                 const char *name)
    : KDialog(parent)
    , m_mainWidgetView(new Ui::AppletView())
    , m_containerArea(cArea)
    , m_insertionPoint(Kicker::self()->insertionPoint())
{
    setCaption(i18n("Add Applet"));
    Q_UNUSED(name);
    setModal(false);

    setButtons(KDialog::User1 | KDialog::Close);
    setButtonGuiItem(User1, KGuiItem(i18n("Load Applet"), "ok"));
    enableButton(KDialog::User1, false);

    KConfigGroup cg = KGlobal::config()->group( "AddAppletDialog Settings");
    restoreDialogSize(cg);

    centerOnScreen(this);

    m_mainWidget = new QWidget(this);
    m_mainWidgetView->setupUi(m_mainWidget);
    setMainWidget(m_mainWidget);

    connect(m_mainWidgetView->appletSearch, SIGNAL(textChanged(const QString&)), this, SLOT(search(const QString&)));
    connect(m_mainWidgetView->appletFilter, SIGNAL(activated(int)), this, SLOT(filter(int)));
    connect(m_mainWidgetView->appletListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(selectApplet(const QModelIndex&)));
    connect(m_mainWidgetView->appletListView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(addCurrentApplet(const QModelIndex&)));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotUser1Clicked()));
    connect(PluginManager::self(), SIGNAL(pluginDestroyed()), this, SLOT(updateAppletList()));

    m_selectedType = AppletInfo::Undefined;

    m_mainWidgetView->appletListView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_mainWidgetView->appletSearch->setClearButtonShown(true);

    QTimer::singleShot(0, this, SLOT(populateApplets()));
}

void AddAppletDialog::updateInsertionPoint()
{
    m_insertionPoint = Kicker::self()->insertionPoint();
}

void AddAppletDialog::closeEvent(QCloseEvent *e)
{
    KConfigGroup cg( KGlobal::config(), "AddAppletDialog Settings");
    KDialog::saveDialogSize( cg );

    KDialog::closeEvent(e);
}

void AddAppletDialog::populateApplets()
{
    // Loading applets
    m_applets = PluginManager::applets(false, &m_applets);

    // Loading built in buttons
    m_applets = PluginManager::builtinButtons(false, &m_applets);

    // Loading special buttons
    m_applets = PluginManager::specialButtons(false, &m_applets);

    qHeapSort(m_applets);

    foreach(AppletInfo appletInfo, m_applets)
    {
        if (appletInfo.isHidden() || appletInfo.name().isEmpty())
            m_applets.erase(&appletInfo);
    }

    m_listModel = new AppletListModel(m_applets, this);
    m_mainWidgetView->appletListView->setModel(m_listModel);

    int i = 0;
    foreach(AppletInfo appletInfo, m_applets)
    {
        if (appletInfo.isUniqueApplet() && PluginManager::self()->hasInstance(appletInfo))
            m_mainWidgetView->appletListView->setRowHidden(i, true);

        i++;
    }

    AppletItemDelegate *appletItemDelegate = new AppletItemDelegate(this);
    appletItemDelegate->setIconSize(48, 48);
    appletItemDelegate->setMinimumItemWidth(200);
    appletItemDelegate->setLeftMargin(20);
    appletItemDelegate->setRightMargin(0);
    appletItemDelegate->setSeparatorPixels(20);
    m_mainWidgetView->appletListView->setItemDelegate(appletItemDelegate);
}

void AddAppletDialog::selectApplet(const QModelIndex &applet)
{
    selectedApplet = applet;

    if (!isButtonEnabled(KDialog::User1))
        enableButton(KDialog::User1, true);
}

void AddAppletDialog::addCurrentApplet(const QModelIndex &selectedApplet)
{
    this->selectedApplet = selectedApplet;
    AppletInfo applet(m_applets[selectedApplet.row()]);

    QPoint prevInsertionPoint = Kicker::self()->insertionPoint();
    Kicker::self()->setInsertionPoint(m_insertionPoint);

    const QWidget* appletContainer = 0;

    if (applet.type() == AppletInfo::Applet)
    {
        appletContainer = m_containerArea->addApplet(applet);
    }
    else if (applet.type() & AppletInfo::Button)
    {
        appletContainer = m_containerArea->addButton(applet);
    }

    if (appletContainer)
    {
        ExtensionContainer* ec = dynamic_cast<ExtensionContainer*>(m_containerArea->topLevelWidget());

        if (ec)
        {
            // unhide the panel and keep it unhidden for at least the time the
            // helper tip will be there
            ec->unhideIfHidden(KickerSettings::mouseOversSpeed() + 2500);
        }

        new AddAppletVisualFeedback(selectedApplet,
                                    m_mainWidgetView->appletListView,
                                    appletContainer,
                                    m_containerArea->popupDirection());
    }

    if (applet.isUniqueApplet() && PluginManager::self()->hasInstance(applet))
    {
        m_mainWidgetView->appletListView->setRowHidden(selectedApplet.row(), true);
        m_mainWidgetView->appletListView->clearSelection();
        enableButton(KDialog::User1, false);
    }

    Kicker::self()->setInsertionPoint(prevInsertionPoint);
}

bool AddAppletDialog::appletMatchesSearch(const AppletInfo *i, const QString &s)
{
    if (i->type() == AppletInfo::Applet &&
        i->isUniqueApplet() && PluginManager::self()->hasInstance(*i))
    {
        return false;
    }

    return (m_selectedType == AppletInfo::Undefined ||
            i->type() & m_selectedType) &&
            (i->name().contains(s, Qt::CaseInsensitive) ||
             i->comment().contains(s, Qt::CaseInsensitive));
}

void AddAppletDialog::search(const QString &s)
{
    AppletInfo *appletInfo;
    for (int i = 0; i < m_listModel->rowCount(); i++)
    {
        appletInfo = static_cast<AppletInfo*>(m_listModel->index(i).internalPointer());
        m_mainWidgetView->appletListView->setRowHidden(i, !appletMatchesSearch(appletInfo, s) ||
                                                          (appletInfo->isUniqueApplet() &&
                                                           PluginManager::self()->hasInstance(*appletInfo)));
    }

    /**
      * If our selection gets hidden because of searching, we deselect it and
      * disable the "Add Applet" button.
      */
    if ((selectedApplet.isValid() &&
        (m_mainWidgetView->appletListView->isRowHidden(selectedApplet.row()))) ||
        (!selectedApplet.isValid()))
    {
        m_mainWidgetView->appletListView->clearSelection();
        enableButton(KDialog::User1, false);
    }
}

void AddAppletDialog::filter(int i)
{
    m_selectedType = AppletInfo::Undefined;

    if (i == 1)
    {
        m_selectedType = AppletInfo::Applet;
    }
    else if (i == 2)
    {
        m_selectedType = AppletInfo::Button;
    }

    AppletInfo *appletInfo;
    QString searchString = m_mainWidgetView->appletSearch->text();
    for (int j = 0; j < m_listModel->rowCount(); j++)
    {
        appletInfo = static_cast<AppletInfo*>(m_listModel->index(j).internalPointer());
        m_mainWidgetView->appletListView->setRowHidden(j, !appletMatchesSearch(appletInfo, searchString) ||
                                                          (appletInfo->isUniqueApplet() &&
                                                           PluginManager::self()->hasInstance(*appletInfo)));
    }

    /**
      * If our selection gets hidden because of filtering, we deselect it and
      * disable the "Add Applet" button.
      */
    if ((selectedApplet.isValid() &&
        (m_mainWidgetView->appletListView->isRowHidden(selectedApplet.row()))) ||
        (!selectedApplet.isValid()))
    {
        m_mainWidgetView->appletListView->clearSelection();
        enableButton(KDialog::User1, false);
    }
}

void AddAppletDialog::slotUser1Clicked()
{
    if (selectedApplet.isValid())
    {
        addCurrentApplet(selectedApplet);
    }
}

void AddAppletDialog::updateAppletList()
{
    search(m_mainWidgetView->appletSearch->text());
}

#include "addapplet.moc"
