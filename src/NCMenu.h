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

   File:       NCMenu.h

   Author:     Michael Andres <ma@suse.de>

/-*/

#ifndef NCMenu_h
#define NCMenu_h

#include <iosfwd>

#include <yui/YTree.h>
#include <yui/YMenuItem.h>
#include "NCPadWidget.h"
#include <yui/ncurses/NCTreePad.h>
#include <yui/ncurses/NCTablePad.h>

class NCMenuLine;


class NCMenu : public YTree, public NCPadWidget
{
private:
    friend std::ostream & operator<<( std::ostream & str, const NCMenu & obj );

    NCMenu & operator=( const NCMenu & );
    NCMenu( const NCMenu & );

    int idx;

    void CreateTreeLine(NCTreePad* pad, YItem* item);

protected:

    virtual NCTreePad * myPad() const
    {
        return dynamic_cast<NCTreePad*>( NCPadWidget::myPad() );
    }



    const NCMenuLine * getTreeLine( unsigned idx ) const;
    NCMenuLine *       modifyTreeLine( unsigned idx );

    virtual const char * location() const {
        return "NCMenu";
    }

    virtual NCPad * CreatePad();
    virtual void    DrawPad();

    virtual void startMultipleChanges() {
        startMultidraw();
    }
    virtual void doneMultipleChanges()	{
        stopMultidraw();
    }
    bool HasHotkey(int key);
    NCursesEvent wHandleHotkey( wint_t key );

public:

    NCMenu( YWidget * parent  );
    virtual ~NCMenu();

    virtual int preferredWidth();
    virtual int preferredHeight();
    virtual void setSize( int newWidth, int newHeight );



    virtual void rebuildTree();

    virtual YMenuItem * getCurrentItem() const;

    virtual YMenuItem * currentItem();

    virtual void deselectAllItems();

    virtual void selectItem( YItem *item, bool selected );
    virtual void selectItem( int index );

    virtual NCursesEvent wHandleInput( wint_t key );

    virtual void setEnabled( bool do_bv );

    virtual bool setKeyboardFocus()
    {
        if ( !grabFocus() )
            return YWidget::setKeyboardFocus();

        return true;
    }

    void deleteAllItems();



};


#endif // NCMenu_h
