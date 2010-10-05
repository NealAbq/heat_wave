// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// face_properties_style.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "all.h"
# include "face_properties_style.h"

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* ctor */
  face_properties_style_type::
face_properties_style_type( QObject * p_parent)
  : holder_base_type    ( p_parent )
  , p_top_face_style_   ( new face_style_type(        this) )
  , p_under_face_style_ ( new face_style_type(        this) )
  , p_top_shininess_    ( new int_range_steps_holder( this, 64, 0, 128) )
  , p_under_shininess_  ( new int_range_steps_holder( this, 32, 0, 128) )
  , p_shading_style_    ( new shading_style_type(     this) )
  , p_is_triangle_mesh_ ( new bool_holder(            this, false) )
  , p_is_interpolate_   ( new bool_holder(            this, false) )
  , p_pixel_width_      ( new int_range_steps_holder( this, 1, 1, 9) )
  , p_antialias_style_  ( new antialias_style_type(   this) )
{
    d_assert( p_top_face_style_  );
    d_assert( p_under_face_style_);
    d_assert( p_top_shininess_   );
    d_assert( p_under_shininess_ );
    d_assert( p_shading_style_   );
    d_assert( p_is_triangle_mesh_);
    d_assert( p_is_interpolate_  );
    d_assert( p_pixel_width_     );
    d_assert( p_antialias_style_ );

    d_verify( connect( p_top_face_style_  , SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_under_face_style_, SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_top_shininess_   , SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_under_shininess_ , SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_shading_style_   , SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_is_triangle_mesh_, SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_is_interpolate_  , SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_pixel_width_     , SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_antialias_style_ , SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
}

  bool
  face_properties_style_type::
is_smooth_shade( ) const
{
    // Is this influenced by p_is_interpolate_? No, although block + interpolate doesn't do much.
    d_assert( p_shading_style_);
    return p_shading_style_->get_value( ) != shading_style_type::e_block;
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// face_properties_style.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
