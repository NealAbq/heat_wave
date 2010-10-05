// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// pack_holder.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Pack-holders are really double-holders. They hold double values which are mapped into a wide
// integer range which is then given to the UI.
//
// What kinds of double holders could there be?
//   Attached directly to double-value controls, like dspin_box.
//     I think it'd be better to have an int holder attach to dspin_box,
//     and know how many decimal places are showing.
//
//   Pass thru to int that doesn't store value.
//     Linear, square, cube, exponential, normal (bell) curve, sin.
//       These are all gradients. We should have a separate mapping class, and allow composition.
//     Need params for each.
//     This is what is implemented in this file.
//
//   Pass thru that stores value.
//     Value may not exactly agree with inner int value due to rounding.
//
// For a general linear pack you need limits in both coord systems:
//   s0, s1, t0, t1
//   Translating values is easy.
//     s = s0 + (t - t0)(s1 - s0)/(t1 - t0)
//   Two linear mapping can be composed into one. This operations is
//   associative but not commutative. It is just the 1D analog of the
//   view-matrix transforms. It's a 2x2 matrix instead of 4x4.
//
// For non-linear mappings (square, exponential), linear map the src range
//   to [0..1], apply the non-linear component (which is normalized to map
//   [0..1] to [0..1], and use a 2nd linear map to get the target coord.
// _______________________________________________________________________________________________

# include "all.h"
# include "pack_holder.h"
# include "util.h"

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Conversion between i_ui, d_linear_pack, and d_log_pack units.
//
//   i_ui           - Integer, used with the UI controls (sliders/scrollbars). Range is
//                      [-billion, +billion] although usually we work in [-10000, +10000].
//   d_linear_pack  - Double, linear packed, range [-million, +million] although we usually
//                      stick to [-10.0, +10.0].
//   d_log_pack     - Double, log packed, range is [+0.0001, +10000] which maps to an i_ui
//                      range of [-10000, 10000].
// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

namespace /* anonymous */ {

int    const  i_linear_to_ui_expand_factor   = 1000;
double const  d_linear_to_ui_expand_factor   = i_linear_to_ui_expand_factor;
double const  d_ui_to_linear_squeeze_factor  = 1 / d_linear_to_ui_expand_factor;

int    const  i_linear_max  = 1000000; /* + million */
int    const  i_linear_min  = - i_linear_max;
double const  d_linear_max  = i_linear_max;
double const  d_linear_min  = i_linear_min;

int    const  i_ui_max      = i_linear_max * i_linear_to_ui_expand_factor; /* + billion */
int    const  i_ui_min      = i_linear_min * i_linear_to_ui_expand_factor; /* - billion */

  inline
  bool
is_valid_ui( int i_ui)
  {
    return (i_ui_min <= i_ui) && (i_ui <= i_ui_max);
  }

  inline
  bool
is_valid_linear_pack( int i_linear_pack)
  {
    return (i_linear_min <= i_linear_pack) && (i_linear_pack <= i_linear_max);
  }

  inline
  bool
is_valid_linear_pack( double d_linear_pack)
  {
    return (d_linear_min <= d_linear_pack) && (d_linear_pack <= d_linear_max);
  }

} /* end namespace anonymous */

// _______________________________________________________________________________________________

  /* static */
  int
  pack_range_steps_holder::
convert_linear_pack_to_ui( double d_linear_pack)
  //
  // Use this when the double value range is limited (maybe [-1.0, +1.0] or [-100.0, +100.0]).
  // We make the limits large here because sometimes (when calc'ing page steps)
  // we can have values here way beyond the limits.
  {
    d_assert( is_valid_linear_pack( d_linear_pack));
    int const
      i_ui =
        static_cast< int >(
          std::floor( ( d_linear_pack * d_linear_to_ui_expand_factor) + 0.5));
    d_assert( is_valid_ui( i_ui));
    return i_ui;
  }

// _______________________________________________________________________________________________

  /* static */
  int
  pack_range_steps_holder::
