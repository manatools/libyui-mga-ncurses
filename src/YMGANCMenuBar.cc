/*
 *  Copyright 2020 by Angelo Naselli <anaselli at linux dot it>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1 of the
 *  License, or (at your option) version 3.0 of the License. This library
 *  is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 *  License for more details. You should have received a copy of the GNU
 *  Lesser General Public License along with this library; if not, write
 *  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 *  Floor, Boston, MA 02110-1301 USA
 */

/*-/
 *
 *  File:         YMGANMenuBar.cc
 *
 *  Author:       Angelo Naselli <anaselli@linux.it>
 *
 * /-*/

#define  YUILogComponent "mga-ncurses"
#include <yui/YUILog.h>
#include <yui/mga/YMGAMenuBar.h>

#include <yui/YTypes.h>
#include <yui/YShortcut.h>
#include <yui/ncurses/NCurses.h>
#include "YMGANCMenuBar.h"
#include "NCMGAPopupMenu.h"
#include <yui/ncurses/YNCursesUI.h>
#include <yui/mga/YMGAMenuItem.h>
#include <yui/ncurses/NCLabel.h>


struct __MBItem
{
  YItem * item;
  wchar_t hotkey;
  wpos pos;
};

struct YMGANCMenuBar::Private
{
  std::vector<struct __MBItem*> items;
  __MBItem *selected;
  unsigned nextSerialNo;


  __MBItem* getNext()
  {
    if (not selected)
    {
        for(__MBItem *i : items)
        {
          YMGAMenuItem * mi = dynamic_cast<YMGAMenuItem *>(i->item);
          YUI_CHECK_NEW(mi);
          if (mi->enabled())
            return i;
        }
        return NULL;
    }

    bool found = false;
    for (uint i=0; i < items.size(); i++)
    {
      if (items[i]->item == selected->item)
      {
        found = true;
      }
      if (found)
      {
        if (i+1 < items.size())
        {
          YMGAMenuItem * mi = dynamic_cast<YMGAMenuItem *>(items[i+1]->item);
          YUI_CHECK_NEW(mi);
          if (mi->enabled())
            return items[i+1];
        }
      }
    }

    return selected;
  };

  __MBItem* getPrevious()
  {
    if (not selected)
    {
        for(__MBItem *i : items)
        {
          YMGAMenuItem * mi = dynamic_cast<YMGAMenuItem *>(i->item);
          YUI_CHECK_NEW(mi);
          if (mi->enabled())
            return i;
        }
        return NULL;
    }

    bool found = false;
    for (uint i=items.size()-1; i >0 ; i--)
    {
      if (items[i]->item == selected->item)
      {
        found = true;
      }
      if (found)
      {
        YMGAMenuItem * mi = dynamic_cast<YMGAMenuItem *>(items[i-1]->item);
        YUI_CHECK_NEW(mi);
        if (mi->enabled())
          return items[i-1];
      }
    }

    return selected;
  };
};

YMGANCMenuBar::YMGANCMenuBar( YWidget * parent )
: YMGAMenuBar( parent )
, NCWidget( parent )
, d(new Private)
{
  YUI_CHECK_NEW ( d );
  d->selected = NULL;
  d->nextSerialNo = 0;

  defsze= wsze(1,10);

  yuiDebug() << std::endl;
}


YMGANCMenuBar::~YMGANCMenuBar()
{
  for (__MBItem *i : d->items)
    delete i;
  d->items.clear();

  delete d;

  yuiDebug() << std::endl;
}


int YMGANCMenuBar::preferredWidth()
{
  return wGetDefsze().W;
}


int YMGANCMenuBar::preferredHeight()
{
  return wGetDefsze().H;
}



void YMGANCMenuBar::setSize( int newwidth, int newheight )
{
  wRelocate( wpos( 0 ), wsze( newheight, newwidth ) );
}

bool YMGANCMenuBar::HasHotkey(int key)
{
  yuiDebug() << key << std::endl;

  if ( key < 0 || UCHAR_MAX < key )
    return false;

  for (YItemIterator it=itemsBegin(); it!=itemsEnd(); it++)
  {
    NClabel label = NCstring((*it)->label());
    label.stripHotkey();
    yuiDebug() << label << std::endl;
    if (label.hasHotkey() && (tolower( key ) == tolower(label.hotkey())))
      return true;
  }

  return false;
}

