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

using std::string;
using std::vector;
using std::endl;


/**
 * A column (one cell) used as a selection marker:
 * `[ ]`/`[x]` or `( )`/`(x)`.
 **/
class NCAlignedTableTag : public NCTableTag
{
public:

    /**
     * Constructor.
     *
     * @param item (must not be nullptr, not owned)
     * @param sel currently selected, draw an `x` inside
     * @param singleSel if true  draw this in a radio-button style `(x)`;
     *                  if false draw this in a checkbox style     `[x]`
     **/
    NCAlignedTableTag(YItem *item, bool sel = false, bool singleSel = false)
      : NCTableTag( item, sel, singleSel )
    {
    }

    virtual ~NCAlignedTableTag() {}

    virtual void DrawAt( NCursesWindow &    w,
                         const wrect        at,
                         NCTableStyle &     tableStyle,
                         NCTableLine::STATE linestate,
                         unsigned           colidx ) const
    {
      // Use parent DrawAt to draw the static part: "[ ]"
      NCTableCol::DrawAt( w, at, tableStyle, linestate, colidx );

      if ( NCTableTag::Selected() )
      {
        // Draw the "x" inside the "[ ]" with different attributes

        setBkgd( w, tableStyle, linestate, DATA );
        wrect drawRect = prefixAdjusted( at );


        const NC::ADJUST adjust = tableStyle.ColAdjust( colidx );
        if ( adjust & NC::LEFT )
        {
          w.addch( drawRect.Pos.L, drawRect.Pos.C + 1, 'x' );
        }
        else if ( adjust & NC::RIGHT )
        {
          w.addch( drawRect.Pos.L, drawRect.Pos.C + drawRect.Sze.W - 2, 'x' );
        }
        else // NC::CENTER
        {
          w.addch( drawRect.Pos.L, drawRect.Pos.C + (drawRect.Sze.W-1)/2 /*- 1*/, 'x' );
        }
      }
    }

};


class NCColSelTableLine : public NCTableLine
{

public:

  NCColSelTableLine( NCColSelTableLine *        parentLine,
                     YItem *                    yitem,
                     std::vector<NCTableCol*> & cells,
                     int                        index  = -1,
                     bool                       nested = false,
                     unsigned                   state  = S_NORMAL )
    : NCTableLine(parentLine,
                  yitem,
                  cells,
                  index,
                  nested,
                  state)
    , _activeColumn( 0 )

      {
      }


    virtual ~NCColSelTableLine() { }

    virtual void setActiveColumn(unsigned int col) { _activeColumn = col; }

    virtual unsigned int activeColumn() { return _activeColumn; }

protected:
  // reimplemented to get a single selected column
  virtual void DrawItems( NCursesWindow & w,
                          const wrect     at,
                          NCTableStyle &  tableStyle,
                          bool            active ) const
  {
    if ( !( at.Sze > wsze( 0 ) ) )
      return;

    wrect    lRect( at );
    unsigned destWidth;

    for ( unsigned col = 0; col < Cols(); ++col )
    {
      if ( col > 0 && tableStyle.ColSepWidth() )
      {
        // draw centered
        destWidth = tableStyle.ColSepWidth() / 2;

        if ( destWidth < (unsigned) lRect.Sze.W )
        {
          w.bkgdset( tableStyle.getBG( _vstate, NCTableCol::SEPARATOR ) );
          w.vline( lRect.Pos.L, lRect.Pos.C + destWidth,
                   lRect.Sze.H, tableStyle.ColSepChar() );
          // skip over
          destWidth = tableStyle.ColSepWidth();

          if ( (unsigned) lRect.Sze.W <= destWidth )
            break;

          lRect.Pos.C += destWidth;
          lRect.Sze.W -= destWidth;
        }
      }

      destWidth = tableStyle.ColWidth( col );

      wrect cRect( lRect );

      // Adjust drawing rectangle for the screen space we just used
      lRect.Pos.C += destWidth;
      lRect.Sze.W -= destWidth;

      if ( lRect.Sze.W < 0 )
        cRect.Sze.W = destWidth + lRect.Sze.W;
      else
        cRect.Sze.W = destWidth;

      if ( _cells[ col ] )
      {
        // Draw item
        if (_activeColumn == col && active)
        {
          _cells[ col ]->DrawAt( w, cRect, tableStyle, S_NORMAL, col );
        }
        else
        {
          _cells[ col ]->DrawAt( w, cRect, tableStyle, _vstate, col );
        }

        // Draw tree hierarchy line graphics over the prefix placeholder

        if ( col == 0 && _prefix )
          drawPrefix( w, cRect, tableStyle );
      }
    }
  }

