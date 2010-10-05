// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// shading_style.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// What the types mean:
//   Block  - normals are the block/triangle true-normal, smooth color shading is off (single color).
//   Simple - normals are the block/triangle true-normal, smooth color shading is on.
//   Strip  - normals are averaged with blocks before/after in the strip, smooth shading.
//   Area   - normals are area averaged, smooth shading.
//
// Area shading with the pipeline vertex/fragment smooth-shader gives you Gouraud shading.
// Area shading with the Blinn-Phong vertex/fragment shaders give you Blinn-Phong shading.
// _______________________________________________________________________________________________

# include "all.h"
# include "shading_style.h"
# include <QtGui/QRadioButton>

// _______________________________________________________________________________________________

  /* ctor */
  shading_style_type::
shading_style_type( QObject * p_parent, value_type init_value /* = e_area */)
  : holder_base_type ( p_parent   )
  , value_           ( init_value )
{ }

  void
  shading_style_type::
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
            emit has_changed__block(  is_block(  ));
            emit has_changed__simple( is_simple( ));
            emit has_changed__strip(  is_strip(  ));
            emit has_changed__area(   is_area(   ));

            wrapper.done_with_no_throws( );
        }
    }
}

  void
  shading_style_type::
attach
 (  QRadioButton *  p_radio_block
  , QRadioButton *  p_radio_simple
  , QRadioButton *  p_radio_strip
  , QRadioButton *  p_radio_area
  , bool            init_value_from_holder
 )
{
    d_assert( p_radio_block );
    d_assert( p_radio_simple);
    d_assert( p_radio_strip );
    d_assert( p_radio_area  );

    d_assert( p_radio_block  != p_radio_simple);
    d_assert( p_radio_block  != p_radio_strip );
    d_assert( p_radio_block  != p_radio_area  );
    d_assert( p_radio_simple != p_radio_strip );
    d_assert( p_radio_simple != p_radio_area  );
    d_assert( p_radio_strip  != p_radio_area  );

    if ( init_value_from_holder ) {
        // Set the init state of the radio buttons from this folder.
        p_radio_block ->setChecked( is_block(  ));
        p_radio_simple->setChecked( is_simple( ));
        p_radio_strip ->setChecked( is_strip(  ));
        p_radio_area  ->setChecked( is_area(   ));
    } else {
        // Set the init value of this holder from the radio buttons.
        if ( p_radio_block->isChecked( ) ) {
            d_assert( ! p_radio_simple->isChecked( ));
            d_assert( ! p_radio_strip ->isChecked( ));
            d_assert( ! p_radio_area  ->isChecked( ));
            set_block( );
        } else
        if ( p_radio_simple->isChecked( ) ) {
            d_assert( ! p_radio_block->isChecked( ));
            d_assert( ! p_radio_strip->isChecked( ));
            d_assert( ! p_radio_area ->isChecked( ));
            set_simple( );
        } else
        if ( p_radio_strip->isChecked( ) ) {
            d_assert( ! p_radio_block ->isChecked( ));
            d_assert( ! p_radio_simple->isChecked( ));
            d_assert( ! p_radio_area  ->isChecked( ));
            set_strip( );
        } else
        if ( p_radio_area->isChecked( ) ) {
            d_assert( ! p_radio_block ->isChecked( ));
            d_assert( ! p_radio_simple->isChecked( ));
            d_assert( ! p_radio_strip ->isChecked( ));
            set_area( );
        } else {
            // None of the radio buttons are on! Choose arbitrarily.
            d_assert( false);
            set_block( ); /* set the holder value */
            p_radio_block->setChecked( true); /* check the radio button */
        }
    }

    // Tell the radio buttons to update the values here.
    d_verify( connect( p_radio_block , SIGNAL( toggled( bool)), this, SLOT( set_block(  bool)) ));
    d_verify( connect( p_radio_simple, SIGNAL( toggled( bool)), this, SLOT( set_simple( bool)) ));
    d_verify( connect( p_radio_strip , SIGNAL( toggled( bool)), this, SLOT( set_strip(  bool)) ));
    d_verify( connect( p_radio_area  , SIGNAL( toggled( bool)), this, SLOT( set_area(   bool)) ));

    // Propagate changes here back to the radio buttons.
    d_verify( connect( this, SIGNAL( has_changed__block(  bool)), p_radio_block , SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__simple( bool)), p_radio_simple, SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__strip(  bool)), p_radio_strip , SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__area(   bool)), p_radio_area  , SLOT( setChecked( bool)) ));
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// shading_style.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
