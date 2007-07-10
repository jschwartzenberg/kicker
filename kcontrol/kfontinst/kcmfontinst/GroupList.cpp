/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "GroupList.h"
#include "FontList.h"
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kiconloader.h>
#include <kde_file.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <ksavefile.h>
#include <kimageeffect.h>
#include <QFont>
#include <QFontDatabase>
#include <QDropEvent>
#include <QHeaderView>
#include <QMenu>
#include <QApplication>
#include <QDomElement>
#include <QTextStream>
#include <QDir>
#include <QPainter>
#include <stdlib.h>
#include <unistd.h>
#include <utime.h>
#include "FcEngine.h"
#include "Misc.h"
#include "KfiConstants.h"
#include <config-workspace.h>

namespace KFI
{

#define GROUPS_DOC "groups"
#define GROUP_TAG  "group"
#define NAME_ATTR  "name"
#define FAMILY_TAG "family"

enum EGroupColumns
{
    COL_GROUP_NAME
};

CGroupListItem::CGroupListItem(const QString &name)
              : itsName(name),
                itsType(CUSTOM),
                itsHighlighted(false),
                itsStatus(CFamilyItem::ENABLED)
{
    itsData.validated=false;
}

CGroupListItem::CGroupListItem(EType type, CGroupList *p)
              : itsType(type),
                itsHighlighted(false),
                itsStatus(CFamilyItem::ENABLED)
{
    switch(itsType)
    {
        case ALL:
            itsName=i18n("All Fonts");
            break;
        case PERSONAL:
            itsName=i18n("Personal Fonts");
            break;
        case SYSTEM:
            itsName=i18n("System Fonts");
            break;
        case STANDARD_TITLE:
            itsName=i18n("Standard:");
            break;
        case GROUPS_TITLE:
            itsName=i18n("Custom:");
            break;
        default:
            itsName=i18n("Unclassified");
    }
    itsData.parent=p;
}

bool CGroupListItem::hasFont(const CFontItem *fnt) const
{
    switch(itsType)
    {
        case CUSTOM:
            return itsFamilies.contains(fnt->family());
        case PERSONAL:
            return !fnt->isSystem();
        case SYSTEM:
            return fnt->isSystem();
        case ALL:
            return true;
        case UNCLASSIFIED:
        {
            QList<CGroupListItem *>::ConstIterator it(itsData.parent->itsGroups.begin()),
                                                   end(itsData.parent->itsGroups.end());

            for(; it!=end; ++it)
                if((*it)->isCustom() && (*it)->families().contains(fnt->family()))
                    return false;
            return true;
        }
        default:
            return false;
    }
    return false;
}

void CGroupListItem::updateStatus(QSet<QString> &enabled, QSet<QString> &disabled, QSet<QString> &partial)
{
    QSet<QString> families(itsFamilies);

    if(0!=families.intersect(partial).count())
        itsStatus=CFamilyItem::PARTIAL;
    else
    {
        families=itsFamilies;

        bool haveEnabled(0!=families.intersect(enabled).count());

        families=itsFamilies;

        bool haveDisabled(0!=families.intersect(disabled).count());

        if(haveEnabled && haveDisabled)
            itsStatus=CFamilyItem::PARTIAL;
        else if(haveEnabled && !haveDisabled)
            itsStatus=CFamilyItem::ENABLED;
        else
            itsStatus=CFamilyItem::DISABLED;
    }
}

bool CGroupListItem::load(QDomElement &elem)
{
    if(elem.hasAttribute(NAME_ATTR))
    {
        itsName=elem.attribute(NAME_ATTR);
        addFamilies(elem);
        return true;
    }
    return false;
}

bool CGroupListItem::addFamilies(QDomElement &elem)
{
    int b4(itsFamilies.count());

    for(QDomNode n=elem.firstChild(); !n.isNull(); n=n.nextSibling())
    {
        QDomElement ent=n.toElement();

        if(FAMILY_TAG==ent.tagName())
            itsFamilies.insert(ent.text());
    }
    return b4!=itsFamilies.count();
}

void CGroupListItem::save(QTextStream &str)
{
    str << " <"GROUP_TAG" "NAME_ATTR"=\"" << Misc::encodeText(itsName, str) << "\">" << endl;
    if(itsFamilies.count())
    {
        QSet<QString>::ConstIterator it(itsFamilies.begin()),
                                     end(itsFamilies.end());

        for(; it!=end; ++it)
            str << "  <"FAMILY_TAG">" << Misc::encodeText(*it, str) << "</"FAMILY_TAG">" << endl;
    }
    str << " </"GROUP_TAG">" << endl;
}

CGroupList::CGroupList(QWidget *parent)
          : QAbstractItemModel(parent),
            itsTimeStamp(0),
            itsModified(false),
            itsParent(parent),
            itsSortOrder(Qt::AscendingOrder)
{
    itsSpecialGroups[CGroupListItem::STANDARD_TITLE]=new CGroupListItem(CGroupListItem::STANDARD_TITLE, this);
    itsGroups.append(itsSpecialGroups[CGroupListItem::STANDARD_TITLE]);
    itsSpecialGroups[CGroupListItem::ALL]=new CGroupListItem(CGroupListItem::ALL, this);
    itsGroups.append(itsSpecialGroups[CGroupListItem::ALL]);
    if(Misc::root())
        itsSpecialGroups[CGroupListItem::PERSONAL]=
        itsSpecialGroups[CGroupListItem::SYSTEM]=NULL;
    else
    {
        itsSpecialGroups[CGroupListItem::PERSONAL]=new CGroupListItem(CGroupListItem::PERSONAL, this);
        itsGroups.append(itsSpecialGroups[CGroupListItem::PERSONAL]);
        itsSpecialGroups[CGroupListItem::SYSTEM]=new CGroupListItem(CGroupListItem::SYSTEM, this);
        itsGroups.append(itsSpecialGroups[CGroupListItem::SYSTEM]);
    }
    itsSpecialGroups[CGroupListItem::UNCLASSIFIED]=
                new CGroupListItem(CGroupListItem::UNCLASSIFIED, this);
    itsGroups.append(itsSpecialGroups[CGroupListItem::UNCLASSIFIED]);
    itsSpecialGroups[CGroupListItem::GROUPS_TITLE]=new CGroupListItem(CGroupListItem::GROUPS_TITLE, this);
    itsGroups.append(itsSpecialGroups[CGroupListItem::GROUPS_TITLE]);
    // Locate groups.xml file - normall will be ~/.config/fontgroups.xml
    QString path(KGlobal::dirs()->localxdgconfdir());

    if(!Misc::dExists(path))
        Misc::createDir(path);

    itsFileName=path+'/'+KFI_GROUPS_FILE;

    rescan();
}

CGroupList::~CGroupList()
{
    save();
    qDeleteAll(itsGroups);
    itsGroups.clear();
}

int CGroupList::columnCount(const QModelIndex &) const
{
    return 1;
}

void CGroupList::update(const QModelIndex &unHighlight, const QModelIndex &highlight)
{
    if(unHighlight.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(unHighlight.internalPointer());
        if(grp)
            grp->setHighlighted(false);
        emit dataChanged(unHighlight, unHighlight);
    }
    if(highlight.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(highlight.internalPointer());
        if(grp)
            grp->setHighlighted(true);
        emit dataChanged(highlight, highlight);
    }
}

void CGroupList::updateStatus(QSet<QString> &enabled, QSet<QString> &disabled,
                              QSet<QString> &partial)
{
    QList<CGroupListItem *>::Iterator it(itsGroups.begin()),
                                      end(itsGroups.end());

    for(; it!=end; ++it)
        if((*it)->isCustom())
            (*it)->updateStatus(enabled, disabled, partial);

    emit layoutChanged();
}

inline QColor midColour(const QColor &a, const QColor &b)
{
    return QColor((a.red()+b.red())>>1, (a.green()+b.green())>>1, (a.blue()+b.blue())>>1);
}

QVariant CGroupList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    CGroupListItem *grp=static_cast<CGroupListItem *>(index.internalPointer());

