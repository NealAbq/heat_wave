// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// date_time.cpp
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
// <ctime> and clock() do not work in linux (although they do in windows).
//   I first implemented this using ::clock( ), defined in <ctime>. That implementation works
//   great in MSWindows, but fails on linux. clock( ) on linux (gcc) apparently measures cpu time
//   spent on the process or the thread(?). When I used it it was like using a clock about 40-100
//   times too fast, and it didn't work even in a single thread when drawing. Maybe the thread
//   sleeps while the graphics accelerator works?
//
// CLOCKS_PER_SEC (from <ctime>):
//   1000 in windows, 1000000 in linux
//
// Timers are not very nailed down yet in C++. XTime may become a standard lib component, and
// you can use it now (thread library, boost/thread/xtime.hpp).
// The boost timer lib (boost/timer.hpp) is probably defunct and relies on clock().
// The boost date_time lib (which we use here) works well.
//
// Qt supplies some timer classes:
//   QTime is a simple timer that remembers a time stamp. It is not a QObject and does not
//   participate in signals/slots.
//   QTimer can emit a periodic or one-shot signal.
//   QBasicTimer is a stripped-down QTimer.
//   I haven't tested to see if these work outside windows.
// _______________________________________________________________________________________________

# include "all.h"
# include "date_time.h"

// _______________________________________________________________________________________________
//
namespace date_time {
// _______________________________________________________________________________________________

  tick_point_type
get_tick_now( )
{
    tick_point_type const now = boost::posix_time::microsec_clock::universal_time( );
    d_assert( is_valid_tick_pt( now));
    return now;
}

  tick_duration_type
get_zero_duration_of_ticks( )
{
    return tick_duration_type( 0, 0, 0);
}

  tick_duration_type
get_one_second_of_ticks( )
{
    return tick_duration_type( 0, 0, 1);
}

  tick_point_type
get_invalid_tick_pt( )
{
    return tick_point_type( boost::posix_time::not_a_date_time);
}

  bool
is_invalid_tick_pt( tick_point_type tick)
{
    return tick.is_special( );
}

// _______________________________________________________________________________________________

  tick_duration_type
get_invalid_tick_dur( )
{
    return tick_duration_type( boost::posix_time::not_a_date_time);
}

  bool
is_invalid_tick_dur( tick_duration_type tick)
{
    return tick.is_special( );
}

  bool
is_valid_tick_dur( tick_duration_type tick)
{
    return ! tick.is_special( );
}

// _______________________________________________________________________________________________

  second_type
convert_ticks_to_seconds( tick_duration_type tick_count)
  {
    d_assert( is_valid_tick_dur( tick_count));
    long const microsecs = tick_count.total_microseconds( );
    return second_type( microsecs) / 1000000.0;
  }

  millisecond_type
convert_ticks_to_milliseconds( tick_duration_type tick_count)
  {
    d_assert( is_valid_tick_dur( tick_count));
    long const microsecs = tick_count.total_microseconds( );
    return millisecond_type( microsecs) / 1000.0;
  }

// _______________________________________________________________________________________________

  tick_duration_type
convert_seconds_to_ticks( second_type seconds)
{
    d_assert( seconds >= 0);

    second_type const  min_as_real  = 0.99 * second_type( boost::integer_traits< long >::const_min);
    second_type const  max_as_real  = 0.99 * second_type( boost::integer_traits< long >::const_max);

    // Check for very very large values.
    // Test coverage alert.
    if ( (seconds < min_as_real) || (seconds > max_as_real) ) {
        second_type const  minutes  = seconds / 60.0;
        if ( (minutes < min_as_real) || (minutes > max_as_real) ) {
            second_type const  hours  = minutes / 60.0;
            if ( hours < min_as_real ) {
                d_assert( false);
                return boost::posix_time::neg_infin;
            }
            if ( hours > max_as_real ) {
                d_assert( false);
                return boost::posix_time::pos_infin;
            }
            return boost::posix_time::hours( long( std::floor( hours + 0.5)));
        }
        return boost::posix_time::minutes( long( std::floor( minutes + 0.5)));
    }

    second_type const  a_million     = 1000000.0;
    second_type const  microseconds  = seconds * a_million;
    if ( (microseconds < min_as_real) || (microseconds > max_as_real) ) {

        second_type const  a_thousand( 1000.0);
        second_type const  milliseconds  = seconds * a_thousand;
        if ( (milliseconds < min_as_real) || (milliseconds > max_as_real) ) {

            return boost::posix_time::seconds( long( std::floor( seconds + 0.5)));
        }
        return boost::posix_time::milliseconds( long( std::floor( milliseconds + 0.5)));
    }
    return boost::posix_time::microseconds( long( std::floor( microseconds + 0.5)));
}

// _______________________________________________________________________________________________

