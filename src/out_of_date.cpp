// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// out_of_date.cpp
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
// Qt supplies some timer classes:
//   QTime is a simple timer that remembers a time stamp. It is not a QObject and does not
//   participate in signals/slots.
//   QTimer can emit a periodic or one-shot signal.
//
// We usually attach this object to some radio buttons and some spin controls.
// _______________________________________________________________________________________________

# include "all.h"
# include "out_of_date.h"

// _______________________________________________________________________________________________

  /* constructor */
  out_of_date_type::
out_of_date_type( )
  : option_               ( always)
  , tick_max_out_of_date_ ( date_time::get_one_second_of_ticks( ))
  , tick_last_update_     ( date_time::get_invalid_tick_pt( ))
  , gen_max_out_of_date_  (  9) /* 9 becomes "every 10 times" */
  , gen_last_update_      ( -1)
{
    d_assert( is_set__tick_max_out_of_date( ));
}

// _______________________________________________________________________________________________

  out_of_date_type::
  second_type
  out_of_date_type::
get_seconds_max_out_of_date( ) const
{
    // Should we return a negative value when tick is not set (invalid)? Or zero?
    if ( ! is_set__tick_max_out_of_date( ) ) {
        d_assert( false); /* is this ever valid? */
        return -1.0;
    }
    second_type const seconds_out_of_date = date_time::convert_ticks_to_seconds( get_tick_max_out_of_date( ));
    d_assert( seconds_out_of_date >= 0);
    return seconds_out_of_date;
}

  void
  out_of_date_type::
set_seconds_max_out_of_date( second_type seconds)
{
    // The ticks start out invalid, so we provide a way to set it to an invalid value.
    set_tick_max_out_of_date(
        (seconds < 0) ?
            date_time::get_invalid_tick_dur( ) :
            date_time::convert_seconds_to_ticks( seconds));
}

// _______________________________________________________________________________________________

  void
  out_of_date_type::
update( gen_type gen_now)
{
    // This assumes that if the clock was not broken before it still isn't broken.
    // Otherwise we should save the last tick_now value we got when testing is_out_of_date( ),
    // or just the last valid return from date_time::get_tick_now( ).
    tick_last_update_ = date_time::get_tick_now( );
    gen_last_update_  = gen_now;
}

  bool
  out_of_date_type::
is_out_of_date( gen_type gen_now) const
  //
  // This updates itself if it returns true.
  // Only call this once when you are testing for an out-of-date condition.
{
    if ( is_always( ) ) { return true ; }
    if ( is_never(  ) ) { return false; }

    if ( is_watch_tick( ) ) { return is_out_of_date_tick( ); }
    if ( is_watch_gen(  ) ) { return is_out_of_date_gen( gen_now); }

    // Always true first time.
    // Checks only gen if clock is broken.
    // Checks only tick if gen not available.
    if ( is_watch_tick_and_gen( ) ) {
        return is_out_of_date_tick( ) && is_out_of_date_gen( gen_now);
    }

    d_assert( is_watch_tick_or_gen( ));
    tick_point_type const  tick_now        = date_time::get_tick_now( );
    bool            const  is_tick_broken  = date_time::is_invalid_tick_pt( tick_now);
    bool            const  is_gen_broken   = (gen_now  < 0);

    // If both are broken (or this is the first time), return true.
    if ( is_tick_broken && is_gen_broken ) {
        return true;
    }
    // If clock is broken, only check gen.
    if ( is_tick_broken ) {
        return is_out_of_date_gen( gen_now);
    }
    // If gen is not available, only check clock.
    if ( is_gen_broken ) {
        return is_out_of_date_tick( tick_now);
    }
    // Normal check.
    return is_out_of_date_tick( tick_now) || is_out_of_date_gen( gen_now);
}

  bool
  out_of_date_type::
is_out_of_date_gen( gen_type gen_now /* = get_gen_now( ) */) const
{
    // Always true if generation is not valid (first time).
    // This same test would work on time since an invalid time is also one that is < 0.
    return
        (gen_now < gen_last_update_) ||
        (gen_last_update_ < 0) ||
        ((gen_now - gen_last_update_) > gen_max_out_of_date_);
}

  bool
  out_of_date_type::
is_out_of_date_tick( ) const
{
    return is_out_of_date_tick( date_time::get_tick_now( ));
}

  bool
  out_of_date_type::
is_out_of_date_tick( tick_point_type tick_now /* = get_tick_now( ) */) const
{
    // Always true if ticks are not valid (first time, or clock is broken).
    if ( date_time::is_invalid_tick_pt(  tick_now             ) ||
         date_time::is_invalid_tick_pt(  tick_last_update_    ) ||
         date_time::is_invalid_tick_dur( tick_max_out_of_date_) )
    {
        return true;
    }

    // If it looks like we're moving backwards in time (tick_now is before tick_last_update)
    // then something is screwy. Our default action is to return true and make it look like
    // things are out-of-date.
    if ( tick_now < tick_last_update_ ) {
        d_assert( false);
        return true; /* we're moving backwards in time? */
    }

    // You should only subtract one time from another if the first time is >= the second.
    tick_duration_type const elapsed_ticks = tick_now - tick_last_update_;
    return elapsed_ticks > tick_max_out_of_date_;
}

// _______________________________________________________________________________________________

  bool
  out_of_date_type::
is_set__tick_max_out_of_date( ) const
{
    d_assert(
        (date_time::get_invalid_tick_dur( ) == tick_max_out_of_date_) ||
        date_time::is_valid_tick_dur( tick_max_out_of_date_));
    return date_time::is_valid_tick_dur( tick_max_out_of_date_);
}

  void
  out_of_date_type::
set_tick_max_out_of_date( tick_duration_type max)
{
    d_assert(
        (date_time::get_invalid_tick_dur( ) == max) || date_time::is_valid_tick_dur( max));
    tick_max_out_of_date_ = max;
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// out_of_date.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