    if(grp)
        switch(index.column())
        {
            case COL_GROUP_NAME:
                switch(role)
                {
                    case Qt::BackgroundRole:
                        switch(grp->type())
                        {
                            case CGroupListItem::STANDARD_TITLE:
                            case CGroupListItem::GROUPS_TITLE:
                            {
                                QPalette pal(QApplication::palette());

                                return midColour(pal.color(QPalette::Active, QPalette::Base),
                                                 pal.color(QPalette::Active, QPalette::Highlight));
                            }
                            default:
                                break;
                        }
                        break;
                    case Qt::FontRole:
                        switch(grp->type())
                        {
                            case CGroupListItem::STANDARD_TITLE:
                            case CGroupListItem::GROUPS_TITLE:
                            {
                                QFont font;
                                font.setPointSizeF(font.pointSizeF()*0.75);
                                font.setBold(true);
                                return font;
                            }
                            default:
                                break;
                        }
                        break;
                    case Qt::DisplayRole:
                        if(grp->isCustom())
                            if(CFamilyItem::DISABLED==grp->status())
                                if(grp->families().count())
                                    return i18n("%1 (Disabled)", grp->name());
                                else
                                    return i18n("%1 (Empty)", grp->name());
                            else if(CFamilyItem::PARTIAL==grp->status())
                                return i18n("%1 (Partial)", grp->name());
                        return grp->name();
                    case Qt::DecorationRole:
                        if(grp->highlighted())
                            switch(grp->type())
                            {
                                case CGroupListItem::ALL:      // Removing from a group
                                    return SmallIcon("list-remove");
                                case CGroupListItem::PERSONAL: // Copying/moving
                                case CGroupListItem::SYSTEM:   // Copying/moving
                                    return SmallIcon(Qt::LeftToRight==QApplication::layoutDirection()
                                                        ? "arrow-right" : "arrow-left");
                                case CGroupListItem::CUSTOM:   // Addint to a group
                                    return SmallIcon("list-add");
                                default:
                                    break;
                            }
                    default:
                        break;
                }
                break;
        }
    return QVariant();
}

