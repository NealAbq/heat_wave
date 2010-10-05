// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_env_type_colors.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef GL_ENV_TYPE_COLORS_H
# define GL_ENV_TYPE_COLORS_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "gl_env_type_primitives.h"
# include "gl_env_fractional_fixed_point.h"

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
namespace gl_env {
// _______________________________________________________________________________________________

// ---------------------------------------------------------------------------------------------
// Color helper functions

// -- get_min_color_component ----
// -- get_max_color_component ----
// -- is_clamped__color_component ----

  template< typename TYPE >
  inline
  TYPE
get_min_color_component( bool allow_negative = false)
  { return get_min_fractional_fixed_point< TYPE >( allow_negative); }

  template< typename TYPE >
  inline
  TYPE
get_max_color_component( )
  { return get_max_fractional_fixed_point< TYPE >( ); }

  template< typename TYPE >
  inline
  bool
is_clamped__color_component( TYPE cc, bool allow_negative = false)
  { return is_clamped__fractional_fixed_point( cc, allow_negative); }

  template< typename TYPE >
  inline
  TYPE
get_clamp__color_component( TYPE cc, bool allow_negative = false)
  { get_clamp__fractional_fixed_point( cc, allow_negative); }

  template< typename TYPE >
  inline
  void
clamp__color_component( TYPE & cc, bool allow_negative = false)
  { clamp__fractional_fixed_point( cc, allow_negative); }

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// CLASS rgb_type< component_type >

  template< typename COMPONENT_TYPE >
  class
rgb_type
  //
  // This is an RGBA color.
{
  // count and typedefs
  public:
    static size_t const                 component_count  = 3;

    typedef COMPONENT_TYPE              component_type ;
  private:
    typedef rgb_type< component_type >  this_type      ;

  // default color-component value
  protected:
    static component_type   get_default_cc( )                 { d_assert( 0 == get_min_color_component< component_type >( false));
                                                                return 0;
                                                              }

  // ctors
  // Use default copy assignment and constructor.
  // This will also let you assign "rgb = rgba" since rgba is a subtype.
  public:
    /* ctor */              rgb_type( )                       : r_( get_default_cc( ))
                                                              , g_( get_default_cc( ))
                                                              , b_( get_default_cc( ))
                                                              { // The above values should all be zero.
                                                                d_assert( is_reset( ));
                                                              }
    /* ctor */              rgb_type
                             (  component_type const & r
                              , component_type const & g
                              , component_type const & b
                             )                                : r_( r), g_( g), b_( b) { }

  // reset
  public:
    this_type &             reset( )                          { return set
                                                                 (  get_default_cc( )
                                                                  , get_default_cc( )
                                                                  , get_default_cc( )
                                                                 );
                                                              }
    bool                    is_reset( )                 const { return
                                                                  (get_r( ) == get_default_cc( )) &&
                                                                  (get_g( ) == get_default_cc( )) &&
                                                                  (get_b( ) == get_default_cc( )) ;
                                                              }
    bool                    not_reset( )                const { return ! is_reset( ); }

  // component_type conversion
  public:
                              template< typename T >
    rgb_type< T >           convert_components_to( )    const { return rgb_type< T >
                                                                  (  convert_fractional_fixed_point_to< T >( get_r( ))
                                                                   , convert_fractional_fixed_point_to< T >( get_g( ))
                                                                   , convert_fractional_fixed_point_to< T >( get_b( ))
                                                                  );
                                                              }

  // is_clamped
  public:
    bool                    is_clamped( bool allow_negative = false)
                                                        const { return
                                                                  is_clamped__color_component( get_r( ), allow_negative) &&
                                                                  is_clamped__color_component( get_g( ), allow_negative) &&
                                                                  is_clamped__color_component( get_b( ), allow_negative) ;
                                                              }
    void                    clamp( bool allow_negative = false)
                                                              { clamp__color_component( ref_r( ), allow_negative);
                                                                clamp__color_component( ref_g( ), allow_negative);
                                                                clamp__color_component( ref_b( ), allow_negative);
                                                              }
    this_type               get_clamp( bool allow_negative = false)
                                                        const { return
                                                                  this_type(
                                                                    get_clamp__color_component( get_r( ), allow_negative),
                                                                    get_clamp__color_component( get_g( ), allow_negative),
                                                                    get_clamp__color_component( get_b( ), allow_negative));
                                                              }

  // get as pointer to array
  public:
    component_type const *  get_as_p3( )                const { d_assert( (& r_) == (((component_type const *) this) + 0));
                                                                d_assert( (& g_) == (((component_type const *) this) + 1));
                                                                d_assert( (& b_) == (((component_type const *) this) + 2));
                                                                return reinterpret_cast< component_type const * >( this);
                                                              }
    component_type       *  ref_as_p3( )                      { d_assert( (& ref_r( )) == (((component_type const *) this) + 0));
                                                                d_assert( (& ref_g( )) == (((component_type const *) this) + 1));
                                                                d_assert( (& ref_b( )) == (((component_type const *) this) + 2));
                                                                return & ref_r( );
                                                              }

  // getters
  public:
    component_type const &  get_r( )                    const { return r_; }
    component_type const &  get_g( )                    const { return g_; }
    component_type const &  get_b( )                    const { return b_; }

  // setters
  public:
    this_type &             set( component_type const & r, component_type const & g, component_type const & b)
                                                              { set_r( r);
                                                                set_g( g);
                                                                set_b( b);
                                                                return *this;
                                                              }
    this_type &             set( this_type const & rgb)       { *this = rgb; /* use default copy assignment */
                                                                return *this;
                                                              }
    this_type &             set_r( component_type const & r)  { r_ = r; return *this; }
    this_type &             set_g( component_type const & g)  { g_ = g; return *this; }
    this_type &             set_b( component_type const & b)  { b_ = b; return *this; }

    component_type &        ref_r( )                          { return r_; }
    component_type &        ref_g( )                          { return g_; }
    component_type &        ref_b( )                          { return b_; }

  // safe bool idiom
  public:
      typedef component_type const this_type::*
    unspecified_bool_type;

      operator
    unspecified_bool_type( )                            const { return is_reset( ) ? 0 : & this_type::r_; }

      unspecified_bool_type
    operator !( )                                       const { return is_reset( ) ? & this_type::r_ : 0; }

  // member vars
  private:
    // We could declare this as an array:
    //   component_type rgb_[ component_count ]
    // If we do, how should we define rgba? We'd like to inherit the methods from rgb,
    // but also treat the components like an array of 4 components.
    component_type  r_ ; // red, must be first
    component_type  g_ ; // green, must be second
    component_type  b_ ; // blue, must be third
};

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// CLASS rgba_type< component_type >

