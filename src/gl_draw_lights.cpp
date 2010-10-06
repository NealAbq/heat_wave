// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_draw_lights.cpp
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
// Notes:
//   The lighting rig can be is_on( ) and (! is_attached_to_gl( )).
//   Why is a light not on?
//      On not requested.
//      Owner not attached.
//      Ran out of indexes.
//
// Improvements:
//   Attach lighting to gl_env::env_type.
//     An env_type should only have one lighting object active at a time.
//     The lighting object should point back to the gl_env::env_type object. We should not
//     use gl_env::global here.
//
//   Make lighting hierarchical.
//     You should be able to define a lighting rig on top of another and selectively either
//     hide or inherit (read-only or mutable) any light attribute.
//     You'd probably want to push a lighting rig anywhere you'd normally push GL_LIGHTING_BIT attributes:
//       gl_env::global::with_saved_server_attributes xx( GL_LIGHTING_BIT)
//
//   Dtor question:
//     When we delete a lighting rig, should we always disable all the lights and/or restore the old state?
//     Ideally, here is how it should work:
//       The env should manage the HGLRC (GL rendering context handle).
//       The env should provide a base lighting rig which serves as the bottom of the stack.
//       Anytime you pop a rig or light off the stack, it should be turned off and unwound (assuming rendering
//         context is valid).
//       When the base env and base lights are deleted, they should turn everything off, but only if the
//         rendering context is still valid.
//       When the env gets rid of a rendering context it doesn't have to unwind everything, but it does have
//         to mark any state that assumes a valid rendering context (like reserved light indexes).
//     Since our env does not manage the rendering context, we cannot do the above.
//       So instead we choose to make our dtors not unwind. The user can unwind explicitly, and if we are
//       pushing/popping lighting rigs we can unwind when we pop.
//
//   In this setup the memory for the lights are managed by the lighting rig.
//   Perhaps it would be better to let the user manage his own light objects. That way they could be
//   static objects, and they could be used in different rigs (although not at the same time).
//
//   The GL_LIGHTING_BIT properties include properties for lighting (which we control here,
//   with the lighting rig), and properties for materials. They are:
//     |
//     Lighting properties:
//       Is overall lighting on or off?
//       Global ambient.
//       set_two_sided_lighting() - GL_LIGHT_MODEL_TWO_SIDE
//       set_local_viewer_light_calculations() - GL_LIGHT_MODEL_LOCAL_VIEWER
//       Individual light properties (for each light):
//         on/off
//         direction or position
//         colors (ambient, diffuse, specular)
//         spot (exponent, cutoff)
//         attenuation
//     |
//     Material properties:
//       Color tracking: is on, which face(s), which material colors are tracking.
//       GL_SHADE_MODEL [set_shade_model_flat() or set_shade_model_smooth()].
//       Material properties:
//         diffuse, ambient, specular, glow or emission, shininess (specular exponent).
//
//   Add to the light:
//     Positional lights.
//     Radius (necessary for positional lights).
//     Is the radius of the light relative to the model or the eye.
//
//     Spot attributes.
//     Attenuation attributes.
//
//     Positional arrow should look different.
//
//   Other functions:
//     Switch off all lights.
//     Remove all lights.
//     Detach the lights from GL (by reseting the gl_index values but not turning the lights
//       off in GL). We could do this in the lighting_type dtor.
//     How many more lights can we turn on?
// _______________________________________________________________________________________________

# include "all.h"
# include "gl_draw_lights.h"
# include "gl_env_global.h"

# include <functional>
# include <boost/bind.hpp>

// _______________________________________________________________________________________________
//
namespace gl_env {
namespace lights {

// _______________________________________________________________________________________________

  /* ctor */
  lighting_type::
lighting_type( )
  : is_attached_to_gl_    ( false)
  , is_on_                ( false)
  , global_ambient_color_ ( 0.2f, 0.2f, 0.2f)
  , lights_               ( )
{ }

// _______________________________________________________________________________________________

