// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// date_time.h
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