  template< typename COMPONENT_TYPE >
  class
rgba_type
  : public rgb_type< COMPONENT_TYPE >
{
  // count and typedefs
  public:
    static size_t const                  component_count  = 4;
    typedef COMPONENT_TYPE               component_type;
  private:
    typedef rgba_type< component_type >  this_type  ;
    typedef rgb_type<  component_type >  super_type ;

  // default alpha value
  protected:
    static component_type   get_default_alpha( )              { return get_max_color_component< component_type >( ); }

  // ctors
  // Use default copy assignment and constructor.
  public:
    /* ctor */              rgba_type( )                      : super_type( )
                                                              , a_( get_default_alpha( ))
                                                              { }

    /* ctor */              rgba_type
                             (  component_type const & r
                              , component_type const & g
                              , component_type const & b
                              , component_type const & a = get_default_alpha( )
                             )                                : super_type( r, g, b)
                                                              , a_( a)
                                                              { }

    /* ctor */              rgba_type
                             (  super_type     const & rgb
                              , component_type const & a = get_default_alpha( )
                             )                                : super_type( rgb)
                                                              , a_( a)
                                                              { }
    // Assignment from supertype.
    // This does NOT preserve the get_a( ) value. It sets it to the default.
    // To preserve alpha use "rgba.ref_as_rgb( ) = rgb".
    this_type &             operator =( super_type const & rgb)
                                                              { return set( rgb); }

  // reset
  public:
    this_type &             reset( )                          { ref_as_rgb( ).reset( );
                                                                return set_a( get_default_alpha( ));
                                                              }
    bool                    is_reset( )                 const { return get_as_rgb( ).is_reset( ) &&
                                                                  (get_a( ) == get_default_alpha( ));
                                                              }
    bool                    not_reset( )                const { return ! is_reset( ); }

  // component_type conversion
  public:
                              template< typename T >
    rgba_type< T >          convert_components_to( )    const { return rgba_type< T >
                                                                  (  convert_fractional_fixed_point_to< T >( super_type::get_r( ))
                                                                   , convert_fractional_fixed_point_to< T >( super_type::get_g( ))
                                                                   , convert_fractional_fixed_point_to< T >( super_type::get_b( ))
                                                                   , convert_fractional_fixed_point_to< T >( get_a( ))
                                                                  );
                                                              }

  // is_clamped
  public:
    bool                    is_clamped( bool allow_negative = false)
                                                        const { return get_as_rgb( ).is_clamped( allow_negative) &&
                                                                  is_clamped__color_component( get_a( ), allow_negative);
                                                              }
    void                    clamp( bool allow_negative = false)
                                                              { ref_as_rgb( ).clamp( allow_negative);
                                                                clamp__color_component( ref_a( ), allow_negative);
                                                              }
    this_type               get_clamp( bool allow_negative = false)
                                                        const { return
                                                                  this_type(
                                                                    get_clamp__color_component( super_type::get_r( ), allow_negative),
                                                                    get_clamp__color_component( super_type::get_g( ), allow_negative),
                                                                    get_clamp__color_component( super_type::get_b( ), allow_negative),
                                                                    get_clamp__color_component( get_a( ), allow_negative));
                                                              }

  // get as rgb (supertype)
  public:
    super_type const &      get_as_rgb( )               const { return *this; }
    super_type       &      ref_as_rgb( )                     { return *this; }

  // get as pointer to array
  public:
    component_type const *  get_as_p4( )                const { d_assert( (& a_) == (((component_type const *) this) + 3));
                                                                return super_type::get_as_p3( );
                                                              }
    component_type       *  ref_as_p4( )                      { d_assert( (& ref_a( )) == (((component_type const *) this) + 3));
                                                                return super_type::ref_as_p3( );
                                                              }

  // getter and setters
  public:
    component_type const &  get_a( )                    const { return a_; }

    this_type &             set
                             (  component_type const & r
                              , component_type const & g
                              , component_type const & b
                              , component_type const & a = get_default_alpha( )
                             )                                { ref_as_rgb( ).set( r, g, b);
                                                                return set_a( a);
                                                              }
    this_type &             set
                             (  super_type     const & rgb
                              , component_type const & a = get_default_alpha( )
                             )                                { ref_as_rgb( ).set( rgb);
                                                                return set_a( a);
                                                              }

    this_type &             set_a( component_type const & a)  { a_ = a; return *this; }
    component_type &        ref_a( )                          { return a_; }

  // safe bool idiom
  public:
      typedef component_type const this_type::*
    unspecified_bool_type;

      operator
    unspecified_bool_type( )                            const { return is_reset( ) ? 0 : & this_type::a_; }

      unspecified_bool_type
    operator !( )                                       const { return is_reset( ) ? & this_type::a_ : 0; }

  // member var
  private:
    component_type  a_ ; // alpha channel
};

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// Related functions and function overloads

