/*
  Copyright 2020 by Angelo Naselli <anaselli at linux dot it>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) version 3.0 of the License. This library
  is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
  License for more details. You should have received a copy of the GNU
  Lesser General Public License along with this library; if not, write
  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
  Floor, Boston, MA 02110-1301 USA

*/


/*-/

   File:       NCMGAPopupMenu.cc

   Author:     Angelo Naselli <anaselli@linux.it>

/-*/

#define  YUILogComponent "mga-ncurses"
#include <yui/YUILog.h>
#include "NCMGAPopupMenu.h"
#include "YMGAMenuItem.h"
#include "NCMenu.h"
#include <yui/ncurses/NCTable.h>

struct NCMGAPopupMenu::Private
{
    NCMenu *menu;
    unsigned maxlen;
    wpos pos;
    bool selected;

    //std::vector<YMGAMenuItem *> items;
    std::map<YMGAMenuItem *, YMGAMenuItem *> itemsMap;
};


NCMGAPopupMenu::NCMGAPopupMenu( const wpos & at, YItemIterator begin, YItemIterator end )
    : NCPopup( at )
    , d(new Private)
{
    YUI_CHECK_NEW ( d );



    std::vector<std::string> row( 2 );

    d->menu = new NCMenu( this );
    d->maxlen = 0;
    d->pos = at;
    d->selected = false;
    //d->menu->setNotify(true);

    yuiDebug() << "Menu position: " << at << std::endl;

    defsze = wsze(1, 1);
    for ( YItemIterator it = begin; it != end; ++it )
    {
        YMGAMenuItem * item = dynamic_cast<YMGAMenuItem *>( *it );
        YUI_CHECK_PTR( item );

        std::string label = item->hasChildren() ? item->label() + " >" : item->label();
        YMGAMenuItem *menuItem = new YMGAMenuItem ( label, item->iconName() );
        menuItem->enable(item->enabled());


        d->menu->addItem( menuItem );
        d->itemsMap[menuItem] = item;

        if (d->maxlen < label.length()+1)
            d->maxlen = label.length()+1;
        // let's assume to have a menu enable scrolling for more than 10 lines
        int h = defsze.H > 9 ? 10 : defsze.H + 1;
        // let's assume to have a menu enable scrolling for more than 40 columns
        int w = d->maxlen > 40 ? 40 : d->maxlen;

        defsze = wsze( h,  w );
        yuiDebug() << "Add Item: " << item->label()  << " len: "<< label.length() << " HxW "<< h << "x" << w << std::endl;
    }
    yuiDebug() << "defsze: " << defsze << "line length: " << d->maxlen << std::endl;

    //d->menu->stripHotkeys();
}


NCMGAPopupMenu::~NCMGAPopupMenu()
{
    d->itemsMap.clear();
    delete d;
}

int NCMGAPopupMenu::preferredHeight()
{
  return defsze.H + 2; // border + scroll
}

int NCMGAPopupMenu::preferredWidth()
{
  return defsze.W + 2; // border + scroll
}


bool NCMGAPopupMenu::HasHotkey(int key)
{
  yuiDebug() << key << std::endl;

  return d->menu->HasHotkey(key);
}

NCursesEvent NCMGAPopupMenu::wHandleHotkey( wint_t key )
{
    yuiDebug() << "Key: " << key << std::endl;
    if ( key >= 0 && key < UCHAR_MAX ) //  < myPad()->setItemByKey( key ) )
    {
      NCursesEvent ev = d->menu->wHandleHotkey(key);
      yuiDebug() << "event: " << ev << std::endl;
      if (ev != NCursesEvent::none)
        return wHandleInput( KEY_SPACE );
    }
    return NCursesEvent::none;
}


NCursesEvent NCMGAPopupMenu::wHandleInput( wint_t ch )
{
    NCursesEvent ret;
    d->selected = false;

    yuiDebug() << "ch: " << int(ch)  << std::endl;
    switch ( ch )
    {
      case KEY_RIGHT:
        {
          YMGAMenuItem * selitem = dynamic_cast<YMGAMenuItem *>(d->menu->currentItem());
          if (selitem)
          {
              YMGAMenuItem * item = d->itemsMap[ selitem ];
              if (item->hasChildren() )
              {
                  ret = NCursesEvent::button;
                  d->selected = true;
              }
          }
        }
      break;
      case 0x20: //Space
      case 0x0A: //Return
          ret = NCursesEvent::button;
          d->selected = true;
      break;
      case KEY_LEFT:
          ret = NCursesEvent::cancel;
          ret.detail = NCursesEvent::CONTINUE;
          break;

      default:
          ret = NCPopup::wHandleInput( ch );
          break;
    }

    return ret;
}


bool NCMGAPopupMenu::postAgain()
{
    // dont mess up postevent.detail here
    bool again = false;

    if (d->selected)
    {
      YMGAMenuItem * selected = dynamic_cast<YMGAMenuItem *>(d->menu->currentItem());

      if ( !selected )
      {
          d->selected = false;
          return false;
      }

      YMGAMenuItem * item = d->itemsMap[ selected ];
      yuiMilestone() << "Menu item: " << item->label() << " " << item->index() << std::endl;

      if ( item->hasChildren() )
      {
          // post submenu
          wpos at( ScreenPos() + wpos( selected->index(), inparent.Sze.W - 1 ) );
          yuiDebug() << "Submenu " << item->label() << " position: " << at << std::endl;

          NCMGAPopupMenu * dialog = new NCMGAPopupMenu( at,
                  item->childrenBegin(),
                  item->childrenEnd() );
          YUI_CHECK_NEW( dialog );

          again = ( dialog->post( &postevent ) == NCursesEvent::CONTINUE );
          // dialog has been closed but ask the menu father to contine eventually, so let's delete it
          YDialog::deleteTopmostDialog();
      }
      else
      {
          // store selection
          postevent.detail = item->index();
          again = false;
      }
    }

    return again;
}