  light_type *
  lighting_type::
find_light_at( int index)
{
    if ( index >= 0 ) {
        typedef std::list< light_type >::iterator iter_type;

        // Loop thru the lights.
        iter_type        p_light        = lights_.begin( );
        iter_type const  p_light_limit  = lights_.end( );
        while ( p_light_limit != p_light ) {
            if ( 0 == index ) return & (*p_light);
            -- index;
        }
    }
    return 0;
}

// _______________________________________________________________________________________________

  void
  lighting_type::
attach_to_gl( )
{
    if ( ! is_attached_to_gl( ) ) {
        is_attached_to_gl_ = true;
        if ( is_on( ) ) {
            init_when_on_and_attached( );
        }
    }
    d_assert( is_attached_to_gl( ));
}

  void
  lighting_type::
detach_from_gl( )
  //
  // We need this for when the rendering context is deleted or changes out from underneath us, and
  // we want to apply the lights to a new rendering context.
  //
  // This removes all the gl_index values from the lights, but leaves their requested-on values alone.
{
    if ( is_attached_to_gl( ) ) {
        // Turn off all the lights.
        std::for_each( lights_.begin( ), lights_.end( ), std::mem_fun_ref( & light_type::detach_from_gl));
        is_attached_to_gl_ = false;
    }
    d_assert( ! is_attached_to_gl( ));
}

// _______________________________________________________________________________________________

  void
  lighting_type::
set_on( )
{
    if ( is_off( ) ) {
        is_on_ = true;
        if ( is_attached_to_gl( ) ) {
            init_when_on_and_attached( );
        }
    }
    d_assert( is_on( ));
}

  void
  lighting_type::
set_off( )
{
    if ( is_on( ) ) {
        // Turn off all the lights.
        // Improvement: What if we only want to temporarily disable the lights and then turn them back on?
        //   This is extra work. We do that when we draw the arrows.
        //   The solution is to have a lighting environment stack and a pushable lighting rig.
        std::for_each( lights_.begin( ), lights_.end( ), std::mem_fun_ref( & light_type::set_off));

        // Disable lighting.
        if ( is_attached_to_gl( ) ) {
            gl_env::global::disable_overall_lighting( );
        }

        // This lighting rig is now off.
        is_on_ = false;
    }
    d_assert( is_off( ));
}

// _______________________________________________________________________________________________

  void
  lighting_type::
init_when_on_and_attached( )
{
    d_assert( is_attached_to_gl( ));
    d_assert( is_on( ));

    setup_current_gl_context( );

    // Turn on any lights that want to be on.
    turn_on_all_the_lights_that_need_it( );
}

  /* private */
  void
  lighting_type::
setup_current_gl_context( ) const
{
    // Don't turn on the lights until after GL is set up.
    d_assert( as_bool( ::gl_env::global::is_init( )));

    // Turn on lighting. When this is off, polygons are uniformly colored according to the
    // dropped color. It's like turning the global ambient white and turning off all the
    // individual lights.
    ::gl_env::global::enable_overall_lighting( );

    // This is really part of the model, not the lighting.
    ::gl_env::global::set_two_sided_lighting( true);

    // This setting is right for ortho projections and (slightly) wrong for perspective.
    // Todo: Try setting this true for perspective projections.
    ::gl_env::global::set_local_viewer_light_calculations( false);

    // We always set the color even if the global ambient is not on.
    // Maybe we should inherit the existing global ambient:
    //   global_ambient_color_ = gl_env::global::get_global_ambient_color< color_component_type >( );
    ::gl_env::global::set_global_ambient_color( get_global_ambient_color( ));
}