NCursesEvent YMGANCMenuBar::wHandleHotkey( wint_t key )
{
  yuiDebug() << key << std::endl;
  NCursesEvent ret;
  __MBItem *sel = NULL;
  for (struct __MBItem *i : d->items)
  {
    if ( (tolower(i->hotkey)) == tolower(key))
    {
      sel = i;
      break;
    }
  }
  YUI_CHECK_NEW(sel);

  YMGAMenuItem *item = dynamic_cast<YMGAMenuItem *>(sel->item);
  YUI_CHECK_NEW(item);
  if (!item->enabled())
    return NCursesEvent::none;

  d->selected = sel;
  //Redraw();
  ret = postMenu();

  return ret;
}

NCursesEvent YMGANCMenuBar::wHandleInput( wint_t key )
{
  yuiDebug() << "wHandleInput " << key << std::endl;
  NCursesEvent ret;

  switch ( key )
  {
    case KEY_LEFT:
      d->selected = d->getPrevious();
      wRedraw();
      break;
    case KEY_RIGHT:
      d->selected = d->getNext();
      wRedraw();
      break;
    case KEY_HOTKEY:
    case KEY_SPACE:
    case KEY_RETURN:
    case KEY_DOWN:
      ret = postMenu();
      break;
  }

  return ret;
}


void YMGANCMenuBar::addItem(YItem* item)
{
  if (itemsBegin() == itemsEnd())
    defsze = wsze(0,0);

  YMGAMenuBar::addItem(item);

  __MBItem *it = new( __MBItem);
  it->item = item;
  d->items.push_back(it);

  NClabel label( NCstring( item->label() ));
  label.stripHotkey();

  unsigned int h = defsze.H > 0 ? defsze.H : 0;
  defsze = wsze( h < label.height() ? label.height() : h,
                   defsze.W + label.width()+5 );
  yuiDebug() <<  "label: " << label << " defsze: " << defsze << std::endl;

  item->setIndex( ++(d->nextSerialNo) );

  if ( item->hasChildren() )
        assignUniqueIndex( item->childrenBegin(), item->childrenEnd() );
}

void YMGANCMenuBar::wRedraw()
{
  if ( !win )
    return;

  int col=0;
  for ( YItemConstIterator it = itemsBegin() ; it != itemsEnd(); ++it )
  {
    __MBItem *sel = NULL;
    for (__MBItem *i : d->items)
    {
      if (i->item == *it)
      {
        sel = i;
        break;
      }
    }
    YUI_CHECK_NEW(sel);
    if (!d->selected)
      d->selected = sel;

    const NCstyle::StWidget & style( widgetStyle(d->selected != sel) );
    // first item of any YMenuItem is the menu name
    NClabel label( NCstring( (*it)->label() ));
    label.stripHotkey();
    YMGAMenuItem * mi = dynamic_cast<YMGAMenuItem *>(*it);
    bool disabled = false;

    if (mi)
    {
      if (mi->hidden())
      {
        yuiDebug() << mi->label() << " hidden" << std::endl;
        continue;
      }
      else if (!mi->enabled())
      {
        disabled = true;

        yuiDebug() << mi->label() << " disabled" << std::endl;
        win->bkgdset(wStyle().disabledList.item.plain);
      }
      else
      {
        win->bkgdset( style.plain );
      }

    }
    win->printw( 0, col, "[" );
    sel->pos = wpos( 0, col+1 );
    sel->hotkey = label.hotkey();

    yuiDebug() <<  sel->item->label() << " pos: " << sel->pos << " hotkey: " << sel->hotkey << " defsize: " << defsze << std::endl;

    if (disabled)
      label.drawAt( *win, wStyle().disabled, sel->pos, wsze( -1, label.width() + 3 ), NC::CENTER );
    else
      label.drawAt( *win, style, sel->pos, wsze( -1, label.width() + 3 ), NC::CENTER );
    col = col + label.width() + 4;
    win->printw( 0, col, "]" );

    haveUtf8() ? win->add_wch( 0, col - 1, WACS_DARROW )
    : win->addch( 0, col - 1, ACS_DARROW );
    col++;
    if (col < win->width())
    {
      win->move(0, col);
      win->bkgdset( style.plain );
      win->clrtoeol();
    }
  }

  #if 0
  win->vline( ' ', win->maxx() - col +1);
  win->vline( 0, win->maxx() - 1, win->height(), ' ' );
  if ( h > 1 )win->vline( 0, win->maxx() - 1, win->height(), ' ' );
  {
  win->box( wrect( 0, win->size() - wsze( 0, 1 ) ) );
}
#endif
}


void YMGANCMenuBar::rebuildMenuTree()
{
  // NOP
}