  template< typename COMPONENT_TYPE >
  int
compare
 (  rgb_type< COMPONENT_TYPE > const & x
  , rgb_type< COMPONENT_TYPE > const & y
 )
  { return
      (x.get_r( ) != y.get_r( )) ? ((x.get_r( ) < y.get_r( )) ? -1 : +1) : (
      (x.get_g( ) != y.get_g( )) ? ((x.get_g( ) < y.get_g( )) ? -1 : +1) : (
      (x.get_b( ) != y.get_b( )) ? ((x.get_b( ) < y.get_b( )) ? -1 : +1) : 0 ));
  }

  template< typename COMPONENT_TYPE >
  int
compare
 (  rgba_type< COMPONENT_TYPE > const & x
  , rgba_type< COMPONENT_TYPE > const & y
 )
  { return
      (x.get_r( ) != y.get_r( )) ? ((x.get_r( ) < y.get_r( )) ? -1 : +1) : (
      (x.get_g( ) != y.get_g( )) ? ((x.get_g( ) < y.get_g( )) ? -1 : +1) : (
      (x.get_b( ) != y.get_b( )) ? ((x.get_b( ) < y.get_b( )) ? -1 : +1) : (
      (x.get_a( ) != y.get_a( )) ? ((x.get_a( ) < y.get_a( )) ? -1 : +1) : 0 )));
  }

// ---------------------------------------------------------------------------------------------

  template< typename COMPONENT_TYPE >
  bool
operator ==
 (  rgb_type< COMPONENT_TYPE > const & x
  , rgb_type< COMPONENT_TYPE > const & y
 )
  { return
      (x.get_r( ) == y.get_r( )) &&
      (x.get_g( ) == y.get_g( )) &&
      (x.get_b( ) == y.get_b( )) ;
  }

  template< typename COMPONENT_TYPE >
  bool
operator ==
 (  rgba_type< COMPONENT_TYPE > const & x
  , rgba_type< COMPONENT_TYPE > const & y
 )
  { return
      (x.get_r( ) == y.get_r( )) &&
      (x.get_g( ) == y.get_g( )) &&
      (x.get_b( ) == y.get_b( )) &&
      (x.get_a( ) == y.get_a( )) ;
  }

// ---------------------------------------------------------------------------------------------