  /* private */
  void
  lighting_type::
turn_on_all_the_lights_that_need_it( )
{
    d_assert( is_attached_to_gl( ));
    d_assert( is_on( ));

    typedef std::list< light_type >::iterator iter_type;

    // Loop thru the lights.
    iter_type        p_light        = lights_.begin( );
    iter_type const  p_light_limit  = lights_.end( );

    // Loop thru the light-index values.
    int_type         index          = 0;
    int_type  const  index_limit    = ::gl_env::global::get_max_light_count( );

    // Assign light-index values to the lights that want to be switched on.
    while ( (p_light_limit != p_light) && (index_limit != index) ) {
        d_assert( as_bool( ::gl_env::global::is_light_index( index)));

        light_type & light = *p_light;
        d_assert( light.is_off( ));
        d_assert( light.get_owner( ) == this);

        if ( light.is_on_requested( ) ) {
            light.turn_on_using_gl_light_index( index);
            ++ index;
        }
        ++ p_light;
    }
}

// _______________________________________________________________________________________________

  /* private */
  void
  lighting_type::
copy_to_current_gl_context( ) const
  //
  // This copies all the state from this rig to the current OpenGL context without attaching.
  // Presumably the rig is already attached to another context.
  // We use this when we create a temporary gl-context that draws to a pixmap.
{
    if ( is_on( ) ) {
        setup_current_gl_context( );

        typedef std::list< light_type >::const_iterator iter_type;

        // Loop thru the lights.
        iter_type        p_light        = lights_.begin( );
        iter_type const  p_light_limit  = lights_.end( );

        while ( p_light_limit != p_light ) {
            light_type const & light = *p_light;
            if ( light.is_on( ) ) {
                light.copy_to_current_gl_context( );
            }
            ++ p_light;
        }
    }
}

// _______________________________________________________________________________________________

  void
  lighting_type::
set_global_ambient_color( color_type const & color)
{
    global_ambient_color_ = color;
    if ( is_on( ) && is_attached_to_gl( ) ) {
        gl_env::global::set_global_ambient_color( get_global_ambient_color( ));
    }
}

// _______________________________________________________________________________________________

  light_type *
  lighting_type::
add_light( )
  //
  // Returns the new light.
  // The light is off at first.
{
    lights_.push_back( light_type( ));
    light_type * p_new_light = & lights_.back( );
    p_new_light->set_owner( this);
    return p_new_light;
}

  // The following functor is only used inside the next method remove_light(..).
  struct /* functor */
is_ptr_match
  { is_ptr_match( light_type * p)  : p_light( p) { d_assert( p != 0); }
      light_type const * p_light;
    bool operator ()( light_type const & light) { return p_light == & light; }
  };

  void
  lighting_type::
remove_light( light_type * p_old_light)
{
    d_assert( p_old_light && (p_old_light->get_owner( ) == this));

    // Switch the light off.
    p_old_light->set_off( );

    // It'd be nice if we could define the following struct in this local scope.
    // It's allowed in msvc9 but not gcc 4.4. I don't think it's standard.
    //
    //      struct /* functor */
    //    is_ptr_match
    //      .. etc, defined above ..

    // Remove the light from the list. This will delete the light object.
  # ifndef NDEBUG
    size_t const debug_pre_count = lights_.size( );
    d_assert( debug_pre_count > 0);
  # endif
    lights_.remove_if( is_ptr_match( p_old_light));
    d_assert( lights_.size( ) == (debug_pre_count - 1));

    // The pointer is now invalid.
    p_old_light = 0;
}

  /* private */
  int_type
  lighting_type::
find_available_gl_light_index( ) const
{
    d_assert( is_attached_to_gl( ));
    d_assert( is_on( ));

    // This is very simple. Starting from zero, this keeps checking to see if
    // an index is available, and returns it if it is. For N lights this is
    // about order( N^2), so it's a little slow.
    //
    // If there were a lot of lights we'd probably want to keep the list sorted
    // or sort it here. Then we could walk the list once to find the minimum free
    // index.

    if ( is_on( ) ) {
        int_type const index_limit  = gl_env::global::get_max_light_count( );
        int_type       index_search = 0;
        while ( index_search < index_limit ) {
            if ( is_gl_light_index_available( index_search) ) {
                return index_search;
            }
            ++ index_search;
        }
    }
    return -1; /* return -1 if no gl light index is available */
}

