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

   File:       NCMGAPopupMenu.h

   Author:     Angelo Naselli <anaselli@linux.it>

/-*/

#ifndef NCMGAPopupMenu_h
#define NCMGAPopupMenu_h

#include <iosfwd>
#include <map>

#include <yui/ncurses/NCPopup.h>
#include <yui/mga/YMGAMenuItem.h>

class NCMGAPopupMenu : public NCPopup
{
private:

    NCMGAPopupMenu & operator=( const NCMGAPopupMenu & );
    NCMGAPopupMenu( const NCMGAPopupMenu & );

    struct Private;
    Private *d;

protected:

    virtual NCursesEvent wHandleInput( wint_t ch );
    virtual bool postAgain();

    virtual int preferredWidth();
    virtual int preferredHeight();

    bool HasHotkey(int key);
    NCursesEvent wHandleHotkey( wint_t key );


public:

    NCMGAPopupMenu( const wpos & at,
                 YItemIterator begin,
                 YItemIterator end );

    virtual ~NCMGAPopupMenu();

};



#endif // NCMGAPopupMenu_h
