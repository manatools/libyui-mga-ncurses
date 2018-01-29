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

#define  YUILogComponent "mga-ncurses"
#include <yui/YUILog.h>
#include "YMGA_NCCBTable.h"
#include <yui/ncurses/NCPopupMenu.h>
#include <yui/YMenuButton.h>
#include <yui/YTypes.h>

using std::endl;

/*
 * Some remarks about single/multi selection:
 * A table in single selection mode has only one line/item selected which is equal to the
 * current item (means the highlighted line). Asking for `CurrentItem in YCP looks for
 * selectedItem() (see YCPPropertyHandler::tryGetSelectionWidgetValue).
 * In multi selection mode there can be several items selected (here is means checked/marked
 * with [x]) and the value is also got from selectedItem() when asking for `SelectedItems
 * (see YCPPropertyHandler::tryGetSelectionWidgetValue).
 * This means for multi selection mode: at the moment there isn't a possibility to get the
 * `CurrentItem. To get the current item (which line of the list is currently highlighted),
 * a virtual function currentItem() like available for the MultiSelectionBox has to be
 * provided to allow NCTable to specify the line number itself (getCurrentItem).
 *
 */
YMGA_NCCBTable::YMGA_NCCBTable ( YWidget * parent, YTableHeader *tableHeader, YCBTableMode mode )
  : YMGA_CBTable ( parent, tableHeader, mode )
  , NCPadWidget ( parent )
  , biglist ( false )
{
  yuiDebug() << std::endl;

  InitPad();

  yuiMilestone() << " Slection mode " << mode <<  std::endl;

  // !!! head is UTF8 encoded, thus should be std::vector<NCstring>
  _header.assign ( tableHeader->columns(), NCstring ( "" ) );
  int columNumber = columns();
  int checkColumn = (mode == YCBTableCheckBoxOnFirstColumn ? 0 : columNumber - 1); 

  for ( int col = 0; col < columNumber; col++ )
  {
    if ( hasColumn ( col ) )
    {
      // set alignment first
      if (col == checkColumn)
      {
        // force alignment to left for checkable colnum
        // to avoid working on YUI NC plugin low level
        setAlignment(col,  YAlignBegin);
      }
      else
        setAlignment ( col, alignment ( col ) );
      // and then append header
      _header[ col ] +=  NCstring ( tableHeader->header ( col ) ) ;
    }
  }


  hasHeadline = myPad()->SetHeadline ( _header );

}




YMGA_NCCBTable::~YMGA_NCCBTable()
{
  yuiDebug() << std::endl;
}



// Change individual cell of a table line (to newtext)
//                    provided for backwards compatibility

void YMGA_NCCBTable::cellChanged ( int index, int colnum, const std::string & newtext )
{
  NCTableLine * cl = myPad()->ModifyLine ( index );

  if ( !cl )
  {
    yuiWarning() << "No such line: " << wpos ( index, colnum ) << newtext << std::endl;
  }
  else
  {
    YCBTableMode mode = tableMode();

    int col = (mode == YCBTableCheckBoxOnFirstColumn) ? colnum + 1: colnum ;
    NCTableCol * cc = cl->GetCol ( col );

    if ( !cc )
    {
      yuiWarning() << "No such colnum: " << wpos ( index, colnum ) << newtext << std::endl;
    }
    else
    {
      // use NCtring to enforce recoding from 'utf8'
      cc->SetLabel ( NCstring ( newtext ) );
      DrawPad();
    }
  }
}



// Change individual cell of a table line (to newtext)

void YMGA_NCCBTable::cellChanged ( const YTableCell *cell )
{

  cellChanged ( cell->itemIndex(), cell->column(), cell->label() );

}



// Set all table headers all at once

void YMGA_NCCBTable::setHeader ( std::vector<std::string> head )
{
  _header.assign ( head.size(), NCstring ( "" ) );
  YTableHeader *th = new YTableHeader();

  for ( unsigned int i = 0; i < head.size(); i++ )
  {
    th->addColumn ( head[ i ] );
    _header[ i ] +=  NCstring ( head[ i ] ) ;
  }

  hasHeadline = myPad()->SetHeadline ( _header );

  YMGA_CBTable::setTableHeader ( th );
}

//
// Return table header as std::string std::vector (alignment removed)
//
void YMGA_NCCBTable::getHeader ( std::vector<std::string> & header )
{
  header.assign ( _header.size(), "" );

  for ( unsigned int i = 0; i < _header.size(); i++ )
  {
    header[ i ] =  _header[i].Str().substr ( 1 ); // remove alignment
  }
}


// Set alignment of i-th table column (left, right, center).
// Create temp. header consisting of single letter;
// setHeader will append the rest.

void YMGA_NCCBTable::setAlignment ( int col, YAlignmentType al )
{
  std::string s;

  switch ( al )
  {
  case YAlignUnchanged:
    s = 'L' ;
    break;

  case YAlignBegin:
    s = 'L' ;
    break;

  case YAlignCenter:
    s = 'C' ;
    break;

  case YAlignEnd:
    s = 'R' ;
    break;
  }

  _header[ col ] = NCstring ( s );
}

