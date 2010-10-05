// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bristle_properties_style.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
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
