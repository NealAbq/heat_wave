// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_env_global.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef GL_ENV_GLOBAL_H
# define GL_ENV_GLOBAL_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "gl_env.h"

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

namespace gl_env {
namespace global {

// ---------------------------------------------------------------------------------------------
// The GL environment should probably be thread specific, not global.
// Starts out un-initialized.
extern env_type global_env;

  inline
  env_type &
get_env( )
  { return global_env; }

// ---------------------------------------------------------------------------------------------

# define FORWARD_0( R, FN)                 inline R FN(                        ) { return get_env( ).FN(              ); }
# define FORWARD_1( R, FN, A)              inline R FN( A a                    ) { return get_env( ).FN( a            ); }
# define FORWARD_2( R, FN, A, B)           inline R FN( A a, B b               ) { return get_env( ).FN( a, b         ); }
# define FORWARD_3( R, FN, A, B, C)        inline R FN( A a, B b, C c          ) { return get_env( ).FN( a, b, c      ); }
# define FORWARD_4( R, FN, A, B, C, D)     inline R FN( A a, B b, C c, D d     ) { return get_env( ).FN( a, b, c, d   ); }
# define FORWARD_5( R, FN, A, B, C, D, E)  inline R FN( A a, B b, C c, D d, E e) { return get_env( ).FN( a, b, c, d, e); }

# define FORWARD_VOID_0( FN)                       inline void FN(                                  ) { get_env( ).FN(                    ); }
# define FORWARD_VOID_1( FN, A)                    inline void FN( A a                              ) { get_env( ).FN( a                  ); }
# define FORWARD_VOID_2( FN, A, B)                 inline void FN( A a, B b                         ) { get_env( ).FN( a, b               ); }
# define FORWARD_VOID_3( FN, A, B, C)              inline void FN( A a, B b, C c                    ) { get_env( ).FN( a, b, c            ); }
# define FORWARD_VOID_4( FN, A, B, C, D)           inline void FN( A a, B b, C c, D d               ) { get_env( ).FN( a, b, c, d         ); }
# define FORWARD_VOID_5( FN, A, B, C, D, E)        inline void FN( A a, B b, C c, D d, E e          ) { get_env( ).FN( a, b, c, d, e      ); }
# define FORWARD_VOID_6( FN, A, B, C, D, E, F)     inline void FN( A a, B b, C c, D d, E e, F f     ) { get_env( ).FN( a, b, c, d, e, f   ); }
# define FORWARD_VOID_7( FN, A, B, C, D, E, F, G)  inline void FN( A a, B b, C c, D d, E e, F f, G g) { get_env( ).FN( a, b, c, d, e, f, g); }

// The "template" macros are flawed. The template param T cannot have more than one thing in it because of how macros
// interpret commas.
# define FORWARD_TEMPLATE_0( R, FN, T)        inline R FN(         ) { return get_env( ).FN T (     ); }
# define FORWARD_TEMPLATE_1( R, FN, T, A)     inline R FN( A a     ) { return get_env( ).FN T ( a   ); }
# define FORWARD_TEMPLATE_2( R, FN, T, A, B)  inline R FN( A a, B b) { return get_env( ).FN T ( a, b); }

# define FORWARD_VOID_TEMPLATE_0( FN, T)        inline void FN(         ) { get_env( ).FN T (     ); }
# define FORWARD_VOID_TEMPLATE_1( FN, T, A)     inline void FN( A a     ) { get_env( ).FN T ( a   ); }
# define FORWARD_VOID_TEMPLATE_2( FN, T, A, B)  inline void FN( A a, B b) { get_env( ).FN T ( a, b); }

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

