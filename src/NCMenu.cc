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

   File:       NCMenu.cc

   Author:     Angelo Naselli <anaselli@linux.it>

/-*/

#define	 YUILogComponent "ncurses"
#include <yui/YUILog.h>
#include "NCMenu.h"
#include <yui/ncurses/YNCursesUI.h>

#include <yui/YMenuItem.h>
#include <yui/YSelectionWidget.h>
#include <yui/mga/YMGAMenuItem.h>


class NCMenuLine : public NCTableLine
{

private:

    YMenuItem *		yitem;

    NCMenuLine * nsibling;
    NCMenuLine * fchild;

    mutable chtype * prefix;


public:

    NCMenuLine( YMenuItem * item )
        : NCTableLine( 0 )
        , yitem( item )
        , nsibling( 0 )
        , fchild( 0 )
        , prefix( 0 )
    {
        YMGAMenuItem *mi = dynamic_cast<YMGAMenuItem*> (yitem);
        if ( mi->hidden())
        {
            yuiDebug() << mi->label() << " hidden" << std::endl;
            SetState(S_HIDDEN);
        }
        else  if ( !mi->enabled() )
        {
            yuiDebug() << mi->label() << " disabled" << std::endl;
            SetState( S_DISABELED );

        }

        // leaving next even if managed into MGAPopupMenu
        if ( yitem->hasChildren() )
        {
            SetState(S_HEADLINE);
            yuiDebug() << mi->label() << " has submenu" << std::endl;
            Append( new NCTableCol( NCstring( yitem->label() + " ..." ) ) );
        }
        else
        {
          SetState(S_HEADLINE);
          Append( new NCTableCol( NCstring( yitem->label() ) ) );
        }
        stripHotkeys();
    }

    virtual ~NCMenuLine() {
        delete [] prefix;
    }

public:

    YMenuItem * YItem() const {
        return yitem;
    }



    virtual void DrawAt( NCursesWindow & w, const wrect at,
                         NCTableStyle & tableStyle,
                         bool active ) const
    {
         NClabel l(NCstring(yitem->label()));
         l.stripHotkey();
         yuiDebug() << yitem->label() << " hotcol: "<< l.hotpos() <<  " hotkey: " << l.hotkey() << std::endl;

        if ( !isSpecial() )
            w.bkgdset( tableStyle.hotBG( vstate, NCTableCol::PLAIN ) );

        //tableStyle.highlightBG( vstate, NCTableCol::HINT);

        yuiDebug() << "tableStyle hotcol: " << tableStyle.listStyle().title << " bg: " << tableStyle.hotBG(vstate, tableStyle.HotCol()) << std::endl;


        NCTableLine::DrawAt( w, at, tableStyle, active );
        // NOTE I couldn't be able to fix hot char representation, so i had to force it
        if (l.hasHotkey())
        {
          w.move(at.Pos.L, l.hotpos());
          w.addch(l.hotkey() | A_UNDERLINE);
        }

        w.move( at.Pos.L, at.Pos.C );

    }
};






NCMenu::NCMenu( YWidget * parent )
    : YTree( parent, "", FALSE, FALSE )
    , NCPadWidget( parent )
{
    yuiDebug() << std::endl;
    // minimum size 3 line 8 coulmn
    defsze = wsze(3, 8);
}



NCMenu::~NCMenu()
{
    yuiDebug() << std::endl;
}




// Return pointer to tree line	at given index
inline const NCMenuLine * NCMenu::getTreeLine( unsigned idx ) const
{
    if ( myPad() )
        return dynamic_cast<const NCMenuLine *>( myPad()->GetLine( idx ) );
    else
        return 0;
}




// Modify tree line at given index
inline NCMenuLine * NCMenu::modifyTreeLine( unsigned idx )
{
    if ( myPad() )
    {
        return dynamic_cast<NCMenuLine *>( myPad()->ModifyLine( idx ) );
    }

    return 0;
}

// Set preferred width
int NCMenu::preferredWidth()
{
  // TODO
    wsze sze = wsze::max( defsze, wsze( 0, labelWidth() + 2 ) );
    return defsze.W + 4;// border and scroll;
}

// Set preferred height
int NCMenu::preferredHeight()
{
  //TODO
    wsze sze = wsze::max( defsze, wsze( 0, labelWidth() + 2 ) );
    return defsze.H;//6;
}

void NCMenu::setSize( int newwidth, int newheight )
{
    wRelocate( wpos( 0 ), wsze( newheight, newwidth ) );
}


// Enable/disable widget
void NCMenu::setEnabled( bool do_bv )
{
    NCWidget::setEnabled( do_bv );
    YWidget::setEnabled( do_bv );
}
// Return YMenuItem pointer for a current line
//		      (under the cursor)
YMenuItem * NCMenu::getCurrentItem() const
{
    YMenuItem * yitem = 0;

    if ( myPad() && myPad()->GetCurrentLine() )
    {
        const NCMenuLine * cline = dynamic_cast<const NCMenuLine *>( myPad()->GetCurrentLine() );

        if ( cline )
            yitem = cline->YItem();
    }

    yuiDebug() << "-> " << ( yitem ? yitem->label().c_str() : "noitem" ) << std::endl;

    return yitem;
}

void NCMenu::deselectAllItems()
{
    YTree::deselectAllItems();
}


