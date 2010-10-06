// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bristle_position.cpp
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
# include "bristle_position.h"
# include <QtGui/QRadioButton>

// _______________________________________________________________________________________________

  /* ctor */
  bristle_position_type::
bristle_position_type( QObject * p_parent, value_type init_value /* = e_quad_center */)
  : holder_base_type ( p_parent   )
  , value_           ( init_value )
{ }

  void
  bristle_position_type::
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
            emit has_changed__triangle_center( is_triangle_center( ));
            emit has_changed__quad_center(     is_quad_center(     ));
            emit has_changed__quad_quads(      is_quad_quads(      ));
            emit has_changed__simple_corners(  is_simple_corners(  ));
            emit has_changed__strip_corners(   is_strip_corners(   ));
            emit has_changed__area_corners(    is_area_corners(    ));
            emit has_changed__area_penta(      is_area_penta(      ));

            wrapper.done_with_no_throws( );
        }
    }
}

  void
  bristle_position_type::
attach
 (  QRadioButton *  p_radio_triangle_center
  , QRadioButton *  p_radio_quad_center
  , QRadioButton *  p_radio_quad_quads
  , QRadioButton *  p_radio_simple_corners
  , QRadioButton *  p_radio_strip_corners
  , QRadioButton *  p_radio_area_corners
  , QRadioButton *  p_radio_area_penta
  , bool            init_value_from_holder
 )
{
    d_assert( p_radio_triangle_center);
    d_assert( p_radio_quad_center    );
    d_assert( p_radio_quad_quads     );
    d_assert( p_radio_simple_corners );
    d_assert( p_radio_strip_corners  );
    d_assert( p_radio_area_corners   );
    d_assert( p_radio_area_penta     );

    if ( init_value_from_holder ) {
        // Set the init state of the radio buttons from this folder.
        p_radio_triangle_center->setChecked( is_triangle_center( ));
        p_radio_quad_center    ->setChecked( is_quad_center(     ));
        p_radio_quad_quads     ->setChecked( is_quad_quads(      ));
        p_radio_simple_corners ->setChecked( is_simple_corners(  ));
        p_radio_strip_corners  ->setChecked( is_strip_corners(   ));
        p_radio_area_corners   ->setChecked( is_area_corners(    ));
        p_radio_area_penta     ->setChecked( is_area_penta(      ));
    } else {
        // Set the init value of this holder from the radio buttons.
        if ( p_radio_triangle_center->isChecked( ) ) {
            d_assert( ! p_radio_quad_center   ->isChecked( ));
            d_assert( ! p_radio_quad_quads    ->isChecked( ));
            d_assert( ! p_radio_simple_corners->isChecked( ));
            d_assert( ! p_radio_strip_corners ->isChecked( ));
            d_assert( ! p_radio_area_corners  ->isChecked( ));
            d_assert( ! p_radio_area_penta    ->isChecked( ));
            set_triangle_center( );
        } else
        if ( p_radio_quad_center->isChecked( ) ) {
            d_assert( ! p_radio_triangle_center->isChecked( ));
            d_assert( ! p_radio_quad_quads     ->isChecked( ));
            d_assert( ! p_radio_simple_corners ->isChecked( ));
            d_assert( ! p_radio_strip_corners  ->isChecked( ));
            d_assert( ! p_radio_area_corners   ->isChecked( ));
            d_assert( ! p_radio_area_penta     ->isChecked( ));
            set_quad_center( );
        } else
        if ( p_radio_quad_quads->isChecked( ) ) {
            d_assert( ! p_radio_triangle_center->isChecked( ));
            d_assert( ! p_radio_quad_center    ->isChecked( ));
            d_assert( ! p_radio_simple_corners ->isChecked( ));
            d_assert( ! p_radio_strip_corners  ->isChecked( ));
            d_assert( ! p_radio_area_corners   ->isChecked( ));
            d_assert( ! p_radio_area_penta     ->isChecked( ));
            set_quad_quads( );
        } else
        if ( p_radio_simple_corners->isChecked( ) ) {
            d_assert( ! p_radio_triangle_center->isChecked( ));
            d_assert( ! p_radio_quad_center    ->isChecked( ));
            d_assert( ! p_radio_quad_quads     ->isChecked( ));
            d_assert( ! p_radio_strip_corners  ->isChecked( ));
            d_assert( ! p_radio_area_corners   ->isChecked( ));
            d_assert( ! p_radio_area_penta     ->isChecked( ));
            set_simple_corners( );
        } else
        if ( p_radio_strip_corners->isChecked( ) ) {
            d_assert( ! p_radio_triangle_center->isChecked( ));
            d_assert( ! p_radio_quad_center    ->isChecked( ));
            d_assert( ! p_radio_quad_quads     ->isChecked( ));
            d_assert( ! p_radio_simple_corners ->isChecked( ));
            d_assert( ! p_radio_area_corners   ->isChecked( ));
            d_assert( ! p_radio_area_penta     ->isChecked( ));
            set_strip_corners( );
        } else
        if ( p_radio_area_corners->isChecked( ) ) {
            d_assert( ! p_radio_triangle_center->isChecked( ));
            d_assert( ! p_radio_quad_center    ->isChecked( ));
            d_assert( ! p_radio_quad_quads     ->isChecked( ));
            d_assert( ! p_radio_simple_corners ->isChecked( ));
            d_assert( ! p_radio_strip_corners  ->isChecked( ));
            d_assert( ! p_radio_area_penta     ->isChecked( ));
            set_area_corners( );
        } else
        if ( p_radio_area_penta->isChecked( ) ) {
            d_assert( ! p_radio_triangle_center->isChecked( ));
            d_assert( ! p_radio_quad_center    ->isChecked( ));
            d_assert( ! p_radio_quad_quads     ->isChecked( ));
            d_assert( ! p_radio_simple_corners ->isChecked( ));
            d_assert( ! p_radio_strip_corners  ->isChecked( ));
            d_assert( ! p_radio_area_corners   ->isChecked( ));
            set_area_penta( );
        } else {
            // None of the radio buttons are on! Choose arbitrarily.
            d_assert( false);
            set_triangle_center( ); /* set the holder value */
            p_radio_triangle_center->setChecked( true); /* check the radio button */
        }
    }

    // Tell the radio buttons to update the values here.
    d_verify( connect( p_radio_triangle_center, SIGNAL( toggled( bool)), this, SLOT( set_triangle_center( bool)) ));
    d_verify( connect( p_radio_quad_center    , SIGNAL( toggled( bool)), this, SLOT( set_quad_center(     bool)) ));
    d_verify( connect( p_radio_quad_quads     , SIGNAL( toggled( bool)), this, SLOT( set_quad_quads(      bool)) ));
    d_verify( connect( p_radio_simple_corners , SIGNAL( toggled( bool)), this, SLOT( set_simple_corners(  bool)) ));
    d_verify( connect( p_radio_strip_corners  , SIGNAL( toggled( bool)), this, SLOT( set_strip_corners(   bool)) ));
    d_verify( connect( p_radio_area_corners   , SIGNAL( toggled( bool)), this, SLOT( set_area_corners(    bool)) ));
    d_verify( connect( p_radio_area_penta     , SIGNAL( toggled( bool)), this, SLOT( set_area_penta(      bool)) ));

    // Propagate changes here back to the radio buttons.
    d_verify( connect( this, SIGNAL( has_changed__triangle_center( bool)), p_radio_triangle_center, SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__quad_center(     bool)), p_radio_quad_center    , SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__quad_quads(      bool)), p_radio_quad_quads     , SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__simple_corners(  bool)), p_radio_simple_corners , SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__strip_corners(   bool)), p_radio_strip_corners  , SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__area_corners(    bool)), p_radio_area_corners   , SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__area_penta(      bool)), p_radio_area_penta     , SLOT( setChecked( bool)) ));
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bristle_position.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
