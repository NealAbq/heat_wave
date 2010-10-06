// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// out_of_date.h
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
# ifndef OUT_OF_DATE_H
# define OUT_OF_DATE_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "date_time.h"

// _______________________________________________________________________________________________

  class
out_of_date_type
{
  // -------------------------------------------------------------------------------------------
  // Types and type names
  public:
    typedef date_time::tick_point_type     tick_point_type    ;
    typedef date_time::tick_duration_type  tick_duration_type ;
    typedef date_time::second_type         second_type        ;

    typedef int                            gen_type           ;
    // gen_type can be a sheet-generation or a draw-generation:
    //   sheet_control.get_sheet_generation( )
    //   heat_widget.get_draw_count( )

    enum e_option
          {  always
           , never
           , watch_tick
           , watch_gen
           , watch_tick_and_gen  /* out-of-date if both tick and gen are */
           , watch_tick_or_gen   /* out-of-date if either tick and gen are */
          };

  // -------------------------------------------------------------------------------------------
  // Consructor
  public:
    /* ctor */  out_of_date_type( )                          ;

  // Disable copy
  private:
    /* copy */  out_of_date_type( out_of_date_type const &)  ; /* no implementation */
    void        operator =(       out_of_date_type const &)  ; /* no implementation */

  // -------------------------------------------------------------------------------------------
  // Setters
  public:
    void        set_option( e_option opt)                    { option_ = opt; }

    // These can be negative, in which case it's like they're not set and they will always
    // evalulate as out-of-date.
    void        set_tick_max_out_of_date( tick_duration_type);
    void        set_seconds_max_out_of_date( second_type)    ;
    void        set_gen_max_out_of_date( gen_type max)       { gen_max_out_of_date_ = max; }

  // -------------------------------------------------------------------------------------------
  // Getters
  public:
    bool        is_always( )                           const { return option_ == always; }
    bool        is_never( )                            const { return option_ == never; }
    bool        is_watch_tick( )                       const { return option_ == watch_tick; }
    bool        is_watch_gen( )                        const { return option_ == watch_gen; }
    bool        is_watch_tick_and_gen( )               const { return option_ == watch_tick_and_gen; }
    bool        is_watch_tick_or_gen( )                const { return option_ == watch_tick_or_gen; }

    bool        is_set__tick_max_out_of_date( )        const ;
    tick_duration_type
                get_tick_max_out_of_date( )            const { return tick_max_out_of_date_; }

    second_type get_seconds_max_out_of_date( )         const ;

    bool        is_set__gen_max_out_of_date( )         const { return 0 <= gen_max_out_of_date_; }
    gen_type    get_gen_max_out_of_date( )             const { return gen_max_out_of_date_; }

  // -------------------------------------------------------------------------------------------
  // Main functions
  public:
    bool        is_out_of_date( gen_type)              const ;

    bool        is_out_of_date_tick( tick_point_type)  const ;
    bool        is_out_of_date_tick( )                 const ;

    bool        is_out_of_date_gen( gen_type)          const ;

    void        update( gen_type)                            ;

  // -------------------------------------------------------------------------------------------
  // Member vars
  private:
    e_option            option_               ;

    tick_duration_type  tick_max_out_of_date_ ;
    tick_point_type     tick_last_update_     ;

    gen_type            gen_max_out_of_date_  ;
    gen_type            gen_last_update_      ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef OUT_OF_DATE_H */
//
// out_of_date.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
