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
#include <yui/ncurses/NCPopupMenu.h>
#include <yui/ncurses/YNCursesUI.h>


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
      return items[0];

    for (uint i=0; i < items.size(); i++)
    {
      if (items[i]->item == selected->item)
      {
        if (i+1 < items.size())
          return items[i+1];
        else return items[i];
      }
    }

    return NULL;
  };

  __MBItem* getPrevious()
  {
    if (not selected)
      return items[0];

    for (uint i=0; i < items.size(); i++)
    {
      if (items[i]->item == selected->item)
      {
        if (i > 0)
          return items[i-1];
        else return items[i];
      }
    }

    return NULL;
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
    if (wint_t(i->hotkey) == key)
    {
      sel = i;
      break;
    }
  }
  YUI_CHECK_NEW(sel);

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
  YMGAMenuBar::addItem(item);

  __MBItem *it = new( __MBItem);
  it->item = item;
  d->items.push_back(it);

  NClabel label( NCstring( item->label() ));
  label.stripHotkey();
  unsigned int h = defsze.H > 0 ? defsze.H : 0;
  defsze = wsze( h < label.height() ? label.height() : h,
                   defsze.W + label.width() );
  yuiMilestone() <<  label << std::endl;

  item->setIndex( ++(d->nextSerialNo) );

  if ( item->hasChildren() )
        assignUniqueIndex( item->childrenBegin(), item->childrenEnd() );
}

void YMGANCMenuBar::wRedraw()
{
  if ( !win )
    return;

  unsigned int col=0;
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
    //unsigned int h = defsze.H > 0 ? defsze.H : 0;
    //defsze = wsze( h < label.height() ? label.height() : h,
    //               defsze.W + label.width() );

    if (d->selected == sel)
    {
      win->box( wrect( sel->pos, wsze( label.height(), label.width()+3) ) );
    }
    win->bkgdset( style.plain );

    win->printw( 0, col, "[" );
    sel->pos = wpos( 0, col+1 );
    sel->hotkey = label.hotkey();

    yuiWarning() <<  sel->item->label() << "  " << sel->pos << " " << sel->hotkey << " " << defsze << std::endl;

    label.drawAt( *win, style, sel->pos, wsze( -1, label.width() + 3 ), NC::CENTER );
    col = col + label.width() + 4;
    win->printw( 0, col, "]" );

    haveUtf8() ? win->add_wch( 0, col - 1, WACS_DARROW )
    : win->addch( 0, col - 1, ACS_DARROW );
    col++;

    //if (d->selected == sel)
    {
      win->bkgdset( style.scrl );
    }
    //

  }

  #if 0
  win->vline( 0, win->maxx() - 1, win->height(), ' ' );
  if ( h > 1 )
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
  // add fix heigth of 1 (dont't use win->height() because win might be invalid, bnc#931154)
  wpos at( ScreenPos() + wpos( 1, d->selected->pos.C ) );
  yuiWarning() <<  " position " << ScreenPos() << " menu position " << at <<std::endl;

  YItem *item = d->selected->item;
  NCPopupMenu * dialog = new NCPopupMenu( at, item->childrenBegin(), item->childrenEnd() );

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