convert_linear_pack_to_ui( int i_linear_pack)
  {
    d_assert( is_valid_linear_pack( i_linear_pack));
    int const i_ui = i_linear_pack * i_linear_to_ui_expand_factor;
    d_assert( is_valid_ui( i_ui));
    return i_ui;
  }

// _______________________________________________________________________________________________

  /* static */
  double
  pack_range_steps_holder::
convert_ui_to_linear_pack( int i_ui)
  //
  // Changes an integer to a double 1000 times smaller.
  // Used when you need a big integer for a UI widget, but you store a small double.
  {
    d_assert( is_valid_ui( i_ui));
    double const d_linear_pack = static_cast< double >( i_ui) * d_ui_to_linear_squeeze_factor;
    d_assert( is_valid_linear_pack( d_linear_pack));
    return d_linear_pack;
  }

// _______________________________________________________________________________________________

  /* static */
  int
  pack_range_steps_holder::
convert_log_pack_to_ui( double d_log_pack)
  //
  // Used when you want to map a double value to a log-scale integer.
  // Double must be > 0. Doubles map to integers as follows:
  //
  //   log_pack   ui == 1000 * std::log( log_pack)
  //   =======    ================
  //   0.0001  -> -9210
  //   0.001   -> -6908
  //   0.01    -> -4605
  //   0.1     -> -2303
  //   1.0     -> 0
  //   10.0    -> +2303
  //   100.0   -> +4605
  //   1000.0  -> +6908
  //   10000.0 -> +9210
  {
    // ln( 10000) is slightly more than +9.21
    // ln( .0001) is slightly less than -9.21
    d_assert( (0.0001 <= d_log_pack) && (d_log_pack <= 10000.0)); // is_valid_log_pack
    // std::log(..) is natural log. std::log10(..) is log-base-10.
    double const d_linear_pack = std::log( d_log_pack);
    int const i_ui = convert_linear_pack_to_ui( d_linear_pack);
    d_assert( (-10000 <= i_ui) && (i_ui <= 10000)); // is_valid_ui_for_log_pack
    return i_ui;
  }

// _______________________________________________________________________________________________

  /* static */
  double
  pack_range_steps_holder::