Qt::ItemFlags CGroupList::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    CGroupListItem *grp=static_cast<CGroupListItem *>(index.internalPointer());

    if(grp)
        switch(grp->type())
        {
            case CGroupListItem::STANDARD_TITLE:
            case CGroupListItem::GROUPS_TITLE:
                return Qt::ItemIsEnabled;
            default:
                break;
        }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;
}

QVariant CGroupList::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::Horizontal==orientation && COL_GROUP_NAME==section)
        switch(role)
        {
            case Qt::DisplayRole:
                return i18n("Group");
            case Qt::TextAlignmentRole:
                return Qt::AlignLeft;
            case Qt::WhatsThisRole:
                return whatsThis();
            default:
                break;
        }

    return QVariant();
}

QModelIndex CGroupList::index(int row, int column, const QModelIndex &parent) const
{
    if(!parent.isValid())
    {
        CGroupListItem *grp=itsGroups.value(row);

        if(grp)
            return createIndex(row, column, grp);
    }

    return QModelIndex();
}

QModelIndex CGroupList::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int CGroupList::rowCount(const QModelIndex &) const
{
    return itsGroups.count();
}

void CGroupList::rescan()
{
    save();
    load();
    sort(0, itsSortOrder);
}

void CGroupList::load()
{
    time_t ts=Misc::getTimeStamp(itsFileName);

    if(!ts || ts!=itsTimeStamp)
    {
        clear();
        itsTimeStamp=ts;
        if(load(itsFileName))
            itsModified=false;
    }
}

bool CGroupList::load(const QString &file)
{
    QFile f(file);
    bool  rv(false);

    if(f.open(QIODevice::ReadOnly))
    {
        QDomDocument doc;

        if(doc.setContent(&f))
            for(QDomNode n=doc.documentElement().firstChild(); !n.isNull(); n=n.nextSibling())
            {
                QDomElement e=n.toElement();

                if(GROUP_TAG==e.tagName() && e.hasAttribute(NAME_ATTR))
                {
                    QString name(e.attribute(NAME_ATTR));

                    CGroupListItem *item=find(name);

                    if(!item)
                    {
                        item=new CGroupListItem(name);
                        itsGroups.append(item);
                        rv=true;
                    }

                    if(item->addFamilies(e))
                        rv=true;
                }
            }
    }
    return rv;
}