  unsigned int _activeColumn;

};


/*
 * Some remarks about single/multi selection:
 *
 * A table in single selection mode has only one line/item selected which is
 * equal to the current item (means the highlighted line). Querying CurrentItem
 * in YCP looks for selectedItem()
 * (see YCPPropertyHandler::tryGetSelectionWidgetValue).
 *
 * In multi selection mode there can be several items selected (here is means
 * checked/marked with [x]) and the value is also got from selectedItem() when
 * asking for `SelectedItems
 * (see YCPPropertyHandler::tryGetSelectionWidgetValue).
 *
 * This means for multi selection mode: at the moment there isn't a possibility
 * to get the CurrentItem. To get the current item (which line of the list is
 * currently highlighted), a virtual function currentItem() like available for
 * the MultiSelectionBox has to be provided to allow NCTable to specify the
 * line number itself (getCurrentItem).
 */
YMGA_NCCBTable::YMGA_NCCBTable( YWidget * parent,
                                YCBTableHeader * tableHeader )
    : YMGA_CBTable( parent, tableHeader )
    , NCPadWidget( parent )
    , _prefixCols( 0 )
    , _nestedItems( false )
    , _bigList( false )
    , _lastSortCol( 0 )
    , _sortReverse( false )
    , _sortStrategy( new NCTableSortDefault() )
    , _currentColumn ( 0 )
{
    // yuiDebug() << endl;

    InitPad();
    rebuildHeaderLine();
}


YMGA_NCCBTable::~YMGA_NCCBTable()
{
    if ( _sortStrategy )
        delete _sortStrategy;
}


void YMGA_NCCBTable::rebuildHeaderLine()
{
  _prefixCols = 0;

  if ( hasMultiSelection() )
    ++_prefixCols;

  vector<NCstring> headers;
  headers.resize( _prefixCols + columns() );

  for ( int i = 0; i < columns(); i++ )
  {
    int col = i + _prefixCols;

    if ( hasColumn( i ) )
    {
      // TODO check if header for checkbox columns must be aligned to left
      // as it was in my last implementation
      // if (isCheckBoxColumn(i))
      // (force alignment to left for checkable column
      // to avoid working on YUI NC plugin low level)
      NCstring hdr( alignmentStr( i ) );
      hdr += header( i );
      // yuiDebug() << "hdr[" << col << "]: \"" << hdr << "\"" << endl;
      headers[ col ] = hdr;
    }
  }

  hasHeadline = myPad()->SetHeadline( headers );
}


NCstring YMGA_NCCBTable::alignmentStr( int col )
{
  switch ( alignment( col ) )
  {
    case YAlignUnchanged:   return "L";
    case YAlignBegin:       return "L";
    case YAlignCenter:      return "C";
    case YAlignEnd:         return "R";

    // No 'default' branch: Let the compiler complain if there is an unhandled enum value.
  }

  return "L";
}


void YMGA_NCCBTable::setCell( int index, int col, const string & newtext )
{
  NCTableLine * currentLine = myPad()->ModifyLine( index );

  if ( !currentLine )
  {
    yuiWarning() << "No such line: " << wpos( index, col ) << newtext << endl;
  }
  else
  {
    NCTableCol * currentCol = currentLine->GetCol( col );

    if ( !currentCol )
    {
      yuiWarning() << "No such col: " << wpos( index, col ) << newtext << endl;
    }
    else
    {
      // use NCtring to enforce recoding from UTF-8
      currentCol->SetLabel( NCstring( newtext ) );
      DrawPad();
    }
  }
}