NCursesEvent YMGANCMenuBar::postMenu()
{
  if (!d->selected)
    return NCursesEvent::none;

  YMGAMenuItem *item = dynamic_cast<YMGAMenuItem *>(d->selected->item);
  YUI_CHECK_NEW(item);
  if (!item->enabled())
     return NCursesEvent::none;

  // add fix heigth of 1 (dont't use win->height() because win might be invalid, bnc#931154)
  wpos at( ScreenPos() + wpos( 1, d->selected->pos.C ) );
  yuiWarning() <<  " position " << ScreenPos() << " menu position " << at <<std::endl;

  NCMGAPopupMenu * dialog = new NCMGAPopupMenu( at, item->childrenBegin(), item->childrenEnd() );

  YUI_CHECK_NEW( dialog );

  int selection = dialog->post();

  if ( selection < 0 )
  {
    YDialog::deleteTopmostDialog();
    return NCursesEvent::none;
  }

  NCursesEvent ret = NCursesEvent::menu;
  ret.selection = findMenuItem( selection );
  yuiMilestone() <<  "selection " << selection << "  " << (ret.selection ? ret.selection->label() : "---") << std::endl;
  if (!ret.selection)
  {
    YDialog::deleteTopmostDialog();
    return NCursesEvent::none;
  }

  yuiMilestone() << dialog->isTopmostDialog() << std::endl;

  YDialog::deleteTopmostDialog();

  return ret;

}



YMenuItem * YMGANCMenuBar::findMenuItem( int index )
{
  return findMenuItem( index, itemsBegin(), itemsEnd() );
}


YMenuItem * YMGANCMenuBar::findMenuItem( int wantedIndex, YItemConstIterator begin, YItemConstIterator end )
{
  for ( YItemConstIterator it = begin; it != end; ++it )
  {
    YMenuItem * item = dynamic_cast<YMenuItem *> (*it);

    if ( item )
    {
      if ( item->index() == wantedIndex )
        return item;

      if ( item->hasChildren() )
      {
        YMenuItem * result = findMenuItem( wantedIndex, item->childrenBegin(), item->childrenEnd() );

        if ( result )
          return result;
      }
    }
  }

  return 0;
}

void YMGANCMenuBar::setEnabled( bool do_bv )
{
  NCWidget::setEnabled( do_bv );
}




void YMGANCMenuBar::assignUniqueIndex( YItemIterator begin, YItemIterator end )
{
    for ( YItemIterator it = begin; it != end; ++it )
    {
        YItem * item = *it;

        item->setIndex( ++(d->nextSerialNo) );

        if ( item->hasChildren() )
            assignUniqueIndex( item->childrenBegin(), item->childrenEnd() );
    }
}


void YMGANCMenuBar::enableItem(YItem* menu_item, bool enable)
{
  YMGAMenuBar::enableItem(menu_item, enable);
}

void YMGANCMenuBar::hideItem(YItem* menu_item, bool invisible)
{
  YMGAMenuBar::hideItem(menu_item, invisible);
}



static void resolveShortcutsConflictFlat(YItemConstIterator begin, YItemConstIterator end)
{
  bool used[ sizeof( char ) << 8 ];
  for ( unsigned i=0; i < sizeof( char ) << 8; i++ )
    used[i] = false;
  std::vector<YMenuItem*> conflicts;

  for ( YItemConstIterator it = begin; it != end; ++it )
  {
    YMenuItem * item = dynamic_cast<YMenuItem *> (*it);

    if ( item )
    {
      if ( item->hasChildren() )
      {
        resolveShortcutsConflictFlat(item->childrenBegin(), item->childrenEnd() );
      }

      char shortcut = YShortcut::normalized(YShortcut::findShortcut(item->label()));

      if (shortcut == 0)
      {
        conflicts.push_back(item);
        yuiMilestone() << "No or invalid shortcut found " << item->label() << endl;
      }
      else if (used[(unsigned)shortcut])
      {
        conflicts.push_back(item);
        yuiWarning() << "Conflicting shortcut found " << item->label() << endl;
      }
      else
      {
        used[(unsigned)shortcut] = true;
      }
    }
    else
    {
      yuiWarning() << "non menu item used in call " << (*it)->label() << endl;
    }
  }

  // cannot use YShortcut directly as YItem is not YWidget
  for(YMenuItem *i: conflicts)
  {
    std::string clean = YShortcut::cleanShortcutString(i->label());
    char new_c = 0;

    size_t index = 0;
    for (; index < clean.size(); ++index)
    {
      char ch = YShortcut::normalized(clean[index]);
      // ch is set to 0 by normalized if not valid
      if (ch != 0 && !used[(unsigned)ch])
      {
        new_c = ch;
        used[(unsigned)ch] = true;
        break;
      }
    }

    if (new_c != 0)
    {
      clean.insert(index, 1, YShortcut::shortcutMarker());
      yuiMilestone() << "New label used: " << clean << endl;
    }
    i->setLabel(clean);
  }
}
