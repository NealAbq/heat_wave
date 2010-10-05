// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// solve_control.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// This supplies the following:
//   The sheet object.
//   Functions to edit/change the sheet object.
//   Solve and auto-solve, including auto-solve timing.
//   Solve timing statistics.
// This should probably be split into two objects: Solve/Auto-solve, and sheet/edit.
//
// Things to add:
//   Run auto-solve for the next 3 minutes.
//   Run auto-solve for N iterations.
//   Limit auto-solve draw frequency.
//   Limit auto-solve duty cycle.
//
//   Clear or scale momentum (history) if rate is changed.
//
//   A raindrop funtion that ONLY affects momentum and not actual values.
//     Or a combo of momentum and value change.
//     When wave solving momentum makes sense, but not when we're solving heat.
//
//   A function that applies a series of deltas over time to an area.
//     This would be nice during heat.
//     We'd have a countdown and we'd be applying a pressure or acting like a tailing-off heat source.
//
// We need a function to clear duration(s).
// We clear when we:
//    change sheet size
//    change technique, method, is parallel
//    change extra-pass count?
//    we could have a reset button? we could let the user set the capacity.
//    we could record extra-pass as a blob or as a bunch.
//
// Things that should maybe affect history:
//   Setting xy solve rates. (We could adjust history.)
//   Setting solve technique or method.
//
// New operation:
//   Undo:
//     Single step (if values are stored in other sheet).
//     Multi-step.
//   After a multi-step solve we may have two sheets with old values in them.
//   Maybe the sheets should remember their generation (and whether they've been changed after
//     being stamped with a generation).
//
// Other kinds of functors:
//   Functor that returns a delta (vector) instead of a value.
//   This could be either added directly the a sheet (values, zero derivative), or could be
//   used to either set or add to the momentum.
//
//   We should be able to apply the functors to several sheets at once.
//   This makes sense with these kinds of functors:
//     Value only, does not depend on current value.
//     Delta only which does not depend on current value.
//   This makes less sense on functors that depend on the current value unless the functor
//   is expecting more than on current values.
//
//   We'd like a collective setter. It'd be like an iterator over a sheet, but when you set
//   the value it'd set the value in more than one sheet behind you.
//   As an iterator, when you'd get a value instead of a single value you'd get a vector or
//   even something of higher dimension.
//   You could provide a function to reduce the vector to a single value (take first, take min/max,
//     take sum, etc).
//
//   If we had a functor that calculated a delta, we'd probably want to set the current and next
//   sheets at the same time. But that's not allowed right now because the current sheet is also
//   our display (and so we freeze it). There are several solutions:
//     Guarantee these operations happen in the UI thread.
//     Keep another copy of the sheet just for draw. We already do that when we reduce the
//       granularity of the display.
//     Generate a bitmap for the display. Then we don't have to lock the current sheet after
//       we've drawn the first time.
//     Change only the next sheet, swap the sheets, and then apply the same changes to the current.
//       This means keeping a third sheet (which we already do sometimes -- the extra sheet).
//     Store momentum explicitly instead of as a diff between current and next.
//       That way we'll probably never have to change both sheets at the same time.
// _______________________________________________________________________________________________

# include "all.h"
# include "solve_control.h"
# include "angle_holder.h"

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* contructor */
  sheet_control_type::
sheet_control_type( QObject * p_parent)
  : QObject                                     ( p_parent)

  , p_sheet_current_                            ( 0)
  , p_sheet_next_                               ( 0)
  , p_sheet_extra_                              ( 0)
  , sheet_a_                                    ( )
  , sheet_b_                                    ( )
  , sheet_c_                                    ( )

  , is_history_delta_in_extra_sheet_            ( false)
  , is_next_sheet_valid_history_                ( false)

  , is_next_solve_pending_                      ( false)
  , are_edges_fixed_                            ( false)
  , is_center_frozen_                           ( false)
  , is_vortex_on_                               ( false)

  , is_auto_solving_                            ( false)
  , is_auto_solve_just_started_                 ( false)
  , is_auto_solve_just_stopped_                 ( false)
  , p_heat_solver_                              ( 0)

  , generation_current_                         ( -1)

  , last_solve_tick_duration_                   ( date_time::get_invalid_tick_dur( ))
  , next_solve_start_tick_                      ( date_time::get_invalid_tick_pt( ))

  , start_finish_durations__in_seconds__from_worker_thread_
                                                ( 64)
  , start_finish_durations__in_seconds__from_control_thread_
                                                ( 64)
  , start_start_durations__in_seconds__during_auto_solve_
                                                ( 64)

  , last_auto_solve_start_tick_                 ( date_time::get_invalid_tick_pt( ))
  , last_auto_solve_finish_tick_                ( date_time::get_invalid_tick_pt( ))

  , min_ticks_auto_solve_finish_to_start_       ( date_time::get_zero_duration_of_ticks( ))
  , min_ticks_auto_solve_start_to_start_        ( date_time::get_zero_duration_of_ticks( ))

  , p_wakeup_for_next_solve_                    ( 0)

  , is_draw_size_limited_                       ( true)
  , is_limited_draw_sheet_written_              ( false)
  , xy_draw_size_limit_                         ( 10000)
  , x_draw_size_limit_                          ( 0)
  , y_draw_size_limit_                          ( 0)
  , ratio_xy_sheet_to_xy_limit_                 ( 1.0f)
  , sheet_limited_draw_                         ( )

  , is_requested_scale_sheet_                   ( false)
  , requested_values_scale_                     ( 1.0f)
  , requested_momentum_scale_                   ( 1.0f)
  , is_requested_ramp_corner_to_corner_         ( false)
  , is_requested_ramp_2_corners_                ( false)
  , is_requested_ramp_4_corners_                ( false)
  , is_requested_bell_corner_1_                 ( false)
  , is_requested_bell_corner_2_                 ( false)
  , is_requested_bell_corner_3_                 ( false)
  , is_requested_bell_corner_4_                 ( false)
  , is_requested_raindrop_up_                   ( false)
  , is_requested_raindrop_down_                 ( false)
  , is_requested_bell_curve_1_                  ( false)
  , is_requested_bell_curve_2_                  ( false)
  , is_requested_bell_curve_4_                  ( false)
  , is_requested_sin_over_dist_1_               ( false)
  , is_requested_sin_over_dist_2_               ( false)
  , is_requested_sin_over_dist_4_               ( false)
  , is_requested_delta_                         ( false)
  , is_requested_stair_steps_                   ( false)
  , is_requested_reverse_wave_                  ( false)
  , is_requested_set_init_test_                 ( false)
  , is_requested_set_sheet_random_noise_        ( false)
  , is_requested_normalize_sheet_               ( false)
  , is_requested_set_xy_sizes_                  ( false)
  , is_requested_set_xy_sizes_with_value_       ( false)
  , requested_set_xy_sizes_x_                   ( 0)
  , requested_set_xy_sizes_y_                   ( 0)
  , requested_set_xy_sizes_value_               ( 0)
{
    // Do this first. Some things (initializing the sheets) assume the solver exists.
    init_solver( );

    // This emits a signal.
    init_sheets( );

    // You must do this after init_sheets( ).
    init_draw_size_limits( );

    assert_valid( );
}

// _______________________________________________________________________________________________

  /* destructor */
  sheet_control_type::
~sheet_control_type( )
{
    // We must stop the solver now if it is running.
    // When this dtor is done the sheets get destroyed, and then later the child objects.
    // Since the solver is a child object it gets destroyed after the sheets. If it is running
    // (in a separate thread) the solver may try to access the sheets after they are destroyed.
    // (We don't have to delete animation early, even if it is running, because it doesn't use
    // a separate thread.)
    if ( p_heat_solver_ ) {
        // Delete this early, before auto-delete does it.
        delete p_heat_solver_;
        p_heat_solver_ = 0;
    }
}

// _______________________________________________________________________________________________

# ifndef NDEBUG
  void
  sheet_control_type::