void YMGA_NCCBTable::cellChanged( const YTableCell * changedCell )
{
  YUI_CHECK_PTR( changedCell );

  YTableItem * ytableItem = changedCell->parent();
  YUI_CHECK_PTR( ytableItem );

  NCTableLine * tableLine = (NCTableLine *) ytableItem->data();
  YUI_CHECK_PTR( tableLine );

  NCTableCol * tableCol = tableLine->GetCol( changedCell->column() );

  if ( tableCol )
  {
    tableCol->SetLabel( changedCell->label() );
    DrawPad();
  }
  else
  {
    yuiError() << "No column #" << changedCell->column()
    << " in item " << ytableItem
    << endl;
  }
}


void YMGA_NCCBTable::setHeader( const vector<string> & headers )
{
  YTableHeader * tableHeader = new YCBTableHeader();

  for ( unsigned i = 0; i < headers.size(); i++ )
  {
    tableHeader->addColumn( headers[ i ] );
  }

  YMGA_CBTable::setTableHeader( tableHeader );
  rebuildHeaderLine();
}


vector<string> YMGA_NCCBTable::getHeader() const
{
  vector<string> headers;
  headers.resize( columns() );

  for ( int col = 0; col < columns(); col++ )
  {
    headers[ col ] = header( col );
  }

  return headers;
}


void YMGA_NCCBTable::addItems( const YItemCollection & itemCollection )
{
  myPad()->ClearTable();
  YMGA_CBTable::addItems( itemCollection );

  if ( keepSorting() )
  {
    rebuildPadLines();
  }
  else
  {
    sortItems( _lastSortCol, _sortReverse );
    // this includes rebuildPadLInes()
  }

  if ( !hasMultiSelection() ) // keep compatibility to help in integration/merge
    selectCurrentItem();

  DrawPad();
}


void YMGA_NCCBTable::addItem( YItem *            yitem,
                              NCTableLine::STATE state )
{
  if ( ! yitem->parent() )            // Only for toplevel items:
    YMGA_CBTable::addItem( yitem );   // Notify the YTable base class

  addPadLine( 0,      // parentLine
              yitem,
              false,  // preventRedraw
              state );
}


void YMGA_NCCBTable::addItem( YItem *            yitem,
                              bool               preventRedraw,
                              NCTableLine::STATE state )
{
  if ( ! yitem->parent() )            // Only for toplevel items:
    YMGA_CBTable::addItem( yitem );   // Notify the YTable base class

  addPadLine( 0,      // parentLine
              yitem,
              preventRedraw,
              state );
}


void YMGA_NCCBTable::addPadLine( NCTableLine *      parentLine,
                                 YItem *            yitem,
                                 bool               preventRedraw,
                                 NCTableLine::STATE state )
{
  YCBTableItem *item = dynamic_cast<YCBTableItem *>( yitem );
  YUI_CHECK_PTR( item );

  // Ideally, _nestedItems should be updated by iterating over ALL items
  // before the first NCTableLine is created (i.e. before the first
  // addPadLine() call) so they all get the proper prefix placeholder for
  // reserving some screen space for the tree line graphics.
  //
  // This additional check is just a second line of defence.

  if ( parentLine || item->hasChildren() )
    _nestedItems = true;

  vector<NCTableCol*> cells;

  if ( hasMultiSelection() ) // keep compatibility to help in integration/merge
  {
    // Add a table tag to hold the "[ ]" / "[x]" marker.
    cells.push_back( new NCTableTag( yitem, yitem->selected() ) );
  }

  // Add all the cells
  for ( YTableCellIterator it = item->cellsBegin(); it != item->cellsEnd(); ++it )
  {
    NCTableCol* tableColumn = NULL;
    int column = (*it)->column();
    if (isCheckBoxColumn(column))
      tableColumn = new NCAlignedTableTag( yitem, item->checked(column) );
    else
      tableColumn = new NCTableCol( NCstring(( *it )->label() ) );

    cells.push_back( tableColumn );
  }
  int index = myPad()->Lines();
  item->setIndex( index );

  // yuiMilestone() << "Adding pad line for " << item << " index: " << item->index() << endl;

  NCColSelTableLine *pLine = dynamic_cast<NCColSelTableLine *>(parentLine);
  if (parentLine)
    YUI_CHECK_NEW( pLine );

  // Create the line itself
  NCColSelTableLine *line = new NCColSelTableLine( pLine,
                                       item,
                                       cells,
                                       index,
                                       _nestedItems,
                                       state );
  YUI_CHECK_NEW( line );
  myPad()->Append( line );

  if ( item->selected() )
    setCurrentItem( item->index() ) ;

  // Recurse over children (if there are any)

  for ( YItemIterator it = item->childrenBegin(); it != item->childrenEnd(); ++it )
  {
    addPadLine( line, *it, preventRedraw, state );
  }

  if ( ! preventRedraw )
    DrawPad();
}


