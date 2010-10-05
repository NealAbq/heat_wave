// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// color_gradient_holder.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// This is a color strip or multi-gradient (the gradients are rgb-linear):
//   For now we have a fixed number of colors at fixed z locations.
//   We also use other colors when drawing:
//     Background color.
//     Label colors, axis colors, overlay colors, hilite colors, etc.
//   This should let you:
//     Add or remove colors from the strip
//     Color strip UI
//       A widget you could embed, menu-pop-up, modal-pop-up, toolbox, or stand-alone.
//       Color choice modal dialog.
// _______________________________________________________________________________________________

# include "all.h"
# include "color_gradient_holder.h"

// _______________________________________________________________________________________________

  /* constructor */
  color_gradient_holder::
color_gradient_holder( QObject * p_parent)
  : holder_base_type( p_parent)
{
    pp_color_holder_[ e_cold  ] =
        new color_holder_type( this, color_type( 0.0f, 0.0f    , 1.0f) /* blue   */, QObject::tr( "Choose cold color" ));

    pp_color_holder_[ e_cool  ] =
        new color_holder_type( this, color_type( 0.0f, 0.66667f, 1.0f) /* aqua   */, QObject::tr( "Choose cool color" ));

    pp_color_holder_[ e_tepid ] =
        new color_holder_type( this, color_type( 1.0f, 0.0f    , 0.0f) /* red    */, QObject::tr( "Choose tepid color"));

    pp_color_holder_[ e_warm  ] =
        new color_holder_type( this, color_type( 1.0f, 0.66667f, 0.0f) /* orange */, QObject::tr( "Choose warm color" ));

    pp_color_holder_[ e_hot   ] =
        new color_holder_type( this, color_type( 1.0f, 1.0f    , 0.0f) /* yellow */, QObject::tr( "Choose hot color"  ));

    d_verify( connect( pp_color_holder_[ e_cold  ], SIGNAL( has_changed( )), this, SIGNAL( has_changed( ))));
    d_verify( connect( pp_color_holder_[ e_cool  ], SIGNAL( has_changed( )), this, SIGNAL( has_changed( ))));
    d_verify( connect( pp_color_holder_[ e_tepid ], SIGNAL( has_changed( )), this, SIGNAL( has_changed( ))));
    d_verify( connect( pp_color_holder_[ e_warm  ], SIGNAL( has_changed( )), this, SIGNAL( has_changed( ))));
    d_verify( connect( pp_color_holder_[ e_hot   ], SIGNAL( has_changed( )), this, SIGNAL( has_changed( ))));
}

// _______________________________________________________________________________________________

  /* getter */
  color_gradient_holder::color_holder_type *
  color_gradient_holder::
get_color_holder_at( int index) const
{
    d_assert( index >= 0);
    d_assert( index <  get_color_count( ));
    return pp_color_holder_[ index ];
}

// _______________________________________________________________________________________________

  static
  float const
s_normal_boundary_array[ 7 ] = { 0.00f, 0.10f, 0.38f, 0.50f, 0.62f, 0.90f, 1.00f };

  float
  color_gradient_holder::
get_normal_min_at( int index) const
{
    d_assert( index >= 0);
    d_assert( index <  get_color_count( ));
    return s_normal_boundary_array[ index ];
}

  float
  color_gradient_holder::
get_normal_mid_at( int index) const
{
    d_assert( index >= 0);
    d_assert( index <  get_color_count( ));
    return s_normal_boundary_array[ index + 1 ];
}

  float
  color_gradient_holder::
get_normal_max_at( int index) const
{
    d_assert( index >= 0);
    d_assert( index <  get_color_count( ));
    return s_normal_boundary_array[ index + 2 ];
}

// _______________________________________________________________________________________________

  color_gradient_holder::color_type
  color_gradient_holder::
get_color_at_normal( float norm_to_match) const
{
    d_assert( get_color_count( ) > 0);

    color_holder_type const * const *        pp_color_holder      = pp_color_holder_;
    color_holder_type const * const * const  pp_color_holder_max  = pp_color_holder + (get_color_count( ) - 1);
    color_holder_type const *                p_color_holder_hi    = * pp_color_holder;

    float const *  p_norm_hi  = s_normal_boundary_array + 1;
    float          norm_hi    = * p_norm_hi;

    if ( norm_to_match <= norm_hi ) {
        return p_color_holder_hi->get_color( );
    }

    while ( pp_color_holder < pp_color_holder_max ) {
        color_holder_type const * const p_color_holder_lo = p_color_holder_hi;
        ++ pp_color_holder;
        p_color_holder_hi = * pp_color_holder;

        float const norm_lo = norm_hi;
        ++ p_norm_hi;
        norm_hi = * p_norm_hi;
        d_assert( norm_hi > norm_lo);

        if ( norm_to_match <= norm_hi ) {
            float const factor = (norm_to_match - norm_lo) / (norm_hi - norm_lo);
            return interpolate( factor,
                p_color_holder_lo->get_color( ),
                p_color_holder_hi->get_color( ));
        }
    }
    return p_color_holder_hi->get_color( );
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// color_gradient_holder.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