// Set current item (under the cursor) to selected
void NCMenu::selectItem( YItem *item, bool selected )
{
    if ( !myPad() )
        return;

    YMenuItem * treeItem =  dynamic_cast<YMenuItem *>( item );
    YUI_CHECK_PTR( treeItem );
    YMenuItem *citem = getCurrentItem();

    //retrieve position of item
    int at = treeItem->index();

//     NCMenuLine * cline = 0;	// current line

    if ( !selected )
    {
        if ( treeItem == citem )
        {
            YTree::deselectAllItems();
        }
        else
        {
            YTree::selectItem ( treeItem, false );
        }
    }
    else
    {
        YTree::selectItem( treeItem, selected );

        //this highlights selected item, possibly unpacks the tree
        //should it be in currently hidden branch
        myPad()->ShowItem( getTreeLine( at ) );
    }
}




// Set current item (at given index) to selected
//		      (overloaded for convenience)
void NCMenu::selectItem( int index )
{
    YItem * item = YTree::itemAt( index );

    if ( item )
    {
        selectItem( item, true );
    }
    else
        YUI_THROW( YUIException( "Can't find selected item" ) );
}




void NCMenu::rebuildTree()
{
    DelPad();
    Redraw();
}




// Creates empty pad
NCPad * NCMenu::CreatePad()
{
    wsze    psze( defPadSze() );
    NCPad * npad = new NCTreePad( psze.H, psze.W, *this );
    npad->bkgd( listStyle().item.hint );
    return npad;
}

bool NCMenu::HasHotkey(int key)
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

NCursesEvent NCMenu::wHandleHotkey( wint_t key )
{
    yuiDebug() << "Key: " << key << std::endl;
    if ( key >= 0 && key < UCHAR_MAX ) //  < myPad()->setItemByKey( key ) )
    {
      int hkey = tolower( key );
      for ( YItemIterator it = itemsBegin(); it < itemsEnd(); ++it )
      {
          YMGAMenuItem *mi = dynamic_cast<YMGAMenuItem*> (*it);
          YUI_CHECK_PTR( mi );
          if (mi->enabled())
          {
            NClabel l (NCstring( mi->label() ));
            l.stripHotkey();
            yuiDebug() << mi->label() << " " << l << " hotpos: " << l.hotpos() << " hkey: " <<  l.hotkey() << std::endl;
            if (tolower(l.hotkey()) == hkey)
            {
              selectItem(mi->index());
              return wHandleInput( KEY_RETURN );
            }
          }
          else
          {
            yuiDebug() << mi->label() << " disabled" << std::endl;
          }
      }
    }
    return NCursesEvent::none;
}



// Creates tree lines and appends them to TreePad
// (called recursively for each child of an item)
void NCMenu::CreateTreeLine( NCTreePad * pad, YItem * item )
{
    //set item index explicitely, it is set to -1 by default
    //which makes selecting items painful
    item->setIndex( idx++ );

    YMenuItem * treeItem = dynamic_cast<YMenuItem *>( item );
    YUI_CHECK_PTR( treeItem );

    // let's assume to have a menu enable scrolling for more than 10 lines
    int h = defsze.H > 8 ? 10 : defsze.H + 1;
    // let's assume to have a menu enable scrolling for more than 40 columns
    int w = int(item->label().length()) <= defsze.W ? defsze.W : (item->label().length() > 39 ? 40 : item->label().length());
    defsze = wsze( h,  w );

    NCMenuLine * line = new NCMenuLine( treeItem );
    pad->Append( line );

    if (item->selected())
    {
        //retrieve position of item
        int at = treeItem->index();

        //this highlights selected item, possibly unpacks the tree
        //should it be in currently hidden branch
        pad->ShowItem( getTreeLine( at ) );
    }

    //line->stripHotkeys();
}

// Returns current item (pure virtual in YTree)
YMenuItem * NCMenu::currentItem()
{
    return getCurrentItem();
}

// Fills TreePad with lines (uses CreateTreeLines to create them)
void NCMenu::DrawPad()
{
    if ( !myPad() )
    {
        yuiWarning() << "PadWidget not yet created" << std::endl;
        return;
    }

    idx = 0;
    // YItemIterator iterates over the toplevel items
    for ( YItemIterator it = itemsBegin(); it < itemsEnd(); ++it )
    {
        CreateTreeLine( myPad(), *it );
    }

    idx = 0;
    NCPadWidget::DrawPad();
}



NCursesEvent NCMenu::wHandleInput( wint_t key )
{
    NCursesEvent ret = NCursesEvent::none;
    YMenuItem * oldCurrentItem = getCurrentItem();

    bool handled = handleInput( key ); // NCTreePad::handleInput()
    const YItem * currentItem = getCurrentItem();

    if ( !currentItem )
        return ret;
    YMGAMenuItem *mi = dynamic_cast<YMGAMenuItem*>(  getCurrentItem() );
    if (!mi)
      return ret;

    if (mi->enabled())
    {
      if ( ! handled )
      {
          switch ( key )
          {
          // KEY_SPACE is handled in NCMenuLine::handleInput
          case KEY_RETURN:

              if ( notify() )
              {
                  return NCursesEvent::Activated;
              }
              else
              {
                ret =  NCursesEvent::button;
              }
              break;
          }
      }

      YTree::selectItem( const_cast<YItem *>( currentItem ), true );

      if ( notify() && immediateMode() && ( oldCurrentItem != currentItem ) )
          ret = NCursesEvent::SelectionChanged;
    }

    yuiDebug() << "Notify: " << ( notify() ? "true" : "false" ) <<
               " Return event: " << ret.reason << std::endl;

    return ret;
}




// clears the table and the lists holding
//		      the values
void NCMenu::deleteAllItems()
{
    YTree::deleteAllItems();
    myPad()->ClearTable();
}
