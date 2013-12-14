/*
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

  File:         YMGANCWidgetFactory.h

  Author:       Angelo Naselli <anaselli@linux.it>

/-*/

#ifndef YMGA_NCCBTable_h
#define YMGA_NCCBTable_h

#include <iosfwd>

#include <yui/mga/YMGA_CBTable.h>
#include <yui/ncurses/NCPadWidget.h>
#include <yui/ncurses/NCTablePad.h>


class YMGA_NCCBTable : public YMGA_CBTable, public NCPadWidget
{
public:

    YMGA_NCCBTable( YWidget * parent, YTableHeader *tableHeader, YCBTableMode mode = YCBTableMode::YCBTableCheckBoxOnFirstColumn);

    virtual ~YMGA_NCCBTable();

    bool bigList() const { return biglist; }

    void setHeader( std::vector <std::string> head );
    void getHeader( std::vector <std::string> & head );

    virtual void setAlignment( int col, YAlignmentType al );

    void setBigList( const bool big ) { biglist = big; }

    void SetSepChar( const chtype colSepchar )  { myPad()->SetSepChar( colSepchar ); }

    void SetSepWidth( const unsigned sepwidth ) { myPad()->SetSepWidth( sepwidth ); }

    void SetHotCol( const int hcol )            { myPad()->SetHotCol( hcol ); }

    virtual void addItem( YItem *yitem );
    virtual void addItems( const YItemCollection & itemCollection );
    virtual void deleteAllItems( );

    virtual int getCurrentItem();
    YItem * getCurrentItemPointer();

    virtual void setCurrentItem( int index );
    virtual void selectItem( YItem *yitem, bool selected );
    void selectCurrentItem();
    virtual void deselectAllItems();

    virtual int preferredWidth();
    virtual int preferredHeight();

    virtual void setSize( int newWidth, int newHeight );

    virtual void setLabel( const std::string & nlabel );

    virtual void setEnabled( bool do_bv );

    bool setItemByKey( int key );

    virtual NCursesEvent wHandleInput( wint_t key );

    virtual bool setKeyboardFocus()
    {
        if ( !grabFocus() )
            return YWidget::setKeyboardFocus();

        return true;
    }

    void stripHotkeys() { myPad()->stripHotkeys(); }

    void setSortStrategy( NCTableSortStrategyBase * newStrategy ) { myPad()->setSortStrategy( newStrategy ); }
    
    void checkItem( YItem* yitem, bool checked = true );

protected:

    /**
     * Overload myPad to narrow the type
     */
    virtual NCTablePad * myPad() const
        { return dynamic_cast<NCTablePad*>( NCPadWidget::myPad() ); }

    bool          biglist;

protected:

    virtual const char * location() const { return "YMGA_NCCBTable"; }

    virtual NCPad * CreatePad();

    virtual void cellChanged( int index, int colnum, const std::string & newtext );
    virtual void cellChanged( const YTableCell *cell );

    virtual void startMultipleChanges() { startMultidraw(); }
    virtual void doneMultipleChanges()  { stopMultidraw(); }

    //internal overloaded version of addItem - both addItem(yitem)
    //and addItems(itemCollection) use it, but in different mode
    virtual void addItem( YItem *yitem, bool allAtOnce );
    void toggleCurrentItem();

private:

    std::vector<NCstring> _header;

    friend std::ostream & operator<<( std::ostream & STREAM, const YMGA_NCCBTable & OBJ );

    YMGA_NCCBTable & operator=( const YMGA_NCCBTable & );
    YMGA_NCCBTable( const YMGA_NCCBTable & );

};


#endif // YMGA_NCCBTable_h