assert_valid( ) const
{
    // This is incomplete.

    sheet_a_.assert_valid( );
    sheet_b_.assert_valid( );
    sheet_c_.assert_valid( );

    d_assert( sheet_a_.get_x_count( ) == sheet_b_.get_x_count( ));
    d_assert( sheet_a_.get_y_count( ) == sheet_b_.get_y_count( ));

    if ( sheet_a_.is_reset( ) ) {
        d_assert( false);
        d_assert( sheet_b_.is_reset( ));
        d_assert( 0 == p_sheet_current_   );
        d_assert( 0 == p_sheet_next_      );
        d_assert( 0 == generation_current_);
    } else {
        d_assert( sheet_a_.not_reset( ));
        d_assert( sheet_b_.not_reset( ));
        if ( (& sheet_a_) == p_sheet_current_ ) {
            d_assert( (& sheet_b_) == p_sheet_next_);
        } else {
            d_assert( (& sheet_a_) == p_sheet_next_);
            d_assert( (& sheet_b_) == p_sheet_current_);
        }
    }
}
# endif

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  sheet_control_type::
init_solver( )
{
    d_assert( ! p_heat_solver_);
    p_heat_solver_ = new heat_solver_type( this);
    d_verify( connect( p_heat_solver_, SIGNAL( finished( )), this, SLOT( finished__from_solver( ))));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  sheet_control_type::
start_auto_solving( )
{
    if ( ! is_auto_solving( ) ) {
        d_assert( date_time::is_invalid_tick_pt( last_auto_solve_start_tick_ ));
        d_assert( date_time::is_invalid_tick_pt( last_auto_solve_finish_tick_));

        is_auto_solving_            = true;
        is_auto_solve_just_started_ = true;

        if ( ! is_next_solve_pending( ) ) {
            solve_next( );
        }

        emit auto_solving_started( true);
    }
}

// _______________________________________________________________________________________________

  void
  sheet_control_type::
stop_auto_solving( )
{
    if ( is_auto_solving( ) ) {

        last_auto_solve_start_tick_  = date_time::get_invalid_tick_pt( );
        last_auto_solve_finish_tick_ = date_time::get_invalid_tick_pt( );
        is_auto_solving_             = false;
        // The current solve calculation will finish.
        // Maybe we shouldn't make it current?

        // We only set is_auto_solve_just_stopped_ if we are waiting for a pending solve.
        // You cannot count on there being one cycle where is_auto_solve_just_stopped_ is true.
        // Unless we are auto-solving with no delays, in which case is_next_solve_pending( ) will
        // always be true when we get here.
        if ( is_next_solve_pending( ) ) {
            is_auto_solve_just_stopped_ = true;
        }

        //emit auto_solving_stopped( );
        emit auto_solving_started( false);
    }
}

// _______________________________________________________________________________________________

  /* slot */
  void
  sheet_control_type::
start_stop_auto_solving( bool is_start)
{
    if ( is_start ) {
        start_auto_solving( );
    } else {
        stop_auto_solving( );
    }
}

// _______________________________________________________________________________________________

  /* slot */
  void
  sheet_control_type::
single_step_solve( )
{
    // If auto-solving is happening, just stop it.
    if ( is_auto_solving( ) ) {
        stop_auto_solving( );
    } else {
        d_assert( date_time::is_invalid_tick_pt( last_auto_solve_start_tick_ ));
        d_assert( date_time::is_invalid_tick_pt( last_auto_solve_finish_tick_));

        if ( ! is_next_solve_pending( ) ) {
            solve_next( );
        }
    }
}

// _______________________________________________________________________________________________

  /* not a slot */
  void
  sheet_control_type::
solve_next( )
{
    d_assert( p_sheet_current_ && p_sheet_next_ && p_sheet_extra_);
    d_assert( ! get_heat_solver( )->is_busy( ));
    d_assert( ! is_next_solve_pending( ));
    d_assert( ! is_auto_solve_just_stopped_);

    // Start the timers.
    record_start_solve( );

    // Make sure there is some history if we're solving e_wave_with_damping.
    // You need the sheet before the current one to solve for a wave.
    if ( (! is_next_sheet_valid_history_) && get_heat_solver( )->is_technique__wave_with_damping( ) ) {
        // We can either use technique heat_solver::e_simultaneous_2d in this case
        // or we can copy the src to the trg to create a history.
        // If (get_extra_pass_count( ) > 0) (and ! are_extra_passes_disabled) we should copy
        // to create the history.
        copy_current_to_next_sheet( );
        is_next_sheet_valid_history_ = true;
    }

    // Kick the solver thread.
    // We disable extra passes when edges are fixed because otherwise wave-solve explodes and
    // heat-solve looks pretty ugly.
    // We disable extra passes when the center is frozen because it looks very bad. It also
    // generates some spurious waves but they don't have the energy to explode (except maybe
    // on very small sheets).
    bool const are_extra_passes_disabled = are_edges_fixed( ) || is_center_frozen( );
    get_heat_solver( )->calc_next( *p_sheet_current_, *p_sheet_next_, *p_sheet_extra_, are_extra_passes_disabled);

    // We are now waiting for a finished__from_solver( ) signal.
    is_next_solve_pending_ = true;
}

// _______________________________________________________________________________________________

  /* private slot */
  void
  sheet_control_type::
finished__from_solver( )
  //
  // This receives a signal from the solver saying the next sheet has been calculated.
  // This signal is queued. It is sent from the solver thread and received (here) in the UI thread.
{
    d_assert( is_next_solve_pending( ));
    is_next_solve_pending_ = false;

    // Do not update the sheet if is_early_exit.
    if ( get_heat_solver( )->get_output_params( )->is_early_exit( ) ) {
        is_next_sheet_valid_history_ = false;
        return;
    }

    // Record the durations.
    record_finish_solve( );

    // This will emit the sheet_is_changed( ) signal.
    after_solve( );

    // Make any changes before we start the next solve.
    honor_requests( );

    // These are always set false at the end of the pending solve.
    is_auto_solve_just_started_ = false;
    is_auto_solve_just_stopped_ = false;

    // Start calculating the next generation immediately when we are auto-solving.
    if ( is_auto_solving( ) ) {

        // Improve: Slow down auto-solve.
        //
        // This will complicate the is_auto_solving( ) mode. We'll want to be able to
        // interrupt auto-solving when the worker-thread is just sleeping and not in
        // the middle of solving. We'll also want status on updates (statistics, progress)
        // and maybe we'll want to be able to get the latest finished sheet while the solver
        // is running, and make that the current sheet (the solver will have to adjust).
        // Almost all sheet operations should be decoupled from the control thread.
        // Why ever interrupt the other thread?
        //   Early exit when the app is shutting down.
        //   Very slow solve that the user wants to stop (1000 extra passes, hangs the machine).
        //   User wants to transform the sheet (flatten, normalized, etc).
        //     Remember, all these transforms need to move into the worker thread.
        //
        // We'll use the following:
        //   min_ticks_auto_solve_finish_to_start_
        //   min_ticks_auto_solve_start_to_start_
        //   p_wakeup_for_next_solve_
        //
        // We can implement the delay by sleeping in the solve worker-thread, either
        // before or after solving. Problem: interrupt sleep to early exit. Better
        // to use a semaphore to construct an interruptable sleep object.

        solve_next( );
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  bool
  sheet_control_type::
is_available_duration_worker_thread( ) const
{
    return ! start_finish_durations__in_seconds__from_worker_thread_.is_empty( );
}

  sheet_control_type::second_type
  sheet_control_type::
get_average_duration_worker_thread_seconds( ) const
{
    return second_type(
        start_finish_durations__in_seconds__from_worker_thread_.
            get_average< second_type >( ));
}

  sheet_control_type::millisecond_type
  sheet_control_type::
get_average_duration_worker_thread_mseconds( ) const
{
    return
        date_time::convert_seconds_to_milliseconds(
            get_average_duration_worker_thread_seconds( ));
}

  void
  sheet_control_type::
clear_solve_stats( )
{
    generation_current_ = 0;
    start_finish_durations__in_seconds__from_worker_thread_  .set_empty( );
    start_finish_durations__in_seconds__from_control_thread_ .set_empty( );
    start_start_durations__in_seconds__during_auto_solve_    .set_empty( );
}

// _______________________________________________________________________________________________

  bool
  sheet_control_type::
is_available_duration_control_thread( ) const
{
    return ! start_finish_durations__in_seconds__from_control_thread_.is_empty( );
}

  sheet_control_type::second_type
  sheet_control_type::
get_average_duration_control_thread_seconds( ) const
{
    return second_type(
        start_finish_durations__in_seconds__from_control_thread_.
            get_average< second_type >( ));
    // return
    //    date_time::convert_ticks_to_seconds(
    //        start_finish_durations_from_control_thread_.get_average< second_type >( ));
}

  sheet_control_type::millisecond_type
  sheet_control_type::
get_average_duration_control_thread_mseconds( ) const
{
    return
        date_time::convert_seconds_to_milliseconds(
            get_average_duration_control_thread_seconds( ));
}

// _______________________________________________________________________________________________

  bool
  sheet_control_type::
is_available_duration_auto_solve_cycle( ) const
{
    return ! start_start_durations__in_seconds__during_auto_solve_.is_empty( );
}

  sheet_control_type::second_type
  sheet_control_type::
get_average_duration_auto_solve_cycle_seconds( ) const
{
    return second_type(
        start_start_durations__in_seconds__during_auto_solve_.
            get_average< second_type >( ));
}

  sheet_control_type::millisecond_type
  sheet_control_type::
get_average_duration_auto_solve_cycle_mseconds( ) const
{
    return
        date_time::convert_seconds_to_milliseconds(
            get_average_duration_auto_solve_cycle_seconds( ));
}

// _______________________________________________________________________________________________

  void
  sheet_control_type::
record_start_solve( )
{
    // Remember next_solve_start_tick_.
    d_assert( date_time::is_invalid_tick_pt( next_solve_start_tick_ ));
    next_solve_start_tick_ = date_time::get_tick_now( );

    // Remember start-to-start timing if we are auto-solving and have been for at least one cycle.
    if ( is_auto_solving( ) ) {
        if ( date_time::is_valid_tick_pt( last_auto_solve_start_tick_) ) {
            d_assert( last_auto_solve_finish_tick_ >= last_auto_solve_start_tick_);
            if ( next_solve_start_tick_ >= last_auto_solve_start_tick_ ) {
                tick_duration_type const start_start_duration = next_solve_start_tick_ - last_auto_solve_start_tick_;
                start_start_durations__in_seconds__during_auto_solve_.
                    record_next(
                        date_time::convert_ticks_to_seconds( start_start_duration));

                d_assert( last_auto_solve_finish_tick_ <= next_solve_start_tick_);
                d_assert( last_solve_tick_duration_ <= start_start_duration);
            }
        }
    } else {
        d_assert( date_time::is_invalid_tick_pt( last_auto_solve_start_tick_ ));
        d_assert( date_time::is_invalid_tick_pt( last_auto_solve_finish_tick_));
    }
}

  void
  sheet_control_type::
record_finish_solve( )
{
    // The last solve in a run of auto-solves tends to be slower than the others.
    // Which leads to the control-thread durations being longer than the auto-solve start-to-start durations,
    // which looks wrong. The single-step control-thread durations tend to be very short, almost as short as the
    // worker-thread durations, because there's nothing interfering.

    // We don't record some statistics when this is the last solve of an auto-solve run, unless the
    // auto-solve run was only one solve long. This is because there is a lot of interference when you
    // push the "stop auto-solve" button, which uses up a surprising amount of CPU and messes up the
    // timing. Since we don't record start-to-start durations on the last run, when we record the other
    // last times it often looks like start-to-start times are less than start-finish times in the control,
    // and even in the worker, thread.
    bool const is_auto_solve_finish_not_start = is_auto_solve_just_stopped_ && ! is_auto_solve_just_started_;

    // Record the duration from the worker thread.
    if ( ! is_auto_solve_finish_not_start ) {
        double const seconds_from_worker_thread  = get_heat_solver( )->get_last_duration__seconds( );
        if ( seconds_from_worker_thread >= 0 ) {
            size_type const worker_thread_solve_count = get_heat_solver( )->get_output_params( )->get_solve_count( );
            if ( worker_thread_solve_count > 1 ) {
                start_finish_durations__in_seconds__from_worker_thread_.
                    record_next( seconds_from_worker_thread / double( worker_thread_solve_count));
            } else
            if ( worker_thread_solve_count == 1 ) {
                start_finish_durations__in_seconds__from_worker_thread_.
                    record_next( seconds_from_worker_thread);
            }
        }
    }

    // Record last_solve_tick_duration_, which is used to predict the duration of the next solve.
    // Also record the duration from the control thread.
    if ( date_time::is_valid_tick_pt( next_solve_start_tick_) ) {
        tick_point_type const finish_tick = date_time::get_tick_now( );
        if ( finish_tick >= next_solve_start_tick_ ) {
            last_solve_tick_duration_  = finish_tick - next_solve_start_tick_;
            if ( ! is_auto_solve_finish_not_start ) {
                start_finish_durations__in_seconds__from_control_thread_.
                    record_next(
                        date_time::convert_ticks_to_seconds( last_solve_tick_duration_));
            }

            // Record:
            //   last_auto_solve_start_tick_
            //   last_auto_solve_finish_tick_
            if ( is_auto_solving( ) ) {
                last_auto_solve_start_tick_  = next_solve_start_tick_;
                last_auto_solve_finish_tick_ = finish_tick;
            } else {
                d_assert( date_time::is_invalid_tick_pt( last_auto_solve_start_tick_ ));
                d_assert( date_time::is_invalid_tick_pt( last_auto_solve_finish_tick_));
            }
        }
    }

    // Forget the tick since it's recorded.
    next_solve_start_tick_ = date_time::get_invalid_tick_pt( );
}

// _______________________________________________________________________________________________

  bool
  sheet_control_type::
is_sheet_change_expected_soon( tick_duration_type tick_count) const
  //
  // True when:
  //   The clocks are working, and
  //   Is auto solving, and
  //   We've auto-solved at least once so we have the last duration, and
  //   The next auto-solve is expected to finish within the next tick_count ticks.
  // Returns false when the next auto-solve is overdue and was expected to finish a while ago.
{
    if ( is_auto_solving( ) ) {
        if ( date_time::is_valid_tick_dur( last_solve_tick_duration_) &&
             date_time::is_valid_tick_pt( next_solve_start_tick_) )
        {
            tick_point_type const now_tick = date_time::get_tick_now( );
            if ( now_tick >= next_solve_start_tick_ ) {
                tick_duration_type const elapsed_tick = now_tick - next_solve_start_tick_;
                if ( ((elapsed_tick + tick_count) >= last_solve_tick_duration_) &&
                     ((elapsed_tick - tick_count) <= last_solve_tick_duration_) )
                {
                    return true;
                }
            }
        }
    }
    return false;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  bool
  sheet_control_type::
are_requests_delayed( ) const
{
    d_assert( p_sheet_next_);
    d_assert( p_sheet_current_);
    return is_next_solve_pending( );
}

  void
  sheet_control_type::
honor_requests( )
{
    d_assert( ! are_requests_delayed( ));

    if ( is_requested_scale_sheet_ ) {
        scale_sheet( requested_values_scale_, requested_momentum_scale_);
        is_requested_scale_sheet_ = false;
        requested_values_scale_   = 1.0f;
        requested_momentum_scale_ = 1.0f;
    }

    if ( is_requested_set_xy_sizes_ ) {
        if ( is_requested_set_xy_sizes_with_value_ ) {
            set_xy_sizes( requested_set_xy_sizes_x_, requested_set_xy_sizes_y_, requested_set_xy_sizes_value_);
        } else {
            set_xy_sizes( requested_set_xy_sizes_x_, requested_set_xy_sizes_y_);
        }
        is_requested_set_xy_sizes_            = false;
        is_requested_set_xy_sizes_with_value_ = false;
        requested_set_xy_sizes_x_             = 0;
        requested_set_xy_sizes_y_             = 0;
        requested_set_xy_sizes_value_         = 0;
    }

    if ( is_requested_bell_curve_1_ ) {
        set_bell_curve_1( );
        is_requested_bell_curve_1_ = false;
    }
    if ( is_requested_bell_curve_2_ ) {
        set_bell_curve_2( );
        is_requested_bell_curve_2_ = false;
    }
    if ( is_requested_bell_curve_4_ ) {
        set_bell_curve_4( );
        is_requested_bell_curve_4_ = false;
    }

    if ( is_requested_sin_over_dist_1_ ) {
        set_sin_over_dist_1( );
        is_requested_sin_over_dist_1_ = false;
    }
    if ( is_requested_sin_over_dist_2_ ) {
        set_sin_over_dist_2( );
        is_requested_sin_over_dist_2_ = false;
    }
    if ( is_requested_sin_over_dist_4_ ) {
        set_sin_over_dist_4( );
        is_requested_sin_over_dist_4_ = false;
    }

    if ( is_requested_set_init_test_ ) {
        set_init_test( );
        is_requested_set_init_test_ = false;
    }

    if ( is_requested_ramp_corner_to_corner_ ) {
        ramp_corner_to_corner( );
        is_requested_ramp_corner_to_corner_ = false;
    }

    if ( is_requested_ramp_2_corners_ ) {
        ramp_2_corners( );
        is_requested_ramp_2_corners_ = false;
    }

    if ( is_requested_ramp_4_corners_ ) {
        ramp_4_corners( );
        is_requested_ramp_4_corners_ = false;
    }

    if ( is_requested_bell_corner_1_ ) {
        bell_corner_1( );
        is_requested_bell_corner_1_ = false;
    }
    if ( is_requested_bell_corner_2_ ) {
        bell_corner_2( );
        is_requested_bell_corner_2_ = false;
    }
    if ( is_requested_bell_corner_3_ ) {
        bell_corner_3( );
        is_requested_bell_corner_3_ = false;
    }
    if ( is_requested_bell_corner_4_ ) {
        bell_corner_4( );
        is_requested_bell_corner_4_ = false;
    }

    if ( is_requested_raindrop_up_ ) {
        raindrop_up( );
        is_requested_raindrop_up_ = false;
    }
    if ( is_requested_raindrop_down_ ) {
        raindrop_down( );
        is_requested_raindrop_down_ = false;
    }

    if ( is_requested_delta_ ) {
        set_delta( );
        is_requested_delta_ = false;
    }

    if ( is_requested_stair_steps_ ) {
        set_stair_steps( );
        is_requested_stair_steps_ = false;
    }

    if ( is_requested_set_sheet_random_noise_ ) {
        set_sheet_random_noise( );
        is_requested_set_sheet_random_noise_ = false;
    }

    if ( is_requested_normalize_sheet_ ) {
        normalize_sheet( );
        is_requested_normalize_sheet_ = false;
    }

    if ( is_requested_reverse_wave_ ) {
        if ( reverse_wave( ) ) {
            is_requested_reverse_wave_ = false;
        }
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Slots for setting sheet values
//
//   We could provide many more of these functions.
//   Some could depend on rotation.
//   We could transform a path from one coord system to another. For example:
//     Start with a continuous path [ 0 .. 1 ]
//     Map it to a circle [ r = 1, angle 0 .. 2pi ]
//     Map to rectangular coord
//     Translate and scale
//     Map to a discrete set of points that describes an ellipse on a (discrete value) plane.
//
//   Alternating square wave.
//   Raise the nearest corner.
//   Draw a diagonal line based on rotation and translation in the UI.
//   Alternates stamping halves, smaller and smaller into the middle.
//   Draw Sierpinski triangle.
//   Squeezes instead of stamps - see squeeze_operator
//   Opposite of squeeze - stretches
//   Inverter instead of squeezer - negative scale or negative squeeze
//   Circle drawer
//   Rect and circular donuts
// _______________________________________________________________________________________________

  /* slot */
  void
  sheet_control_type::
request_set_init_test( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_set_init_test_ = true;
    } else {
        set_init_test( );
    }
}

  void
  sheet_control_type::
set_init_test( )
{
    size_type const x_size = get_x_size( );
    size_type const y_size = get_y_size( );

    // This only works if the sheet is at least 2x2.
    bool const is_too_small = (x_size < 2) || (y_size < 2);

    // Should we save history? Let's see what it looks like.
    prepare_for_transform( is_too_small ? e_flatten_next_sheet : e_copy_current_to_next_sheet);

    if ( ! is_too_small ) {
        // We'll write over most of the sheet, but not the edges.

        // Stamp the outer rings.
        if ( (x_size >= 8) && (y_size >= 8) ) {

            // Fill in the middle 3/4ths.
            {   size_type const y_margin = y_size / 8;
                size_type const x_margin = x_size / 8;
                set_next_sheet_at_margin_rect( x_margin, x_margin, y_margin, y_margin, +0.8f);
            }

            // Fill in the middle 1/2rd.
            {   size_type const y_margin = y_size / 4;
                size_type const x_margin = x_size / 4;
                set_next_sheet_at_margin_rect( x_margin, x_margin, y_margin, y_margin, -0.8f);
            }
        }

        // Fill in the middle 1/4rd.
        if ( (x_size >= 4) && (y_size >= 4) ) {
            size_type const y_margin = (y_size * 3) / 8;
            size_type const x_margin = (x_size * 3) / 8;
            set_next_sheet_at_margin_rect( x_margin, x_margin, y_margin, y_margin, +1.0);
        }

        // Fill in the middle 2x2 square.
        {   size_type const y_margin = (y_size / 2) - 1;
            size_type const x_margin = (x_size / 2) - 1;
            set_next_sheet_at_margin_rect( x_margin, x_margin, y_margin, y_margin, -1.0);
        }

        // Lower a corner.
        set_next_sheet_at( 0, 2, 0, 2, -1.0);

        // Lower a corner.
        set_next_sheet_at( x_size - 2, 2, y_size - 2, 2, -1.0);

        // Raise a corner.
        set_next_sheet_at( x_size - 2, 2, 0, 2, +1.0);

        // Raise a corner.
        set_next_sheet_at( 0, 2, y_size - 2, 2, +1.0);
    }

    // Make the next sheet the current sheet.
    after_transform( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Random noise

  /* slot */
  void
  sheet_control_type::
request_set_sheet_random_noise( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_set_sheet_random_noise_ = true;
    } else {
        set_sheet_random_noise( );
    }
}

  void
  sheet_control_type::
set_sheet_random_noise( )
{
    // Seed the random number generator so it's different every time we run.
    ::srand( ::time( 0));

    unsigned int how_many = get_x_size( ) * get_y_size( ) / 50;
    if ( how_many < 1 ) how_many = 1;

    prepare_for_transform( );

    for ( int countdown = how_many; countdown ; -- countdown ) {
        set_next_sheet_random_rect( 1, 1);

        if ( get_y_size( ) > 8 ) {
            set_next_sheet_random_rect( 1, 2);
            set_next_sheet_random_rect( 1, 3);
            set_next_sheet_random_rect( 1, 4);
        }

        if ( get_x_size( ) > 8 ) {
            set_next_sheet_random_rect( 2, 1);
            set_next_sheet_random_rect( 3, 1);
            set_next_sheet_random_rect( 4, 1);
        }
    }

    after_transform( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Normalize

  /* slot */
  void
  sheet_control_type::
request_normalize_sheet( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_normalize_sheet_ = true;
    } else {
        normalize_sheet( );
    }
}

  void
  sheet_control_type::
normalize_sheet( )
{
    prepare_for_transform( );

    value_type scale = 0;
    p_sheet_next_->normalize( -1, +1, & scale);
    if ( 0 != scale ) {
        maybe_scale_saved_history( scale);
    }

    after_transform( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Flatten

  /* slot */
  void
  sheet_control_type::
request_flatten_sheet( )
{
    request_scale_sheet( 0, 0);
}

  /* slot */
  void
  sheet_control_type::
request_flatten_values( )
{
    request_scale_sheet( 0, 1.0f);
}

  /* slot */
  void
  sheet_control_type::
request_flatten_momentum( )
{
    request_scale_sheet( 1.0f, 0);
}

  /* slot */
  void
  sheet_control_type::
request_half_values( )
{
    request_scale_sheet( 0.5f, 1.0f);
}

  /* slot */
  void
  sheet_control_type::
request_half_momentum( )
{
    request_scale_sheet( 1.0f, 0.5f);
}

  /* slot */
  void
  sheet_control_type::
request_half_both( )
{
    request_scale_sheet( 0.5f, 0.5f);
}

  /* slot */
  void
  sheet_control_type::
request_scale_sheet( float values_scale, float momentum_scale)
{
    if ( are_requests_delayed( ) ) {
        is_requested_scale_sheet_ = true;
        requested_values_scale_   = values_scale  ;
        requested_momentum_scale_ = momentum_scale;
    } else {
        scale_sheet( values_scale, momentum_scale);
    }
}

  void
  sheet_control_type::
scale_sheet( float values_scale, float momentum_scale)
{
    bool const is_flatten_values   = (0 == values_scale  );
    bool const is_flatten_momentum = (0 == momentum_scale);

    bool const is_leave_values     = (1.0f == values_scale  );
    bool const is_leave_momentum   = (1.0f == momentum_scale);

    bool const is_scale_values     = (! is_flatten_values  ) && (! is_leave_values  );
    bool const is_scale_momentum   = (! is_flatten_momentum) && (! is_leave_momentum);

    // Check for no-change.
    if ( is_leave_values && is_leave_momentum ) return;

    // Maybe discard all momentum.
    if ( is_flatten_momentum ) {
        is_next_sheet_valid_history_ = false;

        // If we're only discarding momentum we are done.
        if ( is_leave_values ) return;
    }

    // We're going to need to increment the generation.
    prepare_for_transform( is_flatten_values ? e_flatten_next_sheet : e_copy_current_to_next_sheet);

    // Take care of scaling.
    if ( is_scale_values ) {
        (*p_sheet_next_) *= values_scale;
    }
    if ( is_scale_momentum ) {
        maybe_scale_saved_history( momentum_scale);
    }

    after_transform( );
}

# if 0
  void
  sheet_control_type::
flatten_sheet( )
{
    // There are three ways to discard momentum:
    //   is_next_sheet_valid_history_ = false;
    //
    //   is_history_delta_in_extra_sheet_ = false;  // set this after prepare_for_transform(..)
    //   p_sheet_extra_->fill_sheet( 0);            // do this after prepare_for_transform(..)

    // Discard momentum.
    is_next_sheet_valid_history_ = false;

    // Preserve history, to see what happens.
    prepare_for_transform( e_flatten_next_sheet);

    // The other two ways to discard momentum would go here:
    //   if ( is_history_delta_in_extra_sheet_ ) {
    //     is_history_delta_in_extra_sheet_ = false;  // method 1
    //     p_sheet_extra_->fill_sheet( 0);            // method 2
    //   }

    after_transform( );
}

  void
  sheet_control_type::
flatten_values( )
  //
  // This does not discard momentum.
{
    prepare_for_transform( e_flatten_next_sheet);
    after_transform( );
}

  void
  sheet_control_type::
scale_values( flost scale)
  //
  // This does not discard or scale momentum.
{
    if ( scale == 0 ) {
        flatten_values( );
    } else {
        prepare_for_transform( );
        (*p_sheet_next_) *= scale;
        after_transform( );
    }
}

  void
  sheet_control_type::
flatten_momentum( )
{
    is_next_sheet_valid_history_ = false;

    // We could do this to increment the generation count. But it's not necessary.
    // prepare_for_transform( );
    // after_transform( );
}

  void
  sheet_control_type::
scale_momentum( float scale)
{
    prepare_for_transform( );
    maybe_scale_saved_history( scale);
    after_transform( );
}
# endif

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Bell curve

  /* slot */
  void
  sheet_control_type::
request_bell_curve_1( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_bell_curve_1_ = true;
    } else {
        set_bell_curve_1( );
    }
}

  /* slot */
  void
  sheet_control_type::
request_bell_curve_2( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_bell_curve_2_ = true;
    } else {
        set_bell_curve_2( );
    }
}

  /* slot */
  void
  sheet_control_type::
request_bell_curve_4( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_bell_curve_4_ = true;
    } else {
        set_bell_curve_4( );
    }
}

  void
  sheet_control_type::
set_bell_curve_1( )
{
    // We start with copy_current_to_next_sheet( ) if the next transform uses the old values.
    // ->fill_bell_curve_1( ) stamps the curve on top of a flattened version of the original.
    // It used to just set all the values without regard to the src, and we didn't need to
    // call copy_current_to_next_sheet( ) first.
    prepare_for_transform( );
    p_sheet_next_->fill_bell_curve_1( );
    after_transform( );
}

  void
  sheet_control_type::
set_bell_curve_2( )
{
    prepare_for_transform( );
    p_sheet_next_->fill_bell_curve_2( );
    after_transform( );
}

  void
  sheet_control_type::
set_bell_curve_4( )
{
    prepare_for_transform( );
    p_sheet_next_->fill_bell_curve_4( );
    after_transform( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Sin over distance

  /* slot */
  void
  sheet_control_type::
request_sin_over_dist_1( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_sin_over_dist_1_ = true;
    } else {
        set_sin_over_dist_1( );
    }
}

  /* slot */
  void
  sheet_control_type::
request_sin_over_dist_2( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_sin_over_dist_2_ = true;
    } else {
        set_sin_over_dist_2( );
    }
}

  /* slot */
  void
  sheet_control_type::
request_sin_over_dist_4( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_sin_over_dist_4_ = true;
    } else {
        set_sin_over_dist_4( );
    }
}

  void
  sheet_control_type::
set_sin_over_dist_1( )
{
    prepare_for_transform( );
    p_sheet_next_->fill_sin_over_dist_1( );
    after_transform( );
}

  void
  sheet_control_type::
set_sin_over_dist_2( )
{
    prepare_for_transform( );
    p_sheet_next_->fill_sin_over_dist_2( );
    after_transform( );
}

  void
  sheet_control_type::
set_sin_over_dist_4( )
{
    prepare_for_transform( );
    p_sheet_next_->fill_sin_over_dist_4( );
    after_transform( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Ramp

  struct /* sheet-transform functor */
corner_ramp_functor_type
{
    typedef sheet_type::value_type  value_type ;
    typedef sheet_type::size_type   size_type  ;

      // constructor
    corner_ramp_functor_type( sheet_type const & sheet)
      : size_( sheet.get_x_count( ) + sheet.get_y_count( ))
      , half_size_( size_ / 2)
      { d_assert( size_ > 0); }
      value_type const size_;
      value_type const half_size_;

      // functor
      value_type
    operator ()
     (  value_type  old_value_z
      , size_type   x
      , size_type   y
     )
    {
        return old_value_z + (((x + y) - half_size_) / size_);
    }
};

  void
  sheet_control_type::
ramp_corner_to_corner( )
{
    prepare_for_transform( );
    p_sheet_next_->transform_sheet( corner_ramp_functor_type( *p_sheet_next_));
    after_transform( );
}

  /* slot */
  void
  sheet_control_type::
request_ramp_corner_to_corner( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_ramp_corner_to_corner_ = true;
    } else {
        ramp_corner_to_corner( );
    }
}

// _______________________________________________________________________________________________

  /* slot */
  void
  sheet_control_type::
request_ramp_2_corners( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_ramp_2_corners_ = true;
    } else {
        ramp_2_corners( );
    }
}

  void
  sheet_control_type::
ramp_2_corners( )
  //
  // This should probably be a sheet-setting functor.
  // This could use corner_ramp_functor_type with a few modifications.
  // Instead of using a functor, this demonstrates a more old-fashioned but still servicable
  // way of setting sheet values.
  // A benefit of using transform_sheet( functor) is you're guaranteed to set every sheet value.
{
    prepare_for_transform( );

    size_type const x_size = get_x_size( );
    size_type const y_size = get_y_size( );

    size_type const xy_lo_limit = std::min( x_size, y_size) / 2;
    for ( size_type
             xy_lo = 0
           , x_hi  = x_size - 1
           , y_hi  = y_size - 1
          ;  xy_lo < xy_lo_limit
          ;  ++ xy_lo
           , -- x_hi
           , -- y_hi
        )
    {
        // Other possibilities here:
        //   1 / (xy_lo + 1)
        //   1 / (xy_lo_limit  - xy_lo)
        value_type const sqrt_value = static_cast< value_type >(xy_lo_limit - xy_lo) / xy_lo_limit;
        value_type const pos_value  = sqrt_value * sqrt_value;
        value_type const neg_value  = - pos_value;

        set_next_sheet_at( xy_lo, xy_lo, pos_value);
        set_next_sheet_at( x_hi, y_hi, neg_value);

        for ( size_type
                 inner_xy_lo = 0
               , inner_x_hi  = x_size - 1
               , inner_y_hi  = y_size - 1
              ;  inner_xy_lo < xy_lo
              ;  ++ inner_xy_lo
               , -- inner_x_hi
               , -- inner_y_hi
            )
        {
            set_next_sheet_at( xy_lo, inner_xy_lo, pos_value);
            set_next_sheet_at( inner_xy_lo, xy_lo, pos_value);
            set_next_sheet_at( x_hi, inner_y_hi, neg_value);
            set_next_sheet_at( inner_x_hi, y_hi, neg_value);
        }
    }

    after_transform( );
}

// _______________________________________________________________________________________________

  /* slot */
  void
  sheet_control_type::
request_ramp_4_corners( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_ramp_4_corners_ = true;
    } else {
        ramp_4_corners( );
    }
}

  void
  sheet_control_type::
ramp_4_corners( )
  //
  // This could use corner_ramp_functor_type with a few modifications.
  // This does not set 
{
    prepare_for_transform( );

    size_type const x_size = get_x_size( );
    size_type const y_size = get_y_size( );

    size_type const xy_lo_limit = std::min( x_size, y_size) / 2;
    for ( size_type
             xy_lo = 0
           , x_hi  = x_size - 1
           , y_hi  = y_size - 1
          ;  xy_lo < xy_lo_limit
          ;  ++ xy_lo
           , -- x_hi
           , -- y_hi
        )
    {
        // Other possibilities here:
        //   1 / (xy_lo + 1)
        //   1 / (xy_lo_limit  - xy_lo)
        value_type const sqrt_value = static_cast< value_type >(xy_lo_limit - xy_lo) / xy_lo_limit;
        value_type const pos_value  = sqrt_value * sqrt_value;
        value_type const neg_value  = - pos_value;

        set_next_sheet_at( xy_lo, xy_lo, pos_value);
        set_next_sheet_at( x_hi , y_hi , pos_value);
        set_next_sheet_at( xy_lo, y_hi , neg_value);
        set_next_sheet_at( x_hi , xy_lo, neg_value);

        for ( size_type
                 inner_xy_lo = 0
               , inner_x_hi  = x_size - 1
               , inner_y_hi  = y_size - 1
              ;  inner_xy_lo < xy_lo
              ;  ++ inner_xy_lo
               , -- inner_x_hi
               , -- inner_y_hi
            )
        {
            set_next_sheet_at( xy_lo, inner_xy_lo, pos_value);
            set_next_sheet_at( inner_xy_lo, xy_lo, pos_value);
            set_next_sheet_at( x_hi , inner_y_hi , pos_value);
            set_next_sheet_at( inner_x_hi , y_hi , pos_value);

            set_next_sheet_at( x_hi , inner_xy_lo, neg_value);
            set_next_sheet_at( inner_xy_lo, y_hi , neg_value);
            set_next_sheet_at( xy_lo, inner_y_hi , neg_value);
            set_next_sheet_at( inner_x_hi , xy_lo, neg_value);
        }
    }

    after_transform( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Bell corners

  /* slot */
  void
  sheet_control_type::
request_bell_corner_1( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_bell_corner_1_ = true;
    } else {
        bell_corner_1( );
    }
}

  void
  sheet_control_type::
bell_corner_1( )
{
    prepare_for_transform( );

    value_type const  x_size  = static_cast< value_type >( p_sheet_next_->get_x_count( ));
    value_type const  y_size  = static_cast< value_type >( p_sheet_next_->get_y_count( ));
    p_sheet_next_->transform_sheet
     (  bell_curve_functor_type
         (  0, 0  // corner
          , x_size / 18
          , y_size / 18
          , 0, +1
         )
      , util::assign_sum_type< value_type >( )
     );

    after_transform( );
}

// _______________________________________________________________________________________________

  /* slot */
  void
  sheet_control_type::
request_bell_corner_2( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_bell_corner_2_ = true;
    } else {
        bell_corner_2( );
    }
}

  void
  sheet_control_type::
bell_corner_2( )
{
    prepare_for_transform( );

    value_type const  x_size  = static_cast< value_type >( p_sheet_next_->get_x_count( ));
    value_type const  y_size  = static_cast< value_type >( p_sheet_next_->get_y_count( ));
    p_sheet_next_->transform_sheet
     (  bell_curve_functor_type
         (  0, 0  // corner
          , x_size / 8
          , y_size / 8
          , 0, +1
         )
      , util::assign_sum_type< value_type >( )
     );

    after_transform( );
}

// _______________________________________________________________________________________________

  /* slot */
  void
  sheet_control_type::
request_bell_corner_3( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_bell_corner_3_ = true;
    } else {
        bell_corner_3( );
    }
}

  void
  sheet_control_type::
bell_corner_3( )
{
    prepare_for_transform( );

    value_type const  x_size  = static_cast< value_type >( p_sheet_next_->get_x_count( ));
    value_type const  y_size  = static_cast< value_type >( p_sheet_next_->get_y_count( ));
    p_sheet_next_->transform_sheet
     (  bell_curve_functor_type
         (  0, 0  // corner
          , x_size / 3
          , y_size / 3
          , 0, +1
         )
      , util::assign_sum_type< value_type >( )
     );

    after_transform( );
}

// _______________________________________________________________________________________________

  /* slot */
  void
  sheet_control_type::
request_bell_corner_4( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_bell_corner_4_ = true;
    } else {
        bell_corner_4( );
    }
}

  void
  sheet_control_type::
bell_corner_4( )
{
    prepare_for_transform( );

    value_type const  x_size  = static_cast< value_type >( p_sheet_next_->get_x_count( ));
    value_type const  y_size  = static_cast< value_type >( p_sheet_next_->get_y_count( ));
    p_sheet_next_->transform_sheet
     (  bell_curve_functor_type
         (  0, y_size * 2 / 5
          , x_size / 15
          , y_size / 15
          , 0, +1
         )
      , util::assign_sum_type< value_type >( )
     );

    after_transform( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Raindrops

  /* slot */
  void
  sheet_control_type::
request_raindrop_up( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_raindrop_up_ = true;
    } else {
        raindrop_up( );
    }
}

  /* slot */
  void
  sheet_control_type::
request_raindrop_down( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_raindrop_down_ = true;
    } else {
        raindrop_down( );
    }
}

  void
  sheet_control_type::
raindrop_up( )
{
    raindrop_random( 0.03f, 0.09f, +1, 0);
}

  void
  sheet_control_type::
raindrop_down( )
{
    raindrop_random( 0.03f, 0.09f, -1, 0);
}

  void
  sheet_control_type::
raindrop_random
 (  value_type  xy_delta_lo  /* lo limit of width of raindrop */
  , value_type  xy_delta_hi  /* hi limit of width of raindrop */
  , value_type  z_delta_lo   /* lo limit of depth of the raindrop */
  , value_type  z_delta_hi   /* hi limit of depth of the raindrop */
 )
  // This should be run as an animation that fires at random 1/f intervals.
  // Small changes like this should maybe be folded into the solve, maybe as a virtual sheet of
  // deltas (defined as a functor)?
{
    // These values cannot cross zero. So we assume they're both positive. Lo can be greater than
    // hi although it would not affect the results if you switched them in that case.
    d_assert( xy_delta_lo > 0);
    d_assert( xy_delta_hi > 0);

    value_type const  xy_delta  = (xy_delta_lo == xy_delta_hi) ?
                                    xy_delta_lo :
                                    util::random_uniform< value_type >( xy_delta_lo, xy_delta_hi);

    value_type const  z_delta   = (z_delta_lo == z_delta_hi) ?
                                    z_delta_lo :
                                    util::random_uniform< value_type >( z_delta_lo, z_delta_hi);

    value_type const  x_count   = static_cast< value_type >( p_sheet_next_->get_x_count( ));
    value_type const  y_count   = static_cast< value_type >( p_sheet_next_->get_y_count( ));

    value_type const  x_delta   = xy_delta * x_count;
    value_type const  y_delta   = xy_delta * y_count;
    value_type const  x_center  = util::random_uniform( (x_delta / -4), (x_count + (x_delta / 4)));
    value_type const  y_center  = util::random_uniform( (y_delta / -4), (y_count + (y_delta / 4)));

    prepare_for_transform( );

    p_sheet_next_->transform_sheet
     (  bell_curve_functor_type
         (  x_center, y_center
          , x_delta, y_delta
          , 0, z_delta
         )
      , util::assign_sum_type< value_type >( )
     );

    after_transform( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Delta

  /* slot */
  void
  sheet_control_type::
request_delta( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_delta_ = true;
    } else {
        set_delta( );
    }
}

  void
  sheet_control_type::
set_delta( )
{
    // Don't preserve history.
    prepare_for_transform( e_copy_current_to_next_sheet, false);
    p_sheet_next_->set_delta( );
    p_sheet_next_->normalize( );
    after_transform( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Stair steps

  /* slot */
  void
  sheet_control_type::
request_stair_steps( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_stair_steps_ = true;
    } else {
        set_stair_steps( );
    }
}

  void
  sheet_control_type::
set_stair_steps( )
{
    // If we entirely cover the sheet, we could start with this:
    //   e_do_not_init_next_sheet
    //
    // We could init the sheet in either of these ways:
    //   e_flatten_next_sheet
    //   e_copy_current_to_next_sheet
    //
    // We could also choose to preserve history.
    prepare_for_transform( e_copy_current_to_next_sheet, false);

    size_type const x_size = get_x_size( );
    size_type const y_size = get_y_size( );

    size_type const x_delta = x_size / 7;
    size_type const y_delta = y_size / 8;

    size_type       x_lo    = 0;
    size_type       x_hi    = x_lo + x_delta;

    set_next_sheet_at( x_lo, x_hi, 0, y_size, +1);
    x_lo = x_hi;
    x_hi += x_delta;
    set_next_sheet_at( x_lo, x_hi, 0, y_size, -1);
    x_lo = x_hi;
    x_hi += x_delta;
    set_next_sheet_at( x_lo, x_hi, 0, y_size, +0.7f);
    x_lo = x_hi;
    x_hi += x_delta;
    set_next_sheet_at( x_lo, x_hi, 0, y_size, -0.7f);
    x_lo = x_hi;
    x_hi += x_delta;
    set_next_sheet_at( x_lo, x_hi, 0, y_size, +0.5f);
    x_lo = x_hi;
    x_hi = x_size;

    size_type       y_lo    = 0;
    size_type       y_hi    = y_lo + y_delta;
    set_next_sheet_at( x_lo, x_hi, y_lo, y_hi, -0.9f);
    y_lo = y_hi;
    y_hi += y_delta;
    set_next_sheet_at( x_lo, x_hi, y_lo, y_hi, -0.7f);
    y_lo = y_hi;
    y_hi += y_delta;
    set_next_sheet_at( x_lo, x_hi, y_lo, y_hi, -0.5f);
    y_lo = y_hi;
    y_hi += y_delta;
    set_next_sheet_at( x_lo, x_hi, y_lo, y_hi, -0.3f);
    y_lo = y_hi;
    y_hi += y_delta;
    set_next_sheet_at( x_lo, x_hi, y_lo, y_hi, -0.1f);
    y_lo = y_hi;
    y_hi += y_delta;
    set_next_sheet_at( x_lo, x_hi, y_lo, y_hi, +0.1f);
    y_lo = y_hi;
    y_hi = y_size;
    set_next_sheet_at( x_lo, x_hi, y_lo, y_hi, +0.3f);

    after_transform( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Reverse wave

  /* slot */
  void
  sheet_control_type::
request_reverse_wave( )
{
    if ( are_requests_delayed( ) ) {
        is_requested_reverse_wave_ = true;
    } else {
        reverse_wave( );
    }
}

  bool
  sheet_control_type::
reverse_wave( )
  //
  // Reverse the momentum of the wave.
  // Not very interesting if we're solving just heat and not wave.
{
    if ( is_next_sheet_valid_history_ ) {
        transform__reverse_wave( );
        return true;
    }
    return false;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Methods for setting values in the sheet
// _______________________________________________________________________________________________

// We don't need these next methods, although they should work.
# if 0

  bool
  sheet_control_type::
set_sheet_at
 (  coord_type  x_lo
  , coord_type  x_hi_plus
  , coord_type  y_lo
  , coord_type  y_hi_plus
  , value_type  value
 )
{
    prepare_for_transform( );
    if ( set_next_sheet_at( x_lo, x_hi_plus, y_lo, y_hi_plus, value) ) {
        after_transform( );
        return true;
    }
    after_transform__cancel( );
    return false;
}

  bool
  sheet_control_type::
set_sheet_at_margin_rect
 (  coord_type  x_lo_margin
  , coord_type  x_hi_margin
  , coord_type  y_lo_margin
  , coord_type  y_hi_margin
  , value_type  value
 )
  // The margins are the distances from the edges of the sheet to where we start marking
  // the sheet. We use coord_type instead of size_type because we allow zero margins in
  // this method, while an x-margin of get_x_size( ) is too big. So these margins act more
  // like coords than sizes.
{
    prepare_for_transform( );
    if ( set_next_sheet_at_margin_rect( x_lo_margin, x_hi_margin, y_lo_margin, y_hi_margin, value) ) {
        after_transform( );
        return true;
    }
    after_transform__cancel( );
    return false;
}

  bool
  sheet_control_type::
set_sheet_at_size_rect
 (  coord_type  x_lo
  , size_type   x_size
  , coord_type  y_lo
  , size_type   y_size
  , value_type  value
 )
{
    prepare_for_transform( );
    if ( set_next_sheet_at_size_rect( x_lo, x_size, y_lo, y_size, value) ) {
        after_transform( );
        return true;
    }
    after_transform__cancel( );
    return false;
}

  bool
  sheet_control_type::
set_sheet_at
 (  coord_type  x
  , coord_type  y
  , value_type  value
 )
{
    prepare_for_transform( );
    if ( set_next_sheet_at( x, y, value) ) {
        after_transform( );
        return true;
    }
    after_transform__cancel( );
    return false;
}

# endif

// _______________________________________________________________________________________________

  void
  sheet_control_type::
flatten_next_sheet( value_type value /* = 0 */)
{
    d_assert( ! is_next_solve_pending( ));
    d_assert( p_sheet_next_);
    p_sheet_next_->fill_sheet( value);
}

  bool
  sheet_control_type::
set_next_sheet_random_rect
 (  size_type  x_size
  , size_type  y_size
 )
{
    return
        set_next_sheet_at_size_rect(
            static_cast< coord_type >( ::rand( ) % (get_x_size( ) - (x_size - 1))),
            x_size,
            static_cast< coord_type >( ::rand( ) % (get_y_size( ) - (y_size - 1))),
            y_size,
            ((::rand( ) / static_cast< value_type >( RAND_MAX)) * 2) - 1);
}

  bool
  sheet_control_type::
set_next_sheet_at
 (  coord_type  x
  , coord_type  y
  , value_type  value
 )
{
    d_assert( ! is_next_solve_pending( ));
    d_assert( p_sheet_next_);
    return p_sheet_next_->set_value_at( value, x, y);
}

  bool
  sheet_control_type::
set_next_sheet_at
 (  coord_type  x_lo
  , coord_type  x_hi_plus
  , coord_type  y_lo
  , coord_type  y_hi_plus
  , value_type  value
 )
{
    d_assert( ! is_next_solve_pending( ));
    d_assert( p_sheet_next_);
    return p_sheet_next_->fill_rectangle_coords( value, x_lo, x_hi_plus, y_lo, y_hi_plus);
}

  bool
  sheet_control_type::
set_next_sheet_at_size_rect
 (  coord_type  x_lo
  , size_type   x_size
  , coord_type  y_lo
  , size_type   y_size
  , value_type  value
 )
{
    d_assert( ! is_next_solve_pending( ));
    d_assert( p_sheet_next_);
    return p_sheet_next_->fill_rectangle_widths( value, x_lo, x_size, y_lo, y_size);
}

  bool
  sheet_control_type::
set_next_sheet_at_margin_rect
 (  coord_type  x_lo_margin
  , coord_type  x_hi_margin
  , coord_type  y_lo_margin
  , coord_type  y_hi_margin
  , value_type  value
 )
  // The margins are the distances from the edges of the sheet to where we start marking
  // the sheet. We use coord_type instead of size_type because we allow zero margins in
  // this method, while an x-margin of get_x_size( ) is too big. So these margins act more
  // like coords than sizes.
{
    d_assert( ! is_next_solve_pending( ));
    d_assert( p_sheet_next_);
    return p_sheet_next_->fill_rectangle_margins( value, x_lo_margin, x_hi_margin, y_lo_margin, y_hi_margin);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  namespace /* anonymous */ {
sheet_control_type::size_type  const init_size  = 100;
sheet_control_type::value_type const init_value = 0;
  } /* end namespace anonymous */

  void
  sheet_control_type::
init_sheets( )
{
    d_assert( ! is_next_solve_pending( ));
    d_assert( ! p_sheet_current_);
    d_assert( ! p_sheet_next_);
    d_assert( ! p_sheet_extra_);

    // Allocate the two sheets.
    sheet_a_.set_xy_counts( init_size, init_size, init_value);
    p_sheet_current_ = & sheet_a_;

    // This will be the first sheet because set_init_test( ) below will swap
    // the current and next sheets.
    sheet_b_.set_xy_counts_raw_values( init_size, init_size);
    p_sheet_next_ = & sheet_b_;

    // The extra sheet is sized as needed.
    sheet_c_.reset( ); /* not necessary */
    p_sheet_extra_ = & sheet_c_;

    // There is no history yet.
    is_next_sheet_valid_history_ = false;

    set_init_test( );
    d_assert( get_sheet_generation( ) == 0);
}

  /* slot */
  void
  sheet_control_type::
request_set_xy_sizes
 (  unsigned /* size_type */   x_size
  , unsigned /* size_type */   y_size
  , float    /* value_type */  init_value
 )
{
    if ( are_requests_delayed( ) ) {
        is_requested_set_xy_sizes_            = true;
        is_requested_set_xy_sizes_with_value_ = true;
        requested_set_xy_sizes_x_             = x_size;
        requested_set_xy_sizes_y_             = y_size;
        requested_set_xy_sizes_value_         = init_value;
    } else {
        set_xy_sizes( x_size, y_size, init_value);
    }
}

  void
  sheet_control_type::
set_xy_sizes
 (  size_type   x_size
  , size_type   y_size
  , value_type  init_value
 )
{
    d_assert( ! is_history_delta_in_extra_sheet_);
    d_assert( ! is_next_solve_pending( ));
    d_assert( p_sheet_current_);
    d_assert( p_sheet_next_);
    d_assert( p_sheet_extra_);

    // Clamp the sizes to the min/max allowable.
    if ( x_size < sheet_type::get_min_x_count( ) ) { x_size = sheet_type::get_min_x_count( ); }
    if ( y_size < sheet_type::get_min_y_count( ) ) { y_size = sheet_type::get_min_y_count( ); }

    if ( x_size > sheet_type::get_max_x_count( ) ) { x_size = sheet_type::get_max_x_count( ); }
    if ( y_size > sheet_type::get_max_y_count( ) ) { y_size = sheet_type::get_max_y_count( ); }

    // This doesn't do anything, but I include it because I want to bracket all changes to the
    // next and extra sheets.
    prepare_for_transform( e_do_not_init_next_sheet, false);

    // History will be killed.
    is_next_sheet_valid_history_ = false;

    if ( (get_x_size( ) == x_size) && (get_y_size( ) == y_size) ) {
        // The size is not changing.
        p_sheet_next_->fill_sheet( init_value); /* destroys history */
        after_transform( );
    } else
    {
        // The size is changing.
        p_sheet_next_->set_xy_counts( x_size, y_size, init_value); /* destroys history */
        p_sheet_extra_->reset( );

        // We can do this even though next and current sheets are different sizes.
        // This swaps the current and next pointers.
        after_transform( );

        // Resize the sheet we missed. This used to be the "current", but now it's the "next".
        // This breaks a (minor) rule, that we should bracket changes to the next sheet. In other
        // words, we should not change the next sheet without calling a "prepare" function first.
        p_sheet_next_->set_xy_counts_raw_values( x_size, y_size);

        // Call this whenever we change sheet size.
        after_master_sheet_size_change( );
    }
    d_assert( ! is_next_sheet_valid_history_);
}

  /* slot */
  void
  sheet_control_type::
request_set_xy_sizes
 (  unsigned /* size_type */  x_size
  , unsigned /* size_type */  y_size
 )
{
    if ( are_requests_delayed( ) ) {
        is_requested_set_xy_sizes_            = true;
        is_requested_set_xy_sizes_with_value_ = false;
        requested_set_xy_sizes_x_             = x_size;
        requested_set_xy_sizes_y_             = y_size;
    } else {
        set_xy_sizes( x_size, y_size);
    }
}

  void
  sheet_control_type::
set_xy_sizes
 (  size_type  x_size
  , size_type  y_size
 )
{
    d_assert( ! is_history_delta_in_extra_sheet_);
    d_assert( ! is_next_solve_pending( ));
    d_assert( p_sheet_current_);
    d_assert( p_sheet_next_);
    d_assert( p_sheet_extra_);

    // Clamp the sizes to the min/max allowable.
    if ( x_size < sheet_type::get_min_x_count( ) ) { x_size = sheet_type::get_min_x_count( ); }
    if ( y_size < sheet_type::get_min_y_count( ) ) { y_size = sheet_type::get_min_y_count( ); }

    if ( x_size > sheet_type::get_max_x_count( ) ) { x_size = sheet_type::get_max_x_count( ); }
    if ( y_size > sheet_type::get_max_y_count( ) ) { y_size = sheet_type::get_max_y_count( ); }

    if ( (get_x_size( ) == x_size) && (get_y_size( ) == y_size) ) {
        /* do nothing */
    } else
    {
        prepare_for_transform( );
        if ( is_history_delta_in_extra_sheet_ ) {
            p_sheet_next_->change_xy_counts( x_size, y_size);
            p_sheet_extra_->change_xy_counts( x_size, y_size);
            after_transform( );
            d_assert( is_next_sheet_valid_history_);
        } else
        {
            p_sheet_next_->change_xy_counts( x_size, y_size);
            p_sheet_extra_->reset( );
            after_transform( );
            d_assert( ! is_next_sheet_valid_history_);
            p_sheet_next_->set_xy_counts_raw_values( x_size, y_size);
        }

        // Call this whenever we change sheet size.
        after_master_sheet_size_change( );

        // The above is just one way to change the sizes while maybe preserving history.
        // Here are some other implementation possibilities:
      # if 0
        if ( is_next_sheet_valid_history_ &&
             get_heat_solver( )->is_technique__wave_with_damping( ) )
        {
            // One way to preserve history:
            //   p_sheet_extra_->reset( );
            //   p_sheet_next_->change_xy_counts( x_count, y_count)
            //   p_sheet_current_->change_xy_counts( x_count, y_count)
            //
            // This has some problems:
            //   We're not supposed to change the current sheet.
            //   This does not increment the generation or call after_transform( ).

            // Another way to preserve history:
            //   p_sheet_extra_->reset( );
            //
            //   prepare_for_transform( e_do_not_init_next_sheet, false);
            //   p_sheet_next_->change_xy_counts( x_count, y_count);
            //   after_transform( );
            //   d_assert( ! is_next_sheet_valid_history_);
            //
            //   prepare_for_transform( e_do_not_init_next_sheet, false);
            //   p_sheet_next_->change_xy_counts( x_count, y_count);
            //   after_transform( );
            //   d_assert( ! is_next_sheet_valid_history_);
            //   is_next_sheet_valid_history_ = true;
            //
            // Problems:
            //   Increments generation twice.

            // Another way:
            //   prepare_for_transform( e_do_not_init_next_sheet, false);
            //   (*p_sheet_extra_) = (*p_sheet_next_);
            //   p_sheet_next_->set_xy_counts_raw_values( x_size, y_size);
            //   sheet_type::copy_preserve_heights( *p_sheet_current_, *p_sheet_next_);
            //   after_transform( );
            //   d_assert( ! is_next_sheet_valid_history_);
            //
            //   p_sheet_next_->set_xy_counts_raw_values( x_size, y_size);
            //   sheet_type::copy_preserve_heights( *p_sheet_extra_, *p_sheet_next_);
            //   p_sheet_extra_->reset( );
            //   is_next_sheet_valid_history_ = true;
        } else
        {
            prepare_for_transform( e_do_not_init_next_sheet, false);

            // Resize the next sheet. This destroys the values.
            p_sheet_next_->set_xy_counts_raw_values( x_size, y_size);
            p_sheet_extra_->reset( );

            // The current and next sheets are now different sizes.
            // The next step copies current -> next sheets in a way that preserves the
            // values. We do not call copy_current_to_next_sheet( ) because that assumes the sheets
            // have the same size, which they almost always do.
            sheet_type::copy_preserve_heights( *p_sheet_current_, *p_sheet_next_);

            // We can do this even though next and current sheets are different sizes.
            // This swaps the current and next pointers.
            after_transform( );
            d_assert( ! is_next_sheet_valid_history_);

            // What was the current sheet is now the next sheet.
            // Resize it to be the same size as the other sheet.
            // This destroys the values.
            p_sheet_next_->set_xy_counts_raw_values( x_size, y_size);
        }
      # endif
    }
}

// _______________________________________________________________________________________________

  sheet_type const &
  sheet_control_type::
get_sheet_for_draw( )
{
    d_assert( p_sheet_current_);

    // You can get a quick idea of what the first time derivative for the sheet looks like by
    // returning it here instead of the sheet. For example, this code:

    // The following is an experiment to let you see what the first time derivative for the sheet
    // looks like. It jerks around a little, but it gives you a good idea. You see pretty clear
    // orthogonal effects.
  # if 0
    if ( get_heat_solver( )->get_extra_pass_count( ) == 0 ) {
        // You cannot call maybe_stamp_history_on_extra_sheet( true) here because we could be in the
        // middle of a solve. But we can do this:
        if ( is_history_delta_in_extra_sheet_ ) {
            // We'll probably almost never get here.
            // This is a very buggy kludge of course, especially if you are solving with extra passes.
            // It won't break anything, but the drawn sheet may be changing while we're drawing it.
            return * p_sheet_extra_;
        }
        // Since we're not using extra (we just checked above) we can set it here:
        if ( is_next_sheet_valid_history_ ) {
            // This works even if extra-sheet starts out not sized correctly.
            (*p_sheet_extra_) = (*p_sheet_next_);
            // Current and extra should now have the same size. operator -= asserts it.
            (*p_sheet_extra_) -= (*p_sheet_current_);
            // The time derivative tends to be a pretty flat sheet. Since we don't know the min/max
            // right now just magnify it by 10.
            (*p_sheet_extra_) *= 10;
            return * p_sheet_extra_;
        }
    }
  # endif

    // Check if we have a lower-resolution sheet to return for drawing.
    sheet_type const * p_sheet_limited_draw = maybe_get_limited_draw_sheet( );
    if ( p_sheet_limited_draw ) return * p_sheet_limited_draw;

    // Return full-resolution sheet.
    return * p_sheet_current_;
}

// _______________________________________________________________________________________________

  void
  sheet_control_type::
prepare_for_transform
 (  e_next_sheet_init_type  next_sheet_init        /* = e_copy_current_to_next_sheet */
  , bool                    is_history_meaningful  /* = true */
 )
  // History values:
  //   History will be meaningless after the next transform so don't bother saving it.
  //   History should be saved if possible (not solving wave, but we probably will soon).
  //   History should be saved if necessary (if we are solving wave).
  // Another way to look at it:
  //   Should we discard history no matter what.
  //     Yes if the next transform is incompatible with history.
  //   Should we save history even if we're not wave solving.
  //     Yes if we're only stepping away from wave for a moment.
{
    d_assert( ! is_history_delta_in_extra_sheet_);
    d_assert( ! is_next_solve_pending( ));
    d_assert( p_sheet_current_);
    d_assert( p_sheet_next_);

    maybe_stamp_history_on_extra_sheet(
        is_history_meaningful &&
        get_heat_solver( )->is_technique__wave_with_damping( ));

    switch ( next_sheet_init ) {
      case e_copy_current_to_next_sheet:
        copy_current_to_next_sheet( );
        break;
      case e_flatten_next_sheet:
        flatten_next_sheet( );
        break;
      default:
        d_assert( e_do_not_init_next_sheet == next_sheet_init);
        break;
    }
}

  void
  sheet_control_type::
after_transform__cancel( )
{
    d_assert( ! is_next_solve_pending( ));
    d_assert( p_sheet_current_);
    d_assert( p_sheet_next_);

    // Should we set is_next_sheet_valid_history_ to false? No, it should be OK
    // as long as we did not change the next sheet.

    // Forget any history that may be in the extra sheet.
    is_history_delta_in_extra_sheet_ = false;
}

  void
  sheet_control_type::
after_transform( )
  //
  // We call this after we transform the sheet.
  //
  // We also call this when we change one of these options:
  //   is_center_frozen
  //   is_vortex_on
  // But only if we are
  //   switching these options from off to on, and
  //   not auto-solving
  //
  // We do NOT call this after:
  //   The reverse-wave transform
  //   Canceling a transform
  //   Solve or multi-solve
{
    d_assert( ! is_next_solve_pending( ));
    d_assert( p_sheet_current_);
    d_assert( p_sheet_next_);

    // Should we increment the generation after a generation change, or only after a solve?
    increment_generation( );

    // These are experiments that should be moved if they are kept.
    // These should work whether on not history is valid, as long as the current sheet has
    // meaningful values (and not random noise) in it.
    // These are OK even at generation zero (although we don't go thru here setting up gen zero).
    //
    // Skip maybe_do_edge_fixing( ) after a transform. We only do this after a solve.
    maybe_do_center_freeze( );
    maybe_do_vortex( );

    // Make the next sheet the current one, and the current one the "next" one (except it
    // hasn't been calculated yet). We do this by swapping the pointers. *p_sheet_next is now
    // the "last" instead of the "next" sheet, which only matters if history is valid and we
    // are solving the wave equation.
    boost::swap( p_sheet_current_, p_sheet_next_);

    // Fix up history. During a transform history is saved in the extra sheet (when it is saved at all).
    restore_history_in_extra_sheet_if_available( );

    // If we have a lo-resolution drawing sheet, it will have to be set from the new current sheet.
    after_master_sheet_value_change( );

    // Tell the world the current sheet now has different values.
    emit sheet_is_changed( );
}

  void
  sheet_control_type::
transform__reverse_wave( )
{
    // The caller should check this before invoking this function.
    d_assert( is_next_sheet_valid_history_);

    d_assert( ! is_history_delta_in_extra_sheet_);
    d_assert( ! is_next_solve_pending( ));
    d_assert( p_sheet_current_);
    d_assert( p_sheet_next_);

    // Should we increment the generation after a reverse_wave, or only after a solve?
    // Maybe we should count down backwards?
    increment_generation( );

    // These seem unnecessary when executing a wave reverse. But imagine this scenario:
    //   We are solving with the wave equation.
    //   We stop the solve.
    //   We fix the edges.
    //   We reverse the wave.
    //   We start the edge-solve again.
    // If we didn't do the following we'd have the edges un-fixed for a single generation.
    maybe_do_edge_fixing( );
    maybe_do_center_freeze( );
    maybe_do_vortex( );

    // Make the next sheet the current one, and the current one the "next" one.
    // This reverses the momentum and restores the sheet to how it was the generation before.
    boost::swap( p_sheet_current_, p_sheet_next_);

    // Leave is_next_sheet_valid_history_ set to true.

    // If we have a lo-resolution drawing sheet, it will have to be set from the new current sheet.
    after_master_sheet_value_change( );

    // Tell the world the current sheet now has different values.
    emit sheet_is_changed( );
}

  void
  sheet_control_type::
after_solve( )
{
    d_assert( ! is_history_delta_in_extra_sheet_);
    d_assert( ! is_next_solve_pending( ));
    d_assert( p_sheet_current_);
    d_assert( p_sheet_next_);
    d_assert( p_sheet_extra_);

    // Get the output params from the last solve.
    heat_solver::output_params_type const * const
        p_output_params = get_heat_solver( )->get_output_params( );
    d_assert( ! p_output_params->is_early_exit( ));

    // Increment the generation by solve_count.
    // Maybe we should just increment by 1?
    {   size_type const  solve_count_u  = p_output_params->get_solve_count( );
        gen_type  const  solve_count    = static_cast< gen_type >( solve_count_u);
        d_assert( solve_count > 0);
        d_assert( static_cast< size_type >( solve_count) == solve_count_u);
        increment_generation( solve_count);
    }

    // These are experiments that should be moved if they are kept.
    // These should work whether on not history is valid, as long as the current sheet has
    // meaningful values (and not random noise) in it.
    // These are OK even at generation zero (although we don't go thru here setting up gen zero).
    //
    // These move values from current to next.
    maybe_do_edge_fixing( );
    maybe_do_center_freeze( );
    maybe_do_vortex( );

    // Swap the pointers. *p_sheet_next is now the "last" instead of the "next" sheet, which
    // only matters if history is valid and we are solving the wave equation.
    boost::swap( p_sheet_current_, p_sheet_next_);

    // The history might be in extra. If so we need to swap again.
    if ( p_output_params->is_last_solve_saved_in_extra( ) ) {
        // History is in extra. But we want it in next.
        d_assert( p_sheet_extra_->get_x_count( ) == p_sheet_next_->get_x_count( ));
        d_assert( p_sheet_extra_->get_y_count( ) == p_sheet_next_->get_y_count( ));
        boost::swap( p_sheet_extra_, p_sheet_next_);
        is_next_sheet_valid_history_ = true;
    } else
    if ( p_output_params->is_last_solve_saved_in_src( ) ) {
        // History WAS in src (current), but it's not in "next" (because of the swap above).
        is_next_sheet_valid_history_ = true;
    } else
    /* p_output_params->is_last_solve_not_saved( ) */ {
        d_assert( p_output_params->is_last_solve_not_saved( ));
        is_next_sheet_valid_history_ = false;
    }

    // If we have a lo-resolution drawing sheet, it will have to be set from the new current sheet.
    after_master_sheet_value_change( );

    // Tell the world the current sheet now has different values.
    emit sheet_is_changed( );
}

// _______________________________________________________________________________________________

  void
  sheet_control_type::
copy_current_to_next_sheet( )
{
    d_assert( ! is_next_solve_pending( ));
    d_assert( p_sheet_current_);
    d_assert( p_sheet_next_);

    // The sheets are the same size.
    d_assert( p_sheet_next_->get_x_count( ) == p_sheet_current_->get_x_count( ));
    d_assert( p_sheet_next_->get_y_count( ) == p_sheet_current_->get_y_count( ));

    // Copy all the values.
    (*p_sheet_next_) = (*p_sheet_current_);

    // This is equivalent to the above if the sheets are the same size:
    // d_verify( sheet_type::copy_preserve_heights( *p_sheet_current_, *p_sheet_next_));

    // If the next sheet used to have history it doesn't now.
    is_next_sheet_valid_history_ = false;
}

  void
  sheet_control_type::
increment_generation( gen_type amount /* = 1 */)
  //
  // Should we have separate generation counts for solves and other kinds of transforms?
  // Should this be resettable?
  // During multi-solve should we increment the generation by 1 or by the multi-count?
  // Do we ever decrement this?
{
    // This is how we used to do this, when amount was always 1.
    //    if ( generation_current_ == std::numeric_limits< gen_type >::max( ) ) {
    //        generation_current_ = 0;
    //    } else {
    //        generation_current_ += 1;
    //    }

    gen_type const generation_prev = generation_current_;
    generation_current_ += amount;
    if ( generation_current_ < 0 ) {
        generation_current_ = 0;
    } else
    if ( amount > 0 ) {
        if ( generation_current_ < generation_prev ) {
            generation_current_ = 0;
        }
    } else
    if ( amount < 0 ) {
        if ( generation_current_ > generation_prev ) {
            generation_current_ = 0;
        }
    }
    d_assert( generation_current_ >= 0);
}

// _______________________________________________________________________________________________

  void
  sheet_control_type::
maybe_stamp_history_on_extra_sheet( bool is_history_worth_it)
{
    d_assert( ! is_history_delta_in_extra_sheet_);
    d_assert( ! is_next_solve_pending( ));
    d_assert( p_sheet_current_);
    d_assert( p_sheet_next_);
    d_assert( p_sheet_extra_);

    // It's expensive to save history so only do it if we need it (and if we even
    // have a history to save).
    if ( is_next_sheet_valid_history_ && is_history_worth_it ) {
        // This works even if extra-sheet starts out not sized correctly.
        (*p_sheet_extra_) = (*p_sheet_next_);
        // Current and extra should now have the same size. operator -= asserts it.
        (*p_sheet_extra_) -= (*p_sheet_current_);

        is_history_delta_in_extra_sheet_ = true;
    } else {
        // Not necessary, this is already false.
        is_history_delta_in_extra_sheet_ = false;
    }
}

  void
  sheet_control_type::
maybe_scale_saved_history( value_type scale)
{
    d_assert( p_sheet_extra_);

    if ( is_history_delta_in_extra_sheet_ ) {
        if ( 0 == scale ) {
            is_history_delta_in_extra_sheet_ = false;
        } else {
            // We don't have to assume extra is sized the same as next, or even that it's not reset.
            // Maybe it's possible this will fail if we are in the middle of changing the sheet sizes.
            d_assert( p_sheet_extra_->get_x_count( ) == p_sheet_next_->get_x_count( ));
            d_assert( p_sheet_extra_->get_y_count( ) == p_sheet_next_->get_y_count( ));
            (*p_sheet_extra_) *= scale;
        }
    }
}

  void
  sheet_control_type::
restore_history_in_extra_sheet_if_available( )
  //
  // This must be called after p_current_sheet_ and p_next_sheet_ are swapped.
{
    d_assert( p_sheet_current_);
    d_assert( p_sheet_next_);
    d_assert( p_sheet_extra_);

    if ( is_history_delta_in_extra_sheet_ ) {
        is_history_delta_in_extra_sheet_ = false;
        is_next_sheet_valid_history_ = true;

        // Next will be sized to be the same as current.
        (*p_sheet_next_) = (*p_sheet_current_);
        // Assumes (asserts) extra is same size as next (and thus current).
        (*p_sheet_next_) += (*p_sheet_extra_);
    } else {
        is_next_sheet_valid_history_ = false;
    }
}

// _______________________________________________________________________________________________

  /* slot */
  void
  sheet_control_type::
set__are_edges_fixed( bool are_they)
{
    are_edges_fixed_ = are_they;
}

  void
  sheet_control_type::
maybe_do_edge_fixing( )
  //
  // Fix the edges by copying the previous edge values to the next sheet.
  // Only do this when is_next_sheet_valid_history_ is set, so that the experimental
  // transforms can change the edge values. The edges are only locked when we are solving.
  //
  // This (fixing the edges) should not be here. If it is useful it should be moved into
  // the solver, and from there into the finite_diff functions.
{
    if ( is_next_sheet_valid_history_ && are_edges_fixed( ) ) {
        copy_current_edges_to_next_sheet( );
    }
}

  void
  sheet_control_type::
copy_current_edges_to_next_sheet( )
  //
  // This is just an experiment. Otherwise it would be moved to heat_solver and maybe to the
  // finite_diff solving code.
  //
  // Other experiments performed:
  //
  //   Set damping==1 along all the edges. This is a lot like fixing the edges except the edges
  //   move a little and it has no affect when we're not using the wave equation.
  //   Not very interesting.
  //
  //   Set all the first derivatives (gradients) along the edges to zero. We do this by copying
  //   the row that is one-in from the edge to the edge, so that (x1 - x0) is always zero along
  //   the edge.
  //   This was not very interesting either. It is like trimming the edge off and solving with
  //   a floating edge.
  //
  //   Set all the 2nd derivates along the edge to zero. We do this by looking at the slope
  //   between the 2nd and 3rd rows in, and then setting the 1st row so the slope from the
  //   1st to 2nd row was the same.
  //   This was not very interesting either. It damped the edge but did not stop reflections.
  //
  // Other experiments:
  //   Heat: treat the sheet like it's part of an infinite sheet where the edges approach a fixed
  //   value. We can do this by fixing the edges at the infinite value and set lo r-values leading
  //   to the edge. This way the sheet will slowly settle to the ground value.
  //   This will not work with waves because the waves will reflect off impedance-mismatched edges.
  //
  //   Wave: Fixed and floating edges are easy. To make edges that damp and do not reflect we need
  //   to introduce several layers with slowly increasing damping until the outside edge has damping==1.
  //   The more layers we have the smaller the residual reflection.
{
    bool const is_big_enough__x_size = (get_x_size( ) > 2);
    bool const is_big_enough__y_size = (get_y_size( ) > 2);

    if ( is_big_enough__x_size || is_big_enough__y_size ) {
        sheet_type::qq_const_range_type qq_range_src = p_sheet_current_->get_range_yx( );
        sheet_type::qq_varia_range_type qq_range_trg = p_sheet_next_   ->get_range_yx( );

        if ( is_big_enough__y_size ) {
            std::copy /* front */
             (  qq_range_src.get_iter_lo( ).get_range( ).get_iter_lo( )
              , qq_range_src.get_iter_lo( ).get_range( ).get_iter_post( )
              , qq_range_trg.get_iter_lo( ).get_range( ).get_iter_lo( )
             );
            std::copy /* back */
             (  qq_range_src.get_iter_hi( ).get_range( ).get_iter_lo( )
              , qq_range_src.get_iter_hi( ).get_range( ).get_iter_post( )
              , qq_range_trg.get_iter_hi( ).get_range( ).get_iter_lo( )
             );
        }

        swap_xy_in_place( qq_range_src);
        swap_xy_in_place( qq_range_trg);

        if ( is_big_enough__x_size ) {
            // Offset is usually 1. It keeps us from copying the corner twice.
            size_type const offset = is_big_enough__y_size ? 1 : 0;
            std::copy /* left */
             (  qq_range_src.get_iter_lo( ).get_range( ).get_iter_lo( ) + offset
              , qq_range_src.get_iter_lo( ).get_range( ).get_iter_post( ) - offset
              , qq_range_trg.get_iter_lo( ).get_range( ).get_iter_lo( ) + offset
             );
            std::copy /* right */
             (  qq_range_src.get_iter_hi( ).get_range( ).get_iter_lo( ) + offset
              , qq_range_src.get_iter_hi( ).get_range( ).get_iter_post( ) - offset
              , qq_range_trg.get_iter_hi( ).get_range( ).get_iter_lo( ) + offset
             );
        }
    }
}

// _______________________________________________________________________________________________

  /* slot */
  void
  sheet_control_type::
set__is_center_frozen( bool is_it)
  //
  // This could be animated.
  // This (the frozen center value, and the xy location) could be attached to scroll bars.
{
    if ( is_it != is_center_frozen_ ) {
        is_center_frozen_ = is_it;

        // If we are not auto-solving we do the following so the frozen center shows up.
        // If we are not auto-solving this throws away history. But if we are auto-solving then
        // history is kept. This only matters if we are using history (solving the wave equation).
        if ( is_center_frozen( ) && ! is_auto_solving( ) ) {
            prepare_for_transform( );
            after_transform( );
        }
    }
}

  void
  sheet_control_type::
maybe_do_center_freeze( )
{
    if ( is_center_frozen( ) ) {
        set_next_sheet_at( get_x_size( ) / 2, get_y_size( ) / 2, -1);
    }
}

// _______________________________________________________________________________________________

  /* slot */
  void
  sheet_control_type::
set__is_vortex_on( bool is_it)
  //
  // This should be an animation. It is just here because experiments usually start here and
  // and then get moved to the sheet, the solver, or somewhere else later.
{
    if ( is_it != is_vortex_on_ ) {
        is_vortex_on_ = is_it;

        // If we are not auto-solving we do the following when we turn this on.
        if ( is_vortex_on( ) && ! is_auto_solving( ) ) {
            prepare_for_transform( );
            after_transform( );
        }
    }
}

  void
  sheet_control_type::
maybe_do_vortex( )
{
    if ( is_vortex_on( ) ) {
        float      const  pi          = static_cast< float >( angle_holder::pi);
        float      const  angle       = pi * static_cast< float >( generation_current_ % 30) / 15;

        size_type  const  center_x    = get_x_size( ) / 2;
        size_type  const  center_y    = get_y_size( ) / 2;

        size_type  const  radius_x    = get_x_size( ) * 5 / 11;
        size_type  const  radius_y    = get_y_size( ) * 5 / 11;

        size_type const   x_offset    = static_cast< size_type >( std::cos( angle) * radius_x);
        size_type const   y_offset    = static_cast< size_type >( std::sin( angle) * radius_y);

        set_next_sheet_at( center_x + x_offset, center_y + y_offset, 1);
    }
}

// _______________________________________________________________________________________________
// Draw-size limits - limited resolution draw
//
//   Limited resolution draw could also be accomplished with resolution-changing iterators
//   (based on line_walker_type). This isn't written yet for a 2D iterator.
//
//   Consider pulling this out into a separate class.
//
//   The following member vars are set from the outside:
//     is_draw_size_limited_
//     xy_draw_size_limit_
//
//   The following member vars are calculated:
//     x_draw_size_limit_
//     y_draw_size_limit_
//     sheet_limited_draw_
//     is_limited_draw_sheet_written_

  sheet_type const *
  sheet_control_type::
maybe_get_limited_draw_sheet( )
{
    // Check if we have a lower-resolution sheet to return for drawing.
    if ( is_draw_size_limited( ) && get_x_draw_size_limit( ) && get_y_draw_size_limit( ) ) {
        // Copy the values in the current sheet to the lo-res sheet if it's out-of-date.
        if ( ! is_limited_draw_sheet_written_ ) {
            d_verify( sheet_type::copy_preserve_heights( *p_sheet_current_, sheet_limited_draw_));
            is_limited_draw_sheet_written_ = true;
        }
        // Return the lo-res drawing sheet.
        return & sheet_limited_draw_;
    }
    return 0;
}

  void
  sheet_control_type::
init_draw_size_limits( )
  //
  // Sets these:
  //   x_draw_size_limit_
  //   y_draw_size_limit_
  //   sheet_limited_draw_
  //   is_limited_draw_sheet_written_
  //
  // Based on these:
  //   is_draw_size_limited_
  //   xy_draw_size_limit_
{
    after_change_that_affects_limited_draw( );
}

  void
  sheet_control_type::
after_master_sheet_size_change( )
{
    after_change_that_affects_limited_draw( );
}

  void
  sheet_control_type::
after_master_sheet_value_change( )
{
    // If we have a lo-resolution drawing sheet, it will have to be set from the new current sheet.
    is_limited_draw_sheet_written_ = false;
}

  /* slot */
  void
  sheet_control_type::
set__is_draw_size_limited( bool new__is_limited)
{
    d_assert( (new__is_limited == true) || (new__is_limited == false));

    assert_draw_size_setup( );

    if ( is_draw_size_limited_ != new__is_limited ) {
        is_draw_size_limited_ = new__is_limited;
        after_change_that_affects_limited_draw( );
    }
}

  /* not a slot */
  void
  sheet_control_type::
set_xy_draw_size_limit( size_type new_xy_draw_size_limit)
{
    assert_draw_size_setup( );
    xy_draw_size_limit_ = new_xy_draw_size_limit;
    after_change_that_affects_limited_draw( );
}

  /* slot */
  void
  sheet_control_type::
after_change_that_affects_limited_draw( )
{
    calc_x_and_y_draw_size_limits( );
    setup_sheet_limited_draw( );
    assert_draw_size_setup( );
}

  /* private */
  void
  sheet_control_type::
calc_x_and_y_draw_size_limits( )
  //
  // Call this after changing get_xy_draw_size_limit( ).
  // Calculates:
  //   x_draw_size_limit_
  //   y_draw_size_limit_
  //   ratio_xy_sheet_to_xy_limit_
{
    size_type const x_full  = get_x_size( );
    size_type const y_full  = get_y_size( );
    size_type const xy_full = x_full * y_full;

    if ( xy_full <= get_xy_draw_size_limit( ) ) {
        x_draw_size_limit_ = 0;
        y_draw_size_limit_ = 0;
        ratio_xy_sheet_to_xy_limit_ = 1;
    } else
    /* get_xy_draw_size_limit( ) < xy_full */ {
        // Calculate the ideal sizes.
        float const df_ratio = std::sqrt( static_cast< float >( get_xy_draw_size_limit( )) / xy_full);
        x_draw_size_limit_ = static_cast< size_type >( (df_ratio * x_full) + 0.5f);
        y_draw_size_limit_ = static_cast< size_type >( (df_ratio * y_full) + 0.5f);

        // While the sizes are slightly too big, decrement one or both of them.
        while ( (x_draw_size_limit_ * y_draw_size_limit_) > get_xy_draw_size_limit( ) ) {

            // First decrement both.
            -- x_draw_size_limit_;
            -- y_draw_size_limit_;

            // If decrementing them both brings them under the limit (and it should), then
            // see if we could have gotten away with incrementing only one.
            if ( (x_draw_size_limit_ * y_draw_size_limit_) < get_xy_draw_size_limit( ) ) {
                size_type const xy_draw_inc_x = (x_draw_size_limit_ + 1) * y_draw_size_limit_;
                size_type const xy_draw_inc_y = x_draw_size_limit_ * (y_draw_size_limit_ + 1);
                if ( (xy_draw_inc_x > get_xy_draw_size_limit( )) &&
                     (xy_draw_inc_y > get_xy_draw_size_limit( )) )
                {
                    // Incrementing either overflows the limit.
                    /* do nothing */
                } else
                if ( xy_draw_inc_x > get_xy_draw_size_limit( ) ) {
                    // Incrementing x will overflow, but not y.
                    d_assert( xy_draw_inc_y <= get_xy_draw_size_limit( ));
                    ++ y_draw_size_limit_;
                } else
                if ( xy_draw_inc_y > get_xy_draw_size_limit( ) ) {
                    // Incrementing y will overflow, but not x.
                    d_assert( xy_draw_inc_x <= get_xy_draw_size_limit( ));
                    ++ x_draw_size_limit_;
                } else
                /*  */ {
                    // Incrementing either will not overflow (although incrementing both will).
                    d_assert( xy_draw_inc_x <= get_xy_draw_size_limit( ));
                    d_assert( xy_draw_inc_y <= get_xy_draw_size_limit( ));
                    // Keep the increment where the resulting product is closest to the upper limit.
                    if ( (get_xy_draw_size_limit( ) - xy_draw_inc_x) <
                         (get_xy_draw_size_limit( ) - xy_draw_inc_y) )
                    {
                        // Incrementing x_draw_size_limit_ is a closer fit.
                        ++ x_draw_size_limit_;
                    } else {
                        // Incrementing y_draw_size_limit_ is at least as good, and maybe better, than
                        // incrementing x_draw_size_limit_.
                        // If these are equal we could test further:
                        //   Increment the smallest of x_draw_size_limit_ and y_draw_size_limit_.
                        //   Increment to get the closest to the true floating value.
                        //   Increment whichever gets us closest to the original ratio.
                        ++ y_draw_size_limit_;
                    }
                }
            }
        }

        // We cannot draw a sheet with a size < 2, so make sure both sizes are >= 2.
        if ( x_draw_size_limit_ < 2 ) {
            x_draw_size_limit_ = 2;
            y_draw_size_limit_ = get_xy_draw_size_limit( ) / 2;
            if ( y_draw_size_limit_ < 2 ) {
                y_draw_size_limit_ = 2;
            }
        } else
        if ( y_draw_size_limit_ < 2 ) {
            y_draw_size_limit_ = 2;
            x_draw_size_limit_ = get_xy_draw_size_limit( ) / 2;
            if ( x_draw_size_limit_ < 2 ) {
                x_draw_size_limit_ = 2;
            }
        }

        // We can use the following to correct the aspect ratio of the drawing when it is changed slightly
        // by limiting the sizes.
        d_assert( x_draw_size_limit_ > 1);
        d_assert( y_draw_size_limit_ > 1);
        d_assert( x_full > 1);
        d_assert( y_full > 1);
        ratio_xy_sheet_to_xy_limit_ =
            (static_cast< float >( x_full            ) / y_full            ) /
            (static_cast< float >( x_draw_size_limit_) / y_draw_size_limit_) ;
    }
}

  /* private */
  void
  sheet_control_type::
setup_sheet_limited_draw( )
  //
  // Sets sheet_limited_draw_ and is_limited_draw_sheet_written_.
{
    bool is_draw_sheet_changed = false;

    if ( is_draw_size_limited( ) && get_x_draw_size_limit( ) && get_y_draw_size_limit( ) ) {
        if ( (get_x_draw_size_limit( ) != sheet_limited_draw_.get_x_count( )) ||
             (get_y_draw_size_limit( ) != sheet_limited_draw_.get_y_count( )) )
        {
            if ( (! get_x_draw_size_limit( )) ||
                 (! get_y_draw_size_limit( )) )
            {
                sheet_limited_draw_.reset( );
            } else {
                sheet_limited_draw_.set_xy_counts_raw_values( get_x_draw_size_limit( ), get_y_draw_size_limit( ));
                is_limited_draw_sheet_written_ = false;
            }
            is_draw_sheet_changed = true;
        }
    } else
    if ( sheet_limited_draw_.not_reset( ) ) {
        sheet_limited_draw_.reset( );
        is_draw_sheet_changed = true;
    }

    if ( is_draw_sheet_changed ) {
        emit draw_size_limit_is_changed( );
    }
}

# ifndef NDEBUG
  void
  sheet_control_type::
assert_draw_size_setup( ) const
{
    if ( (0 == x_draw_size_limit_) || (0 == y_draw_size_limit_) ) {
        d_assert( (x_draw_size_limit_ == 0) && (y_draw_size_limit_ == 0));
        d_assert( sheet_limited_draw_.is_reset( ));
    } else {
        d_assert( (x_draw_size_limit_ >= 2) && (y_draw_size_limit_ >= 2));
        if ( is_draw_size_limited_ ) {
            d_assert( sheet_limited_draw_.get_x_count( ) == x_draw_size_limit_);
            d_assert( sheet_limited_draw_.get_y_count( ) == y_draw_size_limit_);
        } else {
            d_assert( sheet_limited_draw_.is_reset( ));
        }
    }
}
# endif

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Everything commented-out after this

# if 0
  bool
  sheet_control_type::
set_sheet_at_r
 (  rcoord_type  rx
  , rcoord_type  ry
  , value_type   value
 )
{
    return
        is_rcoord_in_range( rx) &&
        is_rcoord_in_range( ry) &&
        set_sheet_at( get_x_coord( rx), get_y_coord( ry), value);
}

  bool
  sheet_control_type::
set_sheet_at_r
 (  rcoord_type  rx_lo
  , rcoord_type  rx_hi
  , rcoord_type  ry_lo
  , rcoord_type  ry_hi
  , value_type   value
 )
{
    return
        is_rcoord_in_range( rx_lo) &&
        is_rcoord_in_range( rx_hi) &&
        is_rcoord_in_range( ry_lo) &&
        is_rcoord_in_range( ry_hi) &&
        set_sheet_at(
            get_x_coord( rx_lo), get_x_coord( rx_hi),
            get_y_coord( ry_lo), get_y_coord( ry_hi),
            value);
}

  bool
  sheet_control_type::
set_sheet_at_margin_rect_r
 (  rcoord_type  rx_lo_margin
  , rcoord_type  rx_hi_margin
  , rcoord_type  ry_lo_margin
  , rcoord_type  ry_hi_margin
  , value_type   value
 )
{
    return
        is_rcoord_in_range( rx_lo_margin) &&
        is_rcoord_in_range( rx_hi_margin) &&
        is_rcoord_in_range( ry_lo_margin) &&
        is_rcoord_in_range( ry_hi_margin) &&
        set_sheet_at_margin_rect(
            get_x_coord( rx_lo_margin), get_x_coord( rx_hi_margin),
            get_y_coord( ry_lo_margin), get_y_coord( ry_hi_margin),
            value);
}

  bool
  sheet_control_type::
set_sheet_at_size_rect_r
 (  rcoord_type  rx_lo
  , rsize_type   rx_size
  , rcoord_type  ry_lo
  , rsize_type   ry_size
  , value_type   value
 )
{
    return
        is_rcoord_in_range( rx_lo  ) &&
        is_rsize_in_range(  rx_size) &&
        is_rcoord_in_range( ry_lo  ) &&
        is_rsize_in_range(  ry_size) &&
        set_sheet_at_size_rect(
            get_x_coord( rx_lo), get_x_size( rx_size),
            get_y_coord( ry_lo), get_y_size( ry_size),
            value);
}

// _______________________________________________________________________________________________

  /* static */
  sheet_control_type::coord_type
  sheet_control_type::
get_coord( rcoord_type nc, size_type count)
  //
  // This converts a normal_coord_type (-1.0 .. +1.0) to a coord in the sheet.
  // The coord is clamped to be part of the sheet (0 .. count-1).
{
    if ( nc <= -1 ) return 0;
    coord_type const c = static_cast< coord_type >( ((nc + 1) / 2) * count);
    return (c < count) ? c : (count - 1);
}

  /* static */
  sheet_control_type::size_type
  sheet_control_type::
get_size( rsize_type ns, size_type count)
  //
  // Sizes are just like coords except they range from (1 .. count) instead of (0 .. count-1).
{
    if ( ns <= 0 ) return 1;
    size_type const s = static_cast< size_type >( ns * count);
    return (s < 1) ? 1 : ((s > count) ? count : s);
}
# endif

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// solve_control.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
