// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// antialias_style.cpp
//
//   Copyright (c) Neal Binnendyk 2009, 2010. <nealabq@gmail.com> nealabq.com
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
// The UI lets you choose between 3 anti-alias styles: best, fastest, and none.
// The UI presents this choice as a radio button.
// An anti-alias-style object is a holder obect, coordinating between the Qt widgets (radio buttons),
// the animation engine, and the drawing code when it asks what kind of anti-aliasing to use.
// _______________________________________________________________________________________________

# include "all.h"
# include "antialias_style.h"
# include "gl_env_global.h"
# include <QtGui/QRadioButton>

// _______________________________________________________________________________________________

  /* ctor */
  antialias_style_type::
antialias_style_type( QObject * p_parent, value_type init_value /* = e_none */)
  : holder_base_type ( p_parent   )
  , value_           ( init_value )
{ }

  void
  antialias_style_type::
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
            emit has_changed__best(    is_best(    ));
            emit has_changed__fastest( is_fastest( ));
            emit has_changed__none(    is_none( )  );

            wrapper.done_with_no_throws( );
        }
    }
}

  void
  antialias_style_type::
attach
 (  QRadioButton *  p_radio_best
  , QRadioButton *  p_radio_fastest
  , QRadioButton *  p_radio_none
  , bool            init_value_from_holder
 )
{
    d_assert( p_radio_best   );
    d_assert( p_radio_fastest);
    d_assert( p_radio_none   );

    d_assert( p_radio_best    != p_radio_fastest);
    d_assert( p_radio_best    != p_radio_none   );
    d_assert( p_radio_fastest != p_radio_none   );

    if ( init_value_from_holder ) {
        // Set the init state of the radio buttons from this folder.
        p_radio_best   ->setChecked( is_best(    ));
        p_radio_fastest->setChecked( is_fastest( ));
        p_radio_none   ->setChecked( is_none(    ));
    } else {
        // Set the init value of this holder from the radio buttons.
        if ( p_radio_best->isChecked( ) ) {
            d_assert( ! p_radio_fastest->isChecked( ));
            d_assert( ! p_radio_none   ->isChecked( ));
            set_best( );
        } else
        if ( p_radio_fastest->isChecked( ) ) {
            d_assert( ! p_radio_best->isChecked( ));
            d_assert( ! p_radio_none->isChecked( ));
            set_fastest( );
        } else
        if ( p_radio_none->isChecked( ) ) {
            d_assert( ! p_radio_best   ->isChecked( ));
            d_assert( ! p_radio_fastest->isChecked( ));
            set_none( );
        } else {
            // None of the radio buttons are on! Choose arbitrarily.
            d_assert( false);
            set_none( ); /* set the holder value */
            p_radio_none->setChecked( true); /* check the radio button */
        }
    }

    // Tell the radio buttons to update the values here.
    d_verify( connect( p_radio_best   , SIGNAL( toggled( bool)), this, SLOT( set_best(    bool)) ));
    d_verify( connect( p_radio_fastest, SIGNAL( toggled( bool)), this, SLOT( set_fastest( bool)) ));
    d_verify( connect( p_radio_none   , SIGNAL( toggled( bool)), this, SLOT( set_none(    bool)) ));

    // Propagate changes here back to the radio buttons.
    d_verify( connect( this, SIGNAL( has_changed__best(    bool)), p_radio_best   , SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__fastest( bool)), p_radio_fastest, SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__none(    bool)), p_radio_none   , SLOT( setChecked( bool)) ));
}

  void
  antialias_style_type::
set_gl( bool is_points, bool is_lines) const
  //
  // As for polygon anti-aliasing, the GL_POLYGON_SMOOTH doc says:
  //  "For correct antialiased polygons, an alpha buffer is needed and the polygons must be sorted front to back."
  // This probably means that jaggy edge pixels effectively have an alpha value and blend with the polygons behind.
  // gl_env::global::enable( GL_POLYGON_SMOOTH);
  //
  // gl_env::global::enable( GL_DITHER);
  //
  // Multisample - sub-pixel rendering (like anti-aliasing).
  // Not defined in version 1.1.
  // gl_env::global::enable( GL_MULTISAMPLE);
{
    if ( is_best( ) ) {
        if ( is_points ) {
            // I've seen some glitches with smooth points. Maybe disabling smooth before
            // setting the hint will help. Since this is intermitent I don't know if this
            // is a fix or not.
            gl_env::global::disable( GL_POINT_SMOOTH);
            gl_env::global::set_hint( GL_POINT_SMOOTH_HINT, GL_NICEST);
            gl_env::global::enable( GL_POINT_SMOOTH);
        }
        if ( is_lines ) {
            gl_env::global::disable( GL_LINE_SMOOTH);
            gl_env::global::set_hint( GL_LINE_SMOOTH_HINT, GL_NICEST);
            gl_env::global::enable( GL_LINE_SMOOTH);
        }
    } else
    if ( is_fastest( ) ) {
        if ( is_points ) {
            gl_env::global::disable( GL_POINT_SMOOTH);
            gl_env::global::set_hint( GL_POINT_SMOOTH_HINT, GL_FASTEST);
            gl_env::global::enable( GL_POINT_SMOOTH);
        }
        if ( is_lines ) {
            gl_env::global::disable( GL_LINE_SMOOTH);
            gl_env::global::set_hint( GL_LINE_SMOOTH_HINT, GL_FASTEST);
            gl_env::global::enable( GL_LINE_SMOOTH);
        }
    } else
    /* is_none( ) */ {
        d_assert( is_none( ));
        if ( is_points ) {
            gl_env::global::disable( GL_POINT_SMOOTH);
        }
        if ( is_lines ) {
            gl_env::global::disable( GL_LINE_SMOOTH);
        }
    }
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// antialias_style.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
