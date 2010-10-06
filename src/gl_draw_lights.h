// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_draw_lights.h
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
# pragma once
# ifndef GL_DRAW_LIGHTS_H
# define GL_DRAW_LIGHTS_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include <list>
# include "gl_env_type_primitives.h"
# include "gl_env_type_colors.h"

// _______________________________________________________________________________________________

namespace gl_env {
namespace lights {

// _______________________________________________________________________________________________

class lighting_type ; /* declared below */
class light_type    ; /* declared below */

typedef float_type                         color_component_type ;
typedef rgb_type<  color_component_type >  color_type           ; /* used below */
typedef rgba_type< color_component_type >  color_store_type     ; /* used below (internally) */
typedef double                             angle_type           ; /* used below */
typedef float                              distance_type        ; /* used below */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// lighting_type
// _______________________________________________________________________________________________

  class
lighting_type
{
  friend class light_type;

  // Ctor and Dtor
  public:
    /* ctor */          lighting_type( )                                ;
    /* dtor */          ~lighting_type( )                               { /* detach_from_gl( ) - why bother */ }

  // Disable copy
  private:
    /* copy */          lighting_type( lighting_type const &)           ; // no implementation
    void                operator =(    lighting_type const &)           ; // no implementation

  // On and off
  public:
    bool                is_attached_to_gl( )                      const { return is_attached_to_gl_; }
    void                attach_to_gl( )                                 ; /* init function */
    void                detach_from_gl( )                               ;

    void                copy_to_current_gl_context( )             const ; /* setup gl without attaching */

    bool                is_on(  )                                 const { return   is_on_; }
    bool                is_off( )                                 const { return ! is_on_; }

    void                set_on(  )                                      ; /* could be named attach_to_gl( ) */
    void                set_off( )                                      ;
  protected:
    void                init_when_on_and_attached( )                    ;
    void                turn_on_all_the_lights_that_need_it( )          ;
    void                setup_current_gl_context( )               const ;

  // Global ambient
  public:
    color_type const &  get_global_ambient_color( )               const { return global_ambient_color_; }
    void                set_global_ambient_color( color_type const &)   ;

  // Add and remove individual lights
  public:
    int                 get_light_count( )                        const { return lights_.size( ); }
    light_type *        find_light_at( int)                             ;
    light_type *        add_light( )                                    ;
    void                remove_light( light_type *)                     ;
  protected:
    int_type            find_available_gl_light_index( )          const ;
    bool                is_gl_light_index_available( int_type)    const ;
    void                listen__gl_light_index_is_now_free( int_type)   ; /* called from light_type */

  // Call this from drawing code
  public:
    void                place_in_eye_space
                         (  angle_type     model_rise     // degrees, -90.0 .. +90.0 (although the range doesn't matter and isn't checked)
                          , angle_type     model_spin     // degrees, -180.0 .. +180.0 (although the range doesn't matter and isn't checked)
                          , distance_type  model_distance // zero for ortho projection, otherwise positive (checked)
                         )                                        const ;

  // Member vars
  private:
    bool                     is_attached_to_gl_    ;
    bool                     is_on_                ;
    color_store_type         global_ambient_color_ ;
    std::list< light_type >  lights_               ;
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// light_type
// _______________________________________________________________________________________________

  class
light_type
  //
  // Directional, not positioned, light.
  // No spot effects.
  // No attenuation.
{
  friend class lighting_type; /* owner type */

  // Ctor. The owner is the only one that creates these objects.
  // Dtor. The owner must turn a light off before deleting it.
  // These should be private except std::list<..> has to be able to get them.
  public:
    /* ctor */          light_type( )                              ;
    /* dtor */          ~light_type( )                             { }
    // We could d_assert( is_off( )) in the dtor if the owner dtor either turned off all the
    // lights or diconnected all the lights from GL by clearing the gl_index values.

  // Copy. This does not copy the owner or the gl_index.
  public:
    /* copy */          light_type( light_type const &)            ;
    light_type &        operator =( light_type const &)            ;

  // Owner ptr
  public:
    bool                has_owner( )                         const { return 0 != p_owner_; }
    lighting_type *     get_owner( )                         const { d_assert( p_owner_ != 0); return p_owner_; }
  protected:
    void                set_owner( lighting_type *)                ;

  // On/off
  public:
    bool                is_owner_attached_to_gl( )            const { return get_owner( )->is_attached_to_gl( ); }

    bool                is_on_requested( )                    const { return is_on_requested_; }

    // A light can only be on when:
    //   is_owner_attached_to_gl( ), and
    //   is_on_requested( ), and
    //   a GL light index has been given to the light
    //     an attached owner gives GL index values to the lights
    //     where is_on_requested( ) until it runs out of indexes.
    bool                is_off( )                             const { return get_raw_gl_index( ) == -1; }
    bool                is_on(  )                             const { return ! is_off( ); }

    void                request_on( )                               ; /* will go on if it can, or at next opportunity */
    void                set_off( )                                  ;
  protected:
    void                detach_from_gl( )                           ;
    bool                attempt_to_set_on( )                        ;
    void                turn_on_using_gl_light_index( int_type)     ;
    void                copy_to_current_gl_context( )         const ;
  public:
    bool                has_valid_gl_light_index( )           const ;
    int_type            get_valid_gl_light_index( )           const { d_assert( has_valid_gl_light_index( ));
                                                                      return get_raw_gl_index( );
                                                                    }
    int_type            get_raw_gl_index( )                   const { return gl_index_; }

  // Arrow display, which shows where light is pointing.
  public:
    bool                is_displaying_arrow( )                const { return is_displaying_arrow_; }
    void                set_displaying_arrow( bool is)              { is_displaying_arrow_ = is; }

  // Getters
  public:
    bool                is_move_with_eye__spin( )             const { return is_move_with_eye__spin_; }
    bool                is_move_with_model__spin( )           const { return ! is_move_with_eye__spin( ); }

    bool                is_move_with_eye__rise( )             const { return is_move_with_eye__rise_; }
    bool                is_move_with_model__rise( )           const { return ! is_move_with_eye__rise( ); }

    angle_type          get_spin__degrees( )                  const { return spin__degrees_; }
    angle_type          get_rise__degrees( )                  const { return rise__degrees_; }

    color_type const &  get_ambient_color(  )                 const { return ambient_color_ ; }
    color_type const &  get_diffuse_color(  )                 const { return diffuse_color_ ; }
    color_type const &  get_specular_color( )                 const { return specular_color_; }

  // Setters
  public:
    void                set_move_with_eye__spin( bool is = true)    { is_move_with_eye__spin_ = is; }
    void                set_move_with_model__spin( bool is = true)  { set_move_with_eye__spin( ! is); }

    void                set_move_with_eye__rise( bool is = true)    { is_move_with_eye__rise_ = is; }
    void                set_move_with_model__rise( bool is = true)  { set_move_with_eye__rise( ! is); }

    void                set_spin__degrees( angle_type spin)         { spin__degrees_ = spin; }
    void                set_rise__degrees( angle_type rise)         { rise__degrees_ = rise; }

    void                set_ambient_color(  color_type const &)     ;
    void                set_diffuse_color(  color_type const &)     ;
    void                set_specular_color( color_type const &)     ;

  // GL funnel
  protected:
    void                gl_enable( )                          const ;
    void                gl_disable( )                         const ;

    void                gl_set_light_ambient_color(  )        const ;
    void                gl_set_light_diffuse_color(  )        const ;
    void                gl_set_light_specular_color( )        const ;

    void                place_in_eye_space
                         (  angle_type     model_rise     // degrees, -90.0 .. +90.0 (although the range doesn't matter and isn't checked)
                          , angle_type     model_spin     // degrees, -180.0 .. +180.0 (although the range doesn't matter and isn't checked)
                          , distance_type  model_distance // zero for ortho projection, otherwise positive (checked)
                         )                                    const ;

    void                display_arrow( distance_type)         const ;
    void                display_arrow__directional( )         const ;
    void                display_arrow_ray__directional( int)  const ;

  // Member vars
  private:
    lighting_type *   p_owner_                ;
    int_type          gl_index_               ; /* light index, 0..N, -1 if light not on */

    bool              is_on_requested_        ;
    bool              is_displaying_arrow_    ;

    bool              is_move_with_eye__spin_ ;
    bool              is_move_with_eye__rise_ ;

    angle_type        spin__degrees_          ; /* -180.0 .. +180.0 */
    angle_type        rise__degrees_          ; /* -90.0 .. +90.0 */

    color_store_type  ambient_color_          ;
    color_store_type  diffuse_color_          ;
    color_store_type  specular_color_         ;
};

// _______________________________________________________________________________________________

} /* end namespace lights */
} /* end namespace gl_env */

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef GL_DRAW_LIGHTS_H
//
// gl_draw_lights.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
