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
#include <yui/gtk/YGUI.h>
#include <yui/YUIException.h>
#include <YWE.h>


#include <string>

#include "YMGA_NCCBTable.h"

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


YMGA_CBTable *
YMGANCWidgetFactory::createTable( YWidget * parent, YTableHeader * header, YTableMode mode )
{
    YMGA_NCCBTable * table = new YMGA_NCCBTable( parent, header, mode );
    YUI_CHECK_NEW( table );

    return table;
}