void YMGA_NCCBTable::rebuildPadLines()
{
  myPad()->ClearTable();
  _nestedItems = hasNestedItems( itemsBegin(), itemsEnd() );

  for ( YItemConstIterator it = itemsBegin(); it != itemsEnd(); ++it )
  {
    addPadLine( 0,      // parentLine
                *it,
                true ); // preventRedraw
  }
}

bool YMGA_NCCBTable::hasNestedItems( const YItemCollection & itemCollection ) const
{
  return hasNestedItems( itemCollection.begin(), itemCollection.end() );
}


bool YMGA_NCCBTable::hasNestedItems( YItemConstIterator begin,
                                     YItemConstIterator end ) const
{
  for ( YItemConstIterator it = begin; it != end; ++it )
  {
    if ( (*it)->hasChildren() )
      return true;
  }

  return false;
}


void YMGA_NCCBTable::deleteAllItems()
{
  myPad()->ClearTable();
  YMGA_CBTable::deleteAllItems();
  DrawPad();

  _nestedItems   = false;
  _lastSortCol   = 0;
  _sortReverse   = false;
}


int YMGA_NCCBTable::getCurrentItem() const
{
  if ( myPad()->empty() )
    return -1;

  // The intent of this condition is to return the original index, before
  // sorting. But the condition was accidentally inverted in 2007 and now it
  // always returns the index after sorting.
  // Should we fix it? Depends on whether the current users rely on the
  // current behavior.

  return keepSorting() ? getCurrentIndex() : myPad()->CurPos().L;
}


YItem * YMGA_NCCBTable::getCurrentItemPointer()
{
  const NCTableLine * currentLine = myPad()->GetCurrentLine();

  if ( currentLine )
    return currentLine->origItem();
  else
    return 0;
}


int YMGA_NCCBTable::getCurrentIndex() const
{
  const NCTableLine * currentLine = myPad()->GetCurrentLine();

  return currentLine ? currentLine->index() : -1;
}


void YMGA_NCCBTable::scrollToFirstItem()
{
  if ( myPad()->empty() )
  {
    NCColSelTableLine * l = dynamic_cast<NCColSelTableLine *>(myPad()->ModifyLine(0));
    if (l)
      l->setActiveColumn(_currentColumn);
    myPad()->ScrlLine( 0 );
  }
}


void YMGA_NCCBTable::setCurrentItem( int index )
{
  NCColSelTableLine * l = dynamic_cast<NCColSelTableLine *>(myPad()->ModifyLine(index));
  if (l)
    l->setActiveColumn(_currentColumn);
  myPad()->ScrlLine( index );
}