convert_ui_to_log_pack( int i_ui)
{
    d_assert( (-10000 <= i_ui) && (i_ui <= 10000)); // is_valid_ui_for_log_pack
    // std::exp(..) calcs the "natural" exponential.
    double const d_linear_pack = convert_ui_to_linear_pack( i_ui);
    double const d_log_pack = std::exp( d_linear_pack);
    d_assert( (0.0001 <= d_log_pack) && (d_log_pack <= 10000.0)); // is_valid_log_pack
    return d_log_pack;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// pack_range_steps_holder - constructors
// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* ctor */
  pack_range_steps_holder::
pack_range_steps_holder
 (  QObject *  p_parent
  , double     init_pack    // = 0.0
  , double     min_pack     // = -1.0
  , double     max_pack     // = +1.0
  , bool       is_log_pack  // = false
 )
  : int_range_steps_holder
     (  p_parent
      , convert_pack_to_ui( is_log_pack, init_pack)
      , convert_pack_to_ui( is_log_pack, min_pack)
      , convert_pack_to_ui( is_log_pack, max_pack)
     )
  , is_log_pack_( is_log_pack)
{
    // This is ONLY used with is_log_pack true right now.
    d_assert( is_log_pack);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// double_slide_holder - class
// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

// With the angle animator we store integer arc-seconds underneath. The inner_hi value is 60*60.
double_slide_holder::inner_value_type const  inner_lo_  = 0;
double_slide_holder::inner_value_type const  inner_hi_  = 1024;

// _______________________________________________________________________________________________

  double_slide_holder::inner_value_type
  double_slide_holder::
convert_outer_to_inner_distance( value_type outer_dist) const
{
    if ( outer_dist == 0 ) return 0;
    if ( outer_dist < 0 ) {
        d_assert( outer_dist == -1);
        return -1;
    }
    return util::cast_nearest< inner_value_type >( outer_dist * outer_to_inner_);
}

  double_slide_holder::value_type
  double_slide_holder::
convert_inner_to_outer_distance( inner_value_type inner_dist) const
{
    if ( inner_dist == 0 ) return 0;
    if ( inner_dist < 0 ) {
        d_assert( inner_dist == -1);
        return -1;
    }
    // This is a double->double cast, so not a cast at all.
    return util::cast_nearest< value_type >( inner_dist * inner_to_outer_);
}

// _______________________________________________________________________________________________

  double_slide_holder::inner_value_type
  double_slide_holder::
convert_outer_to_inner( value_type outer) const
  //
  // This does not clamp.
{
    return util::cast_nearest< inner_value_type >( (outer - outer_lo_) * outer_to_inner_) + inner_lo_;
}

  double_slide_holder::value_type
  double_slide_holder::
convert_inner_to_outer( inner_value_type inner) const
  //
  // This does not clamp.
{
    return util::cast_nearest< value_type >( (inner - inner_lo_) * inner_to_outer_) + outer_lo_;
}

// _______________________________________________________________________________________________

  void
  double_slide_holder::
setup_conversions( value_type outer_lo, value_type outer_hi)
{
    d_assert( outer_lo != outer_hi);

    // Set up conversions.
    outer_lo_       = outer_lo;
    inner_to_outer_ = (outer_hi - outer_lo) / (inner_hi_ - inner_lo_);
    outer_to_inner_ = 1 / inner_to_outer_;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  bool
  double_slide_holder::
is_valid( ) const
{
    return
        // The inner holder should be created and set up.
        p_inner_holder_ &&
        p_inner_holder_->is_valid( ) &&

        // The conversion factors cannot be zero.
        (outer_to_inner_ != 0) &&
        (inner_to_outer_ != 0) &&

        // The conversion factors must have the same sign.
        ((outer_to_inner_ < 0) == (inner_to_outer_ < 0)) &&

        // The conversion factors are inverses of each other.
        ( ((-1 <= outer_to_inner_) && (outer_to_inner_ <= +1)) ==
          ((inner_to_outer_ <= -1) || (+1 <= inner_to_outer_)) ) &&

        // The inner limits should be equal to, or at least close to, the guidance limits.
        ( ( p_inner_holder_->get_min_value( )      ==  inner_lo_     ) ||
          ( p_inner_holder_->get_min_value( )      == (inner_lo_ + 1)) ||
          ((p_inner_holder_->get_min_value( ) + 1) ==  inner_lo_     ) ) &&
        ( ( p_inner_holder_->get_max_value( )      ==  inner_hi_     ) ||
          ( p_inner_holder_->get_max_value( )      == (inner_hi_ + 1)) ||
          ((p_inner_holder_->get_max_value( ) + 1) ==  inner_hi_     ) ) ;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* ctor */
  double_slide_holder::
double_slide_holder
 (  QObject *   p_parent
  , value_type  outer_init  // = 0.0
  , value_type  outer_lo    // = -1.0
  , value_type  outer_hi    // = +1.0
 )
  : holder_base_type( p_parent)
  , p_inner_holder_ ( 0)
  , outer_lo_       ( 0)
  , inner_to_outer_ ( 0)
  , outer_to_inner_ ( 0)
{
    setup_conversions( outer_lo, outer_hi);

    // Now that conversions are set up, create the inner holder.
    // Should we use the global lo/hi limits, or the calculated limits? We'll use the
    // calculated limits for now.
    p_inner_holder_ =
        new inner_holder_type
             (  this
              , convert_outer_to_inner( outer_init)
              , convert_outer_to_inner( outer_lo)
              , convert_outer_to_inner( outer_hi)
             );
    d_assert( p_inner_holder_);

    // Relay all the signals from the inner holder.
    d_verify( connect(
        p_inner_holder_, SIGNAL( has_changed( )),
        this, SLOT( intercept__has_changed( ))
    ));
    d_verify( connect(
        p_inner_holder_, SIGNAL( has_changed( int)),
        this, SLOT( intercept__has_changed( int))
    ));
    d_verify( connect(
        p_inner_holder_, SIGNAL( has_changed__range( int, int)),
        this, SLOT( intercept__has_changed__range( int, int))
    ));
    d_verify( connect(
        p_inner_holder_, SIGNAL( has_changed__steps( int, int)),
        this, SLOT( intercept__has_changed__steps( int, int))
    ));

    // Should always be true when outside class methods.
    d_assert( is_valid( ));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* slot (protected) */
  void
  double_slide_holder::
intercept__has_changed( )
{
    // These may come from us or from the UI.
    // If they come from us we will signal has_changed( ) later, after signaling
    // all the value changes.
    if ( ! is_setting( ) ) {
        emit holder_base_type::has_changed( );
    }
}

  /* slot (protected) */
  void
  double_slide_holder::
intercept__has_changed( inner_value_type inner_value)
{
    // These may come from us or from the UI.
    // If they come from us we will signal has_changed( ) later, after signaling
    // all the value changes.
    if ( ! is_setting( ) ) {
        value_type const outer_value = convert_inner_to_outer( inner_value);
        emit has_changed( outer_value);
    }
}

  /* slot (protected) */
  void
  double_slide_holder::
intercept__has_changed__range( inner_value_type /* inner_min */, inner_value_type /* inner_max */)
{
    // We should be the only ones setting these inner values.
    d_assert( is_setting( ));

    // Confirm that everything looks right.
    d_assert( is_valid( ));

    // Our setter will emit this signal later.
}

  /* slot (protected) */
  void
  double_slide_holder::
intercept__has_changed__steps( inner_value_type /* inner_ss */, inner_value_type /* inner_ps */)
{
    // We should be the only ones setting these inner values.
    d_assert( is_setting( ));

    // Confirm that everything looks right.
    d_assert( is_valid( ));

    // Our setter will emit this signal later.
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  double_slide_holder::
set_values
 (  value_type  val
 ,  value_type  min
 ,  value_type  max
 ,  value_type  ss   // -1 means use existing value
 ,  value_type  ps   // -1 means use existing value
 )
{
    d_assert( ! is_setting( ));

    // We generate our own signals instead of relying on the inner object.
    while_setting_value_wrapper_type wrapper( this, false);
    d_assert( is_setting( ));

    // Remember the old values.
    value_type const  old_val  =  get_value( );
    value_type const  old_min  =  get_min_value( );
    value_type const  old_max  =  get_max_value( );
    value_type const  old_ss   =  get_single_step( );
    value_type const  old_ps   =  get_page_step( );

    // Must do this before the conversions that follow.
    setup_conversions( min, max);

    // Send the values to the inner holder.
    p_inner_holder_->
      set_values
       (  convert_outer_to_inner( val)
        , convert_outer_to_inner( min)
        , convert_outer_to_inner( max)
        , convert_outer_to_inner_distance( ss)
        , convert_outer_to_inner_distance( ps)
       );

    // Find the new values. These may be slightly different due to rounding etc.
    value_type const  new_val  =  get_value( );
    value_type const  new_min  =  get_min_value( );
    value_type const  new_max  =  get_max_value( );
    value_type const  new_ss   =  get_single_step( );
    value_type const  new_ps   =  get_page_step( );

    // Emit the value signals.
    if ( (new_ss != old_ss) || (new_ps != old_ps) ) {
        emit has_changed__steps( new_ss, new_ps);
        wrapper.request_signal( );
    }
    if ( (new_min != old_min) || (new_max != old_max) ) {
        emit has_changed__range( new_min, new_max);
        wrapper.request_signal( );
    }
    if ( new_val != old_val ) {
        emit has_changed( new_val);
        wrapper.request_signal( );
    }

    // The wrapper dtor will signal has_changed( ) if appropriate.
    wrapper.done_with_no_throws( );
}

// _______________________________________________________________________________________________

  /* slot */
  void
  double_slide_holder::
set_range( value_type min, value_type max)
{
    set_values( get_value( ), min, max);
}

  /* slot */
  void
  double_slide_holder::
set_steps( value_type single_step, value_type page_step)
{
    set_values( get_value( ), get_min_value( ), get_max_value( ), single_step, page_step);
}

// _______________________________________________________________________________________________

  /* slot */
  void
  double_slide_holder::
set_value( value_type new_value)
{
    // set_value_base( this, value_, new_value);
    set_values( new_value, get_min_value( ), get_max_value( ));
}

  /* slot */
  void
  double_slide_holder::
set_min_value( value_type new_min_value)
{
    set_values( get_value( ), new_min_value, get_max_value( ));
}

  /* slot */
  void
  double_slide_holder::
set_max_value( value_type new_max_value)
{
    set_values( get_value( ), get_min_value( ), new_max_value);
}

  /* slot */
  void
  double_slide_holder::
set_single_step( value_type new_single_step)
{
    set_values( get_value( ), get_min_value( ), get_max_value( ), new_single_step);
}

  /* slot */
  void
  double_slide_holder::
set_page_step( value_type new_page_step)
{
    set_values( get_value( ), get_min_value( ), get_max_value( ), get_single_step( ), new_page_step);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// double_slide_animator_type - class
// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  namespace /* anonymous */ {
  double
guess_min_delta_per_sec( double_slide_holder * p_holder, double init_value, double min_value)
  //
  // This function, and really this entire class, is almost a copy of code in int_holder.cpp.
  // The two animators should be rolled together.
{
    if ( min_value != 0 ) return min_value;

    d_assert( p_holder);
    double const range = p_holder->get_range( );
    d_assert( range > 0);
    double guess = range / 1000;
    d_assert( guess > 0);

    if ( init_value != 0.0 ) {
        if ( guess > init_value ) {
            guess = init_value;
        }
    }

    return guess;
} } /* end namespace anonymous */

  namespace /* anonymous */ {
  double
guess_max_delta_per_sec( double_slide_holder * p_holder, double init_value, double max_value)
{
    if ( max_value != 0 ) return max_value;

    d_assert( p_holder);
    double const range = p_holder->get_range( );
    d_assert( range > 0);
    double guess = (range * 2) / 3;

    if ( init_value != 0 ) {
        if ( guess < init_value ) {
            guess = init_value;
        }
    }

    return guess;
} } /* end namespace anonymous */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* constructor */
  double_slide_animator_type::
double_slide_animator_type
 (  animate_type        *  p_parent
  , double_slide_holder *  p_holder
  , double                 init_delta_per_sec
  , double                 min_delta_per_sec
  , double                 max_delta_per_sec
 )
  : delta_animator_type (  p_parent
                         , false
                         , init_delta_per_sec
                         , guess_min_delta_per_sec( p_holder, init_delta_per_sec, min_delta_per_sec)
                         , guess_max_delta_per_sec( p_holder, init_delta_per_sec, max_delta_per_sec)
                        )
  , p_holder_           ( p_holder)
  { }

// _______________________________________________________________________________________________

  /* overridden virtual */
  holder_base_type *
  double_slide_animator_type::
get_animated( ) const
{
    d_assert( p_holder_);
    return p_holder_;
}

  /* overridden virtual */
  double
  double_slide_animator_type::
get_animating_value( ) const
{
    d_assert( p_holder_);
    return p_holder_->get_value( );
}

  /* overridden virtual */
  void
  double_slide_animator_type::
set_animating_value( double new_value)
{
    d_assert( p_holder_);
    p_holder_->set_value( new_value);
}

  /* overridden virtual */
  bool
  double_slide_animator_type::
is_bumping_min( )
{
    d_assert( p_holder_);
    return p_holder_->is_bumping_min( );
}

  /* overridden virtual */
  bool
  double_slide_animator_type::
is_bumping_max( )
{
    d_assert( p_holder_);
    return p_holder_->is_bumping_max( );
}

  /* overridden virtual */
  void
  double_slide_animator_type::
wrap_to_min( )
{
    d_assert( p_holder_);
    p_holder_->wrap_to_min( );
}

  /* overridden virtual */
  void
  double_slide_animator_type::
wrap_to_max( )
{
    d_assert( p_holder_);
    p_holder_->wrap_to_max( );
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// pack_holder.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
