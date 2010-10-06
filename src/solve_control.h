// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// solve_control.h
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
# ifndef SOLVE_CONTROL_H
# define SOLVE_CONTROL_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "debug.h"
# include "date_time.h"
# include "moving_sum.h"
# include "sheet.h"
# include "heat_solver.h"

# include <QtCore/QObject>
# include <QtCore/QTimer>

// _______________________________________________________________________________________________

  class
sheet_control_type
  : public QObject
{
  Q_OBJECT

  // _______________________________________________________________________________________________
  // Typedefs

  private:
    typedef sheet_control_type             this_type          ;
  public:
    typedef heat_solver::control_type      heat_solver_type   ;

    typedef sheet_type::size_type          size_type          ; /* integer, >= 0 */
    typedef sheet_type::value_type         value_type         ; /* values in the sheet */

    //typedef float                        rsize_type         ; /* 0 .. +1 */
    //typedef rsize_type                   rcoord_type        ; /* -1 .. +1 */

    typedef float                          radian_type        ; /* angle in radians */

    typedef size_type                      coord_type         ; /* 0 .. size-1 */
    typedef int                            gen_type           ; /* generation count */

    typedef date_time::second_type         second_type        ; /* seconds duration type */
    typedef date_time::millisecond_type    millisecond_type   ; /* milliseconds duration type */
    typedef date_time::tick_duration_type  tick_duration_type ;
    typedef date_time::tick_point_type     tick_point_type    ;

  // _______________________________________________________________________________________________
  // Construct/destruct
  public:
    /* ctor */      sheet_control_type( QObject * p_parent = 0)    ;
    /* dtor */      ~sheet_control_type( )                         ;

  // Debug-only
  public:
  # ifdef NDEBUG
    void            assert_valid( )                     const { }
  # else
    void            assert_valid( )                     const ;
  # endif

  // _______________________________________________________________________________________________
  // Auto solve control
  public:
    heat_solver_type *
                    get_heat_solver( )                        { return d_verify_not_zero( p_heat_solver_); }

    void            start_auto_solving( )                     ;
    void            stop_auto_solving( )                      ;
  public slots:
    void            start_stop_auto_solving( bool)            ;
    void            single_step_solve( )                      ;

  protected:
    void            init_solver( )                            ;
    void            solve_next( )                             ;

  private slots:
    void            finished__from_solver( )                  ;

  signals:
    void            auto_solving_started( bool)               ;
    //void          auto_solving_started( )                   ;
    //void          auto_solving_stopped( )                   ;

  // _______________________________________________________________________________________________
  // Duration statistics
  public:
    bool            is_available_duration_worker_thread( )             const ;
    bool            is_available_duration_control_thread( )            const ;
    bool            is_available_duration_auto_solve_cycle( )          const ;

    second_type     get_average_duration_worker_thread_seconds( )      const ;
    second_type     get_average_duration_control_thread_seconds( )     const ;
    second_type     get_average_duration_auto_solve_cycle_seconds( )   const ;

    millisecond_type  get_average_duration_worker_thread_mseconds( )     const ;
    millisecond_type  get_average_duration_control_thread_mseconds( )    const ;
    millisecond_type  get_average_duration_auto_solve_cycle_mseconds( )  const ;

    void            clear_solve_stats( )                      ;

  protected:
    void            record_start_solve( )                     ;
    void            record_finish_solve( )                    ;

  // _______________________________________________________________________________________________
  // Requests
  protected:
    bool            are_requests_delayed( )             const ;
    void            honor_requests( )                         ;

  // _______________________________________________________________________________________________
  // Slots (requests) for changing the sheet values
  public slots: /* requests */
    void            request_set_init_test( )                  ;
    void            request_set_sheet_random_noise( )         ;
    void            request_normalize_sheet( )                ;
    void            request_scale_sheet( float, float)        ;
    void            request_flatten_sheet( )                  ;
    void            request_flatten_values( )                 ;
    void            request_flatten_momentum( )               ;
    void            request_half_values( )                    ;
    void            request_half_momentum( )                  ;
    void            request_half_both( )                      ;
    void            request_ramp_corner_to_corner( )          ;
    void            request_ramp_2_corners( )                 ;
    void            request_ramp_4_corners( )                 ;
    void            request_bell_corner_1( )                  ;
    void            request_bell_corner_2( )                  ;
    void            request_bell_corner_3( )                  ;
    void            request_bell_corner_4( )                  ;
    void            request_raindrop_up( )                    ;
    void            request_raindrop_down( )                  ;
    void            request_bell_curve_1( )                   ;
    void            request_bell_curve_2( )                   ;
    void            request_bell_curve_4( )                   ;
    void            request_sin_over_dist_1( )                ;
    void            request_sin_over_dist_2( )                ;
    void            request_sin_over_dist_4( )                ;
    void            request_delta( )                          ;
    void            request_stair_steps( )                    ;
    void            request_reverse_wave( )                   ;
  protected:
    void            set_init_test( )                          ;
    void            set_sheet_random_noise( )                 ;
    void            normalize_sheet( )                        ;

    void            scale_sheet
                     (  float  values_scale    = 1.0f
                      , float  momentum_scale  = 1.0f
                     )                                        ;

    void            ramp_corner_to_corner( )                  ;
    void            ramp_2_corners( )                         ;
    void            ramp_4_corners( )                         ;
    void            bell_corner_1( )                          ;
    void            bell_corner_2( )                          ;
    void            bell_corner_3( )                          ;
    void            bell_corner_4( )                          ;

    void            raindrop_up( )                            ;
    void            raindrop_down( )                          ;
    void            raindrop_random
                     (  value_type  xy_delta_lo  /* lo limit of width of raindrop */
                      , value_type  xy_delta_hi  /* hi limit of width of raindrop */
                      , value_type  z_delta_lo   /* lo limit of depth of the raindrop */
                      , value_type  z_delta_hi   /* hi limit of depth of the raindrop */
                     )                                        ;

    void            set_bell_curve_1( )                       ;
    void            set_bell_curve_2( )                       ;
    void            set_bell_curve_4( )                       ;
    void            set_sin_over_dist_1( )                    ;
    void            set_sin_over_dist_2( )                    ;
    void            set_sin_over_dist_4( )                    ;
    void            set_delta( )                              ;
    void            set_stair_steps( )                        ;
    bool            reverse_wave( )                           ;

  // _______________________________________________________________________________________________
  // Setting values in the sheet
# if 0
  protected:
    bool            set_sheet_at
                     (  coord_type  x_lo
                      , coord_type  x_hi_plus
                      , coord_type  y_lo
                      , coord_type  y_hi_plus
                      , value_type  value
                     )                                        ;
    bool            set_sheet_at_margin_rect
                     (  coord_type  x_lo_margin
                      , coord_type  x_hi_margin
                      , coord_type  y_lo_margin
                      , coord_type  y_hi_margin
                      , value_type  value
                     )                                        ;
    bool            set_sheet_at_size_rect
                     (  coord_type  x_lo
                      , size_type   x_size
                      , coord_type  y_lo
                      , size_type   y_size
                      , value_type  value
                     )                                        ;
    bool            set_sheet_at
                     (  coord_type  x
                      , coord_type  y
                      , value_type  value
                     )                                        ;
# endif

  // _______________________________________________________________________________________________
  // Internal methods for setting sheet values
  protected:
    void            flatten_next_sheet( value_type = 0)       ;
    bool            set_next_sheet_random_rect
                     (  size_type  x_size
                      , size_type  y_size
                     )                                        ;
    bool            set_next_sheet_at
                     (  coord_type  x
                      , coord_type  y
                      , value_type  value
                     )                                        ;
    bool            set_next_sheet_at
                     (  coord_type  x_lo
                      , coord_type  x_hi_plus
                      , coord_type  y_lo
                      , coord_type  y_hi_plus
                      , value_type  value
                     )                                        ;
    bool            set_next_sheet_at_size_rect
                     (  coord_type  x_lo
                      , size_type   x_size
                      , coord_type  y_lo
                      , size_type   y_size
                      , value_type  value
                     )                                        ;
    bool            set_next_sheet_at_margin_rect
                     (  coord_type  x_lo_margin
                      , coord_type  x_hi_margin
                      , coord_type  y_lo_margin
                      , coord_type  y_hi_margin
                      , value_type  value
                     )                                        ;

  // _______________________________________________________________________________________________
  // Sheet maintenance and generations
  protected:
    void            init_sheets( )                            ;

    enum            e_next_sheet_init_type
                     {  e_do_not_init_next_sheet
                      , e_flatten_next_sheet
                      , e_copy_current_to_next_sheet
                     }                                        ;
    void            prepare_for_transform
                     (  e_next_sheet_init_type  next_sheet_init        = e_copy_current_to_next_sheet
                      , bool                    is_history_meaningful  = true
                     )                                        ;

    void            after_transform__cancel( )                ;
    void            after_transform( )                        ;
    void            transform__reverse_wave( )                ;
    void            after_solve( )                            ;

    void            increment_generation( gen_type amount = 1);
    void            copy_current_to_next_sheet( )             ;
    void            copy_current_edges_to_next_sheet( )       ;

    void            maybe_stamp_history_on_extra_sheet( bool) ;
    void            maybe_scale_saved_history( value_type)    ;
    void            restore_history_in_extra_sheet_if_available( )
                                                              ;

    void            maybe_do_edge_fixing( )                   ;
    void            maybe_do_center_freeze( )                 ;
    void            maybe_do_vortex( )                        ;
  public slots:
    void            set__are_edges_fixed( bool)               ;
    void            set__is_center_frozen( bool)              ;
    void            set__is_vortex_on( bool)                  ;
  public:
    bool            are_edges_fixed( )                  const { return are_edges_fixed_; }
    bool            is_center_frozen( )                 const { return is_center_frozen_; }
    bool            is_vortex_on( )                     const { return is_vortex_on_; }
    bool            is_auto_solving( )                  const { return is_auto_solving_; }
    bool            is_next_solve_pending( )            const { return is_next_solve_pending_; }
    bool            is_sheet_change_expected_soon( tick_duration_type tick_count_to_wait)
                                                        const ;

  public:
    sheet_type const &
                    get_sheet_for_draw( )                     ;
    gen_type        get_sheet_generation( )             const { return generation_current_; }

  signals:
    void            sheet_is_changed( )                       ;
    void            draw_size_limit_is_changed( )             ;

  // _______________________________________________________________________________________________
  // Draw-size limits - limited resolution draw
  public:
    bool            is_draw_size_limited( )             const { return is_draw_size_limited_; }
    size_type       get_xy_draw_size_limit( )           const { return xy_draw_size_limit_; }
    size_type       get_x_draw_size_limit( )            const { return x_draw_size_limit_; }
    size_type       get_y_draw_size_limit( )            const { return y_draw_size_limit_; }
    float           get_ratio_xy_sheet_to_xy_limit( )   const { return ratio_xy_sheet_to_xy_limit_; }

  # ifdef NDEBUG
    void            assert_draw_size_setup( )           const { }
  # else
    void            assert_draw_size_setup( )           const ;
  # endif

  public slots:
    void            set__is_draw_size_limited( bool)          ;
  public: /* not a slot */
    void            set_xy_draw_size_limit( size_type)        ;

  protected:
    sheet_type const *
                    maybe_get_limited_draw_sheet( )           ;

    void            init_draw_size_limits( )                  ;
    void            after_master_sheet_value_change( )        ;
    void            after_master_sheet_size_change( )         ;

    void            after_change_that_affects_limited_draw( ) ;
    void            calc_x_and_y_draw_size_limits( )          ;
    void            setup_sheet_limited_draw( )               ;

  // _______________________________________________________________________________________________
  // Sheet sizes
  public:
    size_type       get_x_size( )                       const { d_assert( p_sheet_current_);
                                                                return p_sheet_current_->get_x_count( );
                                                              }
    size_type       get_y_size( )                       const { d_assert( p_sheet_current_);
                                                                return p_sheet_current_->get_y_count( );
                                                              }

  public slots: /* requests */
    void            request_set_xy_sizes
                     (  unsigned  x_size  // we have to use "unsigned" instead of "size_type" because of qt-moc
                      , unsigned  y_size
                     )                                        ;
    void            request_set_xy_sizes
                     (  unsigned  x_size
                      , unsigned  y_size
                      , float     init_value
                     )                                        ;
  protected:
    void            set_xy_sizes
                     (  size_type  x_size
                      , size_type  y_size
                     )                                        ;
    void            set_xy_sizes
                     (  size_type   x_size
                      , size_type   y_size
                      , value_type  init_value
                     )                                        ;

  // _______________________________________________________________________________________________
  // Setting sheet values using normalized (real or floating) coords
# if 0
  protected:
    bool            set_sheet_at_r
                     (  rcoord_type  rx
                      , rcoord_type  ry
                      , value_type   value
                     )                                        ;
    bool            set_sheet_at_r
                     (  rcoord_type  rx_lo
                       , rcoord_type  rx_hi
                      , rcoord_type  ry_lo
                      , rcoord_type  ry_hi
                      , value_type   value
                     )                                        ;
    bool            set_sheet_at_margin_rect_r
                     (  rcoord_type  rx_lo_margin
                      , rcoord_type  rx_hi_margin
                      , rcoord_type  ry_lo_margin
                      , rcoord_type  ry_hi_margin
                      , value_type   value
                     )                                        ;
    bool            set_sheet_at_size_rect_r
                     (  rcoord_type  rx_lo
                      , rsize_type   rx_size
                      , rcoord_type  ry_lo
                      , rsize_type   ry_size
                      , value_type   value
                     )                                        ;
# endif

  // _______________________________________________________________________________________________
  // Coord conversions
# if 0
  public:
    static coord_type   get_coord( rcoord_type, size_type count)  ;
    static size_type    get_size( rsize_type, size_type count)    ;

    coord_type          get_x_coord( rcoord_type xn)        const { return get_coord( xn, get_x_size( )); }
    coord_type          get_y_coord( rcoord_type yn)        const { return get_coord( yn, get_y_size( )); }

    size_type           get_x_size( rsize_type xs)          const { return get_size( xs, get_x_size( )); }
    size_type           get_y_size( rsize_type ys)          const { return get_size( ys, get_y_size( )); }

    bool                is_x_coord_in_range( coord_type x)  const { return (0 <= x ) && (x  <  get_x_size( )); }
    bool                is_x_size_in_range( size_type xs)   const { return (0 <  xs) && (xs <= get_x_size( )); }

    bool                is_y_coord_in_range( coord_type y)  const { return (0 <= y ) && (y  <  get_y_size( )); }
    bool                is_y_size_in_range( size_type ys)   const { return (0 <  ys) && (ys <= get_y_size( )); }

    static bool         is_rcoord_in_range( rcoord_type r)        { return (-1 <= r) && (r <= +1); }
    static bool         is_rsize_in_range( rsize_type s)          { return ( 0 <= s) && (s <= +1); }
# endif

  // _______________________________________________________________________________________________
  // Member vars

  // --------------------------------------------------------
  // Sheets, arrays of numbers that we draw and solve
  private:
    // p_sheet_current_ alternates between pointing to sheet_a_ and sheet_b_.
    // p_sheet_current_ is the sheet that is painted and read from the outside.
    // p_sheet_current_ can be changed, but only when there is no solve currently in progress.
    sheet_type            *  p_sheet_current_                             ;

    // p_sheet_next_ is only used internally. It is the sheet that the solver is writing to.
    // When the solver is finished, the current and next sheets swap places.
    sheet_type            *  p_sheet_next_                                ;

    // We use the extra sheet for 2 things:
    //
    //  1 To (sometimes) save momentum (history) during some sheet transforms. We don't have to
    //    have to have an extra sheet to do this (we can do it with just current and next) if
    //    we're willing to write on current. This will not be wise if we ever move the transforms
    //    into another thread.
    //
    //  2 When we are solving multiple generations in the worker thread without reporting back
    //    between solves, we need an extra sheet unless the solver can work in place and requires
    //    no history. We also need an extra sheet to return history when multi-solving (N>1) because
    //    the current sheet ends up being several generations behind and the next sheet is of course
    //    being used to return the final result.
    sheet_type            *  p_sheet_extra_                               ;

    // The three pointers above swap around between these three sheets.
    sheet_type               sheet_a_                                     ;
    sheet_type               sheet_b_                                     ;
    sheet_type               sheet_c_                                     ;

    bool                     is_history_delta_in_extra_sheet_             ;
    bool                     is_next_sheet_valid_history_                 ;

    // Is pending means the worker thread is currently performing a solve.
    // In this case the current sheet is locked. It can be read but not changed.
    bool                     is_next_solve_pending_                       ;

    // True when the edges of the sheets are frozen in place.
    bool                     are_edges_fixed_                             ;
    bool                     is_center_frozen_                            ;
    bool                     is_vortex_on_                                ;

  // --------------------------------------------------------
  // Solving engine
  private:
    bool                     is_auto_solving_                             ;
    bool                     is_auto_solve_just_started_                  ;
    bool                     is_auto_solve_just_stopped_                  ;
    heat_solver_type *       p_heat_solver_                               ;

  // Keeping track of the solve generation
  private:
    // The "current" generation is the result of the "last" solve.
    gen_type                 generation_current_                          ;

  // --------------------------------------------------------
  // Timing statistics
  private:
    // We use this (along with next_solve_start_tick_) to sometimes delay drawing when we expect
    // the solver to update very soon.
    // Maybe we should move this to the animate object?
    tick_duration_type       last_solve_tick_duration_                    ;

    // This lets us calculate start_finish_durations__in_seconds__from_control_thread_.
    // This is only set when a solve is pending. Otherwise it is invalid.
    tick_point_type          next_solve_start_tick_                       ;

    // We keep a moving sum (and thus average) of all the recent timings.
    // We don't keep these in holders and we do not emit signals every time they change since they
    // potentially change every time solve updates.
    // Improve. These are stored as doubles for now, but they should be stored as integer, probably
    // unsigned 64-bit (milli- or micro-seconds).
    // moving_sum<..> subtracts and adds values to a running total, and it can drift and get more
    // inaccurate as time goes on.
    // These should probably be moving_sum< tick_duration_type >, especially the last two.
    moving_sum< double >     start_finish_durations__in_seconds__from_worker_thread_  ;
    moving_sum< double >     start_finish_durations__in_seconds__from_control_thread_ ;
    moving_sum< double >     start_start_durations__in_seconds__during_auto_solve_    ;

  // --------------------------------------------------------
  // Speed control for auto-solve
  private:
    // These are -1 when auto-solve is turned off.
    tick_point_type          last_auto_solve_start_tick_                  ;
    tick_point_type          last_auto_solve_finish_tick_                 ;

    // Requests to slow down solving during auto-solve.
    // These are not implemented yet.
    tick_duration_type       min_ticks_auto_solve_finish_to_start_        ; /* min gap time, a percent could be more useful */
    tick_duration_type       min_ticks_auto_solve_start_to_start_         ; /* min cycle time */
    // We could also ask for a duty cycle like 50%.
    // We have two things we can ask for:
    //   Fixed frequency
    //   Percent of potential max frequency

    // This is only used during auto-solve, and only when we are not running full-speed.
    // We use this timer for one-shot wakeups.
    QTimer                *  p_wakeup_for_next_solve_                     ;

  // --------------------------------------------------------
  // Limited draw, to reduce the number of polygons used to draw
  private:
    bool                     is_draw_size_limited_                        ;
    bool                     is_limited_draw_sheet_written_               ;
    size_type                xy_draw_size_limit_                          ;
    size_type                x_draw_size_limit_                           ;
    size_type                y_draw_size_limit_                           ;
    float                    ratio_xy_sheet_to_xy_limit_                  ;
    sheet_type               sheet_limited_draw_                          ;

  // --------------------------------------------------------
  // Requests
  private:
    // Requests to change sheet values are not honored until pending solve is complete.
    // Improve: Requests should be kept in a queue.
    // Improve: Requests should be honored in the worker thread, parallel if possible.
    //   The UI thread should probably never change sheet values and only access them
    //   in order to draw them.
    bool                     is_requested_scale_sheet_                    ;
    float                    requested_values_scale_                      ;
    float                    requested_momentum_scale_                    ;

    bool                     is_requested_ramp_corner_to_corner_          ;
    bool                     is_requested_ramp_2_corners_                 ;
    bool                     is_requested_ramp_4_corners_                 ;
    bool                     is_requested_bell_corner_1_                  ;
    bool                     is_requested_bell_corner_2_                  ;
    bool                     is_requested_bell_corner_3_                  ;
    bool                     is_requested_bell_corner_4_                  ;
    bool                     is_requested_raindrop_up_                    ;
    bool                     is_requested_raindrop_down_                  ;
    bool                     is_requested_bell_curve_1_                   ;
    bool                     is_requested_bell_curve_2_                   ;
    bool                     is_requested_bell_curve_4_                   ;
    bool                     is_requested_sin_over_dist_1_                ;
    bool                     is_requested_sin_over_dist_2_                ;
    bool                     is_requested_sin_over_dist_4_                ;
    bool                     is_requested_delta_                          ;
    bool                     is_requested_stair_steps_                    ;
    bool                     is_requested_reverse_wave_                   ;
    bool                     is_requested_set_init_test_                  ;
    bool                     is_requested_set_sheet_random_noise_         ;
    bool                     is_requested_normalize_sheet_                ;

    bool                     is_requested_set_xy_sizes_                   ;
    bool                     is_requested_set_xy_sizes_with_value_        ;
    size_type                requested_set_xy_sizes_x_                    ;
    size_type                requested_set_xy_sizes_y_                    ;
    value_type               requested_set_xy_sizes_value_                ;

}; /* end class sheet_control_type */

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef SOLVE_CONTROL_H */
//
// solve_control.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