void YMGA_NCCBTable::selectItem( YItem *yitem, bool selected )
{
  if ( ! yitem )
    return;

  YCBTableItem *item = dynamic_cast<YCBTableItem *> ( yitem );
  YUI_CHECK_PTR( item );

  NCTableLine *line = (NCTableLine *) item->data();
  YUI_CHECK_PTR( line );

  const NCTableLine *current_line = myPad()->GetLine( myPad()->CurPos().L );
  YUI_CHECK_PTR( current_line );

  if ( !hasMultiSelection() ) // keep compatibility to help in integration/merge
  {
    if ( !selected && ( line == current_line ) )
    {
      deselectAllItems();
    }
    else
    {
      // first highlight only, then select
      setCurrentItem( line->index() );
      NCColSelTableLine * l = dynamic_cast<NCColSelTableLine *>(myPad()->ModifyLine(line->index()));
      if (l)
        l->setActiveColumn(_currentColumn);
      YMGA_CBTable::selectItem( item, selected );
    }
  }
  else // multiSelect
  {
    YMGA_CBTable::selectItem( item, selected );

    // yuiDebug() << item->label() << " is selected: " << std::boolalpha << selected <<  endl;

    // The NCTableTag holds the "[ ]" / "[x]" selection marker
    NCTableTag * tagCell =  line->tagCell();

    if ( tagCell )
      tagCell->SetSelected( selected );
  }

  DrawPad();
}


/**
 * Mark the currently highlighted table item as selected.
 *
 * Yes, it is really already highlighted, so no need to selectItem() and
 * setCurrentItem() here again. (bsc#493884)
 **/
void YMGA_NCCBTable::selectCurrentItem()
{
  NCColSelTableLine * currentLine = dynamic_cast<NCColSelTableLine *>(myPad()->GetCurrentLine());


  if ( currentLine )
  {
    currentLine->setActiveColumn(_currentColumn);
    YMGA_CBTable::selectItem( currentLine->origItem(), true );
  }
}


void YMGA_NCCBTable::deselectAllItems()
{
  if ( !hasMultiSelection() ) // keep compatibility to help in integration/merge)
  {
    setCurrentItem( -1 );
    YMGA_CBTable::deselectAllItems();
  }
  else
  {
    YItemCollection itemCollection = YTable::selectedItems();
    // This will return nested selected items as well

    for ( YItemConstIterator it = itemCollection.begin();
         it != itemCollection.end();
    ++it )
    {
      // Clear the item's internal selected status flag
      // and update the "[x]" marker on the screen to "[ ]"
      // in the corresponding NCTableTag

      selectItem( *it, false );
    }
  }

  DrawPad();
}


int YMGA_NCCBTable::preferredWidth()
{
  wsze sze = _bigList ? myPad()->tableSize() + 2 : wGetDefsze();
  return sze.W;
}


int YMGA_NCCBTable::preferredHeight()
{
  wsze sze = _bigList ? myPad()->tableSize() + 2 : wGetDefsze();
  return sze.H;
}


void YMGA_NCCBTable::setSize( int newwidth, int newheight )
{
  wRelocate( wpos( 0 ), wsze( newheight, newwidth ) );
}


void YMGA_NCCBTable::setLabel( const string & nlabel )
{
  // not implemented: YTable::setLabel( nlabel );
  NCPadWidget::setLabel( NCstring( nlabel ) );
}


void YMGA_NCCBTable::setEnabled( bool do_bv )
{
  NCWidget::setEnabled( do_bv );
  YMGA_CBTable::setEnabled( do_bv );
}


bool YMGA_NCCBTable::setItemByKey( int key )
{
  return myPad()->setItemByKey( key );
}


NCPad * YMGA_NCCBTable::CreatePad()
{
  wsze    psze( defPadSze() );
  NCPad * npad = new NCTablePad( psze.H, psze.W, *this );
  npad->bkgd( listStyle().item.plain );

  return npad;
}


/**
 * NCurses widget keyboard handler.
 *
 * This is the starting point for handling key events. From here, key events
 * are propagated to the pad and to the items.
 **/