// Append  item (as pointed to by 'yitem')  in one-by-one
// fashion i.e. the whole table gets redrawn afterwards.
void YMGA_NCCBTable::addItem ( YItem *yitem )
{
  addItem ( yitem, false ); // add just this one
}

// Append item (as pointed to by 'yitem') to a table.
// This creates visual representation of new table line
// consisting of individual cells. Depending on the 2nd
// param, table is redrawn. If 'allAtOnce' is set to
// true, it is up to the caller to redraw the table.
void YMGA_NCCBTable::addItem ( YItem *yitem, bool allAtOnce )
{
  YCBTableItem *item = dynamic_cast<YCBTableItem *> ( yitem );
  YUI_CHECK_PTR ( item );
  YMGA_CBTable::addItem ( item );
  unsigned int itemCount;
  YCBTableMode mode = tableMode();

//   if ( mode == YTableSingleLineSelection )
    itemCount = columns();
//   else
//     itemCount = columns() +1;

  std::vector<NCTableCol*> Items ( itemCount );
  int i = 0;

  if ( mode == YCBTableCheckBoxOnFirstColumn )
  {
    // Create the tag first
    Items[0] = new NCTableTag ( yitem, item->checked() );
    i++;
  }
  // and then iterate over cells
  for ( YTableCellIterator it = item->cellsBegin();
        it != item->cellsEnd();
        ++it )
  {
    if ( i >= columns() )
    {
      yuiWarning() << "Item contains too many columns, current is " << i
                    << " but only " << columns() << " columns are configured" << std::endl;
      i++;
    }
    else
    {
      yuiDebug() << "current column is " << i << "/" << columns() << " (" << ( *it )->label() << ")" << std::endl;
      Items[i] = new NCTableCol ( NCstring ( ( *it )->label() ) );
      i++;
    }
  }
  if ( mode == YCBTableCheckBoxOnLastColumn )
  {
    // Create the tag at last column
    Items[columns()-1] = new NCTableTag ( yitem, item->checked() );
  }

  //Insert @idx
  NCTableLine *newline = new NCTableLine ( Items, item->index() );

  YUI_CHECK_PTR ( newline );

  newline->setOrigItem ( item );

  myPad()->Append ( newline );

  if ( item->selected() )
  {
    setCurrentItem ( item->index() ) ;
  }

  //in one-by-one mode, redraw the table (otherwise, leave it
  //up to the caller)
  if ( !allAtOnce )
  {
    DrawPad();
  }
}

// reimplemented here to speed up item insertion
// (and prevent inefficient redrawing after every single addItem
// call)
void YMGA_NCCBTable::addItems ( const YItemCollection & itemCollection )
{

  for ( YItemConstIterator it = itemCollection.begin();
        it != itemCollection.end();
        ++it )
  {
    addItem ( *it, true );
  }
  DrawPad();
}

// Clear the table (in terms of YTable and visually)

void YMGA_NCCBTable::deleteAllItems()
{
  myPad()->ClearTable();
  DrawPad();
  YMGA_CBTable::deleteAllItems();
}



// Return index of currently selected table item

int YMGA_NCCBTable::getCurrentItem()
{
  if ( !myPad()->Lines() )
    return -1;

  return keepSorting() ? myPad()->GetLine ( myPad()->CurPos().L )->getIndex()
         : myPad()->CurPos().L;

}



// Return origin pointer of currently selected table item

YItem * YMGA_NCCBTable::getCurrentItemPointer()
{
  const NCTableLine *cline = myPad()->GetLine ( myPad()->CurPos().L );

  if ( cline )
    return cline->origItem();
  else
    return 0;
}



// Highlight item at 'index'

void YMGA_NCCBTable::setCurrentItem ( int index )
{
  myPad()->ScrlLine ( index );
}



// Mark table item (as pointed to by 'yitem') as selected

void YMGA_NCCBTable::selectItem ( YItem *yitem, bool selected )
{
  if ( ! yitem )
    return;

  YCBTableItem *item = dynamic_cast<YCBTableItem *> ( yitem );
  YUI_CHECK_PTR ( item );

  NCTableLine *line = ( NCTableLine * ) item->data();
  YUI_CHECK_PTR ( line );

  const NCTableLine *current_line = myPad()->GetLine ( myPad()->CurPos().L );
  YUI_CHECK_PTR ( current_line );

  if ( !selected && ( line == current_line ) )
  {
    deselectAllItems();
  }
  else
  {
    // first highlight only, then select
    setCurrentItem ( line->getIndex() );
    YMGA_CBTable::selectItem ( item, selected );
  }

  // and redraw
  DrawPad();
}



// Mark currently highlighted table item as selected
// Yeah, it is really already highlighted, so no need to
// selectItem() and setCurrentItem() here again - #493884

void YMGA_NCCBTable::selectCurrentItem()
{
  const NCTableLine *cline = myPad()->GetLine ( myPad()->CurPos().L );

  if ( cline )
    YMGA_CBTable::selectItem ( cline->origItem(), true );
}