  tick_duration_type
convert_milliseconds_to_ticks( millisecond_type milliseconds)
{
    return convert_seconds_to_ticks( milliseconds / 1000.0);
}

// _______________________________________________________________________________________________

  namespace /* anonymous */ {
second_type const  msecs_per_sec  = second_type( 1000);
second_type const  secs_per_msec  = second_type( 1) / msecs_per_sec;
  } /* end namespace anonymous */

  millisecond_type
convert_seconds_to_milliseconds( second_type seconds)
{
    return seconds * msecs_per_sec;
}

  second_type
convert_milliseconds_to_seconds( millisecond_type milliseconds)
{
    return milliseconds * secs_per_msec;
}

// _______________________________________________________________________________________________
//
} /* end namespace date_time */
// _______________________________________________________________________________________________

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Commented out
// The following is the former implementation of date_time using <ctime>, clock(), and clock_t.
// It worked great in MSWindows, but terribly in Linux (gcc).
# if 0
// _______________________________________________________________________________________________

# include <ctime>

typedef double   real_type     ;
typedef clock_t  tick_point_type  ; /* time point */
typedef clock_t  tick_duration_type ; /* time duration */

  namespace /* anonymous */ {
real_type const  ticks_per_sec   = static_cast< real_type >( CLOCKS_PER_SEC);
real_type const  secs_per_tick   = static_cast< real_type >( 1) / ticks_per_sec;

real_type const  msecs_per_sec   = static_cast< real_type >( 1000);
real_type const  secs_per_msec   = static_cast< real_type >( 1) / msecs_per_sec;

real_type const  msecs_per_tick  = secs_per_tick * msecs_per_sec;
real_type const  ticks_per_msec  = static_cast< real_type >( 1) / msecs_per_tick;
  } /* end namespace anonymous */

// _______________________________________________________________________________________________

  tick_point_type
get_tick_now( )
{
    tick_point_type const now = ::clock( );
    // The tick from ::clock( ) should always be positive.
    d_assert( is_valid_tick_pt( now));
    return now;
}

  tick_duration_type
get_zero_duration_of_ticks( )
{
    return 0;
}

  tick_duration_type
get_one_second_of_ticks( )
{
    return CLOCKS_PER_SEC;
}

  tick_point_type
get_invalid_tick_pt( )
{
    return -1;
}

  bool
is_invalid_tick_pt( tick_point_type tick)
{
    // We allow tick to have a zero value because it might be a delta.
    // If it's a tick from ::clock( ) it should never be zero.
    d_assert( (-1 == tick) || (tick >= 0));
    return tick < 0;
}

// _______________________________________________________________________________________________

  second_type
convert_ticks_to_seconds( real_type tick_count)
  // Should be generic.
{
    d_assert( is_valid_tick_dur( tick_count));
    return tick_count * secs_per_tick;
}

  millisecond_type
convert_ticks_to_milliseconds( real_type tick_count)
  // Should be generic.
{
    d_assert( is_valid_tick_dur( tick_count));
    return tick_count * msecs_per_tick;
}

  second_type
convert_ticks_to_seconds( tick_duration_type tick_count /* can be zero, not negative */)
  {
    d_assert( is_valid_tick_dur( tick_count));
    return convert_ticks_to_seconds( static_cast< real_type >( tick_count));
  }

  millisecond_type
convert_ticks_to_milliseconds( tick_duration_type tick_count /* can be zero, not negative */)
  {
    d_assert( is_valid_tick_dur( tick_count));
    return convert_ticks_to_milliseconds( static_cast< real_type >( tick_count));
  }

  tick_duration_type
convert_seconds_to_ticks( second_type seconds)
{
    d_assert( seconds >= 0);
    return
        static_cast< tick_duration_type >(
            std::floor( (seconds * ticks_per_sec) + 0.5));
}

  tick_duration_type
convert_milliseconds_to_ticks( millisecond_type milliseconds)
{
    d_assert( milliseconds >= 0);
    return
        static_cast< tick_duration_type >(
            std::floor( (milliseconds * ticks_per_msec) + 0.5));
}

  millisecond_type
convert_seconds_to_milliseconds( second_type seconds)
{
    return seconds * msecs_per_sec;
}

  second_type
convert_milliseconds_to_seconds( millisecond_type milliseconds)
{
    return milliseconds * secs_per_msec;
}

// _______________________________________________________________________________________________
//
# endif
// End of former <ctime> implementation.
// _______________________________________________________________________________________________

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// date_time.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