NCursesEvent YMGA_NCCBTable::wHandleInput( wint_t key )
{
  NCursesEvent ret  = NCursesEvent::none;
  bool sendEvent    = false;
  int  currentIndex = getCurrentItem();

  // Call the pad's input handler via NCPadWidget::handleInput()
  // which calls its pad class's input handler
  // which may call the current item's input handler.
  //
  // Notice that most keys are handled on the level of the pad or the item,
  // not here. See
  //
  // - NCTablePad::handleInput()
  // - NCTablePadBase::handleInput()
  // - NCTableLine::handleInput()

  bool handled = handleInput( key ); // NCTablePad::handleInput()

  switch ( key )
  {
    case CTRL( 'o' ):       // Table sorting (Ordering)
      if ( ! handled )
      {
        if ( ! keepSorting() )
        {
          interactiveSort();
          return NCursesEvent::none;
        }
      }
      break;

    case KEY_RIGHT:
      if ( !hasMultiSelection() && currentIndex != -1)
      {
        int col = getNextColumn();
        NCTableLine * currentLine = myPad()->ModifyLine(currentIndex);
        if ( currentLine )
        {
          NCTableCol * currColumn = currentLine->GetCol(col);
          if (currColumn)
          {
            NCColSelTableLine * line = dynamic_cast<NCColSelTableLine *>(currentLine);
            line->setActiveColumn(col);
            DrawPad();
          }
        }
      }
    break;

    case KEY_LEFT:
      if ( !hasMultiSelection() && currentIndex != -1)
      {
        int col = getPreviousColumn();
        NCTableLine * currentLine = myPad()->ModifyLine(currentIndex);
        if ( currentLine )
        {
          NCTableCol * currColumn = currentLine->GetCol(col);
          if (currColumn)
          {
            NCColSelTableLine * line = dynamic_cast<NCColSelTableLine *>(currentLine);
            line->setActiveColumn(col);
            DrawPad();
          }
        }
      }
    break;
      // Even if the event was already handled:
      // Take care about sending UI events to the caller.

    case KEY_RETURN:

      sendEvent = true;

      if ( hasMultiSelection() )
      {
        toggleCurrentItem();

        // Send ValueChanged on Return (like done for NCTree multiSelection)

        if ( notify() && sendEvent )
          return NCursesEvent::ValueChanged;
      }
      else
      {
        if ( notify() && isCheckBoxColumn(_currentColumn))
        {
          YCBTableItem *pItem = dynamic_cast<YCBTableItem*>(getCurrentItemPointer());
          if (pItem)
          {
            setItemChecked( pItem, _currentColumn, !pItem->checked(_currentColumn) );
            pItem->setChangedColumn(_currentColumn);
            YMGA_CBTable::setChangedItem(pItem);
            return NCursesEvent::ValueChanged;
          }
        }
      }
      // FALLTHRU

    case KEY_SPACE:

      if ( !hasMultiSelection() )
      {
        if ( notify() && currentIndex != -1 )
          return NCursesEvent::Activated;
      }
      break;
  }

  if (  currentIndex != getCurrentItem() )
  {
    if ( notify() && immediateMode() )
      ret = NCursesEvent::SelectionChanged;

    if ( !hasMultiSelection() )
    {
      selectCurrentItem();
      DrawPad();
    }
  }

  return ret;
}


void YMGA_NCCBTable::toggleCurrentItem()
{
  YTableItem * item =  dynamic_cast<YTableItem *>( getCurrentItemPointer() );

  if ( item )
    selectItem( item, !( item->selected() ) );
}


void YMGA_NCCBTable::interactiveSort()
{
  //
  // Collect the non-empty column headers
  //

  YItemCollection menuItems;
  menuItems.reserve( columns() );

  for ( int col = 0; col < columns(); col++ )
  {
    string hdr = header( col );

    if ( ! hdr.empty() )
    {
      YMenuItem *item = new YMenuItem( header( col ) ) ;

      // need to set the index explicitly, YMenuItem inherits from YTreeItem
      // and these don't have indexes set
      item->setIndex( col );
      menuItems.push_back( item );
    }
  }

  if ( ! menuItems.empty() )
  {
    //
    // Post a popup with the column headers
    //

    // Get the column; show the popup in the table's upper left corner
    wpos pos( ScreenPos() + wpos( 2, 1 ) );

    NCPopupMenu *dialog = new NCPopupMenu( pos, menuItems.begin(), menuItems.end() );
    int sortCol = dialog->post();

    // close the popup
    YDialog::deleteTopmostDialog();

    if ( sortCol != -1 && hasColumn( sortCol ) )
    {
      //
      // Do the sorting
      //

      yuiDebug() << "Manually sorting by column #"
      << sortCol << ": " << header( sortCol )
      << endl;

      _sortReverse = sortCol == _lastSortCol ?
      ! _sortReverse : false;

      sortItems( sortCol, _sortReverse );

      if ( !hasMultiSelection() )
        selectCurrentItem();

      DrawPad();
    }
  }
}


