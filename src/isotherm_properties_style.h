// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// isotherm_properties_style.h
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
# pragma once
# ifndef ISOTHERM_PROPERTIES_STYLE_H
# define ISOTHERM_PROPERTIES_STYLE_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "bool_holder.h"
# include "int_holder.h"
# include "color_holder.h"

// _______________________________________________________________________________________________

  class
isotherm_properties_style_type
  : public holder_base_type
{
  Q_OBJECT

  public:
    /* ctor */    isotherm_properties_style_type( QObject * p_parent)   ;

  public:
    bool          setup_gl( )                                           ;
    bool          setup_gl_in_clone_context( )                          ;

    bool          teardown_gl( )                                        ;
    bool          teardown_gl_in_clone_context( )                       ;

    bool          setup_gl_in_shared_context( )                   const ;

  protected:
    static bool   create_and_bind_texture_id( gl_env::uint_type &)      ;
    void          build_bound_texture_unit( )                     const ;
    static void   unbind_and_destroy_texture_id( gl_env::uint_type &)   ;
    void          bind_texture_id_in_sister_context( )            const ;
    void          setup_texture_environment( )                    const ;

  public slots:
    void          has_changed__width_or_spacing( )                      ;
    void          has_changed__opacity( )                               ;
    void          has_changed__offset( )                                ;
    void          has_changed__color( )                                 ;

  protected:
    void          setup_stripe_color( )                           const ;
    void          setup_stripe_place( )                           const ;
    void          setup_stripe_look( )                            const ;

  public:
    bool          is_showing( )                                   const { return get_is_showing( )->get_value( ); }
    float         get_shader_z_factor( )                          const ;
    float         get_shader_offset( )                            const { return - get_normal_offset( ); }

    color_holder::color_type const &
                  get_shader_color( )                             const { return get_color( )->get_color( ); }

  protected:
    float         get_normal_width( )                             const ;
    float         get_normal_spacing( )                           const ;
    float         get_normal_offset( )                            const ;
    float         get_normal_opacity( )                           const ;
  public:
    static float  get_normal_0_100( float value_0_100)                  ;
    static float  get_normal_1_100( float value_1_100)                  ;

  public:
    bool_holder            *  get_is_showing( )                   const { return p_is_showing_ ; }
    int_range_steps_holder *  get_width( )                        const { return p_width_      ; }
    int_range_steps_holder *  get_spacing( )                      const { return p_spacing_    ; }
    int_range_steps_holder *  get_offset( )                       const { return p_offset_     ; }
    color_holder           *  get_color( )                        const { return p_color_      ; }
    int_range_steps_holder *  get_opacity( )                      const { return p_opacity_    ; }

  private:
    bool_holder            *  p_is_showing_ ;
    int_range_steps_holder *  p_width_      ;
    int_range_steps_holder *  p_spacing_    ;
    int_range_steps_holder *  p_offset_     ;
    color_holder           *  p_color_      ;
    int_range_steps_holder *  p_opacity_    ;

    gl_env::uint_type         texture_id_   ;
    gl_env::uint_type         texture_id2_  ;
};

// _______________________________________________________________________________________________

  struct
with_isotherm_texture
{
    bool const is_showing_isotherms;

    with_isotherm_texture( isotherm_properties_style_type const *) ;
    ~with_isotherm_texture( )                                      ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef ISOTHERM_PROPERTIES_STYLE_H
//
// isotherm_properties_style.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
