// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// angle_holder.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// _______________________________________________________________________________________________

# include "all.h"
# include "angle_holder.h"

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// angle_holder - constructors
// _______________________________________________________________________________________________

  /* ctor */
  angle_holder::
angle_holder
 (  QObject *       p_parent
  , double          init_degrees
  , angle_arc_type  arc
 )
  : int_range_steps_holder
     (  p_parent
      , degrees_to_arcseconds( init_degrees)
      , degrees_to_arcseconds( arc.get_min_degrees( ))
      , degrees_to_arcseconds( arc.get_max_degrees( ))
     )
{
    d_assert( get_value( ) == degrees_to_arcseconds( init_degrees));
    d_assert( (get_max_value( ) - get_min_value( )) <= degrees_to_arcseconds( 360));
}

  /* ctor */
  angle_holder::
angle_holder
 (  QObject *  p_parent
  , double     init_degrees
  , double     min_degrees
  , double     max_degrees
 )
  : int_range_steps_holder
     (  p_parent
      , degrees_to_arcseconds( init_degrees)
      , degrees_to_arcseconds( min_degrees)
      , degrees_to_arcseconds( max_degrees)
     )
{
    d_assert( get_value( ) == degrees_to_arcseconds( init_degrees));
    d_assert( get_min_value( ) == degrees_to_arcseconds( min_degrees));
    d_assert( get_max_value( ) == degrees_to_arcseconds( max_degrees));
    d_assert( (get_max_value( ) - get_min_value( )) <= degrees_to_arcseconds( 360));
}

// _______________________________________________________________________________________________
//
// angle_holder - constants
// _______________________________________________________________________________________________

// We need pi. There are a couple ways to get it:
//   std::atan( +1.0) * 4
//   std::acos(  0.0) * 2
//   std::acos( -1.0)
//   std::asin( +1.0) * 2
//   M_PI from <cmath> although you may have to define _USE_MATH_DEFINES first.

  /* static */
  double const
  angle_holder::
pi = std::acos( static_cast< double >( -1));

  /* static */
  double const
  angle_holder::
pi_over_2 = pi / 2;

  /* static */
  double const
  angle_holder::
pi_over_180 = pi / 180;

// _______________________________________________________________________________________________
//
// angle_holder - static conversion functions
// _______________________________________________________________________________________________

  /* static */
  double
  angle_holder::
degrees_to_radians( double angle_in_degrees)
{
    return angle_in_degrees * pi_over_180;
}

  /* static */
  double
  angle_holder::
radians_to_degrees( double angle_in_radians)
{
    return angle_in_radians / pi_over_180;
}

// _______________________________________________________________________________________________

// Store angles as integer arcseconds.
// There are 3600 arcseconds in a degree, 1,296,000 arc-seconds in 360 degrees.
int const c_seconds_per_degree = 60 * 60;

// Degrees are usually doubles.
// Arcseconds are usually int.

  /* static */
  double /* degrees are usually doubles */
  angle_holder::
arcseconds_to_degrees( int i_arcseconds)
{
    d_assert( (-720 * c_seconds_per_degree) <= i_arcseconds);
    d_assert( (+720 * c_seconds_per_degree) >= i_arcseconds);
    return static_cast< double >( i_arcseconds) / c_seconds_per_degree;
}

  /* static */
  int
  angle_holder::
degrees_to_arcseconds( double d_degrees)
{
    d_assert( -720.0 <= d_degrees);
    d_assert( +720.0 >= d_degrees);
    return static_cast< int >( std::floor( (d_degrees * c_seconds_per_degree) + 0.5));
}

  /* static */
  int
  angle_holder::
degrees_to_arcseconds( int i_degrees)
{
    d_assert( -720 <= i_degrees);
    d_assert( +720 >= i_degrees);
    return i_degrees * c_seconds_per_degree;
}

// _______________________________________________________________________________________________
//
// angle_holder - get/set methods
// _______________________________________________________________________________________________

  int
  angle_holder::
get_arc_angle__arcseconds( ) const
{
    int const width_arcsec = get_max_angle__arcseconds( ) - get_min_angle__arcseconds( );
    d_assert( (0 < width_arcsec) && (width_arcsec <= degrees_to_arcseconds( 360)));
    return width_arcsec;
}

  bool
  angle_holder::
is_full_circle( ) const
{
    return get_arc_angle__arcseconds( ) == degrees_to_arcseconds( 360);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// angle_animator_type - class
// _______________________________________________________________________________________________

namespace /* anonymous */ {

  bool
find_out_is_wrapping( angle_holder * p_holder)
  //
  // True if the angle can go all the way around the circle.
  {
    d_assert( p_holder);
    return p_holder->is_full_circle( ); /* wraps if it's a full circle */
  }

  double
find_max_angle_per_second( angle_holder * p_holder)
  //
  // +240 if full circle. Otherwise two-thirds of the entire arc.
  {
    d_assert( p_holder);
    return (p_holder->get_arc_angle__degrees( ) * 2) / 3;
  }

} /* end namespace anonymous */

// _______________________________________________________________________________________________

  /* constructor */
  angle_animator_type::
angle_animator_type
 (  animate_type *  p_parent
  , angle_holder *  p_holder
  , double          init_degrees_per_second
 )
  : delta_animator_type (  p_parent
                         , find_out_is_wrapping( p_holder)
                         , init_degrees_per_second
                         , 0.01 /* min delta-angle per second (in degrees) */
                         , find_max_angle_per_second( p_holder)
                        )
  , p_holder_           ( p_holder)
  { }

// _______________________________________________________________________________________________

  /* overridden virtual */
  holder_base_type *
  angle_animator_type::
get_animated( ) const
{
    d_assert( p_holder_);
    return p_holder_;
}

  /* overridden virtual */
  double
  angle_animator_type::
get_animating_value( ) const
{
    d_assert( p_holder_);
    return p_holder_->get_angle__degrees( );
}

  /* overridden virtual */
  void
  angle_animator_type::
set_animating_value( double new_value)
{
    d_assert( p_holder_);
    p_holder_->set_angle__degrees( new_value);
}

  /* overridden virtual */
  bool
  angle_animator_type::
is_bumping_min( )
{
    d_assert( p_holder_);
    return p_holder_->is_bumping_min( );
}

  /* overridden virtual */
  bool
  angle_animator_type::
is_bumping_max( )
{
    d_assert( p_holder_);
    return p_holder_->is_bumping_max( );
}

  /* overridden virtual */
  void
  angle_animator_type::
wrap_to_min( )
{
    d_assert( p_holder_);
    p_holder_->wrap_to_min( );
}

  /* overridden virtual */
  void
  angle_animator_type::
wrap_to_max( )
{
    d_assert( p_holder_);
    p_holder_->wrap_to_max( );
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// angle_holder.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