void YMGA_NCCBTable::sortItems( int sortCol, bool reverse )
{
  // NOTE that if column is reserved to checkboxes sorting does not make sense
  if ( !isCheckBoxColumn(sortCol) )
  {
    myPad()->ClearTable();
    // Sort the YItems.
    //
    // This may feel a little weird since those YItems are owned by the
    // YSelectionWidget parent class. But we are only changing their sort
    // order, not invalidating any item pointers; and the internal sort order
    // is not anything that any calling application code may rely on.
    //
    // Since the NCTable now supports nested items, we can no longer simply
    // sort the NCTableLines to keep this whole sorting localized: In the pad,
    // they are just a flat list, and the hierarchy is not that easy to find
    // out.  But we need the hierarchy to sort each tree level separately in
    // each branch.
    //
    // It is much simpler and less error-prone to just clear the pad (and thus
    // get rid of any existing NCTableLines), sort the YItems and rebuild all
    // the NCTableLines from the newly sorted YItems.

    _sortStrategy->setSortCol( sortCol );
    _sortStrategy->setReverse( reverse );
    _lastSortCol = sortCol;

    sortYItems( itemsBegin(), itemsEnd() );

    rebuildPadLines();
  }
}


void YMGA_NCCBTable::sortYItems( YItemIterator begin,
                                 YItemIterator end )
{
  // Sort the children first as long as the iterators are
  // guaranteed to be valid

  for ( YItemIterator it = begin; it != end; ++it )
  {
    if ( (*it)->hasChildren() )
      sortYItems( (*it)->childrenBegin(), (*it)->childrenEnd() );
  }

  // Sort this level. This may make the iterators invalid.
  _sortStrategy->sort( begin, end );
}


void YMGA_NCCBTable::setSortStrategy( NCTableSortStrategyBase * newStrategy )
{
  if ( _sortStrategy )
    delete _sortStrategy;

  _sortStrategy = newStrategy;
}


void YMGA_NCCBTable::setItemChecked( YItem* yitem, int column, bool checked )
{
  YCBTableItem * item = dynamic_cast<YCBTableItem *> ( yitem );
  YUI_CHECK_PTR ( item );
  NCTableLine *line = ( NCTableLine * ) item->data();
  YUI_CHECK_PTR ( line );

  if ( isCheckBoxColumn(column) )
  {
    YCBTableCell *cell = dynamic_cast<YCBTableCell *>(item->cell(column)); // checkboxItemColumn is true so hasCell is as well
    YUI_CHECK_PTR( cell );
    cell->setChecked( checked );

    NCTableTag *tag =  static_cast<NCTableTag *> ( line->GetCol ( column ) );
    tag->SetSelected ( checked );
  }
}

int YMGA_NCCBTable::getCurrentColumn() const
{
  return _currentColumn;
}


int YMGA_NCCBTable::getNextColumn()
{
  _currentColumn = (_currentColumn + 1) % columns();

  return _currentColumn;
}

int YMGA_NCCBTable::getPreviousColumn()
{
  _currentColumn = (_currentColumn - 1) % columns();

  return _currentColumn;
}
////////////////////////////////////////////////////////////////////////////////////




#ifdef REMOVE
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

/**
 * Toggle item from selected -> deselected and vice versa
 **/
void YMGA_NCCBTable::toggleCurrentItem()
{
  YCBTableItem *item =  dynamic_cast<YCBTableItem *> ( getCurrentItemPointer() );
  YUI_CHECK_PTR ( item );
  checkItem(item, !item->checked());  
}

#endif