bool CGroupList::save()
{
    if(itsModified && save(itsFileName, NULL))
    {
        itsTimeStamp=Misc::getTimeStamp(itsFileName);
        return true;
    }
    return false;
}

bool CGroupList::save(const QString &fileName, CGroupListItem *grp)
{
    KSaveFile file(fileName);

    if(file.open())
    {
        QTextStream str(&file);

        str << "<"GROUPS_DOC">" << endl;

        if(grp)
            grp->save(str);
        else
        {
            QList<CGroupListItem *>::Iterator it(itsGroups.begin()),
                                              end(itsGroups.end());

            for(; it!=end; ++it)
                if((*it)->isCustom())
                    (*it)->save(str);
        }
        str << "</"GROUPS_DOC">" << endl;
        itsModified=false;
        return file.finalize();
    }

    return false;
}

void CGroupList::merge(const QString &file)
{
    if(load(file))
    {
        itsModified=true;
        sort(0, itsSortOrder);
    }
}

void CGroupList::clear()
{
    beginRemoveRows(QModelIndex(), 0, itsGroups.count());
    endRemoveRows();
    itsGroups.removeFirst(); // Remove STANDARD_TITLE
    itsGroups.removeFirst(); // Remove all
    if(itsSpecialGroups[CGroupListItem::SYSTEM])
    {
        itsGroups.removeFirst(); // Remove personal
        itsGroups.removeFirst(); // Remove system
    }
    itsGroups.removeFirst(); // Remove unclassif...
    itsGroups.removeFirst(); // Remove GROUPS_TITLE
    qDeleteAll(itsGroups);
    itsGroups.clear();
    itsGroups.append(itsSpecialGroups[CGroupListItem::STANDARD_TITLE]);
    itsGroups.append(itsSpecialGroups[CGroupListItem::ALL]);
    if(itsSpecialGroups[CGroupListItem::SYSTEM])
    {
        itsGroups.append(itsSpecialGroups[CGroupListItem::PERSONAL]);
        itsGroups.append(itsSpecialGroups[CGroupListItem::SYSTEM]);
    }
    itsGroups.append(itsSpecialGroups[CGroupListItem::UNCLASSIFIED]);
    itsGroups.append(itsSpecialGroups[CGroupListItem::GROUPS_TITLE]);
}

QModelIndex CGroupList::index(CGroupListItem::EType t)
{
    return createIndex(t, 0, itsSpecialGroups[t]);
}

void CGroupList::createGroup(const QString &name)
{
    if(!exists(name))
    {
        itsGroups.append(new CGroupListItem(name));
        itsModified=true;
        save();
        sort(0, itsSortOrder);
    }
}

void CGroupList::renameGroup(const QModelIndex &idx, const QString &name)
{
    if(idx.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(idx.internalPointer());

        if(grp && grp->isCustom() && grp->name()!=name && !exists(name))
        {
            grp->setName(name);
            itsModified=true;
            save();
            sort(0, itsSortOrder);
        }
    }
}

bool CGroupList::removeGroup(const QModelIndex &idx)
{
    if(idx.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(idx.internalPointer());

        if(grp && grp->isCustom() &&
           KMessageBox::Yes==KMessageBox::warningYesNo(itsParent,
                                          i18n("<p>Do you really want to remove \'<b>%1</b>\'?</p>"
                                               "<p><i>This will only remove the group, and not "
                                               "the actual fonts.</i></p>", grp->name()),
                                          i18n("Remove Group"), KGuiItem(i18n("Remove"), "list-remove",
                                          i18n("Remove group"))))
        {
            itsModified=true;
            itsGroups.removeAll(grp);
            delete grp;
            save();
            sort(0, itsSortOrder);
            return true;
        }
    }

    return false;
}

