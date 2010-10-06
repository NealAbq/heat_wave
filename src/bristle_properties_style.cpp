// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bristle_properties_style.cpp
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
# include "bristle_properties_style.h"

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* ctor */
  bristle_properties_style_type::
bristle_properties_style_type( QObject * p_parent)
  : holder_base_type   ( p_parent )
  , p_style_           ( new bristle_style_type(     this) )
  , p_position_        ( new bristle_position_type(  this) )
  , p_top_length_      ( new int_range_steps_holder( this, 16, 0, 100) )
  , p_under_length_    ( new int_range_steps_holder( this, 10, 0, 100) )
  , p_pixel_width_     ( new int_range_steps_holder( this, 1, 1, 9) )
  , p_antialias_style_ ( new antialias_style_type(   this) )
  , p_top_color_       ( new color_holder(           this, color_holder::color_type( 1.0f, 0.0f, 0.7f, 1), tr( "Choose top-bristle color")) )
  , p_under_color_     ( new color_holder(           this, color_holder::color_type( 0.0f, 1.0f, 0.3f, 1), tr( "Choose under-bristle color")) )
{
    d_assert( p_style_          );
    d_assert( p_position_       );
    d_assert( p_top_length_     );
    d_assert( p_under_length_   );
    d_assert( p_pixel_width_    );
    d_assert( p_antialias_style_);
    d_assert( p_top_color_      );
    d_assert( p_under_color_    );

    d_verify( connect( p_style_          , SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_position_       , SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_top_length_     , SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_under_length_   , SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_pixel_width_    , SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_antialias_style_, SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_top_color_      , SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_under_color_    , SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bristle_properties_style.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
