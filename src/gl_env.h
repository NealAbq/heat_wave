// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_env.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef GL_ENV_H
# define GL_ENV_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "util.h"
# include "gl_env_type_primitives.h"
# include "gl_env_type_colors.h"

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
namespace gl_env {
// _______________________________________________________________________________________________

// ---------------------------------------------------------------------------------------------

// Main class defined below.
class env_type;

// ---------------------------------------------------------------------------------------------
// Utility base classes for wrappers and other helpers

class env_base
{
  protected:
    env_base( env_type & e)
      : env( e) { }
      env_type & env ;
};

class env_name_base
{
  protected:
    env_name_base( env_type & e, enum_type n)
      : env( e), name( n) { }
      env_type & env ;
      enum_type const name ;
};

  template< typename TYPE >
class env_val_base
{
  protected:
    env_val_base( env_type & e, TYPE const & v)
      : env( e), val( v) { }
      env_type & env ;
      TYPE const val ;
};

  inline
  bool
as_bool( bool_type b)
  //
  // This function reduces the warnings we get about treating bool_type like a bool.
{
    return b ? true : false;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Debug
// _______________________________________________________________________________________________

# ifdef NDEBUG

inline void  assert_no_error( )                                              { }
inline void  assert_not_vertex_entry_active( env_type &, bool_type = false)  { }

class assert_no_errors
{
  public:
    assert_no_errors( env_type &, bool_type = false)  { }
};

class assert_during_vertex_entry
{
  public:
    assert_during_vertex_entry( env_type &)  { }
};

# else

void  assert_no_error( )                                                                    ;
void  assert_not_vertex_entry_active( env_type &, bool_type is_allowed_during_init = false) ;

class assert_no_errors : public env_val_base< bool_type >
{
  public:
    assert_no_errors( env_type &, bool_type allow_during_init = false)  ;
    ~assert_no_errors( )                                           ;
};

class assert_during_vertex_entry : public env_base
{
  public:
    assert_during_vertex_entry( env_type &)  ;
    ~assert_during_vertex_entry( )           ;
};

# endif

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
class env_type
// _______________________________________________________________________________________________
{
  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Header
  // ---------------------------------------------------------------------------------------
  public:
    /* ctor */  env_type( tag::do_not_init_type const &)
                                                : is_init_( false)
                                                , is_during_init_or_uninit_( false)
                                                { }

    /* ctor */  env_type( bool_type is_pristine, bool_type check_against_minimum_max_values = true)
                                                : is_init_( false)
                                                , is_during_init_or_uninit_( false)
                                                { init( is_pristine, check_against_minimum_max_values); }

    /* dtor */  ~env_type( )                    { uninit( ); }

  // Disable copy
  private:
    /* copy */  env_type(   env_type const &)   ; // no implementation
    void        operator =( env_type const &)   ; // no implementation

  // ---------------------------------------------------------------------------------------
  // State
  public:
    void        init( bool_type is_pristine, bool_type check_against_minimum_max_values = true)
                                                ;
    void        uninit( )                       ;

  public:
    bool_type   is_init( )                const { return is_init_; }
    bool_type   is_during_init_or_uninit( )
                                          const { return is_during_init_or_uninit_; }
    bool_type   is_assuming_pristine( )   const { return is_assuming_pristine_; }
    bool_type   is_checking_against_min_max_values( )
                                          const { return is_checking_against_min_max_values_; }

  public:
    static bool_type  is_extension_explicitly_supported( char_type const * p_extension)
                                                ;

    static bool_type  is_gl_version_1_2( )      ;
    static bool_type  is_gl_version_1_3( )      ;
    static bool_type  is_gl_version_1_4( )      ;
    static bool_type  is_gl_version_1_5( )      ;
    static bool_type  is_gl_version_2_0( )      ;
    static bool_type  is_gl_version_2_1( )      ;
    static bool_type  is_gl_version_3_0( )      ;

  // ---------------------------------------------------------------------------------------
  // Member vars
  private:
    bool_type  is_init_ ;
    bool_type  is_during_init_or_uninit_ ;
    bool_type  is_assuming_pristine_ ;
    bool_type  is_checking_against_min_max_values_ ;

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Active Vertex (begin/end)
  // ---------------------------------------------------------------------------------------
  // Changes to consider:
  //   Keep a count of the vertices in the environment.
  //   Automatically generate the normals.
  private:
    void        init_active_vertex( )                    { is_vertex_entry_active_  = false;
                                                           vertex_entry_mode_       = -1;
                                                           dropped_vertex_count_    = 0;
                                                         }
    void        uninit_active_vertex( )                  { assert_not_vertex_entry_active__during_init_or_uninit( ); }

  public:
    // The following is called while is_during_init_or_uninit( ) is true and is_init( ) is false.
    // By checking is_vertex_entry_active_, it is assuming init_active_vertex( ) is called before any
    // other init_* method that calls a GL function (::glGetIntegerv or ::glIsEnabled).
    void        assert_not_vertex_entry_active__during_init_or_uninit( )
                                                   const { d_assert( is_during_init_or_uninit( ) && ! is_init( ));
                                                           d_assert( ! is_vertex_entry_active_);
                                                         }

  public:
    void        activate_vertex_entry( enum_type mode)   ; // ::glBegin(..)
    void        deactivate_vertex_entry( )               ; // ::glEnd( )

    size_type   get_dropped_vertex_count( )        const { return dropped_vertex_count_; }
    enum_type   get_vertex_entry_mode( )           const { d_assert( as_bool( is_vertex_entry_active( )));
                                                           return vertex_entry_mode_;
                                                         }
    bool_type   is_vertex_entry_active( )          const { d_assert( as_bool( is_init( )));
                                                           return is_vertex_entry_active_;
                                                         }

    bool_type   is_vertex_entry_points( )          const { return is_vertex_entry_active( ) && (get_vertex_entry_mode( ) == GL_POINTS        ); }
    bool_type   is_vertex_entry_lines( )           const { return is_vertex_entry_active( ) && (get_vertex_entry_mode( ) == GL_LINES         ); }
    bool_type   is_vertex_entry_line_strip( )      const { return is_vertex_entry_active( ) && (get_vertex_entry_mode( ) == GL_LINE_STRIP    ); }
    bool_type   is_vertex_entry_line_loop( )       const { return is_vertex_entry_active( ) && (get_vertex_entry_mode( ) == GL_LINE_LOOP     ); }
    bool_type   is_vertex_entry_triangles( )       const { return is_vertex_entry_active( ) && (get_vertex_entry_mode( ) == GL_TRIANGLES     ); }
    bool_type   is_vertex_entry_triangle_strip( )  const { return is_vertex_entry_active( ) && (get_vertex_entry_mode( ) == GL_TRIANGLE_STRIP); }
    bool_type   is_vertex_entry_triangle_fan( )    const { return is_vertex_entry_active( ) && (get_vertex_entry_mode( ) == GL_TRIANGLE_FAN  ); }
    bool_type   is_vertex_entry_quads( )           const { return is_vertex_entry_active( ) && (get_vertex_entry_mode( ) == GL_QUADS         ); }
    bool_type   is_vertex_entry_quad_strip( )      const { return is_vertex_entry_active( ) && (get_vertex_entry_mode( ) == GL_QUAD_STRIP    ); }
    bool_type   is_vertex_entry_polygon( )         const { return is_vertex_entry_active( ) && (get_vertex_entry_mode( ) == GL_POLYGON       ); }

    bool_type   is_vertex_entry_line_type( )       const ;
    bool_type   is_vertex_entry_triangle_type( )   const ;
    bool_type   is_vertex_entry_quad_type( )       const ;

  // ---------------------------------------------------------------------------------------
  // Member vars
  private:
    bool_type  is_vertex_entry_active_ ;
    enum_type  vertex_entry_mode_      ;
    size_type  dropped_vertex_count_   ;

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Vertex entry
  // ---------------------------------------------------------------------------------------
  // For the floating values should we:
  //   d_assert( ! util::is_non_number( x))
  //   util::remove_negative_zero( x)
  public:
    // These templates have no generic implementations.
    // Instead they have specialization implementations, in the macros below.
    template< typename TYPE >            void  drop_vertex( TYPE x, TYPE y) ;
    template< typename TYPE >            void  drop_vertex( TYPE x, TYPE y, TYPE z) ;
    template< typename TYPE >            void  drop_vertex( TYPE x, TYPE y, TYPE z, TYPE w) ;

    template< typename TYPE >            void  drop_vertex_p2( TYPE const * pXY) ;
    template< typename TYPE >            void  drop_vertex_p3( TYPE const * pXYZ) ;
    template< typename TYPE >            void  drop_vertex_p4( TYPE const * pXYZW) ;

    template< int COUNT, typename TYPE > void  drop_vertex_p( TYPE const * pArgs) ;

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Normal (vector) entry
  // ---------------------------------------------------------------------------------------
  // The byte/short/int versions of normal_vector(..) treat the coords like -1..+1 full-range values.
  // We should have separate types for this (instead of byte_type, short_type, int_type).
  //
  // Should we assert that the normal coords are not zero? In the int case they cannot be zero
  // because zero maps to a slightly negative value.
  // Should we check util::is_non_number(..) and correct using util::remove_negative_zero(..)?
  //
  // We could get_3_floats( GL_CURRENT_NORMAL, ..) in setup_normal_vector(..) to make sure we set
  // the normal correctly.
  public:
    // These templates have no generic implementations.
    // Instead they have specialization implementations, in the macros below.
    template< typename TYPE >            void  drop_normal_vector( TYPE x, TYPE y, TYPE z) ;
    template< typename TYPE >            void  drop_normal_vector_p3( TYPE const * pXYZ) ;
    template< int COUNT, typename TYPE > void  drop_normal_vector_p(  TYPE const * pXYZ) ;

    template< typename TYPE >            void  setup_normal_vector( TYPE x, TYPE y, TYPE z) ;
    template< typename TYPE >            void  setup_normal_vector_p3( TYPE const * pXYZ) ;
    template< int COUNT, typename TYPE > void  setup_normal_vector_p(  TYPE const * pXYZ) ;

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Texture coord entry
  // ---------------------------------------------------------------------------------------
  // We could check get_4_floats( GL_CURRENT_TEXTURE_COORDS, ..) in setup_texture_coords(..).
  public:
    // These templates have no generic implementations.
    // Instead they have specialization implementations, in the macros below.
    template< typename TYPE >            void  drop_texture_coords( TYPE s) ;
    template< typename TYPE >            void  drop_texture_coords( TYPE s, TYPE t) ;
    template< typename TYPE >            void  drop_texture_coords( TYPE s, TYPE t, TYPE r) ;
    template< typename TYPE >            void  drop_texture_coords( TYPE s, TYPE t, TYPE r, TYPE q) ;

    template< typename TYPE >            void  setup_texture_coords( TYPE s) ;
    template< typename TYPE >            void  setup_texture_coords( TYPE s, TYPE t) ;
    template< typename TYPE >            void  setup_texture_coords( TYPE s, TYPE t, TYPE r) ;
    template< typename TYPE >            void  setup_texture_coords( TYPE s, TYPE t, TYPE r, TYPE q) ;

    template< typename TYPE >            void  drop_texture_coords_p1( TYPE const * pS) ;
    template< typename TYPE >            void  drop_texture_coords_p2( TYPE const * pST) ;
    template< typename TYPE >            void  drop_texture_coords_p3( TYPE const * pSTR) ;
    template< typename TYPE >            void  drop_texture_coords_p4( TYPE const * pSTRQ) ;

    template< typename TYPE >            void  setup_texture_coords_p1( TYPE const * pS) ;
    template< typename TYPE >            void  setup_texture_coords_p2( TYPE const * pST) ;
    template< typename TYPE >            void  setup_texture_coords_p3( TYPE const * pSTR) ;
    template< typename TYPE >            void  setup_texture_coords_p4( TYPE const * pSTRQ) ;

    template< int COUNT, typename TYPE > void  drop_texture_coords_p(  TYPE const * pArgs) ;
    template< int COUNT, typename TYPE > void  setup_texture_coords_p( TYPE const * pArgs) ;

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Color (material) entry
  // ---------------------------------------------------------------------------------------
  // We could check using get_4_floats( GL_CURRENT_COLOR, ..) in setup_color(..).
  public:
    template< typename TYPE >            void  drop_color(  rgb_type<  TYPE > const & rgb )  { drop_color_p3( rgb .get_as_p3( )); }
    template< typename TYPE >            void  drop_color(  rgba_type< TYPE > const & rgba)  { drop_color_p4( rgba.get_as_p4( )); }

    template< typename TYPE >            void  setup_color( rgb_type<  TYPE > const & rgb )  { setup_color_p3( rgb .get_as_p3( )); }
    template< typename TYPE >            void  setup_color( rgba_type< TYPE > const & rgba)  { setup_color_p4( rgba.get_as_p4( )); }

  public:
    // These templates have no generic implementations.
    // Instead they have specialization implementations, in the macros below.
    template< typename TYPE >            void  drop_color_p3( TYPE const * pRGB) ;
    template< typename TYPE >            void  drop_color_p4( TYPE const * pRGBA) ;

    template< typename TYPE >            void  setup_color_p3( TYPE const * pRGB) ;
    template< typename TYPE >            void  setup_color_p4( TYPE const * pRGBA) ;

    template< int COUNT, typename TYPE > void  drop_color_p(  TYPE const * pArgs) ;
    template< int COUNT, typename TYPE > void  setup_color_p( TYPE const * pArgs) ;

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Material color tracking
  //   For simplicity, material_face and color_facet can track the current color.
  // ---------------------------------------------------------------------------------------
  public:
    bool_type    is_material_color_tracking_enabled( )
                                             { return is_enabled( GL_COLOR_MATERIAL); }
    void         enable_material_color_tracking( )
                                             { enable( GL_COLOR_MATERIAL); }
    void         disable_material_color_tracking( )
                                             { disable( GL_COLOR_MATERIAL); }

    enum_type    get_material_color_tracking__face( ) /* material_face */
                                             { enum_type const face = get_enum( GL_COLOR_MATERIAL_FACE);
                                               d_assert( is_material_face__set( face)); /* set, not get */
                                               return face;
                                             }
    enum_type    get_material_color_tracking__facet( ) /* material color_facet */
                                             { enum_type const facet = get_enum( GL_COLOR_MATERIAL_PARAMETER);
                                               d_assert( is_color_facet__set( facet)); /* set, not get */
                                               return facet;
                                             }
    void         set_material_color_tracking( enum_type face, enum_type facet)
                                             { // You should only change the material-color-tracking properties when
                                               // color-tracking is disabled. The doc (opengl.org) says:
                                               //   Call glColorMaterial before enabling GL_COLOR_MATERIAL.
                                               d_assert( ! is_material_color_tracking_enabled( ));
                                               d_assert( is_material_face__set( face));
                                               d_assert( is_color_facet__set( facet));
                                               assert_no_errors ne( *this);
                                               ::glColorMaterial( face, facet);
                                             }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Material colors
  // ---------------------------------------------------------------------------------------
  // The GL material interface only works with int_type and float_type. An interface for
  // the most natural color-component type (ubyte_type) isn't available.
  // Improve: When color-tracking you should not try to change the specified material
  // colors directly, and instead you should always set the current color.
  // It's only OK to change a material color when one of these is true:
  //   Overall lighting is disabled.
  //   Color tracking is disabled.
  //   The face/facet color you are changing is not the one being tracked. BUT, it may be
  //     unsafe to change any material face/facet color when tracking is on, even a face
  //     and facet that is not being tracked.
  // Maybe when dropping a the current color while specifying a filled polygon we should
  // check that one of these is true. Maybe this is too much checking.
  //   Overall lighting is disabled, or
  //   Lighting is on and color tracking is on.
  public:
      /* method signature, specialized below */
      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can also be assigned to an rgb_type<..> */
    get_material_color( enum_type face, enum_type facet)
      ;

      /* method signature, specialized below */
      template< typename CC_TYPE >
      void
    drop_material_color( enum_type face, enum_type facet, rgba_type< CC_TYPE > const &)
      ;

      /* method signature, specialized below */
      template< typename CC_TYPE >
      void
    setup_material_color( enum_type face, enum_type facet, rgba_type< CC_TYPE > const &)
      ;

      /* overload to take RGB instead of RGBA */
      template< typename CC_TYPE >
      void
    drop_material_color( enum_type face, enum_type facet, rgb_type< CC_TYPE > const & rgb)
      { rgba_type< CC_TYPE > const rgba = rgb;
        drop_material_color( face, facet, rgba);
      }

      /* overload to take RGB instead of RGBA */
      template< typename CC_TYPE >
      void
    setup_material_color( enum_type face, enum_type facet, rgb_type< CC_TYPE > const & rgb)
      { rgba_type< CC_TYPE > const rgba = rgb;
        setup_material_color( face, facet, rgba);
      }

      // Debug for setup_material_color(..).
      template< typename CC_TYPE >
      void
    assert__material_color_is_set( enum_type face, enum_type facet, rgba_type< CC_TYPE > const & color)
      {
        maybe_used_only_for_debug( face );
        maybe_used_only_for_debug( facet);
        maybe_used_only_for_debug( color);
      # ifndef NDEBUG
        if ( GL_FRONT_AND_BACK == face ) {
            d_assert( is_material_face__set( face) && ! is_material_face__get( face));
            assert__material_color_is_set( GL_FRONT, facet, color);
            assert__material_color_is_set( GL_BACK , facet, color);
        } else
        if ( GL_AMBIENT_AND_DIFFUSE == facet ) {
            d_assert( is_color_facet__set( facet) && ! is_color_facet__get( facet));
            assert__material_color_is_set( face, GL_AMBIENT, color);
            assert__material_color_is_set( face, GL_DIFFUSE, color);
        } else
        {   d_assert( is_material_face__get( face));
            d_assert( is_color_facet__get( facet));
            d_assert( get_material_color< CC_TYPE >( face, facet) == color);
        }
      # endif
      }

  // ---------------------------------------------------------------------------------------
  // Material colors, front and back
  public:
      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can be assigned to an rgb_type<..> */
    get_material_back_color( enum_type facet)
      { return get_material_color< CC_TYPE >( GL_BACK, facet); }

      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can be assigned to an rgb_type<..> */
    get_material_front_color( enum_type facet)
      { return get_material_color< CC_TYPE >( GL_FRONT, facet); }

      template< typename COLOR_TYPE >
      void
    setup_material_back_color( enum_type facet, COLOR_TYPE const & color)
      { setup_material_color( GL_BACK, facet, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_front_color( enum_type facet, COLOR_TYPE const & color)
      { setup_material_color( GL_FRONT, facet, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_color( enum_type facet, COLOR_TYPE const & color)
      { setup_material_color( GL_FRONT_AND_BACK, facet, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_back_color( enum_type facet, COLOR_TYPE const & color)
      { drop_material_color( GL_BACK, facet, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_front_color( enum_type facet, COLOR_TYPE const & color)
      { drop_material_color( GL_FRONT, facet, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_color( enum_type facet, COLOR_TYPE const & color)
      { drop_material_color( GL_FRONT_AND_BACK, facet, color); }

  // ---------------------------------------------------------------------------------------
  // Material colors, diffuse
  //
  //   The alpha value for diffuse becomes the alpha value for the material.
  public:
      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can be assigned to an rgb_type<..> */
    get_material_diffuse_color( enum_type face)
      { return get_material_color< CC_TYPE >( face, GL_DIFFUSE); }

      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can be assigned to an rgb_type<..> */
    get_material_back_diffuse_color( )
      { return get_material_back_color< CC_TYPE >( GL_DIFFUSE); }

      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can be assigned to an rgb_type<..> */
    get_material_front_diffuse_color( )
      { return get_material_front_color< CC_TYPE >( GL_DIFFUSE); }

    // -- setup --------------------------------------------------------

      template< typename COLOR_TYPE >
      void
    setup_material_diffuse_color( enum_type face, COLOR_TYPE const & color)
      { setup_material_color( face, GL_DIFFUSE, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_diffuse_color( COLOR_TYPE const & color)
      { setup_material_color( GL_DIFFUSE, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_back_diffuse_color( COLOR_TYPE const & color)
      { setup_material_back_color( GL_DIFFUSE, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_front_diffuse_color( COLOR_TYPE const & color)
      { setup_material_front_color( GL_DIFFUSE, color); }

    // -- drop --------------------------------------------------------

      template< typename COLOR_TYPE >
      void
    drop_material_diffuse_color( enum_type face, COLOR_TYPE const & color)
      { drop_material_color( face, GL_DIFFUSE, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_diffuse_color( COLOR_TYPE const & color)
      { drop_material_color( GL_DIFFUSE, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_back_diffuse_color( COLOR_TYPE const & color)
      { drop_material_back_color( GL_DIFFUSE, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_front_diffuse_color( COLOR_TYPE const & color)
      { drop_material_front_color( GL_DIFFUSE, color); }

  // ---------------------------------------------------------------------------------------
  // Material colors, ambient
  //
  //   The alpha component of ambient color is ignored. But we still provide the 4-value
  //   rgba methods because they work with the underlying interface.
  public:
      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can be assigned to an rgb_type<..> */
    get_material_ambient_color( enum_type face)
      { return get_material_color< CC_TYPE >( face, GL_AMBIENT); }

      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can be assigned to an rgb_type<..> */
    get_material_back_ambient_color( )
      { return get_material_back_color< CC_TYPE >( GL_AMBIENT); }

      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can be assigned to an rgb_type<..> */
    get_material_front_ambient_color( )
      { return get_material_front_color< CC_TYPE >( GL_AMBIENT); }

    // -- setup --------------------------------------------------------

      template< typename COLOR_TYPE >
      void
    setup_material_ambient_color( enum_type face, COLOR_TYPE const & color)
      { setup_material_color( face, GL_AMBIENT, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_ambient_color( COLOR_TYPE const & color)
      { setup_material_color( GL_AMBIENT, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_back_ambient_color( COLOR_TYPE const & color)
      { setup_material_back_color( GL_AMBIENT, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_front_ambient_color( COLOR_TYPE const & color)
      { setup_material_front_color( GL_AMBIENT, color); }

    // -- drop --------------------------------------------------------

      template< typename COLOR_TYPE >
      void
    drop_material_ambient_color( enum_type face, COLOR_TYPE const & color)
      { drop_material_color( face, GL_AMBIENT, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_ambient_color( COLOR_TYPE const & color)
      { drop_material_color( GL_AMBIENT, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_back_ambient_color( COLOR_TYPE const & color)
      { drop_material_back_color( GL_AMBIENT, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_front_ambient_color( COLOR_TYPE const & color)
      { drop_material_front_color( GL_AMBIENT, color); }

  // ---------------------------------------------------------------------------------------
  // Material ambient_and_diffuse
  //
  //   Alpha values are used in the diffuse component.
  public:
      template< typename COLOR_TYPE >
      void
    setup_material_ambient_and_diffuse_color( enum_type face, COLOR_TYPE const & color)
      { setup_material_color( face, GL_AMBIENT_AND_DIFFUSE, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_ambient_and_diffuse_color( COLOR_TYPE const & color)
      { setup_material_color( GL_AMBIENT_AND_DIFFUSE, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_back_ambient_and_diffuse_color( COLOR_TYPE const & color)
      { setup_material_back_color( GL_AMBIENT_AND_DIFFUSE, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_front_ambient_and_diffuse_color( COLOR_TYPE const & color)
      { setup_material_front_color( GL_AMBIENT_AND_DIFFUSE, color); }

    // -- drop --------------------------------------------------------

      template< typename COLOR_TYPE >
      void
    drop_material_ambient_and_diffuse_color( enum_type face, COLOR_TYPE const & color)
      { drop_material_color( face, GL_AMBIENT_AND_DIFFUSE, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_ambient_and_diffuse_color( COLOR_TYPE const & color)
      { drop_material_color( GL_AMBIENT_AND_DIFFUSE, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_back_ambient_and_diffuse_color( COLOR_TYPE const & color)
      { drop_material_back_color( GL_AMBIENT_AND_DIFFUSE, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_front_ambient_and_diffuse_color( COLOR_TYPE const & color)
      { drop_material_front_color( GL_AMBIENT_AND_DIFFUSE, color); }

  // ---------------------------------------------------------------------------------------
  // Material specular
  //
  //   The alpha component of specular color is ignored. But we still provide the 4-value
  //   rgba methods because they work with the underlying interface.
  public:
      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can be assigned to an rgb_type<..> */
    get_material_specular_color( enum_type face)
      { return get_material_color< CC_TYPE >( face, GL_SPECULAR); }

      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can be assigned to an rgb_type<..> */
    get_material_back_specular_color( )
      { return get_material_back_color< CC_TYPE >( GL_SPECULAR); }

      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can be assigned to an rgb_type<..> */
    get_material_front_specular_color( )
      { return get_material_front_color< CC_TYPE >( GL_SPECULAR); }

    // -- setup --------------------------------------------------------

      template< typename COLOR_TYPE >
      void
    setup_material_specular_color( enum_type face, COLOR_TYPE const & color)
      { setup_material_color( face, GL_SPECULAR, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_specular_color( COLOR_TYPE const & color)
      { setup_material_color( GL_SPECULAR, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_back_specular_color( COLOR_TYPE const & color)
      { setup_material_back_color( GL_SPECULAR, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_front_specular_color( COLOR_TYPE const & color)
      { setup_material_front_color( GL_SPECULAR, color); }

    // -- drop --------------------------------------------------------

      template< typename COLOR_TYPE >
      void
    drop_material_specular_color( enum_type face, COLOR_TYPE const & color)
      { drop_material_color( face, GL_SPECULAR, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_specular_color( COLOR_TYPE const & color)
      { drop_material_color( GL_SPECULAR, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_back_specular_color( COLOR_TYPE const & color)
      { drop_material_back_color( GL_SPECULAR, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_front_specular_color( COLOR_TYPE const & color)
      { drop_material_front_color( GL_SPECULAR, color); }

  // ---------------------------------------------------------------------------------------
  // Material glow (emission)
  //
  //   The alpha component of glow color is ignored. But we still provide the 4-value
  //   rgba methods because they work with the underlying interface.
  public:
      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can be assigned to an rgb_type<..> */
    get_material_glow_color( enum_type face)
      { return get_material_color< CC_TYPE >( face, GL_EMISSION); }

      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can be assigned to an rgb_type<..> */
    get_material_back_glow_color( )
      { return get_material_back_color< CC_TYPE >( GL_EMISSION); }

      template< typename CC_TYPE >
      rgba_type< CC_TYPE > /* can be assigned to an rgb_type<..> */
    get_material_front_glow_color( )
      { return get_material_front_color< CC_TYPE >( GL_EMISSION); }

    // -- setup --------------------------------------------------------

      template< typename COLOR_TYPE >
      void
    setup_material_glow_color( enum_type face, COLOR_TYPE const & color)
      { setup_material_color( face, GL_EMISSION, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_glow_color( COLOR_TYPE const & color)
      { setup_material_color( GL_EMISSION, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_back_glow_color( COLOR_TYPE const & color)
      { setup_material_back_color( GL_EMISSION, color); }

      template< typename COLOR_TYPE >
      void
    setup_material_front_glow_color( COLOR_TYPE const & color)
      { setup_material_front_color( GL_EMISSION, color); }

    // -- drop --------------------------------------------------------

      template< typename COLOR_TYPE >
      void
    drop_material_glow_color( enum_type face, COLOR_TYPE const & color)
      { drop_material_color( face, GL_EMISSION, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_glow_color( COLOR_TYPE const & color)
      { drop_material_color( GL_EMISSION, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_back_glow_color( COLOR_TYPE const & color)
      { drop_material_back_color( GL_EMISSION, color); }

      template< typename COLOR_TYPE >
      void
    drop_material_front_glow_color( COLOR_TYPE const & color)
      { drop_material_front_color( GL_EMISSION, color); }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Material shininess
  // ---------------------------------------------------------------------------------------
  public:
    float_type   get_material_shininess_01( enum_type face)             ;//{ return get_material_shininess_128< float_type >( face) / 128.0f; }
    float_type   get_material_shininess_01_front( )                     { return get_material_shininess_01( GL_FRONT); }
    float_type   get_material_shininess_01_back( )                      { return get_material_shininess_01( GL_BACK ); }

    void         drop_material_shininess_01( enum_type face, float_type val)
                                                                        ;//{ drop_material_shininess_128< float_type >( face, val * 128.0f); }
    void         drop_material_shininess_01_front( float_type val)      { drop_material_shininess_01( GL_FRONT, val); }
    void         drop_material_shininess_01_back(  float_type val)      { drop_material_shininess_01( GL_BACK , val); }
    void         drop_material_shininess_01(       float_type val)      { drop_material_shininess_01( GL_FRONT_AND_BACK, val); }

    void         setup_material_shininess_01( enum_type face, float_type val)
                                                                        ;//{ setup_material_shininess_128< float_type >( face, val * 128.0f); }
    void         setup_material_shininess_01_front( float_type val)     { setup_material_shininess_01( GL_FRONT, val); }
    void         setup_material_shininess_01_back(  float_type val)     { setup_material_shininess_01( GL_BACK , val); }
    void         setup_material_shininess_01(       float_type val)     { setup_material_shininess_01( GL_FRONT_AND_BACK, val); }

  public:
    // These first three templates are specialized below.
    template< typename TYPE > TYPE  get_material_shininess_128( enum_type face) ;
    template< typename TYPE > void  drop_material_shininess_128( enum_type face, TYPE val) ;
    template< typename TYPE > void  setup_material_shininess_128( enum_type face, TYPE val) ;

  // Supply the face parameter.
  public:
    template< typename TYPE > TYPE  get_material_shininess_128_front( )            { return get_material_shininess_128< TYPE >( GL_FRONT); }
    template< typename TYPE > TYPE  get_material_shininess_128_back(  )            { return get_material_shininess_128< TYPE >( GL_BACK ); }

    template< typename TYPE > void  drop_material_shininess_128_front( TYPE val)   { drop_material_shininess_128( GL_FRONT, val); }
    template< typename TYPE > void  drop_material_shininess_128_back(  TYPE val)   { drop_material_shininess_128( GL_BACK , val); }
    template< typename TYPE > void  drop_material_shininess_128(       TYPE val)   { drop_material_shininess_128( GL_FRONT_AND_BACK, val); }

    template< typename TYPE > void  setup_material_shininess_128_front( TYPE val)  { setup_material_shininess_128( GL_FRONT, val); }
    template< typename TYPE > void  setup_material_shininess_128_back(  TYPE val)  { setup_material_shininess_128( GL_BACK , val); }
    template< typename TYPE > void  setup_material_shininess_128(       TYPE val)  { setup_material_shininess_128( GL_FRONT_AND_BACK, val); }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Enable and Get
  // ---------------------------------------------------------------------------------------
  public:
    void        enable( enum_type flag)             { assert_no_errors ne( *this);
                                                      ::glEnable( flag);
                                                      d_assert( as_bool( is_enabled( flag)));
                                                    }
    void        disable( enum_type flag)            { assert_no_errors ne( *this);
                                                      ::glDisable( flag);
                                                      d_assert( ! is_enabled( flag));
                                                    }
    bool_type   is_enabled( enum_type flag)         { assert_no_errors ne( *this);
                                                      return ::glIsEnabled( flag);
                                                    }

  // ---------------------------------------------------------------------------------------
  // Get
  //
  //   get_bool(..) vs is_enabled(..):
  //     Although we usually use is_enabled(..) instead of get_bool(..), sometimes get_bool(..)
  //     makes more sense. For example, GL_PACK_SWAP_BYTES, GL_UNPACK_SWAP_BYTES, GL_PACK_LSB_FIRST,
  //     and GL_UNPACK_LSB_FIRST are all bool values, but are NOT documented to work with
  //     glIsEnabled(..).
  //
  //   Additions:
  //     get_color, get_pt, get_matrix
  //     get_1st_of_two_ints(..)
  //     get_2nd_of_two_floats(..)
  //     get_nth_of_8_ints(..)
  //     get that returns a tuple (or array) for multiple values
  //     get_n_ints( name, n, p_return_ints)
  //     get_short(..) etc
  //     get_ushort(..) - see get_line_stipple_pattern( )
  //     get_byte(..) etc
  //     get_ubyte(..) etc (are they any properties stored as bytes)
  //
  public:
    bool_type    get_bool( enum_type flag)                                  ;
    void         get_2_bools( enum_type, bool_type &, bool_type &)          ;

    enum_type    get_enum( enum_type flag)                                  ;
    void         get_2_enums( enum_type flag, enum_type & a, enum_type & b) ;

    uint_type    get_uint( enum_type flag)                                  ;
    void         get_2_uints( enum_type flag, uint_type & a, uint_type & b) ;

    int_type     get_int( enum_type flag)                                                ;
    void         get_2_ints( enum_type, int_type &, int_type &)                          ;
    void         get_3_ints( enum_type, int_type &, int_type &, int_type &)              ;
    void         get_4_ints( enum_type, int_type &, int_type &, int_type &, int_type &)  ;

    float_type   get_float( enum_type flag)                                                              ;
    void         get_2_floats( enum_type flag, float_type &, float_type &)                               ;
    void         get_3_floats( enum_type flag, float_type &, float_type &, float_type &)                 ;
    void         get_4_floats( enum_type flag, float_type &, float_type &, float_type &, float_type &)   ;

    double_type  get_double( enum_type flag)                                                                 ;
    void         get_2_doubles( enum_type flag, double_type &, double_type &)                                ;
    void         get_3_doubles( enum_type flag, double_type &, double_type &, double_type &)                 ;
    void         get_4_doubles( enum_type flag, double_type &, double_type &, double_type &, double_type &)  ;

                   // Replace this with get_matrix. Matrices are the only 16-value gets.
    void         get_16_doubles
                  (  enum_type     flag
                   , double_type & val0 , double_type & val1 , double_type & val2 , double_type & val3
                   , double_type & val4 , double_type & val5 , double_type & val6 , double_type & val7
                   , double_type & val8 , double_type & val9 , double_type & val10, double_type & val11
                   , double_type & val12, double_type & val13, double_type & val14, double_type & val15
                  )                                                                                          ;

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Matrix stack
  // ---------------------------------------------------------------------------------------
  private:
    void         init_matrix_values( )               ;
    void         uninit_matrix_values( )             ;

  public:
    void         push_copy_on_current_matrix_stack( )  ;
    void         pop_current_matrix_stack( )           ;

    void         set_current_matrix_name( enum_type name)  ;

    void         reset_current_matrix( )                             ;
    void         set_current_matrix( float_type const * p16_matrix)  ;
    void         set_current_matrix( double_type const * p16_matrix) ;

    void         multiply_current_matrix( float_type const * p16_matrix)   ;
    void         multiply_current_matrix( double_type const * p16_matrix)  ;

    void         rotate_current_matrix( float_type angle, float_type x_vector, float_type y, float_type z)     ;
    void         rotate_current_matrix( double_type angle, double_type x_vector, double_type y, double_type z) ;

    void         scale_current_matrix( float_type x_scale, float_type y_scale, float_type z_scale)     ;
    void         scale_current_matrix( double_type x_scale, double_type y_scale, double_type z_scale)  ;

    void         translate_current_matrix( float_type x_offset, float_type y, float_type z)    ;
    void         translate_current_matrix( double_type x_offset, double_type y, double_type z) ;

    void         setup_ortho
                  (  double_type  xlo, double_type  xhi
                   , double_type  ylo, double_type  yhi
                   , double_type  zlo, double_type  zhi // aka z_near and z_far
                  )                                  ;
    void         setup_frustum
                  (  double_type  xlo, double_type  xhi
                   , double_type  ylo, double_type  yhi
                   , double_type  zlo, double_type  zhi // aka z_near and z_far
                  )                                  ;

    void         get_viewport_max_sizes( size_type & x_delta, size_type & y_delta)
                                                     ;
    void         get_viewport
                  (  int_type  &  xlo    , int_type  &  ylo
                   , size_type &  x_delta, size_type &  y_delta // aka width and height
                  )                                  ;
    void         set_viewport
                  (  int_type   xlo    , int_type   ylo
                   , size_type  x_delta, size_type  y_delta // aka width and height
                  )                                  ;

  // ---------------------------------------------------------------------------------------
  public:
    enum_type    get_current_matrix_name( )       const { return current_matrix_name_; }
    bool_type    is_current_matrix_modelview(  )  const { return get_current_matrix_name( ) == GL_MODELVIEW ; }
    bool_type    is_current_matrix_projection( )  const { return get_current_matrix_name( ) == GL_PROJECTION; }
    bool_type    is_current_matrix_texture(    )  const { return get_current_matrix_name( ) == GL_TEXTURE   ; }

    int_type     get_stack_depth_modelview_matrix(  )        const { return stack_depth_modelview_matrix_; }
    int_type     get_stack_depth_projection_matrix( )        const { return stack_depth_projection_matrix_; }
    int_type     get_stack_depth_texture_matrix(    )        const { return stack_depth_texture_matrix_; }
    int_type     get_stack_depth( enum_type matrix_name)     const ;
    int_type     get_stack_depth_current_matrix( )           const { return get_stack_depth( get_current_matrix_name( )); }

    enum_type    get_current_matrix_name_from_gl( )                { return get_enum( GL_MATRIX_MODE); }

    int_type     get_stack_depth_modelview_matrix_from_gl(  )      { return get_int( GL_MODELVIEW_STACK_DEPTH ); }
    int_type     get_stack_depth_projection_matrix_from_gl( )      { return get_int( GL_PROJECTION_STACK_DEPTH); }
    int_type     get_stack_depth_texture_matrix_from_gl(    )      { return get_int( GL_TEXTURE_STACK_DEPTH   ); }

    int_type     get_max_stack_depth_modelview_matrix_from_gl(  )  { return get_int( GL_MAX_MODELVIEW_STACK_DEPTH ); }
    int_type     get_max_stack_depth_projection_matrix_from_gl( )  { return get_int( GL_MAX_PROJECTION_STACK_DEPTH); }
    int_type     get_max_stack_depth_texture_matrix_from_gl(    )  { return get_int( GL_MAX_TEXTURE_STACK_DEPTH   ); }

    // Min values for supported maximums, according the the standard (documented at opengl.org):
    int_type     get_min_max_stack_depth_modelview_matrix(  )      { return 32; }
    int_type     get_min_max_stack_depth_projection_matrix( )      { return 2; }
    int_type     get_min_max_stack_depth_texture_matrix(    )      { return 2; }

  // ---------------------------------------------------------------------------------------
  public:
    void         assert_current_matrix_name( )       {
                         d_assert( get_current_matrix_name( ) == get_current_matrix_name_from_gl( ));
                                                     }
    void         assert_current_matrix_name( enum_type name)
                                                    {
                         maybe_used_only_for_debug( name);
                         d_assert( get_current_matrix_name( ) == name);
                         d_assert( get_current_matrix_name( ) == get_current_matrix_name_from_gl( ));
                                                     }
    void         assert_current_stack_depths( )      {
                         d_assert( get_stack_depth_modelview_matrix(  ) == get_stack_depth_modelview_matrix_from_gl(  ));
                         d_assert( get_stack_depth_projection_matrix( ) == get_stack_depth_projection_matrix_from_gl( ));
                         d_assert( get_stack_depth_texture_matrix(    ) == get_stack_depth_texture_matrix_from_gl(    ));
                                                     }

  # ifdef NDEBUG
    void         assert_is_identity_matrix( enum_type)
                                                     { }
  # else
    void         assert_is_identity_matrix( enum_type matrix_name)
                                                     ;
  # endif

  // ---------------------------------------------------------------------------------------
  // Member vars
  private:
    enum_type  current_matrix_name_           ;  // ie GL_MODELVIEW
    int_type   stack_depth_modelview_matrix_  ;  // could be unsigned
    int_type   stack_depth_projection_matrix_ ;  // could be unsigned
    int_type   stack_depth_texture_matrix_    ;  // could be unsigned

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Server-Attribute Save-Stack
  // ---------------------------------------------------------------------------------------
  private:
    void         init_saved_server_attributes_stack_values( )        ;
    void         uninit_saved_server_attributes_stack_values( )      ;
                   /* member var */ int_type stack_depth_saved_server_attributes_ ;

  public:
    void         push_server_attribute_stack( bitfield_type)         ;
    void         pop_server_attribute_stack( )                       ;

    int_type     get_stack_depth_saved_server_attributes( )          { return stack_depth_saved_server_attributes_; }
    int_type     get_stack_depth_saved_server_attributes_from_gl( )  { return get_int( GL_ATTRIB_STACK_DEPTH ); }
    int_type     get_max_stack_depth_saved_server_attributes_from_gl( )
                                                                     { return get_int( GL_MAX_ATTRIB_STACK_DEPTH ); }

    // Min value for supported maximum, according the the standard (documented at opengl.org):
    int_type     get_min_max_stack_depth_saved_server_attributes( )  { return 16; }

    void         assert_saved_server_attributes_stack_deph( )        { d_assert(
                                                                         get_stack_depth_saved_server_attributes( ) ==
                                                                         get_stack_depth_saved_server_attributes_from_gl( ));
                                                                     }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Pixels
  // ---------------------------------------------------------------------------------------
  public:
    void         set_pixel_map( enum_type map_type, size_type map_size, ushort_type const * p_map_values)
                                                    { d_assert( is_pixel_map_type( map_type));
                                                      d_assert( p_map_values != 0);
                                                      assert_no_errors ne( *this);
                                                      ::glPixelMapusv( map_type, map_size, p_map_values);
                                                    }
    void         set_pixel_map( enum_type map_type, size_type map_size, uint_type const * p_map_values)
                                                    { d_assert( is_pixel_map_type( map_type));
                                                      d_assert( p_map_values != 0);
                                                      assert_no_errors ne( *this);
                                                      ::glPixelMapuiv( map_type, map_size, p_map_values);
                                                    }
    void         set_pixel_map( enum_type map_type, size_type map_size, float_type const * p_map_values)
                                                    { d_assert( is_pixel_map_type( map_type));
                                                      d_assert( p_map_values != 0);
                                                      assert_no_errors ne( *this);
                                                      ::glPixelMapfv( map_type, map_size, p_map_values);
                                                    }

                   // set to zero to turn this off for 1D textures
                   // GL_PACK_IMAGE_HEIGHT - for 3D textures, not in 1.1
                   // GL_PACK_SKIP_PIXELS  - offset (xlo border)
                   // GL_PACK_SKIP_ROWS    - offset (ylo border)
                   // GL_PACK_SKIP_IMAGES  - offset (zlo border), for 3D textures, not in 1.1
    void         set_pack_pixel_row_length( int_type a)
                                                    { d_assert( a >= 0);
                                                      assert_no_errors ne( *this);
                                                      ::glPixelStorei( GL_PACK_ROW_LENGTH, a);
                                                    }
                   // set to zero to turn this off for 1D textures
                   // GL_UNPACK_IMAGE_HEIGHT - for 3D textures, not in 1.1
                   // GL_UNPACK_SKIP_PIXELS  - offset (xlo border)
                   // GL_UNPACK_SKIP_ROWS    - offset (ylo border)
                   // GL_UNPACK_SKIP_IMAGES  - offset (zlo border), for 3D textures, not in 1.1
    void         set_unpack_pixel_row_length( int_type a)
                                                    { d_assert( a >= 0);
                                                      assert_no_errors ne( *this);
                                                      ::glPixelStorei( GL_UNPACK_ROW_LENGTH, a);
                                                    }

    void         set_pack_pixel_row_byte_alignment( int_type a)
                                                    { d_assert( (1 == a) || (2 == a) || (4 == a) || (a == 8));
                                                      assert_no_errors ne( *this);
                                                      ::glPixelStorei( GL_PACK_ALIGNMENT, a);
                                                    }
    void         set_unpack_pixel_row_byte_alignment( int_type a)
                                                    { d_assert( (1 == a) || (2 == a) || (4 == a) || (a == 8));
                                                      assert_no_errors ne( *this);
                                                      ::glPixelStorei( GL_UNPACK_ALIGNMENT, a);
                                                    }

    void         set_pack_pixel_byte_swapping( bool is)
                                                    { assert_no_errors ne( *this);
                                                      ::glPixelStorei( GL_PACK_SWAP_BYTES, is ? GL_TRUE : GL_FALSE);
                                                    }
    void         set_unpack_pixel_byte_swapping( bool is)
                                                    { assert_no_errors ne( *this);
                                                      ::glPixelStorei( GL_UNPACK_SWAP_BYTES, is ? GL_TRUE : GL_FALSE);
                                                    }

    void         pack_bitmap_bits_lsb_last( )       { assert_no_errors ne( *this);
                                                      ::glPixelStorei( GL_PACK_LSB_FIRST, GL_FALSE);
                                                    }
    void         pack_bitmap_bits_lsb_first( )      { assert_no_errors ne( *this);
                                                      ::glPixelStorei( GL_PACK_LSB_FIRST, GL_TRUE);
                                                    }
    void         unpack_bitmap_bits_lsb_last( )     { assert_no_errors ne( *this);
                                                      ::glPixelStorei( GL_UNPACK_LSB_FIRST, GL_FALSE);
                                                    }
    void         unpack_bitmap_bits_lsb_first( )    { assert_no_errors ne( *this);
                                                      ::glPixelStorei( GL_UNPACK_LSB_FIRST, GL_TRUE);
                                                    }

    void         set_pixel_transfer_param( enum_type param, float_type value)
                                                    { d_assert( is_pixel_transfer_param( param));
                                                      assert_no_errors ne( *this);
                                                      ::glPixelTransferf( param, value);
                                                    }

                   // color substitution during pixel transfer
    void         enable_pixel_transfer_color_mapping( )
                                                    { assert_no_errors ne( *this);
                                                      ::glPixelTransferi( GL_MAP_COLOR, true);
                                                    }
    void         disable_pixel_transfer_color_mapping( )
                                                    { assert_no_errors ne( *this);
                                                      ::glPixelTransferi( GL_MAP_COLOR, false);
                                                    }
    void         enable_pixel_transfer_stencil_mapping( )
                                                    { assert_no_errors ne( *this);
                                                      ::glPixelTransferi( GL_MAP_STENCIL, true);
                                                    }
    void         disable_pixel_transfer_stencil_mapping( )
                                                    { assert_no_errors ne( *this);
                                                      ::glPixelTransferi( GL_MAP_STENCIL, false);
                                                    }

    void         set_pixel_zoom
                  (  float_type x_scale
                   , float_type y_scale
                  )                                 { d_assert( 0 != x_scale);
                                                      d_assert( 0 != y_scale);
                                                      assert_no_errors ne( *this);
                                                      ::glPixelZoom( x_scale, y_scale);
                                                    }
    void         read_pixels_from_framebuffer
                  (  int_type   x
                   , int_type   y
                   , size_type  x_del
                   , size_type  y_del
                   , enum_type  pixel_format
                   , enum_type  pixel_data_type
                   , void *     p_pixel_data
                  )                                 { d_assert( x >= 0);
                                                      d_assert( y >= 0);
                                                      d_assert( x_del > 0);
                                                      d_assert( y_del > 0);
                                                      d_assert( is_read_write_pixel_format( pixel_format));
                                                      d_assert( is_pixel_data_type( pixel_data_type));
                                                      d_assert( p_pixel_data != 0);
                                                      assert_no_errors ne( *this);
                                                      // affected by glBindBuffer(..)
                                                      ::glReadPixels( x, y, x_del, y_del, pixel_format, pixel_data_type, p_pixel_data);
                                                    }
                   template< typename T >
    void         read_pixels_from_framebuffer
                  (  int_type   x
                   , int_type   y
                   , size_type  x_del
                   , size_type  y_del
                   , enum_type  pixel_format
                   , T *        p_pixel_data
                  )                                 { read_pixels_from_framebuffer(
                                                        x, y, x_del, y_del, pixel_format, type_traits< T >::data_type, p_pixel_data);
                                                    }
    void         read_pixels_from_framebuffer__color_bitmap
                  (  int_type      x
                   , int_type      y
                   , size_type     x_del
                   , size_type     y_del
                   , ubyte_type *  p_pixel_data
                  )                                 { read_pixels_from_framebuffer(
                                                        x, y, x_del, y_del, GL_COLOR_INDEX, GL_BITMAP, p_pixel_data);
                                                    }
    void         read_pixels_from_framebuffer__stencil_bitmap
                  (  int_type      x
                   , int_type      y
                   , size_type     x_del
                   , size_type     y_del
                   , ubyte_type *  p_pixel_data
                  )                                 { read_pixels_from_framebuffer(
                                                        x, y, x_del, y_del, GL_STENCIL_INDEX, GL_BITMAP, p_pixel_data);
                                                    }

                   // Use glRasterPos(..) or glWindowPos(..) to set the (x,y) corner.
    void         draw_pixels_to_framebuffer
                  (  size_type     x_del
                   , size_type     y_del
                   , enum_type     pixel_format
                   , enum_type     pixel_data_type
                   , void const *  p_pixel_data
                  )                                 { d_assert( x_del > 0);
                                                      d_assert( y_del > 0);
                                                      d_assert( is_read_write_pixel_format( pixel_format));
                                                      d_assert( is_pixel_data_type( pixel_data_type));
                                                      d_assert( p_pixel_data != 0);
                                                      // affected by glBindBuffer(..)
                                                      assert_no_errors ne( *this);
                                                      ::glDrawPixels( x_del, y_del, pixel_format, pixel_data_type, p_pixel_data);
                                                    }
                   template< typename T >
    void         draw_pixels_to_framebuffer
                  (  size_type  x_del
                   , size_type  y_del
                   , enum_type  pixel_format
                   , T const *  p_pixel_data
                  )                                 { draw_pixels_to_framebuffer(
                                                        x_del, y_del, pixel_format, type_traits< T >::data_type, p_pixel_data);
                                                    }
    void         draw_pixels_to_framebuffer__color_bitmap
                  (  size_type           x_del
                   , size_type           y_del
                   , ubyte_type const *  p_pixel_data
                  )                                 { draw_pixels_to_framebuffer(
                                                        x_del, y_del, GL_COLOR_INDEX, GL_BITMAP, p_pixel_data);
                                                    }
    void         draw_pixels_to_framebuffer__stencil_bitmap
                  (  size_type           x_del
                   , size_type           y_del
                   , ubyte_type const *  p_pixel_data
                  )                                 { draw_pixels_to_framebuffer(
                                                        x_del, y_del, GL_STENCIL_INDEX, GL_BITMAP, p_pixel_data);
                                                    }

  // ---------------------------------------------------------------------------------------
    void         copy_pixels
                  (  int_type   x
                   , int_type   y
                   , size_type  x_del
                   , size_type  y_del
                   , enum_type  pixel_src
                  )                                 { d_assert( x >= 0);
                                                      d_assert( y >= 0);
                                                      d_assert( x_del > 0);
                                                      d_assert( y_del > 0);
                                                      d_assert( is_copy_pixel_src( pixel_src));
                                                      assert_no_errors ne( *this);
                                                      ::glCopyPixels( x, y, x_del, y_del, pixel_src);
                                                    }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Textures
  // ---------------------------------------------------------------------------------------
  // See:
  //   drop_texture_coords(..)
  //   setup_texture_coords(..)
  //
  //   is_current_matrix_texture( )
  //   get_stack_depth_texture_matrix( )
  //   get_stack_depth_texture_matrix_from_gl( )
  //   get_max_stack_depth_texture_matrix_from_gl( )
  //   get_min_max_stack_depth_texture_matrix( )
  //
  // GL_CURRENT_TEXTURE_COORDS
  public:
                   // Size in pixels
    int_type     get_max_texture_size( )            { return get_int( GL_MAX_TEXTURE_SIZE); } /* only an estimate */
    int_type     get_min_max_texture_size( )        { return 64; }

    void         enable_1D_texturing( )             { enable( GL_TEXTURE_1D); }
    void         disable_1D_texturing( )            { disable( GL_TEXTURE_1D); }
    bool_type    is_1D_texturing_enabled( )         { return is_enabled( GL_TEXTURE_1D); }

    void         enable_2D_texturing( )             { enable( GL_TEXTURE_2D); }
    void         disable_2D_texturing( )            { disable( GL_TEXTURE_2D); }
    bool_type    is_2D_texturing_enabled( )         { return is_enabled( GL_TEXTURE_2D); }

    void         enable_auto_texture_s_gen( )       { enable( GL_TEXTURE_GEN_S); }
    void         disable_auto_texture_s_gen( )      { disable( GL_TEXTURE_GEN_S); }
    bool_type    is_auto_texture_s_gen_enabled( )   { return is_enabled( GL_TEXTURE_GEN_S); }

    void         enable_auto_texture_t_gen( )       { enable( GL_TEXTURE_GEN_T); }
    void         disable_auto_texture_t_gen( )      { disable( GL_TEXTURE_GEN_T); }
    bool_type    is_auto_texture_t_gen_enabled( )   { return is_enabled( GL_TEXTURE_GEN_T); }

    void         enable_auto_texture_r_gen( )       { enable( GL_TEXTURE_GEN_R); }
    void         disable_auto_texture_r_gen( )      { disable( GL_TEXTURE_GEN_R); }
    bool_type    is_auto_texture_r_gen_enabled( )   { return is_enabled( GL_TEXTURE_GEN_R); }

    void         enable_auto_texture_q_gen( )       { enable( GL_TEXTURE_GEN_Q); }
    void         disable_auto_texture_q_gen( )      { disable( GL_TEXTURE_GEN_Q); }
    bool_type    is_auto_texture_q_gen_enabled( )   { return is_enabled( GL_TEXTURE_GEN_Q); }

    uint_type    create_texture_id( )               { uint_type texture_id = 0;
                                                      assert_no_errors ne( *this);
                                                      ::glGenTextures( 1, & texture_id);
                                                      d_assert( texture_id != 0); /* never zero */
                                                      return texture_id;
                                                    }

    void         delete_texture_id( uint_type id)   { d_assert( id != 0);
                                                      assert_no_errors ne( *this);
                                                      ::glDeleteTextures( 1, & id);
                                                    }

    void         bind_1D_texture( uint_type id)     { { assert_no_errors ne( *this);
                                                        ::glBindTexture( GL_TEXTURE_1D, id);
                                                      }
                                                      d_assert( get_bound_1D_texture( ) == id);
                                                    }
    void         bind_2D_texture( uint_type id)     { { assert_no_errors ne( *this);
                                                        ::glBindTexture( GL_TEXTURE_2D, id);
                                                      }
                                                      d_assert( get_bound_2D_texture( ) == id);
                                                    }
    void         unbind_1D_texture( )               { bind_1D_texture( 0); }
    void         unbind_2D_texture( )               { bind_2D_texture( 0); }
    uint_type    get_bound_1D_texture( )            { return get_uint( GL_TEXTURE_BINDING_1D); }
    uint_type    get_bound_2D_texture( )            { return get_uint( GL_TEXTURE_BINDING_2D); }

  // ---------------------------------------------------------------------------------------
    enum_type    get_1D_texture_minifying_filter( ) { int_type filter = 0;
                                                      assert_no_errors ne( *this);
                                                      ::glGetTexParameteriv( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, & filter);
                                                      d_assert( is_texture_minifying_filter( filter));
                                                      return filter;
                                                    }
    enum_type    get_2D_texture_minifying_filter( ) { int_type filter = 0;
                                                      assert_no_errors ne( *this);
                                                      ::glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, & filter);
                                                      d_assert( is_texture_minifying_filter( filter));
                                                      return filter;
                                                    }
    enum_type    get_1D_texture_magnifying_filter( )
                                                    { int_type filter = 0;
                                                      assert_no_errors ne( *this);
                                                      ::glGetTexParameteriv( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, & filter);
                                                      d_assert( is_texture_magnifying_filter( filter));
                                                      return filter;
                                                    }
    enum_type    get_2D_texture_magnifying_filter( )
                                                    { int_type filter = 0;
                                                      assert_no_errors ne( *this);
                                                      ::glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, & filter);
                                                      d_assert( is_texture_magnifying_filter( filter));
                                                      return filter;
                                                    }

    enum_type    get_1D_texture_s_wrap( )           { int_type wrap = 0;
                                                      assert_no_errors ne( *this);
                                                      ::glGetTexParameteriv( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, & wrap);
                                                      d_assert( is_texture_wrap( wrap));
                                                      return wrap;
                                                    }
    enum_type    get_1D_texture_t_wrap( )           { int_type wrap = 0;
                                                      assert_no_errors ne( *this);
                                                      // Does this make sense? Or does 1D only have an S coord?
                                                      ::glGetTexParameteriv( GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, & wrap);
                                                      d_assert( is_texture_wrap( wrap));
                                                      return wrap;
                                                    }
    enum_type    get_2D_texture_s_wrap( )           { int_type wrap = 0;
                                                      assert_no_errors ne( *this);
                                                      ::glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, & wrap);
                                                      d_assert( is_texture_wrap( wrap));
                                                      return wrap;
                                                    }
    enum_type    get_2D_texture_t_wrap( )           { int_type wrap = 0;
                                                      assert_no_errors ne( *this);
                                                      ::glGetTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, & wrap);
                                                      d_assert( is_texture_wrap( wrap));
                                                      return wrap;
                                                    }

    float_type   get_1D_texture_priority( )         { float_type prio = -1;
                                                      assert_no_errors ne( *this);
                                                      ::glGetTexParameterfv( GL_TEXTURE_1D, GL_TEXTURE_PRIORITY, & prio);
                                                      d_assert( (0 <= prio) && (prio <= 1));
                                                      return prio;
                                                    }
    float_type   get_2D_texture_priority( )         { float_type prio = -1;
                                                      assert_no_errors ne( *this);
                                                      ::glGetTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, & prio);
                                                      d_assert( (0 <= prio) && (prio <= 1));
                                                      return prio;
                                                    }
    rgba_type< float_type >
                 get_1D_texture_border_color( )     { rgba_type< float_type > color;
                                                      assert_no_errors ne( *this);
                                                      ::glGetTexParameterfv(
                                                            GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color.ref_as_p4( ));
                                                      return color;
                                                    }
    rgba_type< float_type >
                 get_2D_texture_border_color( )     { rgba_type< float_type > color;
                                                      assert_no_errors ne( *this);
                                                      ::glGetTexParameterfv(
                                                            GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color.ref_as_p4( ));
                                                      return color;
                                                    }

  // ---------------------------------------------------------------------------------------
    void         set_1D_texture_minifying_filter( enum_type filter)
                                                    { d_assert( is_texture_minifying_filter( filter));
                                                      assert_no_errors ne( *this);
                                                      ::glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, filter);
                                                    }
    void         set_2D_texture_minifying_filter( enum_type filter)
                                                    { d_assert( is_texture_minifying_filter( filter));
                                                      assert_no_errors ne( *this);
                                                      ::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
                                                    }
    void         set_1D_texture_magnifying_filter( enum_type filter)
                                                    { d_assert( is_texture_magnifying_filter( filter));
                                                      assert_no_errors ne( *this);
                                                      ::glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, filter);
                                                    }
    void         set_2D_texture_magnifying_filter( enum_type filter)
                                                    { d_assert( is_texture_magnifying_filter( filter));
                                                      assert_no_errors ne( *this);
                                                      ::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
                                                    }
    void         set_1D_texture_s_wrap( enum_type w)
                                                    { d_assert( is_texture_wrap( w));
                                                      assert_no_errors ne( *this);
                                                      ::glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, w);
                                                    }
    void         set_1D_texture_t_wrap( enum_type w)
                                                    { d_assert( is_texture_wrap( w));
                                                      // Does this make sense? Or does 1D only have an S coord?
                                                      assert_no_errors ne( *this);
                                                      ::glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, w);
                                                    }
    void         set_2D_texture_s_wrap( enum_type w)
                                                    { d_assert( is_texture_wrap( w));
                                                      assert_no_errors ne( *this);
                                                      ::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, w);
                                                    }
    void         set_2D_texture_t_wrap( enum_type w)
                                                    { d_assert( is_texture_wrap( w));
                                                      assert_no_errors ne( *this);
                                                      ::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, w);
                                                    }

    void         set_1D_texture_priority( float_type prio)
                                                    { d_assert( (0 <= prio) && (prio <= 1));
                                                      assert_no_errors ne( *this);
                                                      ::glTexParameterf( GL_TEXTURE_1D, GL_TEXTURE_PRIORITY, prio);
                                                    }
    void         set_2D_texture_priority( float_type prio)
                                                    { d_assert( (0 <= prio) && (prio <= 1));
                                                      assert_no_errors ne( *this);
                                                      ::glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, prio);
                                                    }

    void         set_1D_texture_border_color( rgba_type< float_type > const & color)
                                                    { assert_no_errors ne( *this);
                                                      ::glTexParameterfv( GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color.get_as_p4( ));
                                                    }
    void         set_2D_texture_border_color( rgba_type< float_type > const & color)
                                                    { assert_no_errors ne( *this);
                                                      ::glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color.get_as_p4( ));
                                                    }

  // ---------------------------------------------------------------------------------------
    void         set_auto_texture_s_gen_mode( enum_type mode)
                                                    { d_assert( is_auto_texture_gen_mode( mode));
                                                      assert_no_errors ne( *this);
                                                      ::glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, mode);
                                                    }
    void         set_auto_texture_t_gen_mode( enum_type mode)
                                                    { d_assert( is_auto_texture_gen_mode( mode));
                                                      assert_no_errors ne( *this);
                                                      ::glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, mode);
                                                    }
    void         set_auto_texture_r_gen_mode( enum_type mode)
                                                    { d_assert( is_auto_texture_gen_mode( mode));
                                                      assert_no_errors ne( *this);
                                                      ::glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, mode);
                                                    }
    void         set_auto_texture_q_gen_mode( enum_type mode)
                                                    { d_assert( is_auto_texture_gen_mode( mode));
                                                      assert_no_errors ne( *this);
                                                      ::glTexGeni( GL_Q, GL_TEXTURE_GEN_MODE, mode);
                                                    }

  // ---------------------------------------------------------------------------------------
    void         set_auto_texture_s_gen_object_plane( float_type x, float_type y, float_type z, float_type w)
                                                    { d_assert( x || y || z || w); /* cannot all be zero */
                                                      float_type plane[ 4 ] = { x, y, z, w };
                                                      assert_no_errors ne( *this);
                                                      ::glTexGenfv( GL_S, GL_OBJECT_PLANE, plane);
                                                    }
    void         set_auto_texture_s_gen_eye_plane( float_type x, float_type y, float_type z, float_type w)
                                                    { d_assert( x || y || z || w); /* cannot all be zero */
                                                      float_type plane[ 4 ] = { x, y, z, w };
                                                      assert_no_errors ne( *this);
                                                      ::glTexGenfv( GL_S, GL_EYE_PLANE, plane);
                                                    }

    void         set_auto_texture_t_gen_object_plane( float_type x, float_type y, float_type z, float_type w)
                                                    { d_assert( x || y || z || w); /* cannot all be zero */
                                                      float_type plane[ 4 ] = { x, y, z, w };
                                                      assert_no_errors ne( *this);
                                                      ::glTexGenfv( GL_T, GL_OBJECT_PLANE, plane);
                                                    }
    void         set_auto_texture_t_gen_eye_plane( float_type x, float_type y, float_type z, float_type w)
                                                    { d_assert( x || y || z || w); /* cannot all be zero */
                                                      float_type plane[ 4 ] = { x, y, z, w };
                                                      assert_no_errors ne( *this);
                                                      ::glTexGenfv( GL_T, GL_EYE_PLANE, plane);
                                                    }

    void         set_auto_texture_r_gen_object_plane( float_type x, float_type y, float_type z, float_type w)
                                                    { d_assert( x || y || z || w); /* cannot all be zero */
                                                      float_type plane[ 4 ] = { x, y, z, w };
                                                      assert_no_errors ne( *this);
                                                      ::glTexGenfv( GL_R, GL_OBJECT_PLANE, plane);
                                                    }
    void         set_auto_texture_r_gen_eye_plane( float_type x, float_type y, float_type z, float_type w)
                                                    { d_assert( x || y || z || w); /* cannot all be zero */
                                                      float_type plane[ 4 ] = { x, y, z, w };
                                                      assert_no_errors ne( *this);
                                                      ::glTexGenfv( GL_R, GL_EYE_PLANE, plane);
                                                    }

    void         set_auto_texture_q_gen_object_plane( float_type x, float_type y, float_type z, float_type w)
                                                    { d_assert( x || y || z || w); /* cannot all be zero */
                                                      float_type plane[ 4 ] = { x, y, z, w };
                                                      assert_no_errors ne( *this);
                                                      ::glTexGenfv( GL_Q, GL_OBJECT_PLANE, plane);
                                                    }
    void         set_auto_texture_q_gen_eye_plane( float_type x, float_type y, float_type z, float_type w)
                                                    { d_assert( x || y || z || w); /* cannot all be zero */
                                                      float_type plane[ 4 ] = { x, y, z, w };
                                                      assert_no_errors ne( *this);
                                                      ::glTexGenfv( GL_Q, GL_EYE_PLANE, plane);
                                                    }

  // ---------------------------------------------------------------------------------------
  // glTexEnvi( GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, float_type)
  // glTexEnvi( GL_POINT_SPRITE, GL_COORD_REPLACE, bool_type is_enabled)
  //
  //   GL_TEXTURE_ENV  GL_ALPHA_SCALE    1.0, 2.0, or 4.0
  //   GL_TEXTURE_ENV  GL_RGB_SCALE      1.0, 2.0, or 4.0
  //
  //   GL_TEXTURE_ENV  GL_COMBINE_RGB
  //   GL_TEXTURE_ENV  GL_COMBINE_ALPHA
  //   GL_TEXTURE_ENV  GL_RGB_SCALE
  //   GL_TEXTURE_ENV  GL_ALPHA_SCALE
  //   GL_TEXTURE_ENV  GL_SRC0_RGB
  //   GL_TEXTURE_ENV  GL_SRC1_RGB
  //   GL_TEXTURE_ENV  GL_SRC2_RGB
  //   GL_TEXTURE_ENV  GL_SRC0_ALPHA
  //   GL_TEXTURE_ENV  GL_SRC1_ALPHA
  //   GL_TEXTURE_ENV  GL_SRC2_ALPHA
  //
  // Combiners for multi-texture:
  //   GL_TEXTURE_ENV  GL_OPERAND0_RGB
  //   GL_TEXTURE_ENV  GL_OPERAND1_RGB
  //   GL_TEXTURE_ENV  GL_OPERAND2_RGB
  //   GL_TEXTURE_ENV  GL_OPERAND0_ALPHA
  //   GL_TEXTURE_ENV  GL_OPERAND1_ALPHA
  //   GL_TEXTURE_ENV  GL_OPERAND2_ALPHA

    void         set_texture_env_color( rgba_type< float_type > const & color)
                                                    { assert_no_errors ne( *this);
                                                      ::glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color.get_as_p4( ));
                                                    }
    void         set_texture_env_mode( enum_type mode)
                                                    { d_assert( is_texture_env_mode( mode));
                                                      assert_no_errors ne( *this);
                                                      ::glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
                                                    }

  // ---------------------------------------------------------------------------------------
  // need wrappers for glGetTexLevelParameter(..)

    void         set_1D_texture_bits__no_border
                  (  enum_type     internal_format  // ie GL_LUMINANCE
                   , size_type     pixel_count      // power of 2
                   , enum_type     pixel_format     // ie GL_RGBA
                   , enum_type     pixel_data_type  // ie GL_UNSIGNED_BYTE
                   , void const *  p_pixel_data
                  )                                 { d_assert( is_texture_internal_format( internal_format));
                                                      d_assert( util::is_power_of_2( pixel_count));
                                                      d_assert( pixel_count <= get_max_texture_size( ));
                                                      // First param can also be GL_PROXY_TEXTURE_1D. Should use this
                                                      // and ::glGetTexLevelParameter(..) to find out if a texture is
                                                      // too big, and not rely on get_max_texture_size( ).
                                                      d_assert( is_1D_pixel_format( pixel_format));
                                                      d_assert( is_pixel_data_type( pixel_data_type));
                                                      d_assert( p_pixel_data != 0);
                                                      d_assert( (GL_BITMAP == pixel_data_type) ?
                                                                    ((GL_COLOR_INDEX   == pixel_format   ) &&
                                                                     (GL_UNSIGNED_BYTE == pixel_data_type) ) : true);
                                                      int_type const mipmap_level = 0;
                                                      int_type const border_size  = 0;
                                                      assert_no_errors ne( *this);
                                                      ::glTexImage1D
                                                         (  GL_TEXTURE_1D, mipmap_level
                                                          , internal_format, pixel_count, border_size
                                                          , pixel_format, pixel_data_type, p_pixel_data);
                                                    }
                   template< typename T >
    void         set_1D_texture_bits__no_border
                  (  enum_type  internal_format
                   , size_type  pixel_count
                   , enum_type  pixel_format
                   , T const *  p_pixel_data
                  )                                 { set_1D_texture_bits__no_border(
                                                        internal_format, pixel_count, pixel_format,
                                                        type_traits< T >::data_type, p_pixel_data);
                                                    }
                   template< typename T >
    void         set_1D_texture_bits__no_border__luminance
                  (  size_type  pix_count
                   , T const *  p_pix_data
                  )                                 { enum_type const pix_format = GL_LUMINANCE;
                                                      set_1D_texture_bits__no_border( pix_format, pix_count, pix_format, p_pix_data);
                                                    }
                   template< typename T >
    void         set_1D_texture_bits__no_border__luminance_alpha
                  (  size_type  pix_count
                   , T const *  p_pix_data
                  )                                 { enum_type const pix_format = GL_LUMINANCE_ALPHA;
                                                      set_1D_texture_bits__no_border( pix_format, pix_count, pix_format, p_pix_data);
                                                    }
                   template< typename T >
    void         set_1D_texture_bits__no_border__rgb
                  (  size_type  pix_count
                   , T const *  p_pix_data
                  )                                 { enum_type const pix_format = GL_RGB;
                                                      set_1D_texture_bits__no_border( pix_format, pix_count, pix_format, p_pix_data);
                                                    }
                   template< typename T >
    void         set_1D_texture_bits__no_border__rgba
                  (  size_type  pix_count
                   , T const *  p_pix_data
                  )                                 { enum_type const pix_format = GL_RGBA;
                                                      set_1D_texture_bits__no_border( pix_format, pix_count, pix_format, p_pix_data);
                                                    }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Hint
  // ---------------------------------------------------------------------------------------
  public:
    void         set_hint( enum_type trg, enum_type mode)
                                             { d_assert( is_hint_trg( trg));
                                               d_assert( is_hint_mode( mode));
                                               assert_no_errors ne( *this);
                                               ::glHint( trg, mode);
                                             }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Point size
  // ---------------------------------------------------------------------------------------
  //   GL_POINT_BIT (server attribs)
  //   GL_POINTS (vertex entry)
  //   GL_POINT_SMOOTH (enable)
  //   GL_POLYGON_OFFSET_POINT
  //   GL_POINT_SIZE (see below)
  //   GL_POINT_SIZE_RANGE
  //   GL_POINT_SIZE_GRANULARITY
  //   GL_POINT_SMOOTH_HINT
  //   MeshMode1 and MeshMode2
  //   GL_POINT - polygon mode
  public:
    float_type   get_point_size( )           { return get_float( GL_POINT_SIZE); }
    void         set_point_size( float_type point_size)
                                             { d_assert( point_size >= 0);
                                               assert_no_errors ne( *this);
                                               ::glPointSize( point_size);
                                             }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Shade Model
  // ---------------------------------------------------------------------------------------
  public:
    void         set_shade_model( enum_type model)
                                             { d_assert( (GL_FLAT == model) || (GL_SMOOTH == model));
                                               assert_no_errors ne( *this);
                                               ::glShadeModel( model);
                                             }
    void         set_shade_model_flat( )     { set_shade_model( GL_FLAT  ); }
    void         set_shade_model_smooth( )   { set_shade_model( GL_SMOOTH); }

    enum_type    get_shade_model( )          { return get_enum( GL_SHADE_MODEL); }
    bool_type    is_shade_model_flat( )      { return get_shade_model( ) == GL_FLAT; }
    bool_type    is_shade_model_smooth( )    { return get_shade_model( ) == GL_SMOOTH; }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Translucent and transparent
  // ---------------------------------------------------------------------------------------
  public:
    void         enable_blending( )          { enable( GL_BLEND); }
    void         disable_blending( )         { disable( GL_BLEND); }
    bool_type    is_blending_enabled( )      { return is_enabled( GL_BLEND); }

    void         set_blending_factors( enum_type src_factor, enum_type dst_factor)
                                             { d_assert( is_blending_factor__src( src_factor));
                                               d_assert( is_blending_factor__dst( dst_factor));
                                               assert_no_errors ne( *this);
                                               ::glBlendFunc( src_factor, dst_factor);
                                             }
    enum_type    get_blending_src_factor( )  { return get_enum( GL_BLEND_SRC); }
    enum_type    get_blending_dst_factor( )  { return get_enum( GL_BLEND_DST); }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Polygon Mode
  // ---------------------------------------------------------------------------------------
  public:
    void         set_polygon_mode( enum_type face, enum_type mode)
                                             { d_assert( (GL_FRONT == face) || (GL_BACK == face) || (GL_FRONT_AND_BACK == face));
                                               d_assert( (GL_POINT == mode) || (GL_LINE == mode) || (GL_FILL == mode));
                                               assert_no_errors ne( *this);
                                               ::glPolygonMode( face, mode);
                                             }

    void         get_polygon_modes( enum_type & front_mode, enum_type & back_mode)
                                             { get_2_enums( GL_POLYGON_MODE, front_mode, back_mode); }

    enum_type    get_polygon_mode( enum_type face)
                                             { d_assert( (GL_FRONT == face) || (GL_BACK == face));
                                               enum_type front_mode = 0;
                                               enum_type back_mode  = 0;
                                               get_polygon_modes( front_mode, back_mode);
                                               return (GL_BACK == face) ? back_mode : front_mode;
                                             }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Front and Cull Faces
  // ---------------------------------------------------------------------------------------
  public:
    void         set_front_face( enum_type wrap_direction)
                                             { d_assert( (GL_CW == wrap_direction) || (GL_CCW == wrap_direction));
                                               assert_no_errors ne( *this);
                                               ::glFrontFace( wrap_direction);
                                             }
    void         set_front_face_cw( )        { set_front_face( GL_CW); /* clockwise */ }
    void         set_front_face_ccw( )       { set_front_face( GL_CCW); /* counter-clockwise */ }

    enum_type    get_front_face( )           { return get_enum( GL_FRONT_FACE); }

  // ---------------------------------------------------------------------------------------
  public:
    void         set_cull_face( enum_type face)
                                             { d_assert( (GL_FRONT == face) || (GL_BACK == face) || (GL_FRONT_AND_BACK == face));
                                               assert_no_errors ne( *this);
                                               ::glCullFace( face);
                                             }
    enum_type    get_cull_face( )            { return get_enum( GL_CULL_FACE_MODE); }

    void         enable_face_culling( )      { enable( GL_CULL_FACE); }
    void         disable_face_culling( )     { disable( GL_CULL_FACE); }
    bool_type    is_face_culling_enabled( )  { return is_enabled( GL_CULL_FACE); }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Depth and Color Buffers
  // ---------------------------------------------------------------------------------------
  public:
    void         freeze_depth_buffer( )      { assert_no_errors ne( *this);
                                               ::glDepthMask( false);
                                             }
    void         unfreeze_depth_buffer( )    { assert_no_errors ne( *this);
                                               ::glDepthMask( true);
                                             }
    bool_type    is_depth_buffer_frozen( )   { return get_bool( GL_DEPTH_WRITEMASK); }

    void         set_depth_test_fn( enum_type fn)
                                             { d_assert(
                                                 (GL_NEVER   == fn) || (GL_LESS     == fn) || (GL_EQUAL  == fn) || (GL_LEQUAL == fn) ||
                                                 (GL_GREATER == fn) || (GL_NOTEQUAL == fn) || (GL_GEQUAL == fn) || (GL_ALWAYS == fn) );
                                               assert_no_errors ne( *this);
                                               ::glDepthFunc( fn);
                                             }
    enum_type    get_depth_test_fn( )        { return get_enum( GL_DEPTH_FUNC); }

    bool_type    is_depth_test_enabled( )    { return is_enabled( GL_DEPTH_TEST); }
    void         enable_depth_test( )        { enable( GL_DEPTH_TEST); }
    void         disable_depth_test( )       { disable( GL_DEPTH_TEST); }

    void         set_depth_buffer_clear_value( double_type depth) /* clampd_type */
                                             { d_assert( (0 <= depth) && (depth <= 1)); /* clamped */
                                               assert_no_errors ne( *this);
                                               ::glClearDepth( depth);
                                             }
    double_type  get_depth_buffer_clear_value( )
                                             { return get_double( GL_DEPTH_CLEAR_VALUE); }

    void         set_color_buffer_clear_color( rgba_type< float_type > const & color) /* clampf_type */
                                             { d_assert( color.is_clamped( false)); /* clamped, not negative */
                                               { assert_no_errors ne( *this);
                                                 ::glClearColor( color.get_r( ), color.get_g( ), color.get_b( ), color.get_a( ));
                                               }
                                               d_assert( get_color_buffer_clear_color( ) == color);
                                             }
    rgba_type< float_type >
                 get_color_buffer_clear_color( )
                                             { rgba_type< float_type > color;
                                               get_4_floats( GL_COLOR_CLEAR_VALUE,
                                                 color.ref_r( ), color.ref_g( ), color.ref_b( ), color.ref_a( ));
                                               d_assert( color.is_clamped( false)); /* clamped, not negative */
                                               return color;
                                             }

    void         clear_buffers( bitfield_type which_buffers)
                                             { d_assert( 0 == (
                                                 ( ~ (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                                                      GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)
                                                 ) & which_buffers));
                                               assert_no_errors ne( *this);
                                               ::glClear( which_buffers);
                                             }
    void         clear_depth_buffer( )       { clear_buffers( GL_DEPTH_BUFFER_BIT); }
    void         clear_color_buffer( )       { clear_buffers( GL_COLOR_BUFFER_BIT); }
    void         clear_color_and_depth_buffer( )
                                             { clear_buffers( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Clip planes
  // ---------------------------------------------------------------------------------------
  private:
    void         init_clip_planes( )         { max_clip_plane_count_ = get_max_clip_plane_count_from_gl( );
                                               d_assert( max_clip_plane_count_ >= get_min_max_clip_plane_count( ));
                                             }
    void         uninit_clip_planes( )       { }
                   /* member var */ int_type max_clip_plane_count_ ;

  // ---------------------------------------------------------------------------------------
  public:
    int_type     get_max_clip_plane_count( ) { return max_clip_plane_count_; }
    int_type     get_max_clip_plane_count_from_gl( )
                                             { return get_int( GL_MAX_CLIP_PLANES); }
    int_type     get_min_max_clip_plane_count( )
                                             { return 6; }

    // Should we check (index < get_min_max_clip_plane_count( )) when is_checking_against_min_max_values( ) is true?
    void         assert_valid_clip_plane_index( int_type index)
                                             { maybe_used_only_for_debug( index);
                                               d_assert( index >= 0);
                                               d_assert( index < get_max_clip_plane_count( ));
                                             }
    void         assert_portable_valid_clip_plane_index( int_type index)
                                             { maybe_used_only_for_debug( index);
                                               d_assert( index >= 0);
                                               d_assert( index < get_min_max_clip_plane_count( ));
                                             }

  // ---------------------------------------------------------------------------------------
    bool_type    is_clip_plane_enabled( int_type index)
                                             { assert_valid_clip_plane_index( index);
                                               return is_enabled( GL_CLIP_PLANE0 + index);
                                             }

    void         enable_clip_plane( int_type index)
                                             { assert_valid_clip_plane_index( index);
                                               enable( GL_CLIP_PLANE0 + index);
                                             }
    void         disable_clip_plane( int_type index)
                                             { assert_valid_clip_plane_index( index);
                                               disable( GL_CLIP_PLANE0 + index);
                                             }

    void         set_clip_plane_equation( int_type index, double_type x, double_type y, double_type z, double_type w)
                                             { assert_valid_clip_plane_index( index);
                                               double_type equation[ 4 ];
                                               equation[ 0 ] = x;
                                               equation[ 1 ] = y;
                                               equation[ 2 ] = z;
                                               equation[ 3 ] = w;
                                               assert_no_errors ne( *this);
                                               ::glClipPlane( GL_CLIP_PLANE0 + index, equation);
                                             }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Light Model
  //   GL_LIGHT_MODEL_COLOR_CONTROL is version 1.2 and not covered here.
  // ---------------------------------------------------------------------------------------
  public:
    // Why can't we use is_enabled(..) and enabled(..) here?
    bool_type    is_two_sided_lighting( )    { return get_bool( GL_LIGHT_MODEL_TWO_SIDE); }
    void         enable_two_sided_lighting( )
                                             { set_two_sided_lighting( true); }
    void         disable_two_sided_lighting( )
                                             { set_two_sided_lighting( false); }
    void         set_two_sided_lighting( bool_type is_two_sided)
                                             { assert_no_errors ne( *this);
                                               ::glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, is_two_sided ? 1 : 0);
                                             }

  // ---------------------------------------------------------------------------------------
    // Why can't we use is_enabled(..) and enabled(..) here?
    bool_type    is_local_viewer_light_calculations( )
                                             { return get_int( GL_LIGHT_MODEL_LOCAL_VIEWER); }
    void         enable_local_viewer_light_calculations( )
                                             { set_local_viewer_light_calculations( true); }
    void         disable_local_viewer_light_calculations( )
                                             { set_local_viewer_light_calculations( false); }
    void         set_local_viewer_light_calculations( bool_type is_local_viewer)
                                             { assert_no_errors ne( *this);
                                               ::glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, is_local_viewer ? 1 : 0);
                                             }

  // ---------------------------------------------------------------------------------------
    template< typename CC_TYPE > rgba_type< CC_TYPE >
                 get_global_ambient_color( ) ;

  // ---------------------------------------------------------------------------------------
    void         set_global_ambient_color( rgba_type< int_type > const & color)
                                             ;
    void         set_global_ambient_color( rgba_type< float_type > const & color)
                                             ;
                   template< typename CC_TYPE >
    void         set_global_ambient_color( rgb_type< CC_TYPE > const & color)
                                             { rgba_type< CC_TYPE > rgba_color = color;
                                               set_global_ambient_color( rgba_color);
                                             }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Lighting
  //
  //   There is also a ::glGetLight*(..) set of functions that are not wrapped here.
  // ---------------------------------------------------------------------------------------
  private:
    void         init_lighting( )            { max_light_count_ = get_max_light_count_from_gl( );
                                               d_assert( max_light_count_ >= get_min_max_light_count( ));
                                             }
    void         uninit_lighting( )          { }
                   /* member var */ int_type max_light_count_ ;

  // ---------------------------------------------------------------------------------------
  public:
    int_type     get_max_light_count( )      { return max_light_count_; }
    int_type     get_max_light_count_from_gl( )
                                             { return get_int( GL_MAX_LIGHTS); }
    static int_type
                 get_min_max_light_count( )  { return 8; }

    // Should we check (index < get_min_max_light_count( )) when is_checking_against_min_max_values( ) is true?
    bool         is_light_index( int_type index)
                                             { return (0 <= index) && (index < get_max_light_count( )); }
    static bool  is_portable_light_index( int_type index)
                                             { return (0 <= index) && (index < get_min_max_light_count( )); }

    static bool  is_light_facet( enum_type facet)
                                             { return
                                                 (GL_AMBIENT  == facet) ||
                                                 (GL_DIFFUSE  == facet) ||
                                                 (GL_SPECULAR == facet) ;
                                             }

  // ---------------------------------------------------------------------------------------
  public:
    bool_type    is_overall_lighting_enabled( )
                                             { return is_enabled( GL_LIGHTING); }
    void         enable_overall_lighting( )  { enable( GL_LIGHTING); }
    void         disable_overall_lighting( ) { disable( GL_LIGHTING); }

  // ---------------------------------------------------------------------------------------
    bool_type    is_light_on( int_type index)
                                             { d_assert( is_light_index( index));
                                               return is_enabled( GL_LIGHT0 + index);
                                             }

    void         set_light_on( int_type index)
                                             { d_assert( is_light_index( index));
                                               enable( GL_LIGHT0 + index);
                                             }
    void         set_light_off( int_type index)
                                             { d_assert( is_light_index( index));
                                               disable( GL_LIGHT0 + index);
                                             }

  // ---------------------------------------------------------------------------------------
    void         set_light_color( int_type index, enum_type facet, rgba_type< float_type > const & color)
                                             { d_assert( is_light_index( index));
                                               d_assert( is_light_facet( facet));
                                               assert_no_errors ne( *this);
                                               ::glLightfv( index + GL_LIGHT0, facet, color.get_as_p4( ));
                                             }

    void         set_light_color( int_type index, enum_type facet, rgba_type< int_type > const & color)
                                             { d_assert( is_light_index( index));
                                               d_assert( is_light_facet( facet));
                                               assert_no_errors ne( *this);
                                               ::glLightiv( index + GL_LIGHT0, facet, color.get_as_p4( ));
                                             }

                   template< typename CC_TYPE >
    void         set_light_color( int_type index, enum_type facet, rgb_type< CC_TYPE > const & color)
                                             { rgba_type< CC_TYPE > rgba_color = color;
                                               set_light_color( index, facet, rgba_color);
                                             }

                   template< typename COLOR_TYPE >
    void         set_light_ambient_color( int_type index, COLOR_TYPE const & color)
                                             { set_light_color( index, GL_AMBIENT, color); }

                   template< typename COLOR_TYPE >
    void         set_light_diffuse_color( int_type index, COLOR_TYPE const & color)
                                             { set_light_color( index, GL_DIFFUSE, color); }

                   template< typename COLOR_TYPE >
    void         set_light_specular_color( int_type index, COLOR_TYPE const & color)
                                             { set_light_color( index, GL_SPECULAR, color); }

  // ---------------------------------------------------------------------------------------
    void         set_light_position( int_type index, int_type x, int_type y, int_type z, int_type w = 1)
                                             { d_assert( is_light_index( index));
                                               d_assert( x || y || z);
                                               int_type xyzw[ 4 ];
                                               xyzw[ 0 ] = x;
                                               xyzw[ 1 ] = y;
                                               xyzw[ 2 ] = z;
                                               xyzw[ 3 ] = w;
                                               assert_no_errors ne( *this);
                                               ::glLightiv( index + GL_LIGHT0, GL_POSITION, xyzw);
                                             }
    void         set_light_position( int_type index, float_type x, float_type y, float_type z, float_type w = 1.0f)
                                             { d_assert( is_light_index( index));
                                               d_assert( x || y || z);
                                               float_type xyzw[ 4 ];
                                               xyzw[ 0 ] = x;
                                               xyzw[ 1 ] = y;
                                               xyzw[ 2 ] = z;
                                               xyzw[ 3 ] = w;
                                               assert_no_errors ne( *this);
                                               ::glLightfv( index + GL_LIGHT0, GL_POSITION, xyzw);
                                             }

    void         set_light_direction( int_type index, int_type x, int_type y, int_type z)
                                             { set_light_position( index, x, y, z, 0); }
    void         set_light_direction( int_type index, float_type x, float_type y, float_type z)
                                             { set_light_position( index, x, y, z, 0.0f); }

  // ---------------------------------------------------------------------------------------
    void         set_spotlight_direction( int_type index, int_type x, int_type y, int_type z)
                                             { d_assert( is_light_index( index));
                                               d_assert( x || y || z);
                                               int_type xyz[ 3 ];
                                               xyz[ 0 ] = x;
                                               xyz[ 1 ] = y;
                                               xyz[ 2 ] = z;
                                               assert_no_errors ne( *this);
                                               ::glLightiv( index + GL_LIGHT0, GL_SPOT_DIRECTION, xyz);
                                             }
    void         set_spotlight_direction( int_type index, float_type x, float_type y, float_type z)
                                             { d_assert( is_light_index( index));
                                               d_assert( x || y || z);
                                               float_type xyz[ 3 ];
                                               xyz[ 0 ] = x;
                                               xyz[ 1 ] = y;
                                               xyz[ 2 ] = z;
                                               assert_no_errors ne( *this);
                                               ::glLightfv( index + GL_LIGHT0, GL_SPOT_DIRECTION, xyz);
                                             }

  // ---------------------------------------------------------------------------------------
    void         set_spotlight_no_focus( int_type index)
                                             { set_spotlight_focus_128( index, 0); }
    void         set_spotlight_focus_01( int_type index, float_type focus_01)
                                             { set_spotlight_focus_128( index, focus_01 * 128.0f); }
    void         set_spotlight_focus_128( int_type index, int_type focus)
                                             { d_assert( is_light_index( index));
                                               d_assert( focus >= 0);
                                               d_assert( focus <= 128);
                                               assert_no_errors ne( *this);
                                               ::glLighti( index + GL_LIGHT0, GL_SPOT_EXPONENT, focus);
                                             }
    void         set_spotlight_focus_128( int_type index, float_type focus)
                                             { d_assert( is_light_index( index));
                                               d_assert( focus >= 0);
                                               d_assert( focus <= 128);
                                               assert_no_errors ne( *this);
                                               ::glLightf( index + GL_LIGHT0, GL_SPOT_EXPONENT, focus);
                                             }

  // ---------------------------------------------------------------------------------------
    void         set_spotlight_cutoff_angle( int_type index, int_type angle)
                                             { d_assert( is_light_index( index));
                                               d_assert( angle >= 0);
                                               d_assert( (angle <= 90) || (angle == 180));
                                               assert_no_errors ne( *this);
                                               ::glLighti( index + GL_LIGHT0, GL_SPOT_CUTOFF, angle);
                                             }
    void         set_spotlight_cutoff_angle( int_type index, float_type angle)
                                             { d_assert( is_light_index( index));
                                               d_assert( angle >= 0);
                                               d_assert( (angle <= 90) || (angle == 180));
                                               assert_no_errors ne( *this);
                                               ::glLightf( index + GL_LIGHT0, GL_SPOT_CUTOFF, angle);
                                             }
    void         reset_spotlight_cutoff_angle( int_type index)
                                             { set_spotlight_cutoff_angle( index, 180); }

  // ---------------------------------------------------------------------------------------
    // It's probably a mistake to specify zero const_attenuation, but it's allowed.
    void         set_light_attenuation
                  (  int_type index
                   , int_type const_attenuation
                   , int_type linear_attenuation
                   , int_type quadratic_attenuation
                  )                          { d_assert( is_light_index( index));
                                               int_type const index_plus = index + GL_LIGHT0;

                                               d_assert( 0 <= const_attenuation);
                                               d_assert( 0 <= linear_attenuation);
                                               d_assert( 0 <= quadratic_attenuation);

                                               { assert_no_errors ne( *this);
                                                 ::glLighti( index_plus, GL_CONSTANT_ATTENUATION , const_attenuation    );
                                               }
                                               { assert_no_errors ne( *this);
                                                 ::glLighti( index_plus, GL_LINEAR_ATTENUATION   , linear_attenuation   );
                                               }
                                               { assert_no_errors ne( *this);
                                                 ::glLighti( index_plus, GL_QUADRATIC_ATTENUATION, quadratic_attenuation);
                                             } }
    void         set_light_attenuation
                  (  int_type   index
                   , float_type const_attenuation
                   , float_type linear_attenuation
                   , float_type quadratic_attenuation
                  )                          { d_assert( is_light_index( index));
                                               int_type const index_plus = index + GL_LIGHT0;

                                               d_assert( 0 <= const_attenuation);
                                               d_assert( 0 <= linear_attenuation);
                                               d_assert( 0 <= quadratic_attenuation);

                                               { assert_no_errors ne( *this);
                                                 ::glLightf( index_plus, GL_CONSTANT_ATTENUATION , const_attenuation    );
                                               }
                                               { assert_no_errors ne( *this);
                                                 ::glLightf( index_plus, GL_LINEAR_ATTENUATION   , linear_attenuation   );
                                               }
                                               { assert_no_errors ne( *this);
                                                 ::glLightf( index_plus, GL_QUADRATIC_ATTENUATION, quadratic_attenuation);
                                             } }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Lines
  // ---------------------------------------------------------------------------------------
  public:
    bool_type    is_line_stipple_enabled( )  { return is_enabled( GL_LINE_STIPPLE); }
    void         enable_line_stipple( )      { enable( GL_LINE_STIPPLE); }
    void         disable_line_stipple( )     { disable( GL_LINE_STIPPLE); }

    int_type     get_line_stipple_repeat_factor( )
                                             { int_type const repeat_factor = get_int( GL_LINE_STIPPLE_REPEAT);
                                               d_assert( (1 <= repeat_factor) && (repeat_factor <= 256));
                                               return repeat_factor;
                                             }
    ushort_type  get_line_stipple_pattern( ) { int_type const pattern = get_int( GL_LINE_STIPPLE_PATTERN);
                                               d_assert( (0 <= pattern) && (pattern <= 0xffff));
                                               return static_cast< ushort_type >( pattern);
                                             }
    void         set_line_stipple( int_type repeat_factor, ushort_type pattern)
                                             { d_assert( (1 <= repeat_factor) && (repeat_factor <= 256));
                                               assert_no_errors ne( *this);
                                               ::glLineStipple( repeat_factor, pattern);
                                             }

    // There's also:
    //   GL_LINE_SMOOTH - enable
    //
    //   GL_LINE_WIDTH_GRANULARITY  - defunct after 1.2
    //   GL_LINE_WIDTH_RANGE        - defunct after 1.2
    //   GL_ALIASED_LINE_WIDTH_RANGE
    //   GL_SMOOTH_LINE_WIDTH_RANGE
    //   GL_SMOOTH_LINE_WIDTH_GRANULARITY
    float_type   get_line_width( )           { return get_float( GL_LINE_WIDTH); }
    void         set_line_width( float_type width)
                                             { d_assert( 0 <= width);
                                               assert_no_errors ne( *this);
                                               ::glLineWidth( width);
                                             }

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Shaders and shader programs (GLSL) (GL version 2.0)
  // ---------------------------------------------------------------------------------------
  public:
    static
    bool_type    are_shaders_supported( )    ;
    bool_type    are_shaders_explicitly_supported( )
                                             ;
    uint_type    get_active_shader_program( )
                                             ;

    void         use_shader_program( uint_type program_id)
                                             ; /* see get_active_shader_program( ) */

    uint_type    create_shader_program( uint_type vertex_shader_id, uint_type fragment_shader_id)
                                             ;
    uint_type    create_shader( enum_type shader_type, char_type const * p_src_string)
                                             ;

    bool_type    is_shader_program( uint_type program_id)
                                             ;
    bool_type    is_shader( uint_type shader_id)
                                             ;

    uint_type    create_shader_program( )    ;
    void         delete_shader_program( uint_type program_id)
                                             ; // allow program_id == 0

    uint_type    create_shader( enum_type shader_type)
                                             ;
    void         delete_shader( uint_type shader_id)
                                             ; // allow shader_id == 0

    void         attach_shader( uint_type program_id, uint_type shader_id)
                                             ;
    void         detach_shader( uint_type program_id, uint_type shader_id)
                                             ;
    void         detach_all_shaders( uint_type program_id)
                                             ;

    void         link_shader_program( uint_type program_id)
                                             ;
    void         validate_shader_program( uint_type program_id)
                                             ;

    void         set_shader_src( uint_type shader_id, char_type const * p_src_string)
                                             ;
    void         compile_shader( uint_type shader_id)
                                             ;

    int_type     get_shader_program_status_value( uint_type program_id, enum_type name, int_type init_value)
                                             ;
    int_type     get_shader_status_value( uint_type shader_id, enum_type name, int_type init_value)
                                             ;

    bool_type    did_shader_program_op_fail_with_errors( enum_type op_name, uint_type program_id)
                                             ;
    bool_type    did_shader_op_fail_with_errors( enum_type op_name, uint_type shader_id)
                                             ;

    int_type     get_uniform_location( uint_type program_id, char_type const * p_uniform_name)
                                             ;
    void         get_uniform_values( uint_type program_id, int_type uniform_id, float_type * p_values)
                                             ;
    void         get_uniform_values( uint_type program_id, int_type uniform_id, int_type * p_values)
                                             ;
    void         set_uniform( int_type uniform_id, rgb_type< float_type > const & color)
                                             { set_uniform( uniform_id, color.get_r( ), color.get_g( ), color.get_b( )); }
    void         set_uniform( int_type uniform_id, rgba_type< float_type > const & color)
                                             { set_uniform( uniform_id, color.get_r( ), color.get_g( ), color.get_b( ), color.get_a( )); }
    void         set_uniform( int_type uniform_id, float_type)
                                             ;
    void         set_uniform( int_type uniform_id, float_type, float_type)
                                             ;
    void         set_uniform( int_type uniform_id, float_type, float_type, float_type)
                                             ;
    void         set_uniform( int_type uniform_id, float_type, float_type, float_type, float_type)
                                             ;
    void         set_uniform( int_type uniform_id, int_type)
                                             ;
    void         set_uniform( int_type uniform_id, int_type, int_type)
                                             ;
    void         set_uniform( int_type uniform_id, int_type, int_type, int_type)
                                             ;
    void         set_uniform( int_type uniform_id, int_type, int_type, int_type, int_type)
                                             ;

  // ---------------------------------------------------------------------------------------
  // ---------------------------------------------------------------------------------------
  // Shaders and shader programs (GLSL) (ARB extension)
  // ---------------------------------------------------------------------------------------
  public:
    static
    bool_type    are_shaders_supported__ARB( )
                                             ;
    bool_type    are_shaders_explicitly_supported__ARB( )
                                             ;
    GLhandleARB  get_active_shader_program__ARB( )
                                             ;

    void         use_shader_program__ARB( GLhandleARB program_id)
                                             ;

    GLhandleARB  create_shader_program__ARB( GLhandleARB vertex_shader_id, GLhandleARB fragment_shader_id)
                                             ;
    GLhandleARB  create_shader__ARB( enum_type shader_type, GLcharARB const * p_src_string)
                                             ;

    bool_type    is_shader_program__ARB( GLhandleARB program_id)
                                             ;

    GLhandleARB  create_shader_program__ARB( )
                                             ;
    GLhandleARB  create_shader__ARB( enum_type shader_type)
                                             ;
    void         delete_shader_object__ARB( GLhandleARB object_id)
                                             ;

    void         attach_shader__ARB( GLhandleARB program_id, GLhandleARB shader_id)
                                             ;
    void         detach_shader__ARB( GLhandleARB parent_id, GLhandleARB shader_id)
                                             ;
    void         detach_all_shaders__ARB( GLhandleARB parent_id)
                                             ;

    void         link_shader_program__ARB( GLhandleARB program_id)
                                             ;
    void         validate_shader_program__ARB( GLhandleARB program_id)
                                             ;
    void         set_shader_src__ARB( GLhandleARB shader_id, GLcharARB const * p_src_string)
                                             ;
    void         compile_shader__ARB( GLhandleARB shader_id)
                                             ;

    int_type     get_object_status_value__ARB( GLhandleARB object_id, enum_type name, int_type init_value)
                                             ;
    bool_type    did_object_op_fail_with_errors__ARB( enum_type op_name, GLhandleARB object_id)
                                             ;

    int_type     get_uniform_location__ARB( uint_type program_id, char_type const * p_uniform_name)
                                             ;
    void         get_uniform_values__ARB( uint_type program_id, int_type uniform_id, float_type * p_values)
                                             ;
    void         get_uniform_values__ARB( uint_type program_id, int_type uniform_id, int_type * p_values)
                                             ;
    void         set_uniform__ARB( int_type uniform_id, rgb_type< float_type > const & color)
                                             { set_uniform__ARB( uniform_id, color.get_r( ), color.get_g( ), color.get_b( )); }
    void         set_uniform__ARB( int_type uniform_id, rgba_type< float_type > const & color)
                                             { set_uniform__ARB( uniform_id, color.get_r( ), color.get_g( ), color.get_b( ), color.get_a( )); }
    void         set_uniform__ARB( int_type uniform_id, float_type)
                                             ;
    void         set_uniform__ARB( int_type uniform_id, float_type, float_type)
                                             ;
    void         set_uniform__ARB( int_type uniform_id, float_type, float_type, float_type)
                                             ;
    void         set_uniform__ARB( int_type uniform_id, float_type, float_type, float_type, float_type)
                                             ;
    void         set_uniform__ARB( int_type uniform_id, int_type)
                                             ;
    void         set_uniform__ARB( int_type uniform_id, int_type, int_type)
                                             ;
    void         set_uniform__ARB( int_type uniform_id, int_type, int_type, int_type)
                                             ;
    void         set_uniform__ARB( int_type uniform_id, int_type, int_type, int_type, int_type)
                                             ;

// _______________________________________________________________________________________________
//
}; /* end class env_type */
// _______________________________________________________________________________________________

// _______________________________________________________________________________________________
//
// Template specializations
// _______________________________________________________________________________________________

// get_material_color(..) ----------------------------------------------------------------------

  /* specialization for float_type */
  template< >
  inline
  rgba_type< float_type >
  env_type::
get_material_color< float_type >( enum_type face, enum_type facet)
  {
    d_assert( is_material_face__get( face));
    d_assert( is_color_facet__get( facet));
    rgba_type< float_type > color;
    assert_no_errors wrapper( *this);
    ::glGetMaterialfv( face, facet, color.ref_as_p4( ));
    // Should we check to see if the color is clamped? Or if it is negative?
    //   d_assert( color.is_clamped( allow_negative_color_components));
    return color;
  }

  /* specialization for int_type */
  template< >
  inline
  rgba_type< int_type >
  env_type::
get_material_color< int_type >( enum_type face, enum_type facet)
  {
    d_assert( is_material_face__get( face));
    d_assert( is_color_facet__get( facet));
    rgba_type< int_type > color;
    assert_no_errors wrapper( *this);
    ::glGetMaterialiv( face, facet, color.ref_as_p4( ));
    // Should we check to see if the color is clamped? Or if it is negative?
    //   d_assert( color.is_clamped( allow_negative_color_components));
    return color;
  }

// setup_material_color(..) --------------------------------------------------------------------

  /* specialization for float_type */
  template< >
  inline
  void
  env_type::
setup_material_color< float_type >( enum_type face, enum_type facet, rgba_type< float_type > const & color)
  {
    d_assert( is_material_face__set( face));
    d_assert( is_color_facet__set( facet));
    // Should we check color.is_clamped(..)?

    // Should we d_assert(
    //              (! is_material_color_tracking_enabled( )) ||
    //              (! is_overall_lighting_enabled( )))
    //   The doc for glColorMaterial(..) says to "call glColorMaterial before enabling GL_COLOR_MATERIAL".
    //   This probably means only for the faces/facets that will be tracked, although the doc also says
    //   "glColorMaterial makes it possible to change a subset of material parameters for each vertex using only
    //   the glColor command, without calling glMaterial. If ONLY SUCH A SUBSET of parameters is to be specified for each
    //   vertex, calling glColorMaterial is preferable to calling glMaterial." (emphasis mine)
    //   This statement implies you should not change ANY face/facet color thru glMaterial..(..) while color-tracking
    //   is on, even those that are not tracked.
    //
    //   The safest course is to never call setup_material_color(..) or drop_material_color(..) while any tracking is
    //   enabled. And maybe not even change material shininess?
    //
    //   However we can always call drop_color(..) since it affects lines and points. Although if we not tracking,
    //   and polygon mode is GL_FILL, and we are entering polygons, maybe then we should warn if we are dropping colors?
    //
    //   The doc also says "If the GL version is 1.1 or greater, and GL_COLOR_MATERIAL is enabled, evaluated color values
    //   affect the results of the lighting equation as if the current color were being modified, but no change is made
    //   to the tracking lighting parameter of the current color."

    assert_no_errors wrapper( *this);
    ::glMaterialfv( face, facet, color.get_as_p4( ));
    assert__material_color_is_set( face, facet, color);
  }

  /* specialization for int_type */
  template< >
  inline
  void
  env_type::
setup_material_color< int_type >( enum_type face, enum_type facet, rgba_type< int_type > const & color)
  {
    d_assert( is_material_face__set( face));
    d_assert( is_color_facet__set( facet));
    // Should we check color.is_clamped(..)?
    assert_no_errors wrapper( *this);
    ::glMaterialiv( face, facet, color.get_as_p4( ));
    assert__material_color_is_set( face, facet, color);
  }

// drop_material_color(..) ---------------------------------------------------------------------

  /* specialization for float_type */
  template< >
  inline
  void
  env_type::
drop_material_color< float_type >( enum_type face, enum_type facet, rgba_type< float_type > const & color)
  {
    d_assert( is_material_face__set( face));
    d_assert( is_color_facet__set( facet));
    // Should we check color.is_clamped(..)?
    assert_during_vertex_entry wrapper( *this);
    ::glMaterialfv( face, facet, color.get_as_p4( ));
    // You cannot call get_material_color(..) during vertex-entry mode.
  }

  /* specialization for int_type */
  template< >
  inline
  void
  env_type::
drop_material_color< int_type >( enum_type face, enum_type facet, rgba_type< int_type > const & color)
  {
    d_assert( is_material_face__set( face));
    d_assert( is_color_facet__set( facet));
    // Should we check color.is_clamped(..)?
    assert_during_vertex_entry wrapper( *this);
    ::glMaterialiv( face, facet, color.get_as_p4( ));
    // You cannot call get_material_color(..) during vertex-entry mode.
  }

// get_material_shininess_128(..) --------------------------------------------------------------

  // Specialization implementations of the above.
  template< >
  inline
  int_type
  env_type::
get_material_shininess_128< int_type >( enum_type face)
  {
    d_assert( (GL_FRONT == face) || (GL_BACK == face));
    int_type vals[ 2 ] = { -1, -1 };
    { assert_no_errors ne( *this);
      ::glGetMaterialiv( face, GL_SHININESS, vals);
    }
    d_assert( -1 == vals[ 1 ]);
    d_assert( (0 <= vals[ 0 ]) && (vals[ 0 ] <= 128));
    return vals[ 0 ];
  }

  template< > 
  inline
  float_type
  env_type::
get_material_shininess_128< float_type >( enum_type face)
  {
    d_assert( (GL_FRONT == face) || (GL_BACK == face));
    float_type vals[ 2 ] = { -1, -1 };
    { assert_no_errors ne( *this);
      ::glGetMaterialfv( face, GL_SHININESS, vals);
    }
    d_assert( -1 == vals[ 1 ]);
    d_assert( (0 <= vals[ 0 ]) && (vals[ 0 ] <= 128));
    return vals[ 0 ];
  }

// drop_material_shininess_128(..) -------------------------------------------------------------

  template< >
  inline
  void
  env_type::
drop_material_shininess_128< int_type >( enum_type face, int_type val)
  {
    d_assert( (GL_FRONT == face) || (GL_BACK == face) || (GL_FRONT_AND_BACK == face));
    d_assert( (0 <= val) && (val <= 128));
    { assert_during_vertex_entry av( *this);
      ::glMateriali( face, GL_SHININESS, val);
    }
  }

  template< >
  inline
  void
  env_type::
drop_material_shininess_128< float_type >( enum_type face, float_type val)
  {
    d_assert( (GL_FRONT == face) || (GL_BACK == face) || (GL_FRONT_AND_BACK == face));
    d_assert( (0 <= val) && (val <= 128));
    { assert_during_vertex_entry av( *this);
      ::glMaterialf( face, GL_SHININESS, val);
    }
  }

// setup_material_shininess_128(..) ------------------------------------------------------------

  template< >
  inline
  void
  env_type::
setup_material_shininess_128< int_type >( enum_type face, int_type val)
  {
    d_assert( (GL_FRONT == face) || (GL_BACK == face) || (GL_FRONT_AND_BACK == face));
    d_assert( (0 <= val) && (val <= 128));
    { assert_no_errors ne( *this);
      ::glMateriali( face, GL_SHININESS, val);
    }
  # ifndef NDEBUG
    if ( (GL_FRONT == face) || (GL_FRONT_AND_BACK == face) ) {
      d_assert( get_material_shininess_128_front< int_type >( ) == val);
    }
    if ( (GL_BACK == face) || (GL_FRONT_AND_BACK == face) ) {
      d_assert( get_material_shininess_128_back< int_type >( ) == val);
    }
  # endif
  }

  template< >
  inline
  void
  env_type::
setup_material_shininess_128< float_type >( enum_type face, float_type val)
  {
    d_assert( (GL_FRONT == face) || (GL_BACK == face) || (GL_FRONT_AND_BACK == face));
    d_assert( (0 <= val) && (val <= 128));
    { assert_no_errors ne( *this);
      ::glMaterialf( face, GL_SHININESS, val);
    }
  # ifndef NDEBUG
    if ( (GL_FRONT == face) || (GL_FRONT_AND_BACK == face) ) {
      d_assert( get_material_shininess_128_front< float_type >( ) == val);
    }
    if ( (GL_BACK == face) || (GL_FRONT_AND_BACK == face) ) {
      d_assert( get_material_shininess_128_back< float_type >( ) == val);
    }
  # endif
  }

// get_global_ambient_color(..) ----------------------------------------------------------------

  template< >
  inline
  rgba_type< int_type >
  env_type::
get_global_ambient_color( )
  {
    rgba_type< int_type > color;
    get_4_ints( GL_LIGHT_MODEL_AMBIENT, color.ref_r( ), color.ref_g( ), color.ref_b( ), color.ref_a( ));
    return color;
  }

  template< >
  inline
  rgba_type< float_type >
  env_type::
get_global_ambient_color( )
  {
    rgba_type< float_type > color;
    get_4_floats( GL_LIGHT_MODEL_AMBIENT, color.ref_r( ), color.ref_g( ), color.ref_b( ), color.ref_a( ));
    return color;
  }

// _______________________________________________________________________________________________
//
// Inline methods (implementations, not specializations)
//
//   These are not in the body of the class because they have to be defined after the above
//   template specializations are available.
// _______________________________________________________________________________________________

  inline
  float_type
  env_type::
get_material_shininess_01( enum_type face)
  {
    return get_material_shininess_128< float_type >( face) / 128.0f;
  }

  inline
  void
  env_type::
drop_material_shininess_01( enum_type face, float_type val)
  {
    drop_material_shininess_128< float_type >( face, val * 128.0f);
  }

  inline
  void
  env_type::
setup_material_shininess_01( enum_type face, float_type val)
  {
    setup_material_shininess_128< float_type >( face, val * 128.0f);
  }

// _______________________________________________________________________________________________

  inline
  void
  env_type::
set_global_ambient_color( rgba_type< int_type > const & color)
  {
    // Should we allow negative rgb values? what do they do?
    { assert_no_errors ne( *this);
      ::glLightModeliv( GL_LIGHT_MODEL_AMBIENT, color.get_as_p4( ));
    }
    d_assert( get_global_ambient_color< int_type >( ) == color);
  }

  inline
  void
  env_type::
set_global_ambient_color( rgba_type< float_type > const & color)
  {
    // should we allow negative rgb values? what do they do?
    // should we allow values outside the range -1..+1? what do they do?
    { assert_no_errors ne( *this);
      ::glLightModelfv( GL_LIGHT_MODEL_AMBIENT, color.get_as_p4( ));
    }
    // The following sometimes fails because the numbers don't match exactly, probably
    // because GL is storing the colors as RGB bytes. So the floats in color are too
    // accurate.
    // We should change this to make sure each color component is within 1% of the set
    // value. Although even that might fail if the underlying components are truncated
    // to 5 or 6 bits, as they are with 16- or 18-bit color.
    //
    // d_assert( get_global_ambient_color< float_type >( ) == color);
  }

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Macros to define method specializations
// _______________________________________________________________________________________________

// ---------------------------------------------------------------------------------------------
// What these macros do:
//   NAME_EXPAND(    drop, vertex)       -> drop_vertex
//   NAME_EXPAND_P(  drop, vertex)       -> drop_vertex_p
//   NAME_EXPAND_PN( drop, vertex)       -> drop_vertex_p3
//
// These are the names of methods that we define below.
//
# define NAME_EXPAND(      PREFIX, NAME)           CONCAT_3( PREFIX, _, NAME)
# define NAME_EXPAND_P(    PREFIX, NAME)           CONCAT_4( PREFIX, _, NAME, _p)
# define NAME_EXPAND_PN(   PREFIX, NAME, NUMBER)   CONCAT_5( PREFIX, _, NAME, _p, NUMBER)
# define GL_NAME_EXPAND(   GL_NAME, NUMBER, GL_TYPE_SUFFIX)  :: CONCAT_3( GL_NAME, NUMBER, GL_TYPE_SUFFIX)
# define GL_NAME_EXPAND_V( GL_NAME, NUMBER, GL_TYPE_SUFFIX)  :: CONCAT_4( GL_NAME, NUMBER, GL_TYPE_SUFFIX, v)

// ---------------------------------------------------------------------------------------------
// How these work.
// The following line:
//
//   SPECIALIZE_VERTEX_DROP_METHOD_2_ARGS( vertex, float_type, f, glVertex)
//
// will expand into a (template specialization) implementation that looks like this:
//
//     template< >
//     void
//     gl_env::
//   drop_vertex< float >( float x, float y)
//     { assert_during_vertex_entry av( *this);
//       ::glVertex2f( x, y);
//     }
//
// It should be preceded by a generic template declaration in the class:
//
//     template< typename TYPE >
//     void
//   drop_vertex( TYPE x, TYPE y) ;
//
# define DEFINE_VERTEX_ENTRY_METHOD_1_ARGS_W(                         \
           PREFIX, NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME, WRAPPER_TYPE) \
        template< > inline void env_type::                            \
      NAME_EXPAND( PREFIX, NAME)< TYPE >( TYPE a)                     \
        { WRAPPER_TYPE wrapper( *this);                               \
          GL_NAME_EXPAND( GL_NAME, 1, GL_TYPE_SUFFIX)( a);            \
        }                                                             /* end macro */

# define DEFINE_VERTEX_ENTRY_METHOD_2_ARGS_W(                         \
           PREFIX, NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME, WRAPPER_TYPE) \
        template< > inline void env_type::                            \
      NAME_EXPAND( PREFIX, NAME)< TYPE >( TYPE a, TYPE b)             \
        { WRAPPER_TYPE wrapper( *this);                               \
          GL_NAME_EXPAND( GL_NAME, 2, GL_TYPE_SUFFIX)( a, b);         \
        }                                                             /* end macro */

# define DEFINE_VERTEX_ENTRY_METHOD_3_ARGS_W(                         \
           PREFIX, NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME, WRAPPER_TYPE) \
        template< > inline void env_type::                            \
      NAME_EXPAND( PREFIX, NAME)< TYPE >( TYPE a, TYPE b, TYPE c)     \
        { WRAPPER_TYPE wrapper( *this);                               \
          GL_NAME_EXPAND( GL_NAME, 3, GL_TYPE_SUFFIX)( a, b, c);      \
        }                                                             /* end macro */

# define DEFINE_VERTEX_ENTRY_METHOD_4_ARGS_W(                             \
           PREFIX, NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME, WRAPPER_TYPE)     \
        template< > inline void env_type::                                \
      NAME_EXPAND( PREFIX, NAME)< TYPE >( TYPE a, TYPE b, TYPE c, TYPE d) \
        { WRAPPER_TYPE wrapper( *this);                                   \
          GL_NAME_EXPAND( GL_NAME, 4, GL_TYPE_SUFFIX)( a, b, c, d);       \
        }                                                                 /* end macro */

// ---------------------------------------------------------------------------------------------

# define SPECIALIZE_VERTEX_DROP_METHOD_1_ARGS(      NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME) \
         DEFINE_VERTEX_ENTRY_METHOD_1_ARGS_W( drop, NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME, assert_during_vertex_entry)

# define SPECIALIZE_VERTEX_DROP_METHOD_2_ARGS(      NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME) \
         DEFINE_VERTEX_ENTRY_METHOD_2_ARGS_W( drop, NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME, assert_during_vertex_entry)

# define SPECIALIZE_VERTEX_DROP_METHOD_3_ARGS(      NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME) \
         DEFINE_VERTEX_ENTRY_METHOD_3_ARGS_W( drop, NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME, assert_during_vertex_entry)

# define SPECIALIZE_VERTEX_DROP_METHOD_4_ARGS(      NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME) \
         DEFINE_VERTEX_ENTRY_METHOD_4_ARGS_W( drop, NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME, assert_during_vertex_entry)

// ---------------------------------------------------------------------------------------------

# define SPECIALIZE_VERTEX_SETUP_METHOD_1_ARGS(      NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME) \
         DEFINE_VERTEX_ENTRY_METHOD_1_ARGS_W( setup, NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME, assert_no_errors)

# define SPECIALIZE_VERTEX_SETUP_METHOD_2_ARGS(      NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME) \
         DEFINE_VERTEX_ENTRY_METHOD_2_ARGS_W( setup, NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME, assert_no_errors)

# define SPECIALIZE_VERTEX_SETUP_METHOD_3_ARGS(      NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME) \
         DEFINE_VERTEX_ENTRY_METHOD_3_ARGS_W( setup, NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME, assert_no_errors)

# define SPECIALIZE_VERTEX_SETUP_METHOD_4_ARGS(      NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME) \
         DEFINE_VERTEX_ENTRY_METHOD_4_ARGS_W( setup, NAME, TYPE, GL_TYPE_SUFFIX, GL_NAME, assert_no_errors)

// ---------------------------------------------------------------------------------------------
// How this works.
// The following line:
//
//   SPECIALIZE_VERTEX_DROP_METHOD_PTR_ARGS( vertex, 3, float_type, f, glVertex)
//
// will expand into 2 (template specialization) implementations that looks like this:
//
//     template< >
//     void
//     gl_env::
//   drop_vertex_p3< float >( float const * pArgs)
//     { assert_during_vertex_entry av( *this);
//       ::glVertex3fv( pArgs);
//     }
//
//     template< >
//     void
//     gl_env::
//   drop_vertex< 3, float >( float const * pArgs)
//     { drop_vertex_p3( pArgs); }
//
// It should be preceded by these generic template declarations:
//
//     template< typename TYPE >
//     void
//   drop_vertex_p3( TYPE const *) ;
//
//     template< int COUNT, typename TYPE >
//     void
//   drop_vertex_p( TYPE const *) ;
//
# define DEFINE_VERTEX_ENTRY_METHOD_PTR_ARGS_W(                          \
           PREFIX, NAME, NUMBER, TYPE, GL_TYPE_SUFFIX,                   \
           GL_NAME, WRAPPER_TYPE)                                        \
                                                                         \
        template< > inline void env_type::                               \
      NAME_EXPAND_PN( PREFIX, NAME, NUMBER)< TYPE >( TYPE const * pArgs) \
        { WRAPPER_TYPE wrapper( *this);                                  \
          GL_NAME_EXPAND_V( GL_NAME, NUMBER, GL_TYPE_SUFFIX)( pArgs);    \
        }                                                                \
                                                                         \
        template< > inline void env_type::                               \
      NAME_EXPAND_P( PREFIX, NAME)< NUMBER, TYPE >( TYPE const * pArgs)  \
        { NAME_EXPAND_PN( PREFIX, NAME, NUMBER)( pArgs); }               /* end macro */

// ---------------------------------------------------------------------------------------------

# define SPECIALIZE_VERTEX_DROP_METHOD_PTR_ARGS(      NAME, NUMBER, TYPE, GL_TYPE_SUFFIX, GL_NAME) \
         DEFINE_VERTEX_ENTRY_METHOD_PTR_ARGS_W( drop, NAME, NUMBER, TYPE, GL_TYPE_SUFFIX, GL_NAME, assert_during_vertex_entry)

# define SPECIALIZE_VERTEX_SETUP_METHOD_PTR_ARGS(      NAME, NUMBER, TYPE, GL_TYPE_SUFFIX, GL_NAME) \
         DEFINE_VERTEX_ENTRY_METHOD_PTR_ARGS_W( setup, NAME, NUMBER, TYPE, GL_TYPE_SUFFIX, GL_NAME, assert_no_errors)

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

// Specialization implementations of vertex-entry template decls. 36 method implementations.
# define DEFINE_VERTEX_METHODS( TYPE, GL_TYPE_SUFFIX)                                   \
    SPECIALIZE_VERTEX_DROP_METHOD_2_ARGS(   vertex,    TYPE, GL_TYPE_SUFFIX, glVertex)  \
    SPECIALIZE_VERTEX_DROP_METHOD_3_ARGS(   vertex,    TYPE, GL_TYPE_SUFFIX, glVertex)  \
    SPECIALIZE_VERTEX_DROP_METHOD_4_ARGS(   vertex,    TYPE, GL_TYPE_SUFFIX, glVertex)  \
    SPECIALIZE_VERTEX_DROP_METHOD_PTR_ARGS( vertex, 2, TYPE, GL_TYPE_SUFFIX, glVertex)  \
    SPECIALIZE_VERTEX_DROP_METHOD_PTR_ARGS( vertex, 3, TYPE, GL_TYPE_SUFFIX, glVertex)  \
    SPECIALIZE_VERTEX_DROP_METHOD_PTR_ARGS( vertex, 4, TYPE, GL_TYPE_SUFFIX, glVertex)  /* end macro */

DEFINE_VERTEX_METHODS( short_type , s )  // vertex(..) -> ::glVertex2s(..), ::glVertex3s(..), ::glVertex4s(..), ::glVertex2sv(..), etc
DEFINE_VERTEX_METHODS( int_type   , i )  // vertex(..) -> ::glVertex2i(..), ::glVertex3i(..), ::glVertex4i(..), ::glVertex2iv(..), etc
DEFINE_VERTEX_METHODS( float_type , f )  // vertex(..) -> ::glVertex2f(..), ::glVertex3f(..), ::glVertex4f(..), ::glVertex2fv(..), etc
DEFINE_VERTEX_METHODS( double_type, d )  // vertex(..) -> ::glVertex2d(..), ::glVertex3d(..), ::glVertex4d(..), ::glVertex2dv(..), etc

# undef DEFINE_VERTEX_METHODS

// ---------------------------------------------------------------------------------------------

// Specialization implementations of normal-entry template decls. 30 method implementations.
# define DEFINE_NORMAL_METHODS( TYPE, GL_TYPE_SUFFIX)                                           \
    SPECIALIZE_VERTEX_DROP_METHOD_3_ARGS(    normal_vector,    TYPE, GL_TYPE_SUFFIX, glNormal)  \
    SPECIALIZE_VERTEX_SETUP_METHOD_3_ARGS(   normal_vector,    TYPE, GL_TYPE_SUFFIX, glNormal)  \
    SPECIALIZE_VERTEX_DROP_METHOD_PTR_ARGS(  normal_vector, 3, TYPE, GL_TYPE_SUFFIX, glNormal)  \
    SPECIALIZE_VERTEX_SETUP_METHOD_PTR_ARGS( normal_vector, 3, TYPE, GL_TYPE_SUFFIX, glNormal)  /* end macro */

DEFINE_NORMAL_METHODS( byte_type  , b )  // normal(..) -> ::glNormal3b(..), ::glNormal3bv(..)
DEFINE_NORMAL_METHODS( short_type , s )  // normal(..) -> ::glNormal3s(..), ::glNormal3sv(..)
DEFINE_NORMAL_METHODS( int_type   , i )  // normal(..) -> ::glNormal3i(..), ::glNormal3iv(..)
DEFINE_NORMAL_METHODS( float_type , f )  // normal(..) -> ::glNormal3f(..), ::glNormal3fv(..)
DEFINE_NORMAL_METHODS( double_type, d )  // normal(..) -> ::glNormal3d(..), ::glNormal3dv(..)

# undef DEFINE_NORMAL_METHOD

// ---------------------------------------------------------------------------------------------

// Specialization implementations of above template decls. 96 method implementations.
# define DEFINE_TEXTURE_METHODS( TYPE, GL_TYPE_SUFFIX)                                             \
    SPECIALIZE_VERTEX_DROP_METHOD_1_ARGS(    texture_coords,    TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
    SPECIALIZE_VERTEX_DROP_METHOD_2_ARGS(    texture_coords,    TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
    SPECIALIZE_VERTEX_DROP_METHOD_3_ARGS(    texture_coords,    TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
    SPECIALIZE_VERTEX_DROP_METHOD_4_ARGS(    texture_coords,    TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
    SPECIALIZE_VERTEX_DROP_METHOD_PTR_ARGS(  texture_coords, 1, TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
    SPECIALIZE_VERTEX_DROP_METHOD_PTR_ARGS(  texture_coords, 2, TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
    SPECIALIZE_VERTEX_DROP_METHOD_PTR_ARGS(  texture_coords, 3, TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
    SPECIALIZE_VERTEX_DROP_METHOD_PTR_ARGS(  texture_coords, 4, TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
                                                                                                   \
    SPECIALIZE_VERTEX_SETUP_METHOD_1_ARGS(   texture_coords,    TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
    SPECIALIZE_VERTEX_SETUP_METHOD_2_ARGS(   texture_coords,    TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
    SPECIALIZE_VERTEX_SETUP_METHOD_3_ARGS(   texture_coords,    TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
    SPECIALIZE_VERTEX_SETUP_METHOD_4_ARGS(   texture_coords,    TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
    SPECIALIZE_VERTEX_SETUP_METHOD_PTR_ARGS( texture_coords, 1, TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
    SPECIALIZE_VERTEX_SETUP_METHOD_PTR_ARGS( texture_coords, 2, TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
    SPECIALIZE_VERTEX_SETUP_METHOD_PTR_ARGS( texture_coords, 3, TYPE, GL_TYPE_SUFFIX, glTexCoord)  \
    SPECIALIZE_VERTEX_SETUP_METHOD_PTR_ARGS( texture_coords, 4, TYPE, GL_TYPE_SUFFIX, glTexCoord)  /* end macro */

DEFINE_TEXTURE_METHODS( short_type ,  s )
DEFINE_TEXTURE_METHODS( int_type   ,  i )
DEFINE_TEXTURE_METHODS( float_type ,  f )
DEFINE_TEXTURE_METHODS( double_type,  d )

# undef DEFINE_TEXTURE_METHODS

// ---------------------------------------------------------------------------------------------

// Specialization implementations of above template decls.
# define DEFINE_COLOR_METHODS( TYPE, GL_TYPE_SUFFIX)                                   \
    SPECIALIZE_VERTEX_DROP_METHOD_PTR_ARGS(  color, 3, TYPE, GL_TYPE_SUFFIX, glColor)  \
    SPECIALIZE_VERTEX_DROP_METHOD_PTR_ARGS(  color, 4, TYPE, GL_TYPE_SUFFIX, glColor)  \
    SPECIALIZE_VERTEX_SETUP_METHOD_PTR_ARGS( color, 3, TYPE, GL_TYPE_SUFFIX, glColor)  \
    SPECIALIZE_VERTEX_SETUP_METHOD_PTR_ARGS( color, 4, TYPE, GL_TYPE_SUFFIX, glColor)  /* end macro */

DEFINE_COLOR_METHODS( byte_type  ,  b )
DEFINE_COLOR_METHODS( ubyte_type , ub )
DEFINE_COLOR_METHODS( short_type ,  s )
DEFINE_COLOR_METHODS( ushort_type, us )
DEFINE_COLOR_METHODS( int_type   ,  i )
DEFINE_COLOR_METHODS( uint_type  , ui )
DEFINE_COLOR_METHODS( float_type ,  f )
DEFINE_COLOR_METHODS( double_type,  d )

# undef DEFINE_COLOR_METHODS

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------

// Get rid of macros used to define methods above.
# undef DEFINE_VERTEX_ENTRY_METHOD_PTR_ARGS_W
# undef SPECIALIZE_VERTEX_DROP_METHOD_PTR_ARGS
# undef SPECIALIZE_VERTEX_SETUP_METHOD_PTR_ARGS

# undef DEFINE_VERTEX_ENTRY_METHOD_4_ARGS_W
# undef DEFINE_VERTEX_ENTRY_METHOD_3_ARGS_W
# undef DEFINE_VERTEX_ENTRY_METHOD_2_ARGS_W
# undef DEFINE_VERTEX_ENTRY_METHOD_1_ARGS_W

# undef SPECIALIZE_VERTEX_DROP_METHOD_4_ARGS
# undef SPECIALIZE_VERTEX_DROP_METHOD_3_ARGS
# undef SPECIALIZE_VERTEX_DROP_METHOD_2_ARGS
# undef SPECIALIZE_VERTEX_DROP_METHOD_1_ARGS

# undef SPECIALIZE_VERTEX_SETUP_METHOD_4_ARGS
# undef SPECIALIZE_VERTEX_SETUP_METHOD_3_ARGS
# undef SPECIALIZE_VERTEX_SETUP_METHOD_2_ARGS
# undef SPECIALIZE_VERTEX_SETUP_METHOD_1_ARGS

# undef GL_NAME_EXPAND_V
# undef GL_NAME_EXPAND
# undef NAME_EXPAND_PN
# undef NAME_EXPAND_P
# undef NAME_EXPAND

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Wrapper Type - Save Server Attributes
// _______________________________________________________________________________________________

  class
with_saved_server_attributes : private env_val_base< int_type >
{
  public:
    /* ctor */  with_saved_server_attributes( env_type & e, bitfield_type bits)
                    : env_val_base< int_type >( e, e.get_stack_depth_saved_server_attributes( ))
                    { e.push_server_attribute_stack( bits); }

    /* dtor */  ~with_saved_server_attributes( )
                    { env.pop_server_attribute_stack( );
                      d_assert( env.get_stack_depth_saved_server_attributes( ) == val);
                    }
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Wrapper Types - Active Vertex
// _______________________________________________________________________________________________

  /* non-templated version, for when mode is not const */
  class
with_active_vertex_entry : env_base
{
  public:
    /* ctor */  with_active_vertex_entry( env_type & e, enum_type mode)
                    : env_base( e)
                    { e.activate_vertex_entry( mode); }

    /* dtor */  ~with_active_vertex_entry( )
                    { env.deactivate_vertex_entry( ); }
};

  template< enum_type VERTEX_MODE >
  class
with_active_vertex_entry_
  : public with_active_vertex_entry
{
  public:
    /* ctor */  with_active_vertex_entry_( env_type & e)
                    : with_active_vertex_entry( e, VERTEX_MODE)
                    { }
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

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Wrapper Types - Current Matrix
// _______________________________________________________________________________________________

// ---------------------------------------------------------------------------------------------
// Mixin Types - Opening Mixins (private)

  class
current_matrix__on_open_assert
{
  protected:
  current_matrix__on_open_assert( env_type & e, enum_type n)
    { e.assert_current_matrix_name( n); }
};

  class
current_matrix__on_open_set
{
  protected:
  current_matrix__on_open_set( env_type & e, enum_type n)
    { e.set_current_matrix_name( n); }
  };

  class
current_matrix__on_open_push
{
  protected:
  current_matrix__on_open_push( env_type & e, enum_type)
    { e.push_copy_on_current_matrix_stack( ); }
};

// ---------------------------------------------------------------------------------------------
// Mixin Types - Closing Mixins (private)

# ifdef NDEBUG
  class
current_matrix__on_close_assert
{
  protected:
  current_matrix__on_close_assert( env_type &, enum_type)
    { }
};
# else
  class
current_matrix__on_close_assert
  : protected env_name_base
{
  protected:
  current_matrix__on_close_assert( env_type & e, enum_type n)
    : env_name_base( e, n) { }
  ~current_matrix__on_close_assert( )
    { env.assert_current_matrix_name( name); }
};
# endif

  class
current_matrix__on_close_set
  : protected env_name_base
{
  protected:
  current_matrix__on_close_set( env_type & e, enum_type n)
    : env_name_base( e, n)
    { }
  ~current_matrix__on_close_set( )
    { env.set_current_matrix_name( name); }
};

  class
current_matrix__on_close_pop
  : protected env_name_base
{
  protected:
  current_matrix__on_close_pop( env_type & e, enum_type n)
    : env_name_base( e, n)
    , init_depth( e.get_stack_depth( n))
    { }
    int_type const init_depth ;

  ~current_matrix__on_close_pop( )
    { d_assert( env.get_current_matrix_name( ) == name);
      env.pop_current_matrix_stack( );
      // Putting the next line (assert depth) after the line above (pop) means
      // we are assuming this is constructed before current_matrix__on_open_push.
      // Otherwise we'd switch the order of these two lines.
      d_assert( env.get_stack_depth_current_matrix( ) == init_depth);
    }
};

  class
current_matrix__on_close_restore
  : protected env_name_base
{
  protected:
  current_matrix__on_close_restore( env_type & e, enum_type)
    // This ctor must run before current_matrix__on_open_set so it can get the matrix
    // name from before the set. This is why we ctor the on_close classes before the
    // on_open classes.
    : env_name_base( e, e.get_current_matrix_name( ))
    { }
  ~current_matrix__on_close_restore( )
    { env.set_current_matrix_name( name); }
};

// ---------------------------------------------------------------------------------------------
// Wrappers - Current Matrix
//
//   All of these wrapper objects take two params: env and matrix_name.
//   Matrix_name is one of:
//     GL_MODELVIEW
//     GL_PROJECTION
//     GL_TEXTURE
//
//   The matrix_name GL_COLOR is not supported here because it isn't a part of 1.1.
//
//   From the mixins we construct the following wrappers:
//     Open         Close               Wrapper class
//     ----------------------------------------------------------------------------------
//     assert       assert              with_current_matrix__assert__assert
//     set          assert              with_current_matrix__set__assert
//     set          restore             with_current_matrix__set__restore
//
//     assert/push  assert/pop          with_current_matrix__assert_push__assert_pop
//     assert/push  set/pop             with_current_matrix__assert_push__set_pop
//     set/push     assert/pop          with_current_matrix__set_push__assert_pop
//     set/push     set/pop             with_current_matrix__set_push__set_pop
//
//     set/push     assert/pop/restore  with_current_matrix__set_push__assert_pop_restore
//     set/push     set/pop/restore     with_current_matrix__set_push__set_pop_restore

# define DEFINE_CURRENT_MATRIX_WRAPPER_1_1( OPEN, CLOSE )                               \
      class                                                                             \
    with_current_matrix__ ## OPEN ## __ ## CLOSE                                        \
      : protected                                                                       \
          current_matrix__on_close_ ## CLOSE ,                                          \
          current_matrix__on_open_  ## OPEN                                             \
    { public:                                                                           \
      with_current_matrix__ ## OPEN ## __ ## CLOSE ( env_type & e, enum_type n)         \
        : current_matrix__on_close_ ## CLOSE( e, n)                                     \
        , current_matrix__on_open_  ## OPEN(  e, n)                                     \
        { }                                                                             \
    };                                                                                  /* end macro */

# define DEFINE_CURRENT_MATRIX_WRAPPER_2_2( OPEN_1, OPEN_2, CLOSE_1, CLOSE_2 )          \
      class                                                                             \
    with_current_matrix__ ## OPEN_1 ## _ ## OPEN_2 ## __                                \
                          ## CLOSE_1 ## _ ## CLOSE_2                                    \
      : protected                                                                       \
          /* Reverse the order of close mixins because dtors unwind last-to-first */    \
          current_matrix__on_close_ ## CLOSE_2 ,                                        \
          current_matrix__on_close_ ## CLOSE_1 ,                                        \
          current_matrix__on_open_  ## OPEN_1  ,                                        \
          current_matrix__on_open_  ## OPEN_2                                           \
    { public:                                                                           \
      with_current_matrix__ ## OPEN_1 ## _ ## OPEN_2 ## __                              \
                            ## CLOSE_1 ## _ ## CLOSE_2                                  \
       (  env_type &  env                                                               \
        , enum_type   matrix_name                                                       \
       )                                                                                \
        : current_matrix__on_close_ ## CLOSE_2( env, matrix_name)                       \
        , current_matrix__on_close_ ## CLOSE_1( env, matrix_name)                       \
        , current_matrix__on_open_  ## OPEN_1(  env, matrix_name)                       \
        , current_matrix__on_open_  ## OPEN_2(  env, matrix_name)                       \
        { }                                                                             \
    };                                                                                  /* end macro */

# define DEFINE_CURRENT_MATRIX_WRAPPER_2_3( OPEN_1, OPEN_2, CLOSE_1, CLOSE_2, CLOSE_3 ) \
      class                                                                             \
    with_current_matrix__ ## OPEN_1 ## _ ## OPEN_2 ## __                                \
                          ## CLOSE_1 ## _ ## CLOSE_2 ## _ ## CLOSE_3                    \
      : protected                                                                       \
          /* Reverse the order of close mixins because dtors unwind last-to-first */    \
          current_matrix__on_close_ ## CLOSE_3 ,                                        \
          current_matrix__on_close_ ## CLOSE_2 ,                                        \
          current_matrix__on_close_ ## CLOSE_1 ,                                        \
          current_matrix__on_open_  ## OPEN_1 ,                                         \
          current_matrix__on_open_  ## OPEN_2                                           \
    { public:                                                                           \
      with_current_matrix__ ## OPEN_1 ## _ ## OPEN_2 ## __                              \
                            ## CLOSE_1 ## _ ## CLOSE_2 ## _ ## CLOSE_3                  \
       (  env_type &  env                                                               \
        , enum_type   matrix_name                                                       \
       )                                                                                \
        : current_matrix__on_close_ ## CLOSE_3( env, matrix_name)                       \
        , current_matrix__on_close_ ## CLOSE_2( env, matrix_name)                       \
        , current_matrix__on_close_ ## CLOSE_1( env, matrix_name)                       \
        , current_matrix__on_open_  ## OPEN_1(  env, matrix_name)                       \
        , current_matrix__on_open_  ## OPEN_2(  env, matrix_name)                       \
        { }                                                                             \
    };                                                                                  /* end macro */

DEFINE_CURRENT_MATRIX_WRAPPER_1_1( assert,          assert )
DEFINE_CURRENT_MATRIX_WRAPPER_1_1( set   ,          assert )
DEFINE_CURRENT_MATRIX_WRAPPER_1_1( set   ,          restore)

DEFINE_CURRENT_MATRIX_WRAPPER_2_2( assert, push,    assert, pop )
DEFINE_CURRENT_MATRIX_WRAPPER_2_2( assert, push,    set   , pop )
DEFINE_CURRENT_MATRIX_WRAPPER_2_2( set   , push,    assert, pop )
DEFINE_CURRENT_MATRIX_WRAPPER_2_2( set   , push,    set   , pop )

DEFINE_CURRENT_MATRIX_WRAPPER_2_3( set   , push,    assert, pop, restore )
DEFINE_CURRENT_MATRIX_WRAPPER_2_3( set   , push,    set   , pop, restore )

# undef DEFINE_CURRENT_MATRIX_WRAPPER_2_3
# undef DEFINE_CURRENT_MATRIX_WRAPPER_2_2
# undef DEFINE_CURRENT_MATRIX_WRAPPER_1_1

// _______________________________________________________________________________________________
//
} /* end namespace gl_env */
// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef GL_ENV_H
//
// gl_env.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