void CGroupList::removeFromGroup(const QModelIndex &group, const QSet<QString> &families)
{
    if(group.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(group.internalPointer());

        if(grp && grp->isCustom())
        {
            QSet<QString>::ConstIterator it(families.begin()),
                                         end(families.end());
            bool                         update(false);

            for(; it!=end; ++it)
                if(removeFromGroup(grp, *it))
                    update=true;

            if(update)
                emit refresh();
        }
    }
}

QString CGroupList::whatsThis() const
{
    return i18n("<h3>Font Groups</h3><p>This list displays the font groups available on your system. "
                                       "There are 2 main types of font groups:"
               "<ul><li><b>Standard</b> are special groups used by the font manager.<ul>%1</ul></li>"
                   "<li><b>Custom</b> are groups created by you. To add a font family to one of "
                                     "these groups simply drag it from the list of fonts, and drop "
                                     "onto the desired group. To remove a family from the group, drag "
                                     "the font onto the \"All Fonts\" group.</li>"
                   "</ul></p>",
                Misc::root()
                    ? i18n("<li><i>All Fonts</i> contains all the fonts installed on your system.</li>"
                            "<li><i>Unclassified</i> contains all fonts that have not yet been placed "
                                                    "within a \"Custom\" group.</li>")
                    : i18n("<li><i>All Fonts</i> contains all the fonts installed on your system - "
                                                "both  \"System\" and \"Personal\".</li>"
                            "<li><i>System</i> contains all fonts that are installed system-wide (i.e. "
                                            "available to all users).</li>"
                            "<li><i>Personal</i> contains your personal fonts.</li>"
                            "<li><i>Unclassified</i> contains all fonts that have not yet been placed "
                                                    "within a \"Custom\" group.</li>"));
}

void CGroupList::addToGroup(const QModelIndex &group, const QSet<QString> &families)
{
    if(group.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(group.internalPointer());

        if(grp && grp->isCustom())
        {
            QSet<QString>::ConstIterator it(families.begin()),
                                         end(families.end());
            bool                         update(false);

            for(; it!=end; ++it)
                if(!grp->hasFamily(*it))
                {
                    grp->addFamily(*it);
                    update=true;
                    itsModified=true;
                }

            if(update)
                emit refresh();
        }
    }
}

void CGroupList::removeFamily(const QString &family)
{
    QList<CGroupListItem *>::ConstIterator it(itsGroups.begin()),
                                           end(itsGroups.end());

    for(; it!=end; ++it)
        removeFromGroup(*it, family);
}

bool CGroupList::removeFromGroup(CGroupListItem *grp, const QString &family)
{
    if(grp && grp->isCustom() && grp->hasFamily(family))
    {
        grp->removeFamily(family);
        itsModified=true;
        return true;
    }

    return false;
}

static bool groupNameLessThan(const CGroupListItem *f1, const CGroupListItem *f2)
{
    return f1 && f2 && (f1->type()<f2->type() ||
                       (f1->type()==f2->type() && QString::localeAwareCompare(f1->name(), f2->name())<0));
}

static bool groupNameGreaterThan(const CGroupListItem *f1, const CGroupListItem *f2)
{
    return f1 && f2 && (f1->type()<f2->type() ||
                       (f1->type()==f2->type() && QString::localeAwareCompare(f1->name(), f2->name())>0));
}

void CGroupList::sort(int, Qt::SortOrder order)
{
    itsSortOrder=order;

    qSort(itsGroups.begin(), itsGroups.end(),
          Qt::AscendingOrder==order ? groupNameLessThan : groupNameGreaterThan);

    emit layoutChanged();
}

Qt::DropActions CGroupList::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList CGroupList::mimeTypes() const
{
    QStringList types;
    types << KFI_FONT_DRAG_MIME;
    return types;
}

CGroupListItem * CGroupList::find(const QString &name)
{
    QList<CGroupListItem *>::ConstIterator it(itsGroups.begin()),
                                           end(itsGroups.end());

    for(; it!=end; ++it)
        if((*it)->name()==name)
            return (*it);

    return NULL;
}

