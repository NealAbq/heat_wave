// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// isotherm_properties_style.cpp
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
# include "isotherm_properties_style.h"
# include "gl_env_global.h"

using gl_env:: int_type   ;
using gl_env:: uint_type  ;
using gl_env:: ubyte_type ;
using gl_env:: float_type ;

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  with_isotherm_texture::
with_isotherm_texture( isotherm_properties_style_type const * p_props)
  : is_showing_isotherms( p_props->get_is_showing( )->get_value( ))
{
    if ( is_showing_isotherms ) {
        // This is meaningless to GLSL.
        gl_env::global::enable_auto_texture_s_gen( );
        gl_env::global::enable_1D_texturing( );
    }
}

  with_isotherm_texture::
~with_isotherm_texture( )
{
    if ( is_showing_isotherms ) {
        gl_env::global::disable_1D_texturing( );
        gl_env::global::disable_auto_texture_s_gen( );
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* ctor */
  isotherm_properties_style_type::
isotherm_properties_style_type( QObject * p_parent)
  : holder_base_type ( p_parent )
  , p_is_showing_    ( new bool_holder(            this, false))
  , p_width_         ( new int_range_steps_holder( this, 30, 5, 100) )
  , p_spacing_       ( new int_range_steps_holder( this, 50, 1, 100) )
  , p_offset_        ( new int_range_steps_holder( this, 50, 0, 100) ) /* modulo */
  , p_color_         ( new color_holder(           this, color_holder::color_type( 0.0f, 0.0f, 0.0f, 1), tr( "Choose isotherm color")) )
  , p_opacity_       ( new int_range_steps_holder( this, 60, 1, 100) )
  , texture_id_      ( 0)
  , texture_id2_     ( 0)
{
    d_assert( p_is_showing_);
    d_assert( p_width_     );
    d_assert( p_spacing_   );
    d_assert( p_offset_    );
    d_assert( p_color_     );
    d_assert( p_opacity_   );

    d_verify( connect( p_is_showing_, SIGNAL( has_changed( )), this, SIGNAL( has_changed( )) ));
    d_verify( connect( p_width_     , SIGNAL( has_changed( )), this, SLOT( has_changed__width_or_spacing( )) ));
    d_verify( connect( p_spacing_   , SIGNAL( has_changed( )), this, SLOT( has_changed__width_or_spacing( )) ));
    d_verify( connect( p_offset_    , SIGNAL( has_changed( )), this, SLOT( has_changed__offset( )) ));
    d_verify( connect( p_color_     , SIGNAL( has_changed( )), this, SLOT( has_changed__color( )) ));
    d_verify( connect( p_opacity_   , SIGNAL( has_changed( )), this, SLOT( has_changed__opacity( )) ));
}

// _______________________________________________________________________________________________

  bool
  isotherm_properties_style_type::
setup_gl( )
{
    d_assert( 0 == texture_id_);
    if ( create_and_bind_texture_id( texture_id_) ) {
        d_assert( 0 != texture_id_);

        build_bound_texture_unit( );
        setup_texture_environment( );
        return true;
    }
    d_assert( 0 == texture_id_);
    return false;
}

  bool
  isotherm_properties_style_type::
setup_gl_in_clone_context( )
{
    d_assert( 0 == texture_id2_);
    if ( create_and_bind_texture_id( texture_id2_) ) {
        d_assert( 0 != texture_id2_);

        build_bound_texture_unit( );
        setup_texture_environment( );
        return true;
    }
    d_assert( 0 == texture_id2_);
    return false;
}

// _______________________________________________________________________________________________

  bool
  isotherm_properties_style_type::
teardown_gl( )
{
    if ( 0 != texture_id_ ) {
        unbind_and_destroy_texture_id( texture_id_);
        d_assert( 0 == texture_id_);
        return true;
    }
    return false;
}

  bool
  isotherm_properties_style_type::
teardown_gl_in_clone_context( )
{
    if ( 0 != texture_id2_ ) {
        unbind_and_destroy_texture_id( texture_id2_);
        d_assert( 0 == texture_id2_);
        return true;
    }
    return false;
}

// _______________________________________________________________________________________________

  /* static */
  bool
  isotherm_properties_style_type::
create_and_bind_texture_id( uint_type & texture_id)
{
    d_assert( 0 == texture_id);
    d_assert( gl_env::global::get_bound_1D_texture( ) == texture_id);

    // We can even bind other 1D textures if we wrap those bindings in:
    //   gl_env::global::with_saved_server_attributes tx( GL_TEXTURE_BIT);
    texture_id = gl_env::global::create_texture_id( );
    if ( 0 == texture_id ) {
        return false;
    }

    gl_env::global::bind_1D_texture( texture_id); /* remains bound for life of context */

    d_assert( 0 != texture_id);
    d_assert( gl_env::global::get_bound_1D_texture( ) == texture_id);
    return true;
}

  void
  isotherm_properties_style_type::
build_bound_texture_unit( ) const
{
    d_assert( gl_env::global::get_bound_1D_texture( ) != 0);

    // Use GL_NEAREST for sharp lines.
    // Use GL_LINEAR (or setup mipmaps) if you want smooth transitions.
    // These define the texture unit.
    gl_env::global::set_1D_texture_minifying_filter( GL_NEAREST);
    gl_env::global::set_1D_texture_magnifying_filter( GL_NEAREST);
    gl_env::global::set_1D_texture_s_wrap( GL_REPEAT);

    // Fill the texture pixels.
    setup_stripe_look( );
}

  /* static */
  void
  isotherm_properties_style_type::
unbind_and_destroy_texture_id( uint_type & texture_id)
{
    d_assert( 0 != texture_id);
    d_assert( gl_env::global::get_bound_1D_texture( ) == texture_id);

    gl_env::global::unbind_1D_texture( );
    gl_env::global::delete_texture_id( texture_id);
    texture_id = 0;

    d_assert( 0 == texture_id);
    d_assert( gl_env::global::get_bound_1D_texture( ) == texture_id);
}

// _______________________________________________________________________________________________

  bool
  isotherm_properties_style_type::
setup_gl_in_shared_context( ) const
{
    if ( 0 != texture_id_ ) {
        bind_texture_id_in_sister_context( );
        setup_texture_environment( );
        return true;
    }
    return false;
}

  void
  isotherm_properties_style_type::
bind_texture_id_in_sister_context( ) const
{
    d_assert( 0 != texture_id_);
    d_assert( 0 == gl_env::global::get_bound_1D_texture( ));

    gl_env::global::bind_1D_texture( texture_id_); /* remains bound for life of context */

    d_assert( 0 != texture_id_);
    d_assert( gl_env::global::get_bound_1D_texture( ) == texture_id_);
}

// _______________________________________________________________________________________________

  void
  isotherm_properties_style_type::
setup_texture_environment( ) const
{
    // Setup texture parameters. These are server attributes and can pushed with:
    //   gl_env::global::with_saved_server_attributes tx( GL_TEXTURE_BIT);

    // We will GL_BLEND with an environmental color and luminance.
    // These are NOT part of the texture unit.
    // These are ignored (and not accessible) by GLSL shaders.
    gl_env::global::set_auto_texture_s_gen_mode( GL_OBJECT_LINEAR);
    gl_env::global::set_texture_env_mode( GL_BLEND);

    // Set properties that are not texture unit, and not accessible thru GLSL.
    setup_stripe_place( );
    setup_stripe_color( );
}

// _______________________________________________________________________________________________

  /* slot */
  void
  isotherm_properties_style_type::
has_changed__width_or_spacing( )
{
    setup_stripe_look(  ); /* texture unit props */
    setup_stripe_place( ); /* texture environment props */
    emit has_changed( );
}

  /* slot */
  void
  isotherm_properties_style_type::
has_changed__opacity( )
{
    setup_stripe_look( ); /* texture unit props */
    emit has_changed( );
}

  /* slot */
  void
  isotherm_properties_style_type::
has_changed__offset( )
{
    setup_stripe_place( ); /* texture environment props */
    emit has_changed( );
}

  /* slot */
  void
  isotherm_properties_style_type::
has_changed__color( )
{
    setup_stripe_color( ); /* texture environment props */
    emit has_changed( );
}

// _______________________________________________________________________________________________

  void
  isotherm_properties_style_type::
setup_stripe_color( ) const
{
    // This will not work with GLSL shaders.
    gl_env::global::set_texture_env_color( get_shader_color( ));
}

// _______________________________________________________________________________________________

  void
  isotherm_properties_style_type::
setup_stripe_place( ) const
  //
  // Call this if the stripe width and/or offset changes.
{
    // This will not work with GLSL shaders.
    gl_env::global::set_auto_texture_s_gen_object_plane
     (  0.0f
      , 0.0f
      , get_shader_z_factor( )
      , get_shader_offset( )
     );
}

// _______________________________________________________________________________________________

  /* private function */
  namespace /* anonymous */ {
  void
prepare_isotherm_texture( float_type width_f, float_type opacity_f)
  //
  // This should be part of a gl_env texture object.
  //
  // Load two stripes into the current-bound 1-D texture.
  // The low stripe is a luminance value corresponding to opacity_f, and the high
  // strip is all zeros.
  // This is intended to be used with GL_BLEND.
  //
  // We could make this work with GL_DECAL and we wouldn't need to set the color with
  // set_texture_env_color(..). The formula for DECAL is (At is alpha-texture, Af is
  // alpha-fragment before the texture is applied):
  //   C = Cf * (1 - At) + Ct * At
  //   A = Af
  //
  // We could also make this work with GL_REPLACE. The formula is very simple:
  //   C = Ct
  //   A = At
  //
  // width_f should be [0 .. 1]
  // opacity_f should be [0 .. 1]
{

    // We load the texture as ubytes.
    // If we decide to load it as floats instead we can just use opacity_f directly.
    d_assert( (0 < opacity_f) && (opacity_f <= 1.0));
    ubyte_type const    opacity_ub  = static_cast< ubyte_type >( 255.0f * opacity_f);

    int_type const      available_buf_size  = 512;
    ubyte_type          buf[ available_buf_size ];

    static int_type     buf_size_i  = 0;
    if ( 0 == buf_size_i ) {
        int_type const allowed_buf_size = gl_env::global::get_max_texture_size( );
        buf_size_i = std::min( available_buf_size, allowed_buf_size);
        d_assert( buf_size_i > 0);
        d_assert( buf_size_i >= gl_env::global::get_min_max_texture_size( ));
    }
    static
    float_type const    buf_size_f  = buf_size_i;

    d_assert( (0 < width_f) && (width_f <= 1.0));
    int_type const      border      = static_cast< int_type >( buf_size_f * width_f);
    d_assert( (0 <= border) && (border <= buf_size_i));
    // Should we set border to 1 if it's zero?
    // Should we set border to (buf_size_i - 1) if it's buf_size_i?

    // Pointer to the first transparent pixel.
    ubyte_type * const  p_border    = buf + border;

    // Fill the texture in as two stripes.
    std::fill( buf, p_border, opacity_ub);
    std::fill( p_border, buf + buf_size_i, 0);

    // The following is necessary before we call:
    //   set_1D_texture_bits__no_border__..(..)
    // Although we are only reading one row, the memory is on the stack and we
    // don't want to worry about its alignment. Although it will probably work
    // without setting alignment to 1.
    gl_env::global::set_unpack_pixel_row_byte_alignment( 1);

    // Load the striped buffer into the currently-bound 1D texture.
    gl_env::global::set_1D_texture_bits__no_border__luminance( buf_size_i, buf);
} } /* namespace anonymous */

  void
  isotherm_properties_style_type::
setup_stripe_look( ) const
  //
  // Call this if the strip width and/or opacity changes.
{
    float const  width    = get_normal_width( );
    float const  spacing  = get_normal_spacing( );
    float const  opacity  = get_normal_opacity( );

    prepare_isotherm_texture( width / (width + spacing), opacity);
}

// _______________________________________________________________________________________________

  float
  isotherm_properties_style_type::
get_shader_z_factor( ) const
  //
  // Returns a value in range [1, big)
{
    float const  width    = get_normal_width(   );  /* (0, 0.25] */
    float const  spacing  = get_normal_spacing( );  /* (0, 1.00] */
    return 1.25f / (width + spacing);  /* [1, big) */
}

// _______________________________________________________________________________________________

  float
  isotherm_properties_style_type::
get_normal_width( ) const
  //
  // Returns width in range (0, 0.25].
{
    float width = get_normal_1_100( get_width( )->get_value( ));
    width /= 2;
    width *= width;
    d_assert( (0 < width) && (width <= 0.25f));
    return width;
}

  float
  isotherm_properties_style_type::
get_normal_spacing( ) const
  //
  // Returns spacing in range (0, 1].
{
    float const spacing = get_normal_1_100( get_spacing( )->get_value( ));
    d_assert( (0 < spacing) && (spacing <= 1));
    return spacing;
}

  float
  isotherm_properties_style_type::
get_normal_offset( ) const
  //
  // Returns spacing in range [0, 1].
{
    float const offset = get_normal_0_100( get_offset( )->get_value( ));
    d_assert( (0 <= offset) && (offset <= 1));
    return offset;
}

  float
  isotherm_properties_style_type::
get_normal_opacity( ) const
  //
  // Returns spacing in range (0, 1].
{
    float const opacity = get_normal_1_100( get_opacity( )->get_value( ));
    d_assert( (0 < opacity) && (opacity <= 1));
    return opacity;
}

// _______________________________________________________________________________________________

  /* static */
  float
  isotherm_properties_style_type::
get_normal_0_100( float value_0_100)
  //
  // In clamped to [0, 100].
  // Out in range [0.01, 1].
  //
  // This should be part of a general set of functions:
  //    get_clamped< lo, hi >( v)
  //    get_modulo< lo, hi, algorithm >( v)
  //    verify_clamped< lo, hi >( v)
  //    get_half, get_double, get_square, get_min( var args), verify positive, range 0..1, -1..+1, etc.
{
    if ( value_0_100 < 0 ) {
        value_0_100 = 0;
    } else
    if ( value_0_100 > 100 ) {
        value_0_100 = 100;
    }
    return value_0_100 / 100.0f;
}

  /* static */
  float
  isotherm_properties_style_type::
get_normal_1_100( float value_1_100)
  //
  // In clamped to [1, 100].
  // Out in range [0.01, 1].
{
    if ( value_1_100 < 1 ) {
        value_1_100 = 1;
    } else
    if ( value_1_100 > 100 ) {
        value_1_100 = 100;
    }
    return value_1_100 / 100.0f;
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// isotherm_properties_style.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