  // The following functor is only used inside the next method is_gl_light_index_available(..).
  // It'd be nice if we could declare it in a scope local to that function.
  struct /* functor */
is_gl_light_index_match
  { is_gl_light_index_match( int_type i) : index_( i) { }
      int_type const index_;
    bool operator ()( light_type const & light) { return light.get_raw_gl_index( ) == index_; }
  };

  /* private */
  bool
  lighting_type::
is_gl_light_index_available( int_type index) const
{
    d_assert( is_attached_to_gl( ));
    d_assert( is_on( ));

    return lights_.end( ) ==
        std::find_if( lights_.begin( ), lights_.end( ), is_gl_light_index_match( index));
}

  /* private */
  void
  lighting_type::
listen__gl_light_index_is_now_free( int_type /* index_just_freed */)
{
    // If we were switching lights on and off constantly, and there were a lot of lights,
    // we could use this information to keep track of which indexes are being used.
}

// _______________________________________________________________________________________________

  void
  lighting_type::
place_in_eye_space
 (  angle_type     model_rise     // degrees, -90.0 .. +90.0 (although the range doesn't matter and isn't checked)
  , angle_type     model_spin     // degrees, -180.0 .. +180.0 (although the range doesn't matter and isn't checked)
  , distance_type  model_distance // zero for ortho projection, otherwise positive (checked)
 ) const
{
    d_assert( is_attached_to_gl( ));
    if ( is_on( ) ) {
        std::for_each( lights_.begin( ), lights_.end( ),
            boost::bind( & light_type::place_in_eye_space,
                _1, model_rise, model_spin, model_distance));
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// light_type
//
//   Improvement? Should we 
// _______________________________________________________________________________________________

  /* default ctor */
  light_type::
light_type( )
  : p_owner_                ( 0)
  , gl_index_               ( -1)
  , is_on_requested_        ( false)
  , is_move_with_eye__spin_ ( false)
  , is_move_with_eye__rise_ ( false)
  , spin__degrees_          ( 0)
  , rise__degrees_          ( 0)
  , ambient_color_          ( )
  , diffuse_color_          ( )
  , specular_color_         ( )
{
    d_assert( ! has_owner( ));
    d_assert( is_off( ));
}

  /* copy ctor */
  light_type::
light_type( light_type const & b)
 : p_owner_  ( 0)
 , gl_index_ ( -1)
{
    // This is an incomplete copy. It does not copy the owner, the gl_index, or the is_on( ) state.
    *this = b;
}

  /* copy assignment */
  light_type &
  light_type::
operator =( light_type const & b)
{
    // This is an incomplete copy. It does not copy the owner or the gl_index.
    // It will try to copy the is_on( ) state.

    // Turn the light off before copying.
    set_off( );

    // Copy all the light attributes.
    set_move_with_eye__spin( b.is_move_with_eye__spin( ));
    set_move_with_eye__rise( b.is_move_with_eye__rise( ));

    set_spin__degrees( b.get_spin__degrees( ));
    set_rise__degrees( b.get_rise__degrees( ));

    set_ambient_color(  b.get_ambient_color( ));
    set_diffuse_color(  b.get_diffuse_color( ));
    set_specular_color( b.get_specular_color( ));

    // Turn the light on if appropriate.
    if ( b.is_on_requested( ) ) {
        request_on( );
    }

    return *this;
}

  /* private */
  void
  light_type::
set_owner( lighting_type * p_owner)
{
    // Call this before trying to turn the light on.
    d_assert( ! is_on_requested( ));

    d_assert( 0 != p_owner);
    d_assert( ! has_owner( ));
    p_owner_ = p_owner;
    d_assert( has_owner( ));
}

// _______________________________________________________________________________________________

  void
  light_type::
request_on( )
{
    if ( ! is_on_requested( ) ) {
        is_on_requested_ = true;
        attempt_to_set_on( );
    }
}

  /* private */
  bool
  light_type::
attempt_to_set_on( )
{
    // Only turn on a light that wants to be turned on.
    if ( is_on_requested( ) ) {

        // Only attempt the set the light on if it is now off.
        if ( is_off( ) ) {

            // We can not turn on a light that's not pluged in.
            if ( has_owner( ) && is_owner_attached_to_gl( ) ) {
                // Try to get a valid light-index from the owner.
                int_type const index = get_owner( )->find_available_gl_light_index( );
                if ( gl_env::global::is_light_index( index) ) {
                    turn_on_using_gl_light_index( index);
                    return true;
                }
            }
        }
    }
    return false;
}

  /* private */
  void
  light_type::
turn_on_using_gl_light_index( int_type index)
{
    d_assert( is_owner_attached_to_gl( ));
    d_assert( as_bool( ::gl_env::global::is_light_index( index)));
    d_assert( is_on_requested( ));

    // With a valid light index we can switch on. Reserve this index.
    d_assert( is_off( ));
    d_assert( ! has_valid_gl_light_index( ));
    gl_index_ = index;
    d_assert( has_valid_gl_light_index( ));
    d_assert( is_on( ));

    copy_to_current_gl_context( );
}

  /* private */
  void
  light_type::
copy_to_current_gl_context( ) const
{
    d_assert( is_on( ));

    // Set the light colors before actually switching it on.
    gl_set_light_ambient_color( );
    gl_set_light_diffuse_color( );
    gl_set_light_specular_color( );

    // Switch it on.
    gl_enable( );
}

  void
  light_type::
set_off( )
{
    if ( is_on_requested( ) ) {

        // We need to turn the light off if the request was honored and the light is actually on.
        if ( is_on( ) && is_owner_attached_to_gl( ) ) {
            // Turn the light off.
            gl_disable( );

            // Remember the index.
            int_type const former_index = get_raw_gl_index( );

            // Improvements:
            // Should we return the lights to a default color before switching them off?
            // Or we could return them to the color they were before we switched them on?
            //
            // Should we check to see if a light is already on before we switch it on?
            // If it is should we assume someone else owns it?
            // Same thought applies to light position/attenuation/spot attribs.

            // Free the gl light index. We only keep the index while the light is on.
            d_assert( is_on( ));
            d_assert( has_valid_gl_light_index( ));
            gl_index_ = -1; /* dump the index */
            d_assert( ! has_valid_gl_light_index( ));
            d_assert( is_off( ));

            // Tell the owner an index is free.
            d_assert( has_owner( ));
            get_owner( )->listen__gl_light_index_is_now_free( former_index);
        }

        // We are no longer requested to be on.
        is_on_requested_ = false;
    }
    d_assert( ! is_on_requested( ));
    d_assert( is_off( ));
}

  void
  light_type::
detach_from_gl( )
{
    // Forget the gl light index. It refers to a rendering context that is no longer available.
    // This may already by -1, but set it in any case.
    gl_index_ = -1; /* dump the index */
}

// _______________________________________________________________________________________________

  bool
  light_type::
has_valid_gl_light_index( ) const
{
    d_assert( is_owner_attached_to_gl( ));
    if ( gl_env::global::is_light_index( get_raw_gl_index( )) ) {
        d_assert( -1 != get_raw_gl_index( ));
        d_assert( is_on( )); /* a light must be on in order to have a valid gl_light_index */
        d_assert( is_on_requested( )); /* must be true before a light can be on */
        d_assert( has_owner( )); /* a light must be owned before it can be on */
        return true;
    }
    d_assert( -1 == get_raw_gl_index( )); /* -1 is the invalid index we use */
    d_assert( is_off( )); /* light is off whenever gl_light_index is not valid */
    return false;
}

// _______________________________________________________________________________________________

  void
  light_type::
set_ambient_color( color_type const & color)
{
    // You are allowed to change the light color while the light is on.
    // You do not have do disable the light first.

    ambient_color_ = color;
    if ( is_on( ) && is_owner_attached_to_gl( ) ) {
        gl_set_light_ambient_color( );
    }
}

  void
  light_type::
set_diffuse_color( color_type const & color)
{
    diffuse_color_ = color;
    if ( is_on( ) && is_owner_attached_to_gl( ) ) {
        gl_set_light_diffuse_color( );
    }
}

  void
  light_type::
set_specular_color( color_type const & color)
{
    specular_color_ = color;
    if ( is_on( ) && is_owner_attached_to_gl( ) ) {
        gl_set_light_specular_color( );
    }
}

// _______________________________________________________________________________________________

  /* private */
  void
  light_type::
gl_enable( ) const
{
    d_assert( is_owner_attached_to_gl( ));
    int_type const index = get_valid_gl_light_index( );

    d_assert( ! gl_env::global::is_light_on( index));
    gl_env::global::set_light_on( index);
    d_assert( as_bool( gl_env::global::is_light_on( index)));
}

  /* private */
  void
  light_type::
gl_disable( ) const
{
    d_assert( is_owner_attached_to_gl( ));
    int_type const index = get_valid_gl_light_index( );

    d_assert( as_bool( gl_env::global::is_light_on( index)));
    gl_env::global::set_light_off( index);
    d_assert( ! gl_env::global::is_light_on( index));
}

// _______________________________________________________________________________________________

  /* private */
  void
  light_type::
gl_set_light_ambient_color( ) const
{
    d_assert( is_owner_attached_to_gl( ));
    gl_env::global::set_light_ambient_color( get_valid_gl_light_index( ), ambient_color_);
}

  /* private */
  void
  light_type::
gl_set_light_diffuse_color( ) const
{
    d_assert( is_owner_attached_to_gl( ));
    gl_env::global::set_light_diffuse_color( get_valid_gl_light_index( ), diffuse_color_);
}

  /* private */
  void
  light_type::
gl_set_light_specular_color( ) const
{
    d_assert( is_owner_attached_to_gl( ));
    gl_env::global::set_light_specular_color( get_valid_gl_light_index( ), specular_color_);
}

// _______________________________________________________________________________________________

  void
  light_type::
place_in_eye_space
 (  angle_type     model_rise__degrees
  , angle_type     model_spin__degrees
  , distance_type  model_distance // zero for an ortho projection, otherwise positive
 ) const
{
    d_assert( is_owner_attached_to_gl( ));
    d_assert( model_distance >= 0);

    if ( is_on( ) || is_displaying_arrow( ) ) {
        // Push a new copy of the modelview matrix so we can change it and later pop it.
        gl_env::global::with_current_matrix__assert_push__assert_pop mv( GL_MODELVIEW);

        if ( is_move_with_model__rise( ) ) {
            gl_env::global::rotate_current_matrix( model_rise__degrees, -1, 0, 0);
        }
        if ( is_move_with_model__spin( ) ) {
            gl_env::global::rotate_current_matrix( model_spin__degrees, 0, 0, +1);
        }
        gl_env::global::rotate_current_matrix( get_spin__degrees( ), 0, 0, +1);
        gl_env::global::rotate_current_matrix( get_rise__degrees( ), -1, 0, 0);

        if ( is_on( ) ) {
            // We don't use model_distance for now because the lights are always directional.
            // But if they were positional we'd use the model_distance to place them, but only
            // if the light-radius moved with the eye.
            // But for an ortho projection, moving the light with the eye is the same as
            // making the light directional.

            // Light is directional.
            // The light shines from this point to the orgin, so the direction of the light is
            // really the negative of the specified vector.
            // This position is translated by the inverse of the mv matrix and stored in eye coords.

            // In this case the light direction will end us as a unit vector, because the transforms
            // above only rotate and do not apply the anisotropic scaling we apply when drawing the
            // sheet. This is not exactly right though; we should probably apply the scaling also.
            // But then this vector wouldn't be unit-length unless we inversely scaled it first to
            // the elliptical scaling of the model. Although that's not too hard.
            // We assume this is a UNIT VECTOR in the GLSL shader code.
            // If we ever start placing point lights (instead of directional) we may want to start
            // scaling in order to keep the lights in the model coord system.
            gl_env::global::set_light_direction( get_valid_gl_light_index( ), 0, -1, 0);
        }

        if ( is_displaying_arrow( ) ) {
            display_arrow( model_distance);
        }
    }
}

  /* private */
  void
  light_type::
display_arrow( distance_type model_distance) const
{
    // For perspective or ortho with fixed light (model-relative) position, display a cone or triangles.
    // For directional or ortho with eye-relative position, display a cylinder.
    d_assert( model_distance >= 0);
    maybe_used_only_for_debug( model_distance);

    // We're always directional for now.
    display_arrow__directional( );
}

  /* private */
  void
  light_type::
display_arrow__directional( ) const
  //
  // Experiment: Draw the arrow translucent? If we do this we need to draw it last, and
  // we need to break it up (or clip) where it intersects other transparent polygons so
  // we back-front sort correctly.
  //
  // Experiment: If we add spot and/or position params to the light we will want to indicate
  // these in the arrow drawing. Position could be a cone from light to model-origin, or a drawing
  // of a lightbulb at position.
  //   Directional spot: Saw edges where the points are the angle. Points flatten to 180 for no spot.
  //     Or maybe a cone eminating from the origin? We could use a 3D texture here.
  //   Positional spot: Cone at position?
  // A cone shape on the properties dialog would be useful for spot feedback.
{
    // Leave material colors, color-tracking, lighting, etc the way we found it.
    gl_env::global::with_saved_server_attributes at( GL_LIGHTING_BIT);

    // We can disable overall lighting and use straight color to draw the arrow.
    // Or we can disable color-tracking and use material glow or ambient.
    //   The arrow is parallel to the light so diffuse/specular colors don't do anything
    //   unless we have other lights on at the same time.
    gl_env::global::disable_overall_lighting( ); // GL_LIGHTING_BIT, GL_ENABLE_BIT
    // gl_env::global::disable_material_color_tracking( );

    // We want smooth shading.
    // gl_env::global::set_shade_model_smooth( ); // GL_LIGHTING_BIT
    d_assert( as_bool( gl_env::global::is_shade_model_smooth( )));

    // We want to see the back faces.
    // gl_env::global::disable_face_culling( ); // GL_POLYGON_BIT, GL_ENABLE_BIT
    d_assert( ! gl_env::global::is_face_culling_enabled( ));

    // We want filled polys, not just outlines.
    // gl_env::global::set_polygon_mode( GL_FRONT_AND_BACK, GL_FILL); // GL_POLYGON_BIT
  # ifndef NDEBUG
    { enum_type front_mode = 0;
      enum_type back_mode  = 0;
      gl_env::global::get_polygon_modes( front_mode, back_mode);
      d_assert( GL_FILL == front_mode);
      d_assert( GL_FILL == back_mode );
    }
  # endif

    // Leave the model-view matrix the way we found it.
    gl_env::global::with_current_matrix__assert_push__assert_pop mv( GL_MODELVIEW);

    // Draw a tube with a hexagon cross section. Don't cap the tube because the ends are
    // outside of the clip-box anyway.
    // We should texture this.
    float_type const angle_degrees = 60.0f;
    int countdown = 2;
    for ( ; ; ) {
        display_arrow_ray__directional( countdown);

        if ( 0 == countdown ) break;
        -- countdown;

        // Rotate around the y-axis.
        gl_env::global::rotate_current_matrix( angle_degrees, 0, +1, 0);
    }
}

  /* private */
  void
  light_type::
display_arrow_ray__directional( int countdown) const
{
    // Draw a long narrow rectangle above and below the xy-plane on the y-axis.

    static float_type const  pos_half_width  = 0.02f;
    static float_type const  neg_half_width  = - pos_half_width;

    // If you make pos_inf very large, it looks bad in no-perspective (ortho-projection) mode.
    // It also goes off forever when pointing up or to the side.
    // If it's very large the light arrow is clipped by the front-z and back-z plane.
    // These planes are pretty close in a perspective projection, but in an orhto projection they
    // are very far out.
    // If you make pos_inf too small the arrow gets swallowed up when you make the model xy and z
    // scales larger.
    static float_type const  pos_inf         = 5.0f;
    static float_type const  neg_inf         = - pos_inf;
    static float_type const  pos_close       = 1.5f;
    static float_type const  neg_close       = - pos_close;

    static float_type const  pos_z_coord     = std::sqrt( 3.0f) * pos_half_width;
    static float_type const  neg_z_coord     = - pos_z_coord;

    // Choose a color according to the countdown.
    color_type const  fore_color  = (2 == countdown) ? get_ambient_color( ) : (
                                    (1 == countdown) ? get_diffuse_color( ) : get_specular_color( ));
    color_type const  back_color  = interpolate( 0.2f, color_type( ), fore_color);

    // We are at (0, -Y, 0) looking at the orgin. The near locations are negative.

    // Draw three quads to make the top strip.
    {   // Draw the raw as a strip of 3 quads.
        gl_env::global::with_active_vertex_entry_quad_strip al;

        // All three quads have the same normal.
        // Although the normal isn't used since lighting is disabled.
        gl_env::global::drop_normal_vector( 0.0f, 0.0f, +1.0f);

        // The fore quad is all fore-color.
        // The middle quad is fore-color at the negative-y corners.
        gl_env::global::drop_color( fore_color);

        // Start the fore-quad.
        gl_env::global::drop_vertex( neg_half_width, neg_inf, pos_z_coord);
        gl_env::global::drop_vertex( pos_half_width, neg_inf, pos_z_coord);

        // Finish the fore-quad and start the middle quad.
        gl_env::global::drop_vertex( neg_half_width, neg_close, pos_z_coord);
        gl_env::global::drop_vertex( pos_half_width, neg_close, pos_z_coord);

        // The middle quad is back-color at the positive-y corners.
        // The back quad is all back-color.
        gl_env::global::drop_color( back_color);

        // Finish the middle quad and start the back quad.
        gl_env::global::drop_vertex( neg_half_width, pos_close, pos_z_coord);
        gl_env::global::drop_vertex( pos_half_width, pos_close, pos_z_coord);

        // Finish the back-quad.
        gl_env::global::drop_vertex( neg_half_width, pos_inf, pos_z_coord);
        gl_env::global::drop_vertex( pos_half_width, pos_inf, pos_z_coord);
    }

    // Draw three quads to make the bottom strip.
    {   // Draw the raw as a strip of 3 quads.
        gl_env::global::with_active_vertex_entry_quad_strip al;

        // All three quads have the same normal.
        gl_env::global::drop_normal_vector( 0.0f, 0.0f, -1.0f);

        // The fore quad is all fore-color.
        // The middle quad is fore-color at the negative-y corners.
        gl_env::global::drop_color( fore_color);

        // Start the fore-quad.
        gl_env::global::drop_vertex( pos_half_width, neg_inf, neg_z_coord);
        gl_env::global::drop_vertex( neg_half_width, neg_inf, neg_z_coord);

        // Finish the fore-quad and start the middle quad.
        gl_env::global::drop_vertex( pos_half_width, neg_close, neg_z_coord);
        gl_env::global::drop_vertex( neg_half_width, neg_close, neg_z_coord);

        // The middle quad is back-color at the positive-y corners.
        // The back quad is all back-color.
        gl_env::global::drop_color( back_color);

        // Finish the middle quad and start the back quad.
        gl_env::global::drop_vertex( pos_half_width, pos_close, neg_z_coord);
        gl_env::global::drop_vertex( neg_half_width, pos_close, neg_z_coord);

        // Finish the back-quad.
        gl_env::global::drop_vertex( pos_half_width, pos_inf, neg_z_coord);
        gl_env::global::drop_vertex( neg_half_width, pos_inf, neg_z_coord);
    }
}

// _______________________________________________________________________________________________
//
} /* end namespace lights */
} /* end namespace gl_env */
// _______________________________________________________________________________________________

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_draw_lights.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