  template< typename COMPONENT_TYPE >
  bool
operator <
 (  rgb_type< COMPONENT_TYPE > const & x
  , rgb_type< COMPONENT_TYPE > const & y
 )
  { return
        (x.get_r( ) < y.get_r( )) ? true : (
        (x.get_g( ) < y.get_g( )) ? true : (
        (x.get_b( ) < y.get_b( )) ? true : false ));
  }

  template< typename COMPONENT_TYPE >
  bool
operator <
 (  rgba_type< COMPONENT_TYPE > const & x
  , rgba_type< COMPONENT_TYPE > const & y
 )
  { return
        (x.get_r( ) < y.get_r( )) ? true : (
        (x.get_g( ) < y.get_g( )) ? true : (
        (x.get_b( ) < y.get_b( )) ? true : (
        (x.get_a( ) < y.get_a( )) ? true : false )));
  }

// ---------------------------------------------------------------------------------------------
// These could be taken care of with boost::totally_ordered<T> or namespace std::rel_ops.

  template< typename COMPONENT_TYPE >
  inline
  bool
operator !=
 (  rgb_type< COMPONENT_TYPE > const & x
  , rgb_type< COMPONENT_TYPE > const & y
 )
  { return ! (x == y); }

  template< typename COMPONENT_TYPE >
  inline
  bool
operator !=
 (  rgba_type< COMPONENT_TYPE > const & x
  , rgba_type< COMPONENT_TYPE > const & y
 )
  { return ! (x == y); }

// ---------------------------------------------------------------------------------------------

  template< typename COMPONENT_TYPE >
  inline
  bool
operator >
 (  rgb_type< COMPONENT_TYPE > const & x
  , rgb_type< COMPONENT_TYPE > const & y
 )
  { return (y < x); }

  template< typename COMPONENT_TYPE >
  inline
  bool
operator >
 (  rgba_type< COMPONENT_TYPE > const & x
  , rgba_type< COMPONENT_TYPE > const & y
 )
  { return (y < x); }

// ---------------------------------------------------------------------------------------------

  template< typename COMPONENT_TYPE >
  inline
  bool
operator >=
 (  rgb_type< COMPONENT_TYPE > const & x
  , rgb_type< COMPONENT_TYPE > const & y
 )
  { return ! (x < y); }

  template< typename COMPONENT_TYPE >
  inline
  bool
operator >=
 (  rgba_type< COMPONENT_TYPE > const & x
  , rgba_type< COMPONENT_TYPE > const & y
 )
  { return ! (x < y); }

// ---------------------------------------------------------------------------------------------

  template< typename COMPONENT_TYPE >
  inline
  bool
operator <=
 (  rgb_type< COMPONENT_TYPE > const & x
  , rgb_type< COMPONENT_TYPE > const & y
 )
  { return ! (x > y); }

  template< typename COMPONENT_TYPE >
  inline
  bool
operator <=
 (  rgba_type< COMPONENT_TYPE > const & x
  , rgba_type< COMPONENT_TYPE > const & y
 )
  { return ! (x > y); }

// ---------------------------------------------------------------------------------------------
// swap - use default

// ---------------------------------------------------------------------------------------------
// hash_value

  template< typename COMPONENT_TYPE >
  std::size_t
hash_value( rgb_type< COMPONENT_TYPE > const & x)
  {
    std::size_t seed = 0;
    boost::hash_combine( seed, x.get_r( ));
    boost::hash_combine( seed, x.get_g( ));
    boost::hash_combine( seed, x.get_b( ));
    return seed;
  }

  template< typename COMPONENT_TYPE >
  std::size_t
hash_value( rgba_type< COMPONENT_TYPE > const & x)
  {
    std::size_t seed = 0;
    boost::hash_combine( seed, x.get_r( ));
    boost::hash_combine( seed, x.get_g( ));
    boost::hash_combine( seed, x.get_b( ));
    boost::hash_combine( seed, x.get_a( ));
    return seed;
  }

// _______________________________________________________________________________________________
// interpolate(..) gives us the bezier functions.

  rgb_type< float_type >
interpolate( float factor, rgb_type< float_type > const &, rgb_type< float_type > const &);

  rgba_type< float_type >
interpolate( float factor, rgba_type< float_type > const &, rgba_type< float_type > const &);

  rgb_type< double_type >
interpolate( double factor, rgb_type< double_type > const &, rgb_type< double_type > const &);

  rgba_type< double_type >
interpolate( double factor, rgba_type< double_type > const &, rgba_type< double_type > const &);

// _______________________________________________________________________________________________
//
} /* end namespace gl_env */
// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef GL_ENV_TYPE_COLORS_H
//
// gl_env_type_colors.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
