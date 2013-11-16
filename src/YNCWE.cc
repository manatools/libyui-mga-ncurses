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

  File:         YNCWE.cc

  Author:       Angelo Naselli <anaselli@linux.it>

/-*/

#include "YNCWE.h"
#include "YMGANCWidgetFactory.h"
#include <yui/YUIException.h>

YNCWE::YNCWE() : YExternalWidgets()
{
}

YNCWE::~YNCWE()
{
}

YExternalWidgetFactory* YNCWE::createExternalWidgetFactory()
{
  YMGANCWidgetFactory * factory = new YMGANCWidgetFactory();
  YUI_CHECK_NEW( factory );

  return factory;
}

YExternalWidgets * createWE(  )
{
    if ( ! YExternalWidgets::externalWidgets() )
    {
        YNCWE * ue = new YNCWE( );
        YUI_CHECK_NEW ( ue );
    }

    return YExternalWidgets::externalWidgets();
}