bool CGroupList::exists(const QString &name)
{
    if(NULL!=find(name))
    {
        KMessageBox::error(itsParent, i18n("<qt>A group named <b>\'%1\'</b> already "
                                           "exists!</qt>", name));
        return true;
    }

    return false;
}

CGroupListView::CGroupListView(QWidget *parent, CGroupList *model)
              : QTreeView(parent)
{
    setModel(model);
    sortByColumn(COL_GROUP_NAME, Qt::AscendingOrder);
    setSelectionMode(QAbstractItemView::SingleSelection);
    resizeColumnToContents(COL_GROUP_NAME);
    setSortingEnabled(true);
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DropOnly);
    setDropIndicatorShown(true);
    setDragEnabled(false);
    header()->setSortIndicatorShown(true);
    setRootIsDecorated(false);
    itsMenu=new QMenu(this);

    itsDeleteAct=itsMenu->addAction(KIcon("list-remove"), i18n("Remove..."),
                                    this, SIGNAL(del()));
    itsEnableAct=itsMenu->addAction(KIcon("enablefont"), i18n("Enable..."),
                                    this, SIGNAL(enable()));
    itsDisableAct=itsMenu->addAction(KIcon("disablefont"), i18n("Disable..."),
                                     this, SIGNAL(disable()));
    itsMenu->addSeparator();
    itsRenameAct=itsMenu->addAction(i18n("Rename..."), this, SLOT(rename()));
    itsMenu->addSeparator();
    itsPrintAct=itsMenu->addAction(KIcon("document-print"), i18n("Print..."),
                                   this, SIGNAL(print()));

    itsActionMenu=new QMenu(this);
    itsActionMenu->addAction(KIcon("goto-page"), i18n("Move Here"), this, SIGNAL(moveFonts()));
    itsActionMenu->addAction(KIcon("edit-copy"), i18n("Copy Here"), this, SIGNAL(copyFonts()));
    itsActionMenu->addSeparator();
    itsActionMenu->addAction(KIcon("process-stop"), i18n("Cancel"));

    setWhatsThis(model->whatsThis());
    header()->setWhatsThis(whatsThis());
    connect(this, SIGNAL(addFamilies(const QModelIndex &,  const QSet<QString> &)),
            model, SLOT(addToGroup(const QModelIndex &,  const QSet<QString> &)));
    connect(this, SIGNAL(removeFamilies(const QModelIndex &,  const QSet<QString> &)),
            model, SLOT(removeFromGroup(const QModelIndex &,  const QSet<QString> &)));
}

CGroupListItem::EType CGroupListView::getType()
{
    QModelIndexList selectedItems(selectedIndexes());

    if(selectedItems.count() && selectedItems.last().isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(selectedItems.last().internalPointer());

        return grp->type();
    }

    return CGroupListItem::ALL;
}

void CGroupListView::controlMenu(bool del, bool en, bool dis, bool p)
{
    itsDeleteAct->setEnabled(del);
    itsRenameAct->setEnabled(del);
    itsEnableAct->setEnabled(en);
    itsDisableAct->setEnabled(dis);
    itsPrintAct->setEnabled(p);
}

void CGroupListView::selectionChanged(const QItemSelection &selected,
                                      const QItemSelection &deselected)
{
    QModelIndexList deselectedItems(deselected.indexes());

    QAbstractItemView::selectionChanged(selected, deselected);

    QModelIndexList selectedItems(selectedIndexes());

    if(0==selectedItems.count() && 1==deselectedItems.count())
        selectionModel()->select(deselectedItems.last(), QItemSelectionModel::Select);
    else
        emit itemSelected(selectedItems.count()
                            ? selectedItems.last()
                            : QModelIndex());
}