// Mark all items as deselected

void YMGA_NCCBTable::deselectAllItems()
{
  setCurrentItem ( -1 );
  YMGA_CBTable::deselectAllItems();

  DrawPad();
}



// return preferred size

int YMGA_NCCBTable::preferredWidth()
{
  wsze sze = ( biglist ) ? myPad()->tableSize() + 2 : wGetDefsze();
  return sze.W;
}



// return preferred size

int YMGA_NCCBTable::preferredHeight()
{
  wsze sze = ( biglist ) ? myPad()->tableSize() + 2 : wGetDefsze();
  return sze.H;
}



// Set new size of the widget

void YMGA_NCCBTable::setSize ( int newwidth, int newheight )
{
  wRelocate ( wpos ( 0 ), wsze ( newheight, newwidth ) );
}




void YMGA_NCCBTable::setLabel ( const std::string & nlabel )
{
  // not implemented: YTable::setLabel( nlabel );
  NCPadWidget::setLabel ( NCstring ( nlabel ) );
}



// Set widget state (enabled vs. disabled)

void YMGA_NCCBTable::setEnabled ( bool do_bv )
{
  NCWidget::setEnabled ( do_bv );
  YMGA_CBTable::setEnabled ( do_bv );
}




bool YMGA_NCCBTable::setItemByKey ( int key )
{
  return myPad()->setItemByKey ( key );
}





// Create new NCTablePad, set its background
NCPad * YMGA_NCCBTable::CreatePad()
{
  wsze    psze ( defPadSze() );
  NCPad * npad = new NCTablePad ( psze.H, psze.W, *this );
  npad->bkgd ( listStyle().item.plain );

  return npad;
}



// Handle 'special' keys i.e those not handled by parent NCPad class
// (space, return). Set items to selected, if appropriate.

NCursesEvent YMGA_NCCBTable::wHandleInput ( wint_t key )
{
  NCursesEvent ret;
  int citem  = getCurrentItem();

  if ( ! handleInput ( key ) )
  {
    switch ( key )
    {
    case CTRL ( 'o' ) :
    {
      if ( ! keepSorting() )
      {
        // get the column
        wpos at ( ScreenPos() + wpos ( win->height() / 2, 1 ) );

        YItemCollection ic;
        ic.reserve ( _header.size() );
        unsigned int i = 0;

        for ( std::vector<NCstring>::const_iterator it = _header.begin();
              it != _header.end() ; it++, i++ )
        {
          // strip the align mark
          std::string col = ( *it ).Str();
          col.erase ( 0, 1 );

          YMenuItem *item = new YMenuItem ( col ) ;
          //need to set index explicitly, MenuItem inherits from TreeItem
          //and these don't have indexes set
          item->setIndex ( i );
          ic.push_back ( item );
        }

        NCPopupMenu *dialog = new NCPopupMenu ( at, ic.begin(), ic.end() );

        int column = dialog->post();

        if ( column != -1 )
          myPad()->setOrder ( column, true ); //enable sorting in reverse order

        //remove the popup
        YDialog::deleteTopmostDialog();

        return NCursesEvent::none;
      }
    }

    case KEY_RETURN:
      if ( notify() && citem != -1 )
          return NCursesEvent::Activated;
    case KEY_SPACE:
      toggleCurrentItem();
      // send ValueChanged on Return (like done for NCTree multiSelection)
      if ( notify())
      {
        YCBTableItem *pItem = dynamic_cast<YCBTableItem*>(getCurrentItemPointer());
        YMGA_CBTable::setChangedItem(pItem);
        return NCursesEvent::ValueChanged;
      }
      break;

    }
  }


  if ( citem != getCurrentItem() )
  {
    if ( notify() && immediateMode() )
      ret = NCursesEvent::SelectionChanged;

    selectCurrentItem();
  }

  return ret;
}


void YMGA_NCCBTable::checkItem ( YItem* yitem, bool checked )
{
  YCBTableItem * item = dynamic_cast<YCBTableItem *> ( yitem );
  YUI_CHECK_PTR ( item );
  NCTableLine *line = ( NCTableLine * ) item->data();
  YUI_CHECK_PTR ( line );

  item->check(checked);
  
  int checkable_column = 0;
  YCBTableMode mode = tableMode();
  
  if ( mode == YCBTableCheckBoxOnLastColumn )
    checkable_column = columns() -1;

  yuiDebug() << item->label() << " is now " << ( checked?"checked":"unchecked" ) <<  endl;

  NCTableTag *tag =  static_cast<NCTableTag *> ( line->GetCol ( checkable_column ) );
  tag->SetSelected ( checked );
}

/**
 * Toggle item from selected -> deselected and vice versa
 **/
void YMGA_NCCBTable::toggleCurrentItem()
{
  YCBTableItem *item =  dynamic_cast<YCBTableItem *> ( getCurrentItemPointer() );
  YUI_CHECK_PTR ( item );
  checkItem(item, !item->checked());  
}
