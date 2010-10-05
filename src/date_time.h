// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// date_time.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef DATE_TIME_H
# define DATE_TIME_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "debug.h"

//Remove this #define if you need to compile and link the boost date_time lib.
# define BOOST_DATE_TIME_NO_LIB
# include <boost/date_time/posix_time/posix_time_types.hpp>

// _______________________________________________________________________________________________
//
namespace date_time {
// _______________________________________________________________________________________________

typedef boost::posix_time::ptime          tick_point_type      ;
typedef boost::posix_time::time_duration  tick_duration_type   ;

typedef double                            second_type          ; /* duration */
typedef second_type                       millisecond_type     ; /* duration */

// _______________________________________________________________________________________________

  extern
  tick_point_type
get_tick_now( )
  ;

  extern
  tick_duration_type
get_zero_duration_of_ticks( )
  ;

  extern
  tick_duration_type
get_one_second_of_ticks( )
  ;

// _______________________________________________________________________________________________

  extern
  tick_point_type
get_invalid_tick_pt( )
  ;

  extern
  bool
is_invalid_tick_pt( tick_point_type)
  ;

  inline
  bool
is_valid_tick_pt( tick_point_type tick)
  {
    return ! is_invalid_tick_pt( tick);
  }

// _______________________________________________________________________________________________

  extern
  tick_duration_type
get_invalid_tick_dur( )
  ;

  extern
  bool
is_invalid_tick_dur( tick_duration_type)
  ;

  extern
  bool
is_valid_tick_dur( tick_duration_type)
  ;

// _______________________________________________________________________________________________

  extern
  second_type
convert_ticks_to_seconds( tick_duration_type /* can be zero, must be valid */)
  ;

  extern
  millisecond_type
convert_ticks_to_milliseconds( tick_duration_type /* can be zero, must be valid */)
  ;

// _______________________________________________________________________________________________

  extern
  tick_duration_type
convert_seconds_to_ticks( second_type /* must not be negative */)
  ;

  extern
  tick_duration_type
convert_milliseconds_to_ticks( millisecond_type /* must not be negative */)
  ;

  extern
  millisecond_type
convert_seconds_to_milliseconds( second_type /* can be negative */)
  ;

  extern
  second_type
convert_milliseconds_to_seconds( millisecond_type /* can be negative */)
  ;

// _______________________________________________________________________________________________
//
} /* end namespace date_time */
// _______________________________________________________________________________________________

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef DATE_TIME_H */
//
// date_time.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
