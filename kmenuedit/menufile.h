/*
 *   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as 
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef __menufile_h__
#define __menufile_h__

#include <QtXml/qdom.h>

//Added by qt3to4:
#include <Q3PtrList>

class MenuFile
{
public:
   MenuFile(const QString &file);
   ~MenuFile();
   
   bool load();
   bool save();
   void create();
   QString error() { return m_error; } // Returns the last error message

   enum ActionType {
       ADD_ENTRY = 0,
       REMOVE_ENTRY,
       ADD_MENU,
       REMOVE_MENU,
       MOVE_MENU
   };

   struct ActionAtom
   {
      ActionType action;
      QString arg1;
      QString arg2;
   };

   /**
    * Create action atom and push it on the stack
    */
   ActionAtom *pushAction(ActionType action, const QString &arg1, const QString &arg2);

   /**
    * Pop @p atom from the stack.
    * @p atom must be last item on the stack
    */
   void popAction(ActionAtom *atom);

   /**
    * Perform the specified action
    */  
   void performAction(const ActionAtom *);
   
   /**
    * Perform all actions currently on the stack, remove them from the stack and
    * save result
    * @return whether save was successful
    */
   bool performAllActions();
   
   /**
    * Returns whether the stack contains any actions
    */
   bool dirty();
   
   void addEntry(const QString &menuName, const QString &menuId);
   void removeEntry(const QString &menuName, const QString &menuId);
   
   void addMenu(const QString &menuName, const QString &menuFile);
   void moveMenu(const QString &oldMenu, const QString &newMenu);
   void removeMenu(const QString &menuName);

   void setLayout(const QString &menuName, const QStringList &layout);

   /**
    * Returns a unique menu-name for a new menu under @p menuName 
    * inspired by @p newMenu and not part of @p excludeList
    */
   QString uniqueMenuName(const QString &menuName, const QString &newMenu, const QStringList &excludeList);

protected:
   /**
    * Finds menu @p menuName in @p elem. 
    * If @p create is true, the menu is created if it doesn't exist yet.
    * @return The menu dom-node of @p menuName
    */
   QDomElement findMenu(QDomElement elem, const QString &menuName, bool create);
   
private:
   QString m_error;
   QString m_fileName;

   QDomDocument m_doc;
   bool m_bDirty;
   
   Q3PtrList<ActionAtom> m_actionList;
   QStringList m_removedEntries;
};


#endif