void CGroupListView::rename()
{
    QModelIndex index(currentIndex());

    if(index.isValid())
    {
        CGroupListItem *grp=static_cast<CGroupListItem *>(index.internalPointer());

        if(grp && grp->isCustom())
        {
            bool    ok;
            QString name(KInputDialog::getText(i18n("Rename Group"),
                                               i18n("Please enter a new name for group:"),
                                               grp->name(), &ok, this));

            if(ok && !name.isEmpty() && name!=grp->name())
                ((CGroupList *)model())->renameGroup(index, name);
        }
    }
}

void CGroupListView::contextMenuEvent(QContextMenuEvent *ev)
{
    if(indexAt(ev->pos()).isValid())
        itsMenu->popup(ev->globalPos());
}

void CGroupListView::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->provides(KFI_FONT_DRAG_MIME))
        event->acceptProposedAction();
}

void CGroupListView::dragMoveEvent(QDragMoveEvent *event)
{
    if(event->provides(KFI_FONT_DRAG_MIME))
    {
        QModelIndex index(indexAt(event->pos()));

        if(index.isValid())
        {
            if(COL_GROUP_NAME!=index.column())
                index=((CGroupList *)model())->createIdx(index.row(), COL_GROUP_NAME, index.internalPointer());

            CGroupListItem        *dest=static_cast<CGroupListItem *>(index.internalPointer());
            CGroupListItem::EType type=getType();

            if(dest)
                if(!selectedIndexes().contains(index))
                {
                    bool ok(true);

                    if(dest->isCustom())
                        emit info(i18n("Drop here to add the selected fonts to \"%1\".", dest->name()));
                    else if(CGroupListItem::CUSTOM==type && dest->isAll())
                        emit info(i18n("Drop here to remove the selected fonts from the current group."));
                    else if(!Misc::root() && dest->isPersonal() && CGroupListItem::SYSTEM==type)
                        emit info(i18n("Drop here to copy, or move, the selected fonts to your peronal folder."));
                    else if(!Misc::root() && dest->isSystem() && CGroupListItem::PERSONAL==type)
                        emit info(i18n("Drop here to copy, or move, the selected fonts to the system folder."));
                    else
                        ok=false;

                    if(ok)
                    {
                        drawHighlighter(index);
                        event->acceptProposedAction();
                        return;
                    }
                }
        }
        event->ignore();
        drawHighlighter(QModelIndex());
        emit info(QString());
    }
}

void CGroupListView::dragLeaveEvent(QDragLeaveEvent *)
{
    drawHighlighter(QModelIndex());
    emit info(QString());
}

void CGroupListView::dropEvent(QDropEvent *event)
{
    emit info(QString());
    drawHighlighter(QModelIndex());
    if(event->provides(KFI_FONT_DRAG_MIME))
    {
        event->acceptProposedAction();

        QSet<QString> families;
        QByteArray    encodedData(event->mimeData()->data(KFI_FONT_DRAG_MIME));
        QDataStream   ds(&encodedData, QIODevice::ReadOnly);
        QModelIndex   from(selectedIndexes().last()),
                      to(indexAt(event->pos()));

        ds >> families;
        // Are we mvoeing/copying, removing a font from the current group?
        if(to.isValid() && from.isValid())
            if( ((static_cast<CGroupListItem *>(from.internalPointer()))->isSystem() &&
                 (static_cast<CGroupListItem *>(to.internalPointer()))->isPersonal()) ||
                ((static_cast<CGroupListItem *>(from.internalPointer()))->isPersonal() &&
                 (static_cast<CGroupListItem *>(to.internalPointer()))->isSystem()))
                itsActionMenu->popup(QCursor::pos());
            else if((static_cast<CGroupListItem *>(from.internalPointer()))->isCustom() &&
                    !(static_cast<CGroupListItem *>(to.internalPointer()))->isCustom())
                emit removeFamilies(from, families);
            else
                emit addFamilies(to, families);

        if(isUnclassified())
            emit unclassifiedChanged();
    }
}

void CGroupListView::drawHighlighter(const QModelIndex &idx)
{
    if(itsCurrentDropItem!=idx)
    {
        ((CGroupList *)model())->update(itsCurrentDropItem, idx);
        itsCurrentDropItem=idx;
    }
}

}

#include "GroupList.moc"