  inline
  void
init( bool_type is_pristine, bool_type check_against_minimum_max_values = true)
  { get_env( ).init( is_pristine, check_against_minimum_max_values); }

FORWARD_VOID_0(         uninit)
FORWARD_0( bool_type,   is_init)
FORWARD_0( bool_type,   is_assuming_pristine)
FORWARD_0( bool_type,   is_checking_against_min_max_values)

// ---------------------------------------------------------------------------------------------

FORWARD_VOID_1(         activate_vertex_entry, enum_type)

FORWARD_VOID_0(         deactivate_vertex_entry         )

FORWARD_0( bool_type,   is_vertex_entry_active          )
FORWARD_0( enum_type,   get_vertex_entry_mode           )
FORWARD_0( size_type,   get_dropped_vertex_count        )

FORWARD_0( bool_type,   is_vertex_entry_points          )
FORWARD_0( bool_type,   is_vertex_entry_lines           )
FORWARD_0( bool_type,   is_vertex_entry_line_strip      )
FORWARD_0( bool_type,   is_vertex_entry_line_loop       )
FORWARD_0( bool_type,   is_vertex_entry_triangles       )
FORWARD_0( bool_type,   is_vertex_entry_triangle_strip  )
FORWARD_0( bool_type,   is_vertex_entry_triangle_fan    )
FORWARD_0( bool_type,   is_vertex_entry_quads           )
FORWARD_0( bool_type,   is_vertex_entry_quad_strip      )
FORWARD_0( bool_type,   is_vertex_entry_polygon         )

FORWARD_0( bool_type,   is_vertex_entry_line_type       )
FORWARD_0( bool_type,   is_vertex_entry_triangle_type   )
FORWARD_0( bool_type,   is_vertex_entry_quad_type       )

// ---------------------------------------------------------------------------------------------

template< typename TYPE > FORWARD_VOID_2( drop_vertex, TYPE, TYPE)
template< typename TYPE > FORWARD_VOID_3( drop_vertex, TYPE, TYPE, TYPE)
template< typename TYPE > FORWARD_VOID_4( drop_vertex, TYPE, TYPE, TYPE, TYPE)

template< typename TYPE > FORWARD_VOID_1( drop_vertex_p2, TYPE const *)
template< typename TYPE > FORWARD_VOID_1( drop_vertex_p3, TYPE const *)
template< typename TYPE > FORWARD_VOID_1( drop_vertex_p4, TYPE const *)

template< int COUNT, typename TYPE > FORWARD_VOID_TEMPLATE_1( drop_vertex_p, < COUNT >, TYPE const *)

// ---------------------------------------------------------------------------------------------

template< typename TYPE > FORWARD_VOID_3( drop_normal_vector, TYPE, TYPE, TYPE)
template< typename TYPE > FORWARD_VOID_1( drop_normal_vector_p3, TYPE const *)

template< int COUNT, typename TYPE > FORWARD_VOID_TEMPLATE_1( drop_normal_vector_p, < COUNT >, TYPE const *)

template< typename TYPE > FORWARD_VOID_3( setup_normal_vector, TYPE, TYPE, TYPE)
template< typename TYPE > FORWARD_VOID_1( setup_normal_vector_p3, TYPE const *)

template< int COUNT, typename TYPE > FORWARD_VOID_TEMPLATE_1( setup_normal_vector_p, < COUNT >, TYPE const *)

// ---------------------------------------------------------------------------------------------

template< typename TYPE >  FORWARD_VOID_1( drop_texture_coords, TYPE)
template< typename TYPE >  FORWARD_VOID_2( drop_texture_coords, TYPE, TYPE)
template< typename TYPE >  FORWARD_VOID_3( drop_texture_coords, TYPE, TYPE, TYPE)
template< typename TYPE >  FORWARD_VOID_4( drop_texture_coords, TYPE, TYPE, TYPE, TYPE)

template< typename TYPE >  FORWARD_VOID_1( setup_texture_coords, TYPE)
template< typename TYPE >  FORWARD_VOID_2( setup_texture_coords, TYPE, TYPE)
template< typename TYPE >  FORWARD_VOID_3( setup_texture_coords, TYPE, TYPE, TYPE)
template< typename TYPE >  FORWARD_VOID_4( setup_texture_coords, TYPE, TYPE, TYPE, TYPE)

template< typename TYPE >  FORWARD_VOID_1( drop_texture_coords_p1, TYPE const *)
template< typename TYPE >  FORWARD_VOID_1( drop_texture_coords_p2, TYPE const *)
template< typename TYPE >  FORWARD_VOID_1( drop_texture_coords_p3, TYPE const *)
template< typename TYPE >  FORWARD_VOID_1( drop_texture_coords_p4, TYPE const *)

template< typename TYPE >  FORWARD_VOID_1( setup_texture_coords_p1, TYPE const *)
template< typename TYPE >  FORWARD_VOID_1( setup_texture_coords_p2, TYPE const *)
template< typename TYPE >  FORWARD_VOID_1( setup_texture_coords_p3, TYPE const *)
template< typename TYPE >  FORWARD_VOID_1( setup_texture_coords_p4, TYPE const *)

template< int COUNT, typename TYPE > FORWARD_VOID_TEMPLATE_1( drop_texture_coords_p, < COUNT >, TYPE const *)
template< int COUNT, typename TYPE > FORWARD_VOID_TEMPLATE_1( setup_texture_coords_p, < COUNT >, TYPE const *)

// ---------------------------------------------------------------------------------------------

template< typename CC_TYPE > FORWARD_VOID_1( drop_color, rgb_type<  CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_color, rgba_type< CC_TYPE > const &)

template< typename CC_TYPE > FORWARD_VOID_1( setup_color, rgb_type<  CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_color, rgba_type< CC_TYPE > const &)

template< typename TYPE > FORWARD_VOID_1( drop_color_p3, TYPE const *)
template< typename TYPE > FORWARD_VOID_1( drop_color_p4, TYPE const *)

template< typename TYPE > FORWARD_VOID_1( setup_color_p3, TYPE const *)
template< typename TYPE > FORWARD_VOID_1( setup_color_p4, TYPE const *)

template< int COUNT, typename TYPE > FORWARD_VOID_TEMPLATE_1(  drop_color_p, < COUNT >, TYPE const *)
template< int COUNT, typename TYPE > FORWARD_VOID_TEMPLATE_1( setup_color_p, < COUNT >, TYPE const *)

// ---------------------------------------------------------------------------------------------

FORWARD_0( bool_type,   is_material_color_tracking_enabled)
FORWARD_VOID_0(         enable_material_color_tracking)
FORWARD_VOID_0(         disable_material_color_tracking)

FORWARD_0( enum_type,   get_material_color_tracking__face)
FORWARD_0( enum_type,   get_material_color_tracking__facet)

FORWARD_VOID_2(         set_material_color_tracking, enum_type, enum_type)

// ---------------------------------------------------------------------------------------------

template< typename CC_TYPE > FORWARD_TEMPLATE_2( rgba_type< CC_TYPE >, get_material_color      , < CC_TYPE >, enum_type, enum_type)
template< typename CC_TYPE > FORWARD_TEMPLATE_1( rgba_type< CC_TYPE >, get_material_back_color , < CC_TYPE >, enum_type)
template< typename CC_TYPE > FORWARD_TEMPLATE_1( rgba_type< CC_TYPE >, get_material_front_color, < CC_TYPE >, enum_type)

template< typename CC_TYPE > FORWARD_VOID_3( setup_material_color , enum_type, enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_2( setup_material_color      , enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_2( setup_material_back_color , enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_2( setup_material_front_color, enum_type, rgba_type< CC_TYPE > const &)

template< typename CC_TYPE > FORWARD_VOID_3( drop_material_color , enum_type, enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_2( drop_material_color      , enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_2( drop_material_back_color , enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_2( drop_material_front_color, enum_type, rgba_type< CC_TYPE > const &)

// ---------------------------------------------------------------------------------------------

template< typename CC_TYPE > FORWARD_TEMPLATE_1( rgba_type< CC_TYPE >, get_material_diffuse_color      , < CC_TYPE >, enum_type)
template< typename CC_TYPE > FORWARD_TEMPLATE_0( rgba_type< CC_TYPE >, get_material_back_diffuse_color , < CC_TYPE >)
template< typename CC_TYPE > FORWARD_TEMPLATE_0( rgba_type< CC_TYPE >, get_material_front_diffuse_color, < CC_TYPE >)

template< typename CC_TYPE > FORWARD_VOID_2( setup_material_diffuse_color , enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_diffuse_color      , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_back_diffuse_color , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_front_diffuse_color, rgba_type< CC_TYPE > const &)

template< typename CC_TYPE > FORWARD_VOID_2( drop_material_diffuse_color , enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_diffuse_color      , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_back_diffuse_color , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_front_diffuse_color, rgba_type< CC_TYPE > const &)

// ---------------------------------------------------------------------------------------------

template< typename CC_TYPE > FORWARD_VOID_2( drop_material_ambient_and_diffuse_color, enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_ambient_and_diffuse_color           , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_back_ambient_and_diffuse_color      , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_front_ambient_and_diffuse_color     , rgba_type< CC_TYPE > const &)

template< typename CC_TYPE > FORWARD_VOID_2( setup_material_ambient_and_diffuse_color, enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_ambient_and_diffuse_color           , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_back_ambient_and_diffuse_color      , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_front_ambient_and_diffuse_color     , rgba_type< CC_TYPE > const &)

// ---------------------------------------------------------------------------------------------

template< typename CC_TYPE > FORWARD_TEMPLATE_1( rgba_type< CC_TYPE >, get_material_ambient_color      , < CC_TYPE >, enum_type)
template< typename CC_TYPE > FORWARD_TEMPLATE_0( rgba_type< CC_TYPE >, get_material_back_ambient_color , < CC_TYPE >)
template< typename CC_TYPE > FORWARD_TEMPLATE_0( rgba_type< CC_TYPE >, get_material_front_ambient_color, < CC_TYPE >)

template< typename CC_TYPE > FORWARD_VOID_2( setup_material_ambient_color , enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_ambient_color      , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_back_ambient_color , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_front_ambient_color, rgba_type< CC_TYPE > const &)

template< typename CC_TYPE > FORWARD_VOID_2( drop_material_ambient_color , enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_ambient_color      , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_back_ambient_color , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_front_ambient_color, rgba_type< CC_TYPE > const &)

// ---------------------------------------------------------------------------------------------

template< typename CC_TYPE > FORWARD_TEMPLATE_1( rgba_type< CC_TYPE >, get_material_specular_color      , < CC_TYPE >, enum_type)
template< typename CC_TYPE > FORWARD_TEMPLATE_0( rgba_type< CC_TYPE >, get_material_back_specular_color , < CC_TYPE >)
template< typename CC_TYPE > FORWARD_TEMPLATE_0( rgba_type< CC_TYPE >, get_material_front_specular_color, < CC_TYPE >)

template< typename CC_TYPE > FORWARD_VOID_2( setup_material_specular_color , enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_specular_color      , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_back_specular_color , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_front_specular_color, rgba_type< CC_TYPE > const &)

template< typename CC_TYPE > FORWARD_VOID_2( drop_material_specular_color , enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_specular_color      , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_back_specular_color , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_front_specular_color, rgba_type< CC_TYPE > const &)

// ---------------------------------------------------------------------------------------------

template< typename CC_TYPE > FORWARD_TEMPLATE_1( rgba_type< CC_TYPE >, get_material_glow_color      , < CC_TYPE >, enum_type)
template< typename CC_TYPE > FORWARD_TEMPLATE_0( rgba_type< CC_TYPE >, get_material_back_glow_color , < CC_TYPE >)
template< typename CC_TYPE > FORWARD_TEMPLATE_0( rgba_type< CC_TYPE >, get_material_front_glow_color, < CC_TYPE >)

template< typename CC_TYPE > FORWARD_VOID_2( setup_material_glow_color , enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_glow_color      , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_back_glow_color , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( setup_material_front_glow_color, rgba_type< CC_TYPE > const &)

template< typename CC_TYPE > FORWARD_VOID_2( drop_material_glow_color , enum_type, rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_glow_color      , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_back_glow_color , rgba_type< CC_TYPE > const &)
template< typename CC_TYPE > FORWARD_VOID_1( drop_material_front_glow_color, rgba_type< CC_TYPE > const &)

// ---------------------------------------------------------------------------------------------

FORWARD_1( float_type,  get_material_shininess_01       , enum_type )
FORWARD_0( float_type,  get_material_shininess_01_front             )
FORWARD_0( float_type,  get_material_shininess_01_back              )

FORWARD_VOID_2(         drop_material_shininess_01      , enum_type, float_type)
FORWARD_VOID_1(         drop_material_shininess_01_front           , float_type)
FORWARD_VOID_1(         drop_material_shininess_01_back            , float_type)
FORWARD_VOID_1(         drop_material_shininess_01                 , float_type)

FORWARD_VOID_2(         setup_material_shininess_01     , enum_type, float_type)
FORWARD_VOID_1(         setup_material_shininess_01_front          , float_type)
FORWARD_VOID_1(         setup_material_shininess_01_back           , float_type)
FORWARD_VOID_1(         setup_material_shininess_01                , float_type)

template< typename TYPE > FORWARD_VOID_2(  drop_material_shininess_128 , enum_type, TYPE)
template< typename TYPE > FORWARD_VOID_2(  setup_material_shininess_128, enum_type, TYPE)

  //template< typename TYPE > FORWARD_1( TYPE, get_material_shininess_128  , enum_type)
  template< typename TYPE >
  inline
  TYPE
get_material_shininess_128( enum_type e)
  {
    return get_env( ).get_material_shininess_128< TYPE >( e);
  }

  // template< typename TYPE > FORWARD_0( TYPE, get_material_shininess_128_front)
  template< typename TYPE >
  inline
  TYPE
get_material_shininess_128_front( )
  {
    return get_env( ).get_material_shininess_128_front< TYPE >( );
  }

  // template< typename TYPE > FORWARD_0( TYPE, get_material_shininess_128_back )
  template< typename TYPE >
  inline
  TYPE
get_material_shininess_128_back( )
  {
    return get_env( ).get_material_shininess_128_back< TYPE >( );
  }

template< typename TYPE > FORWARD_VOID_1(  drop_material_shininess_128_front, TYPE)
template< typename TYPE > FORWARD_VOID_1(  drop_material_shininess_128_back , TYPE)
template< typename TYPE > FORWARD_VOID_1(  drop_material_shininess_128      , TYPE)

template< typename TYPE > FORWARD_VOID_1(  setup_material_shininess_128_front, TYPE)
template< typename TYPE > FORWARD_VOID_1(  setup_material_shininess_128_back , TYPE)
template< typename TYPE > FORWARD_VOID_1(  setup_material_shininess_128      , TYPE)

// ---------------------------------------------------------------------------------------------

FORWARD_VOID_1(         enable, enum_type)
FORWARD_VOID_1(         disable, enum_type)
FORWARD_1( bool_type,   is_enabled, enum_type)

FORWARD_1( bool_type,   get_bool, enum_type)
FORWARD_1( enum_type,   get_enum, enum_type)
FORWARD_1( uint_type,   get_uint, enum_type)

FORWARD_VOID_3(         get_2_bools, enum_type, bool_type &, bool_type &)
FORWARD_VOID_3(         get_2_enums, enum_type, enum_type &, enum_type &)
FORWARD_VOID_3(         get_2_uints, enum_type, uint_type &, uint_type &)

FORWARD_1( int_type,    get_int, enum_type)
FORWARD_VOID_3(         get_2_ints, enum_type, int_type&, int_type&)
FORWARD_VOID_4(         get_3_ints, enum_type, int_type&, int_type&, int_type&)
FORWARD_VOID_5(         get_4_ints, enum_type, int_type&, int_type&, int_type&, int_type&)

FORWARD_1( float_type,  get_float, enum_type)
FORWARD_VOID_3(         get_2_floats, enum_type, float_type&, float_type&)
FORWARD_VOID_4(         get_3_floats, enum_type, float_type&, float_type&, float_type&)
FORWARD_VOID_5(         get_4_floats, enum_type, float_type&, float_type&, float_type&, float_type&)

FORWARD_1( double_type, get_double, enum_type)
FORWARD_VOID_3(         get_2_doubles, enum_type, double_type&, double_type&)
FORWARD_VOID_4(         get_3_doubles, enum_type, double_type&, double_type&, double_type&)
FORWARD_VOID_5(         get_4_doubles, enum_type, double_type&, double_type&, double_type&, double_type &)

  inline
  void
get_16_doubles
 (  enum_type flag
  , double_type & val0 , double_type & val1 , double_type & val2 , double_type & val3
  , double_type & val4 , double_type & val5 , double_type & val6 , double_type & val7
  , double_type & val8 , double_type & val9 , double_type & val10, double_type & val11
  , double_type & val12, double_type & val13, double_type & val14, double_type & val15
 )
  { get_env( ).get_16_doubles( flag,
                val0 , val1 , val2 , val3 ,
                val4 , val5 , val6 , val7 ,
                val8 , val9 , val10, val11,
                val12, val13, val14, val15);
  }

// ---------------------------------------------------------------------------------------------

FORWARD_VOID_0( push_copy_on_current_matrix_stack)
FORWARD_VOID_0( pop_current_matrix_stack)

FORWARD_VOID_1( set_current_matrix_name, enum_type)

FORWARD_VOID_0( reset_current_matrix)
FORWARD_VOID_1( set_current_matrix, float_type const *)
FORWARD_VOID_1( set_current_matrix, double_type const *)

FORWARD_VOID_1( multiply_current_matrix, float_type const *)
FORWARD_VOID_1( multiply_current_matrix, double_type const *)

FORWARD_VOID_4( rotate_current_matrix, float_type, float_type, float_type, float_type)
FORWARD_VOID_4( rotate_current_matrix, double_type, double_type, double_type, double_type)

FORWARD_VOID_3( scale_current_matrix, float_type, float_type, float_type)
FORWARD_VOID_3( scale_current_matrix, double_type, double_type, double_type)

FORWARD_VOID_3( translate_current_matrix, float_type, float_type, float_type)
FORWARD_VOID_3( translate_current_matrix, double_type, double_type, double_type)

  inline
  void
setup_ortho
 (  double_type  xlo, double_type  xhi
  , double_type  ylo, double_type  yhi
  , double_type  zlo, double_type  zhi // aka z_near and z_far
 )
  { get_env( ).setup_ortho( xlo, xhi, ylo, yhi, zlo, zhi); }

  inline
  void
setup_frustum
 (  double_type  xlo, double_type  xhi
  , double_type  ylo, double_type  yhi
  , double_type  zlo, double_type  zhi // aka z_near and z_far
 )
  { get_env( ).setup_frustum( xlo, xhi, ylo, yhi, zlo, zhi); }

FORWARD_VOID_2(    get_viewport_max_sizes, size_type&, size_type&)
FORWARD_VOID_4(    get_viewport, int_type&, int_type&, size_type&, size_type&)
FORWARD_VOID_4(    set_viewport, int_type , int_type , size_type , size_type )

// ---------------------------------------------------------------------------------------------

FORWARD_0( enum_type,   get_current_matrix_name)
FORWARD_0( bool_type,   is_current_matrix_modelview)
FORWARD_0( bool_type,   is_current_matrix_projection)
FORWARD_0( bool_type,   is_current_matrix_texture)

FORWARD_0( int_type,    get_stack_depth_modelview_matrix)
FORWARD_0( int_type,    get_stack_depth_projection_matrix)
FORWARD_0( int_type,    get_stack_depth_texture_matrix)
FORWARD_1( int_type,    get_stack_depth, enum_type)
FORWARD_0( int_type,    get_stack_depth_current_matrix)

FORWARD_0( enum_type,   get_current_matrix_name_from_gl)

FORWARD_0( int_type,    get_stack_depth_modelview_matrix_from_gl)
FORWARD_0( int_type,    get_stack_depth_projection_matrix_from_gl)
FORWARD_0( int_type,    get_stack_depth_texture_matrix_from_gl)

FORWARD_0( int_type,    get_max_stack_depth_modelview_matrix_from_gl)
FORWARD_0( int_type,    get_max_stack_depth_projection_matrix_from_gl)
FORWARD_0( int_type,    get_max_stack_depth_texture_matrix_from_gl)

FORWARD_0( int_type,    get_min_max_stack_depth_modelview_matrix)
FORWARD_0( int_type,    get_min_max_stack_depth_projection_matrix)
FORWARD_0( int_type,    get_min_max_stack_depth_texture_matrix)

FORWARD_VOID_0(         assert_current_matrix_name)
FORWARD_VOID_1(         assert_current_matrix_name, enum_type)
FORWARD_VOID_0(         assert_current_stack_depths)

FORWARD_VOID_1(         assert_is_identity_matrix, enum_type)

FORWARD_VOID_1(         push_server_attribute_stack, bitfield_type)
FORWARD_VOID_0(         pop_server_attribute_stack)

FORWARD_0( int_type,    get_stack_depth_saved_server_attributes)
FORWARD_0( int_type,    get_stack_depth_saved_server_attributes_from_gl)
FORWARD_0( int_type,    get_max_stack_depth_saved_server_attributes_from_gl)

FORWARD_0( int_type,    get_min_max_stack_depth_saved_server_attributes)

FORWARD_VOID_0(         assert_saved_server_attributes_stack_deph)

// ---------------------------------------------------------------------------------------------

FORWARD_VOID_3(         set_pixel_map                       , enum_type, size_type, ushort_type const *)
FORWARD_VOID_3(         set_pixel_map                       , enum_type, size_type, uint_type   const *)
FORWARD_VOID_3(         set_pixel_map                       , enum_type, size_type, float_type  const *)
FORWARD_VOID_1(         set_pack_pixel_row_length           , int_type)
FORWARD_VOID_1(         set_unpack_pixel_row_length         , int_type)
FORWARD_VOID_1(         set_pack_pixel_row_byte_alignment   , int_type)
FORWARD_VOID_1(         set_unpack_pixel_row_byte_alignment , int_type)
FORWARD_VOID_1(         set_pack_pixel_byte_swapping        , bool)
FORWARD_VOID_1(         set_unpack_pixel_byte_swapping      , bool)
FORWARD_VOID_0(         pack_bitmap_bits_lsb_last           )
FORWARD_VOID_0(         pack_bitmap_bits_lsb_first          )
FORWARD_VOID_0(         unpack_bitmap_bits_lsb_last         )
FORWARD_VOID_0(         unpack_bitmap_bits_lsb_first        )
FORWARD_VOID_2(         set_pixel_transfer_param            , enum_type, float_type)
FORWARD_VOID_0(         enable_pixel_transfer_color_mapping     )
FORWARD_VOID_0(         disable_pixel_transfer_color_mapping    )
FORWARD_VOID_0(         enable_pixel_transfer_stencil_mapping   )
FORWARD_VOID_0(         disable_pixel_transfer_stencil_mapping  )

FORWARD_VOID_2(         set_pixel_zoom                ,  float_type, float_type)
FORWARD_VOID_7(         read_pixels_from_framebuffer  , int_type, int_type, size_type, size_type, enum_type, enum_type, void *)

                          template< typename T >
FORWARD_VOID_6(         read_pixels_from_framebuffer  , int_type, int_type, size_type, size_type, enum_type, T *)
FORWARD_VOID_5(         read_pixels_from_framebuffer__color_bitmap
                                                      , int_type, int_type, size_type, size_type, ubyte_type *)
FORWARD_VOID_5(         read_pixels_from_framebuffer__stencil_bitmap
                                                      , int_type, int_type, size_type, size_type, ubyte_type *)

FORWARD_VOID_5(         draw_pixels_to_framebuffer    , size_type, size_type, enum_type, enum_type, void const *)
                          template< typename T >
FORWARD_VOID_4(         draw_pixels_to_framebuffer    , size_type, size_type, enum_type, T const *)
FORWARD_VOID_3(         draw_pixels_to_framebuffer__color_bitmap
                                                      , size_type, size_type, ubyte_type const *)
FORWARD_VOID_3(         draw_pixels_to_framebuffer__stencil_bitmap
                                                      , size_type, size_type, ubyte_type const *)

FORWARD_VOID_5(         copy_pixels                   , int_type, int_type, size_type, size_type, enum_type)

// ---------------------------------------------------------------------------------------------

FORWARD_0( int_type,    get_max_texture_size            )
FORWARD_0( int_type,    get_min_max_texture_size        )

FORWARD_VOID_0(         enable_1D_texturing             )
FORWARD_VOID_0(         disable_1D_texturing            )
FORWARD_0( bool_type,   is_1D_texturing_enabled         )

FORWARD_VOID_0(         enable_2D_texturing             )
FORWARD_VOID_0(         disable_2D_texturing            )
FORWARD_0( bool_type,   is_2D_texturing_enabled         )

FORWARD_VOID_0(         enable_auto_texture_s_gen       )
FORWARD_VOID_0(         disable_auto_texture_s_gen      )
FORWARD_0( bool_type,   is_auto_texture_s_gen_enabled   )

FORWARD_VOID_0(         enable_auto_texture_t_gen       )
FORWARD_VOID_0(         disable_auto_texture_t_gen      )
FORWARD_0( bool_type,   is_auto_texture_t_gen_enabled   )

FORWARD_VOID_0(         enable_auto_texture_r_gen       )
FORWARD_VOID_0(         disable_auto_texture_r_gen      )
FORWARD_0( bool_type,   is_auto_texture_r_gen_enabled   )

FORWARD_VOID_0(         enable_auto_texture_q_gen       )
FORWARD_VOID_0(         disable_auto_texture_q_gen      )
FORWARD_0( bool_type,   is_auto_texture_q_gen_enabled   )

FORWARD_0( uint_type,   create_texture_id               )
FORWARD_VOID_1(         delete_texture_id    , uint_type)
FORWARD_VOID_1(         bind_1D_texture      , uint_type)
FORWARD_VOID_1(         bind_2D_texture      , uint_type)
FORWARD_VOID_0(         unbind_1D_texture               )
FORWARD_VOID_0(         unbind_2D_texture               )
FORWARD_0( uint_type,   get_bound_1D_texture            )
FORWARD_0( uint_type,   get_bound_2D_texture            )

FORWARD_0( enum_type,   get_1D_texture_minifying_filter )
FORWARD_0( enum_type,   get_2D_texture_minifying_filter )
FORWARD_0( enum_type,   get_1D_texture_magnifying_filter)
FORWARD_0( enum_type,   get_2D_texture_magnifying_filter)
FORWARD_0( enum_type,   get_1D_texture_s_wrap           )
FORWARD_0( enum_type,   get_1D_texture_t_wrap           )
FORWARD_0( enum_type,   get_2D_texture_s_wrap           )
FORWARD_0( enum_type,   get_2D_texture_t_wrap           )
FORWARD_0( float_type,  get_1D_texture_priority         )
FORWARD_0( float_type,  get_2D_texture_priority         )

FORWARD_0( rgba_type< float_type >, get_1D_texture_border_color)
FORWARD_0( rgba_type< float_type >, get_2D_texture_border_color)

FORWARD_VOID_1(         set_1D_texture_minifying_filter , enum_type )
FORWARD_VOID_1(         set_2D_texture_minifying_filter , enum_type )
FORWARD_VOID_1(         set_1D_texture_magnifying_filter, enum_type )
FORWARD_VOID_1(         set_2D_texture_magnifying_filter, enum_type )
FORWARD_VOID_1(         set_1D_texture_s_wrap           , enum_type )
FORWARD_VOID_1(         set_1D_texture_t_wrap           , enum_type )
FORWARD_VOID_1(         set_2D_texture_s_wrap           , enum_type )
FORWARD_VOID_1(         set_2D_texture_t_wrap           , enum_type )
FORWARD_VOID_1(         set_1D_texture_priority         , float_type)
FORWARD_VOID_1(         set_2D_texture_priority         , float_type)
FORWARD_VOID_1(         set_1D_texture_border_color     , rgba_type< float_type > const &)
FORWARD_VOID_1(         set_2D_texture_border_color     , rgba_type< float_type > const &)

FORWARD_VOID_1(         set_auto_texture_s_gen_mode     , enum_type )
FORWARD_VOID_1(         set_auto_texture_t_gen_mode     , enum_type )
FORWARD_VOID_1(         set_auto_texture_r_gen_mode     , enum_type )
FORWARD_VOID_1(         set_auto_texture_q_gen_mode     , enum_type )

FORWARD_VOID_4(         set_auto_texture_s_gen_object_plane, float_type, float_type, float_type, float_type)
FORWARD_VOID_4(         set_auto_texture_s_gen_eye_plane   , float_type, float_type, float_type, float_type)
FORWARD_VOID_4(         set_auto_texture_t_gen_object_plane, float_type, float_type, float_type, float_type)
FORWARD_VOID_4(         set_auto_texture_t_gen_eye_plane   , float_type, float_type, float_type, float_type)
FORWARD_VOID_4(         set_auto_texture_r_gen_object_plane, float_type, float_type, float_type, float_type)
FORWARD_VOID_4(         set_auto_texture_r_gen_eye_plane   , float_type, float_type, float_type, float_type)
FORWARD_VOID_4(         set_auto_texture_q_gen_object_plane, float_type, float_type, float_type, float_type)
FORWARD_VOID_4(         set_auto_texture_q_gen_eye_plane   , float_type, float_type, float_type, float_type)

FORWARD_VOID_1(         set_texture_env_color, rgba_type< float_type > const &)
FORWARD_VOID_1(         set_texture_env_mode , enum_type)

FORWARD_VOID_5(         set_1D_texture_bits__no_border, enum_type, size_type, enum_type, enum_type, void const *)

                          template< typename T >
FORWARD_VOID_4(         set_1D_texture_bits__no_border, enum_type, size_type, enum_type, T const *)

                          template< typename T >
FORWARD_VOID_2(         set_1D_texture_bits__no_border__luminance, size_type, T const *)
                          template< typename T >
FORWARD_VOID_2(         set_1D_texture_bits__no_border__luminance_alpha, size_type, T const *)
                          template< typename T >
FORWARD_VOID_2(         set_1D_texture_bits__no_border__rgb , size_type, T const *)
                          template< typename T >
FORWARD_VOID_2(         set_1D_texture_bits__no_border__rgba, size_type, T const *)

// ---------------------------------------------------------------------------------------------

FORWARD_VOID_2(         set_hint, enum_type, enum_type)

FORWARD_VOID_1(         set_point_size, float_type)
FORWARD_0( float_type,  get_point_size)

FORWARD_VOID_1(         set_shade_model, enum_type)
FORWARD_VOID_0(         set_shade_model_flat)
FORWARD_VOID_0(         set_shade_model_smooth)

FORWARD_0( enum_type,   get_shade_model)
FORWARD_0( bool_type,   is_shade_model_flat)
FORWARD_0( bool_type,   is_shade_model_smooth)

FORWARD_VOID_0(         enable_blending)
FORWARD_VOID_0(         disable_blending)
FORWARD_0( bool_type,   is_blending_enabled)

FORWARD_VOID_2(         set_blending_factors, enum_type, enum_type)
FORWARD_0( enum_type,   get_blending_src_factor)
FORWARD_0( enum_type,   get_blending_dst_factor)

FORWARD_VOID_2(         set_polygon_mode , enum_type  , enum_type  )
FORWARD_VOID_2(         get_polygon_modes, enum_type &, enum_type &)
FORWARD_1( enum_type,   get_polygon_mode , enum_type)

FORWARD_VOID_1(         set_front_face, enum_type)
FORWARD_VOID_0(         set_front_face_cw)
FORWARD_VOID_0(         set_front_face_ccw)

FORWARD_0( enum_type,   get_front_face)

FORWARD_VOID_1(         set_cull_face, enum_type)
FORWARD_0( enum_type,   get_cull_face)

FORWARD_VOID_0(         enable_face_culling)
FORWARD_VOID_0(         disable_face_culling)
FORWARD_0( bool_type,   is_face_culling_enabled)

FORWARD_VOID_0(         freeze_depth_buffer)
FORWARD_VOID_0(         unfreeze_depth_buffer)
FORWARD_0( bool_type,   is_depth_buffer_frozen)

FORWARD_VOID_1(         set_depth_test_fn, enum_type)
FORWARD_0( enum_type,   get_depth_test_fn)

FORWARD_0( bool_type,   is_depth_test_enabled)
FORWARD_VOID_0(         enable_depth_test)
FORWARD_VOID_0(         disable_depth_test)

FORWARD_VOID_1(         set_depth_buffer_clear_value, double_type)
FORWARD_0( double_type, get_depth_buffer_clear_value)

FORWARD_VOID_1(         set_color_buffer_clear_color, rgba_type< float_type > const &)
FORWARD_0( rgba_type< float_type >,
                        get_color_buffer_clear_color)

FORWARD_VOID_1(         clear_buffers, bitfield_type)
FORWARD_VOID_0(         clear_depth_buffer)
FORWARD_VOID_0(         clear_color_buffer)
FORWARD_VOID_0(         clear_color_and_depth_buffer)

FORWARD_0( int_type,    get_max_clip_plane_count)
FORWARD_0( int_type,    get_max_clip_plane_count_from_gl)
FORWARD_0( int_type,    get_min_max_clip_plane_count)
FORWARD_VOID_1(         assert_valid_clip_plane_index, int_type)
FORWARD_VOID_1(         assert_portable_valid_clip_plane_index, int_type)

FORWARD_1( bool_type,   is_clip_plane_enabled, int_type)
FORWARD_VOID_1(         enable_clip_plane, int_type)
FORWARD_VOID_1(         disable_clip_plane, int_type)

FORWARD_VOID_5(         set_clip_plane_equation, int_type, double_type, double_type, double_type, double_type)

FORWARD_0( bool_type,   is_two_sided_lighting)
FORWARD_VOID_0(         enable_two_sided_lighting)
FORWARD_VOID_0(         disable_two_sided_lighting)
FORWARD_VOID_1(         set_two_sided_lighting, bool_type)

FORWARD_0( bool_type,   is_local_viewer_light_calculations)
FORWARD_VOID_0(         enable_local_viewer_light_calculations)
FORWARD_VOID_0(         disable_local_viewer_light_calculations)
FORWARD_VOID_1(         set_local_viewer_light_calculations, bool_type)

template< typename CC_TYPE >
FORWARD_TEMPLATE_0( rgba_type< CC_TYPE >,
                        get_global_ambient_color, < CC_TYPE >)

template< typename COLOR_TYPE >
FORWARD_VOID_1(         set_global_ambient_color, COLOR_TYPE const &)

// ---------------------------------------------------------------------------------------------

FORWARD_0( int_type,    get_max_light_count)
FORWARD_0( int_type,    get_max_light_count_from_gl)
FORWARD_0( int_type,    get_min_max_light_count) /* static */

FORWARD_1( bool_type,   is_light_index, int_type)
FORWARD_1( bool_type,   is_portable_light_index, int_type) /* static */
FORWARD_1( bool_type,   is_light_facet, enum_type) /* static */

FORWARD_0( bool_type,   is_overall_lighting_enabled)
FORWARD_VOID_0(         enable_overall_lighting)
FORWARD_VOID_0(         disable_overall_lighting)

FORWARD_1( bool_type,   is_light_on, int_type)

FORWARD_VOID_1(         set_light_on, int_type)
FORWARD_VOID_1(         set_light_off, int_type)

template< typename COLOR_TYPE > FORWARD_VOID_3( set_light_color, int_type, enum_type, COLOR_TYPE const &)

template< typename COLOR_TYPE > FORWARD_VOID_2( set_light_ambient_color , int_type, COLOR_TYPE const &)
template< typename COLOR_TYPE > FORWARD_VOID_2( set_light_diffuse_color , int_type, COLOR_TYPE const &)
template< typename COLOR_TYPE > FORWARD_VOID_2( set_light_specular_color, int_type, COLOR_TYPE const &)

FORWARD_VOID_4(         set_light_position, int_type, int_type, int_type, int_type)
FORWARD_VOID_5(         set_light_position, int_type, int_type, int_type, int_type, int_type)
FORWARD_VOID_4(         set_light_position, int_type, float_type, float_type, float_type)
FORWARD_VOID_5(         set_light_position, int_type, float_type, float_type, float_type, float_type)

FORWARD_VOID_4(         set_light_direction, int_type, int_type  , int_type  , int_type  )
FORWARD_VOID_4(         set_light_direction, int_type, float_type, float_type, float_type)

FORWARD_VOID_4(         set_spotlight_direction, int_type, int_type  , int_type  , int_type  )
FORWARD_VOID_4(         set_spotlight_direction, int_type, float_type, float_type, float_type)

FORWARD_VOID_1(         set_spotlight_no_focus, int_type)
FORWARD_VOID_2(         set_spotlight_focus_01, int_type, float_type)
FORWARD_VOID_2(         set_spotlight_focus_128, int_type, int_type)
FORWARD_VOID_2(         set_spotlight_focus_128, int_type, float_type)

FORWARD_VOID_2(         set_spotlight_cutoff_angle, int_type, int_type)
FORWARD_VOID_2(         set_spotlight_cutoff_angle, int_type, float_type)
FORWARD_VOID_1(         reset_spotlight_cutoff_angle, int_type)

FORWARD_VOID_4(         set_light_attenuation, int_type, int_type  , int_type  , int_type  )
FORWARD_VOID_4(         set_light_attenuation, int_type, float_type, float_type, float_type)

// ---------------------------------------------------------------------------------------------

  class
with_saved_server_attributes
  : public gl_env::with_saved_server_attributes
  { public:
    with_saved_server_attributes( bitfield_type bits)
      : gl_env::with_saved_server_attributes( get_env( ), bits) { }
  };

// ---------------------------------------------------------------------------------------------

  class
with_active_vertex_entry
  : public gl_env::with_active_vertex_entry
  { public:
    with_active_vertex_entry( enum_type mode)
      : gl_env::with_active_vertex_entry( get_env( ), mode) { }
  };

  template< enum_type VERTEX_MODE >
  class
with_active_vertex_entry_
  : public gl_env::with_active_vertex_entry_< VERTEX_MODE >
  { public:
    with_active_vertex_entry_( )
      : gl_env::with_active_vertex_entry_< VERTEX_MODE >( get_env( )) { }
  };

typedef with_active_vertex_entry_< GL_POINTS         >  with_active_vertex_entry_points         ;
typedef with_active_vertex_entry_< GL_LINES          >  with_active_vertex_entry_lines          ;
typedef with_active_vertex_entry_< GL_LINE_STRIP     >  with_active_vertex_entry_line_strip     ;
typedef with_active_vertex_entry_< GL_LINE_LOOP      >  with_active_vertex_entry_line_loop      ;
typedef with_active_vertex_entry_< GL_TRIANGLES      >  with_active_vertex_entry_triangles      ;
typedef with_active_vertex_entry_< GL_TRIANGLE_STRIP >  with_active_vertex_entry_triangle_strip ;
typedef with_active_vertex_entry_< GL_TRIANGLE_FAN   >  with_active_vertex_entry_triangle_fan   ;
typedef with_active_vertex_entry_< GL_QUADS          >  with_active_vertex_entry_quads          ;
typedef with_active_vertex_entry_< GL_QUAD_STRIP     >  with_active_vertex_entry_quad_strip     ;
typedef with_active_vertex_entry_< GL_POLYGON        >  with_active_vertex_entry_polygon        ;

// ---------------------------------------------------------------------------------------------

# define FORWARD_with_current_matrix_class( CLASS_NAME)  \
      class                                              \
    CLASS_NAME                                           \
      : public gl_env::CLASS_NAME                        \
      { public:                                          \
        CLASS_NAME( enum_type matrix)                    \
          : gl_env::CLASS_NAME( get_env( ), matrix) { }  \
      };                                                 /* end macro */

FORWARD_with_current_matrix_class( with_current_matrix__assert__assert              )
FORWARD_with_current_matrix_class( with_current_matrix__set__assert                 )
FORWARD_with_current_matrix_class( with_current_matrix__set__restore                )

FORWARD_with_current_matrix_class( with_current_matrix__assert_push__assert_pop     )
FORWARD_with_current_matrix_class( with_current_matrix__assert_push__set_pop        )
FORWARD_with_current_matrix_class( with_current_matrix__set_push__assert_pop        )
FORWARD_with_current_matrix_class( with_current_matrix__set_push__set_pop           )

FORWARD_with_current_matrix_class( with_current_matrix__set_push__assert_pop_restore)
FORWARD_with_current_matrix_class( with_current_matrix__set_push__set_pop_restore   )

# undef FORWARD_with_current_matrix_class

// ---------------------------------------------------------------------------------------------

FORWARD_0( bool_type,   is_line_stipple_enabled)
FORWARD_VOID_0(         enable_line_stipple)
FORWARD_VOID_0(         disable_line_stipple)

FORWARD_0( int_type,    get_line_stipple_repeat_factor)
FORWARD_0( ushort_type, get_line_stipple_pattern)
FORWARD_VOID_2(         set_line_stipple, int_type, ushort_type)

FORWARD_0( float_type,  get_line_width)
FORWARD_VOID_1(         set_line_width, float_type)

// ---------------------------------------------------------------------------------------------

FORWARD_0( bool_type,   are_shaders_supported)
FORWARD_0( bool_type,   are_shaders_explicitly_supported)
FORWARD_0( uint_type,   get_active_shader_program)
FORWARD_VOID_1(         use_shader_program, uint_type)
FORWARD_2( uint_type,   create_shader_program, uint_type, uint_type)
FORWARD_2( uint_type,   create_shader, enum_type, char_type const *)
FORWARD_1( bool_type,   is_shader_program, uint_type)
FORWARD_1( bool_type,   is_shader, uint_type)

FORWARD_0( uint_type,   create_shader_program)
FORWARD_VOID_1(         delete_shader_program, uint_type)
FORWARD_1( uint_type,   create_shader, enum_type)
FORWARD_VOID_1(         delete_shader, uint_type)
FORWARD_VOID_2(         attach_shader, uint_type, uint_type)
FORWARD_VOID_2(         detach_shader, uint_type, uint_type)
FORWARD_VOID_1(         detach_all_shaders, uint_type)
FORWARD_VOID_1(         link_shader_program, uint_type)
FORWARD_VOID_1(         validate_shader_program, uint_type)
FORWARD_VOID_2(         set_shader_src, uint_type, char_type const *)
FORWARD_VOID_1(         compile_shader, uint_type)

FORWARD_3( int_type,    get_shader_program_status_value, uint_type, enum_type, int_type)
FORWARD_3( int_type,    get_shader_status_value, uint_type, enum_type, int_type)
FORWARD_2( bool_type,   did_shader_program_op_fail_with_errors, enum_type, uint_type)
FORWARD_2( bool_type,   did_shader_op_fail_with_errors, enum_type, uint_type)

FORWARD_2( int_type,    get_uniform_location, uint_type, char_type const *)
FORWARD_VOID_3(         get_uniform_values, uint_type, int_type, float_type *)
FORWARD_VOID_3(         get_uniform_values, uint_type, int_type, int_type *)
FORWARD_VOID_2(         set_uniform, int_type, float_type)
FORWARD_VOID_3(         set_uniform, int_type, float_type, float_type)
FORWARD_VOID_4(         set_uniform, int_type, float_type, float_type, float_type)
FORWARD_VOID_5(         set_uniform, int_type, float_type, float_type, float_type, float_type)
FORWARD_VOID_2(         set_uniform, int_type, int_type)
FORWARD_VOID_3(         set_uniform, int_type, int_type, int_type)
FORWARD_VOID_4(         set_uniform, int_type, int_type, int_type, int_type)
FORWARD_VOID_5(         set_uniform, int_type, int_type, int_type, int_type, int_type)
FORWARD_VOID_2(         set_uniform, int_type, rgb_type< float_type > const &)
FORWARD_VOID_2(         set_uniform, int_type, rgba_type< float_type > const &)

// ---------------------------------------------------------------------------------------------

FORWARD_0( bool_type,   are_shaders_supported__ARB)
FORWARD_0( bool_type,   are_shaders_explicitly_supported__ARB)
FORWARD_0( GLhandleARB, get_active_shader_program__ARB)
FORWARD_VOID_1(         use_shader_program__ARB, GLhandleARB)
FORWARD_2( GLhandleARB, create_shader_program__ARB, GLhandleARB, GLhandleARB)
FORWARD_2( GLhandleARB, create_shader__ARB, enum_type, GLcharARB const *)
FORWARD_1( bool_type,   is_shader_program__ARB, GLhandleARB)

FORWARD_0( GLhandleARB, create_shader_program__ARB)
FORWARD_1( GLhandleARB, create_shader__ARB, enum_type)
FORWARD_VOID_1(         delete_shader_object__ARB, GLhandleARB)
FORWARD_VOID_2(         attach_shader__ARB, GLhandleARB, GLhandleARB)
FORWARD_VOID_2(         detach_shader__ARB, GLhandleARB, GLhandleARB)
FORWARD_VOID_1(         detach_all_shaders__ARB, GLhandleARB)
FORWARD_VOID_1(         link_shader_program__ARB, GLhandleARB)
FORWARD_VOID_1(         validate_shader_program__ARB, GLhandleARB)
FORWARD_VOID_2(         set_shader_src__ARB, GLhandleARB, GLcharARB const *)
FORWARD_VOID_1(         compile_shader__ARB, GLhandleARB)

FORWARD_3( int_type,    get_object_status_value__ARB, GLhandleARB, enum_type, int_type)
FORWARD_2( bool_type,   did_object_op_fail_with_errors__ARB, enum_type, GLhandleARB)

FORWARD_2( int_type,    get_uniform_location__ARB, uint_type, char_type const *)
FORWARD_VOID_3(         get_uniform_values__ARB, uint_type, int_type, float_type *)
FORWARD_VOID_3(         get_uniform_values__ARB, uint_type, int_type, int_type *)
FORWARD_VOID_2(         set_uniform__ARB, int_type, float_type)
FORWARD_VOID_3(         set_uniform__ARB, int_type, float_type, float_type)
FORWARD_VOID_4(         set_uniform__ARB, int_type, float_type, float_type, float_type)
FORWARD_VOID_5(         set_uniform__ARB, int_type, float_type, float_type, float_type, float_type)
FORWARD_VOID_2(         set_uniform__ARB, int_type, int_type)
FORWARD_VOID_3(         set_uniform__ARB, int_type, int_type, int_type)
FORWARD_VOID_4(         set_uniform__ARB, int_type, int_type, int_type, int_type)
FORWARD_VOID_5(         set_uniform__ARB, int_type, int_type, int_type, int_type, int_type)
FORWARD_VOID_2(         set_uniform__ARB, int_type, rgb_type< float_type > const &)
FORWARD_VOID_2(         set_uniform__ARB, int_type, rgba_type< float_type > const &)

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

# undef FORWARD_VOID_TEMPLATE_2
# undef FORWARD_VOID_TEMPLATE_1
# undef FORWARD_VOID_TEMPLATE_0

# undef FORWARD_TEMPLATE_2
# undef FORWARD_TEMPLATE_1
# undef FORWARD_TEMPLATE_0

# undef FORWARD_VOID_7
# undef FORWARD_VOID_6
# undef FORWARD_VOID_5
# undef FORWARD_VOID_4
# undef FORWARD_VOID_3
# undef FORWARD_VOID_2
# undef FORWARD_VOID_1
# undef FORWARD_VOID_0

# undef FORWARD_5
# undef FORWARD_4
# undef FORWARD_3
# undef FORWARD_2
# undef FORWARD_1
# undef FORWARD_0

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

} /* end namespace global */
} /* end namespace gl_env */

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef GL_ENV_GLOBAL_H
//
// gl_env_global.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
