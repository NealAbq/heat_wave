// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bristle_style.cpp
//
//   Copyright (c) Neal Binnendyk 2009, 2010.
//     <nealabq@gmail.com>
//     <http://nealabq.com/>
//
//   |=== GPL License Notice ====================================================================|
//   | This code is free software: you can redistribute it and/or modify it under the terms      |
//   | of the GNU General Public License as published by the Free Software Foundation, either    |
//   | version 3 of the License, or (at your option) any later version.                          |
//   |                                                                                           |
//   | This code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;    |
//   | without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. |
//   | See the GNU General Public License for more details: <http://www.gnu.org/licenses/>       |
//   |=== END License Notice ====================================================================|
//
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "all.h"
# include "bristle_style.h"
# include <QtGui/QRadioButton>

// _______________________________________________________________________________________________

  /* ctor */
  bristle_style_type::
bristle_style_type( QObject * p_parent, value_type init_value /* = e_none */)
  : holder_base_type ( p_parent   )
  , value_           ( init_value )
{ }

  void
  bristle_style_type::
set_value( value_type new_value)
{
    if ( value_ != new_value ) {
        if ( is_setting( ) ) {
            // This should not recurse with a new value while setting the value.
            d_assert( false);
        } else {
            while_setting_value_wrapper_type wrapper( this);
            value_ = new_value;

            // Emit the signals that update any attached radio buttons.
            // We probably only have to emit one signal here, the true signal.
            // This is usually not necessary. Maybe we should skip it if we can.
            emit has_changed__lines( is_lines( ));
            emit has_changed__dots(  is_dots(  ));
            emit has_changed__none(  is_none(  ));

            wrapper.done_with_no_throws( );
        }
    }
}

  void
  bristle_style_type::
attach
 (  QRadioButton *  p_radio_lines
  , QRadioButton *  p_radio_dots
  , QRadioButton *  p_radio_none
  , bool            init_value_from_holder
 )
{
    d_assert( p_radio_lines);
    d_assert( p_radio_dots );
    d_assert( p_radio_none );

    d_assert( p_radio_lines != p_radio_dots);
    d_assert( p_radio_lines != p_radio_none);
    d_assert( p_radio_dots  != p_radio_none);

    if ( init_value_from_holder ) {
        // Set the init state of the radio buttons from this folder.
        p_radio_lines->setChecked( is_lines( ));
        p_radio_dots ->setChecked( is_dots(  ));
        p_radio_none ->setChecked( is_none(  ));
    } else {
        // Set the init value of this holder from the radio buttons.
        if ( p_radio_lines->isChecked( ) ) {
            d_assert( ! p_radio_dots->isChecked( ));
            d_assert( ! p_radio_none->isChecked( ));
            set_lines( );
        } else
        if ( p_radio_dots->isChecked( ) ) {
            d_assert( ! p_radio_lines->isChecked( ));
            d_assert( ! p_radio_none ->isChecked( ));
            set_dots( );
        } else
        if ( p_radio_none->isChecked( ) ) {
            d_assert( ! p_radio_lines->isChecked( ));
            d_assert( ! p_radio_dots ->isChecked( ));
            set_none( );
        } else {
            // None of the radio buttons are on! Choose arbitrarily.
            d_assert( false);
            set_none( ); /* set the holder value */
            p_radio_none->setChecked( true); /* check the radio button */
        }
    }

    // Tell the radio buttons to update the values here.
    d_verify( connect( p_radio_lines, SIGNAL( toggled( bool)), this, SLOT( set_lines( bool)) ));
    d_verify( connect( p_radio_dots , SIGNAL( toggled( bool)), this, SLOT( set_dots(  bool)) ));
    d_verify( connect( p_radio_none , SIGNAL( toggled( bool)), this, SLOT( set_none(  bool)) ));

    // Propagate changes here back to the radio buttons.
    d_verify( connect( this, SIGNAL( has_changed__lines( bool)), p_radio_lines, SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__dots(  bool)), p_radio_dots , SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__none(  bool)), p_radio_none , SLOT( setChecked( bool)) ));
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bristle_style.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
