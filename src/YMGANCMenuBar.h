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

  File:         YMGANMenuBar.h

  Author:       Angelo Naselli <anaselli@linux.it>

/-*/

#ifndef YMGANMenuBar_h
#define YMGANMenuBar_h

#include <yui/mga/YMGAMenuBar.h>
#include <yui/ncurses/NCWidget.h>
#include <yui/YUI.h>
#include <yui/YApplication.h>

class YMGANCMenuBar : public YMGAMenuBar, public NCWidget
{
private:

    friend std::ostream & operator<<( std::ostream & str, const YMGANCMenuBar & obj );

    YMGANCMenuBar & operator=( const YMGANCMenuBar & );
    YMGANCMenuBar( const YMGANCMenuBar & );

    bool haveUtf8() { return YUI::app()->hasFullUtf8Support(); }

protected:

    virtual const char * location() const { return "YMGANCMenuBar"; }

    virtual void wRedraw();

    NCursesEvent postMenu();

public:

    YMGANCMenuBar( YWidget * parent );
    virtual ~YMGANCMenuBar();

    virtual int preferredWidth();
    virtual int preferredHeight();

    virtual void setSize( int newWidth, int newHeight );

    virtual void rebuildMenuTree();

    virtual NCursesEvent wHandleHotkey( wint_t key );
    virtual NCursesEvent wHandleInput( wint_t key );


    virtual bool setKeyboardFocus()
    {
      if ( !grabFocus() )
          return YWidget::setKeyboardFocus();

      return true;
    }

    virtual void setEnabled( bool do_bv );

    /**
     * Add an YMenuItem first item represents the menu name, other sub items menu entries
     *
     * Reimplemented from YSelectionWidget.
     **/
    virtual void addItem( YItem * item );

    /**
     * Reimplemnted to check all the hotkeys from YMenuItems
     *
     */
    virtual bool HasHotkey(int key) ;

    /**
    * Enable YMGAMenuItem (menu name or menu entry) to enable/disable it into menubar or menu
    *
    * Reimplemented from YMGAMenuBar.
    **/
    virtual void enableItem(YItem * menu_item, bool enable=true);

    /**
    * Hide YMGAMenuItem (menu name or menu entry) to hide/show it into menubar or menu
    *
    * Reimplemented from YMGAMenuBar.
    **/
    virtual void hideItem(YItem * menu_item, bool invisible=true);

private:
    /**
     * Recursively find the first menu item with the specified index.
     * Returns 0 if there is no such item.
     **/
    YMenuItem * findMenuItem( int index );

    /**
     * Recursively find the first menu item with the specified index
     * from iterator 'begin' to iterator 'end'.
     *
     * Returns 0 if there is no such item.
     **/
    YMenuItem * findMenuItem( int index, YItemConstIterator begin, YItemConstIterator end );

    /**
     * Alias for findMenuItem(). Reimplemented to ensure consistent behaviour
     * with YSelectionWidget::itemAt().
     **/
    YMenuItem * itemAt( int index )
    { return findMenuItem( index ); }


    void assignUniqueIndex( YItemIterator begin, YItemIterator end );

    struct Private;
    Private *d;
};

#endif //YMGANMenuBar_h
