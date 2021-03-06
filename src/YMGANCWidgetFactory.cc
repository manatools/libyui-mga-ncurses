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

  File:         YMGANCWidgetFactory.cc

  Author:       Angelo Naselli <anaselli@linux.it>

/-*/

#define YUILogComponent "mga-gtk"
#include <yui/YUILog.h>

#include "YMGANCWidgetFactory.h"
#include <yui/ncurses/YNCursesUI.h>
#include <yui/YUIException.h>
#include <YExternalWidgets.h>


#include <string>

#include "YMGA_NCCBTable.h"
#include "YMGANCMenuBar.h"

using std::string;


YMGANCWidgetFactory::YMGANCWidgetFactory()
    : YMGAWidgetFactory()
{
    // NOP
}


YMGANCWidgetFactory::~YMGANCWidgetFactory()
{
    // NOP
}


YMGA_CBTable * YMGANCWidgetFactory::createCBTable( YWidget * parent, YTableHeader * header )
{
    YCBTableHeader *hdr = dynamic_cast<YCBTableHeader *>(header);
    YUI_CHECK_NEW(hdr);
    YMGA_NCCBTable * table = new YMGA_NCCBTable( parent, hdr );
    YUI_CHECK_NEW( table );

    return table;
}

YMGAMenuBar * YMGANCWidgetFactory::createMenuBar(YWidget* parent)
{
  YMGANCMenuBar * menubar = new YMGANCMenuBar(parent);
  YUI_CHECK_NEW( menubar );

  return menubar;
}


