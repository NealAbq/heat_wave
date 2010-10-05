// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// animate.cpp
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
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// UI:
//   Optional
//     Show start/start duration for animation. As either msecs or fps.
//     Actual fps (including "as fast as possible" of "no limit").
//     Requested fps.
//     See out_of_date_ui_type object.
//
//   For each animated control:
//     Check box: animate.set_on( bool)
//     Slider:
//       animate.set_tenths_degrees_per_second_1_1000( int)
//       animate.set_tenths_degrees_per_second_minus_plus_1000( int)
//
// Timer:
//   This uses a periodic timer.
//   QTimer::singleShot( 600000, this, SLOT( wakeup( ))) is also available.
//
//   The signal wakeup( ) probably filips an update( ) or repaint( ) slot in a widget.
//
//   We can rely on paint messages being processed even if the timer is set to full-speed
//   because WM_PAINT messages have higher priority than WM_TIMER messages.
//   Otherwise we'd want to repaint( ) instead of update( ).
//
//   The timer seems to fire at a lower frequency than requested. This implies that the
//   timer doesn't start timing for the next tick until after the last tick is finished.
//   Or maybe the timing is grainy and rounds down.
//   In any case, maybe this should be tweaking the timer to be faster when it runs
//   slow (and slowing it down if it's running fast).
//   But we must be careful not to tweak too much in case the timing is being driven
//   by something external:
//
//     Max frequency is limited by how fast and busy the machine is. But this should not be
//     a problem as we can just tweak the delay down to zero.
//
//     Min freq could be driven by something like auto-solve, in which case we should still
//     keep waking up, we just don't have to keep passing the wakeup() signals to the
//     heat widget.
//
//   We should intercept the timeout() signals from the periodic timer. Before we emit wakeup(),
//   we sould ask if we're expecting an externally-driven update soon. If so we can skip this wakeup(),
//   and maybe set a one-shot timer to look later and make sure we really have externally updated
//   later as expected.
//
//   When we start animation we always emit a wakeup() signal at first. But again we could watch to
//   make sure we haven't externally updated very recently.
//
// We keep a move-count instead of a wakeup-count because:
//   We'd have to define a new slot to catch timeout( ), count, and then signal wakeup( ).
//   We keep the duration stats during move, not during wakeup.
// _______________________________________________________________________________________________

# include "all.h"
# include "animate.h"
# include "bool_holder.h"
# include "pack_holder.h"

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// animate_type -- class
//
// This holds all the animators.
//
// Animation movement is time driven. It is not driven by another periodic/incrementing event,
// such as solver generations ticking forward.
// The exception is the single-step animation button, which moves the animation forward one frame.
// A frame has an implied time-duration, set when you the user sets a frames-per-second value.
//
// Suggestion: A way of animating in slow motion.
//
// This collaborates with a window, which tells it when it is about to paint.
// The animator does not update any values until just before a paint, in order to get more
// accurate timing.
//
// This holds a bunch of animators. Each can be turned on/off and is responsible for moving
// something. Since a collection of animators can also be treated like an animator, this
// class (animate_type) could be designed to be an animator also.
//
// Suggestion: Group animators, that hold a collection of subords and animate them.
//   This could lend a tree-like structure to the animators in the UI.
// Suggestion: Time transformations. Just as 3D coord transforms can be described using 4x4
//   matrices, time values can be considered a 1D coord and transformed via a 2x2 matrix.
//   This would allow linear transforms of the sort:
//     time_trg = (scale * time_src) + offset
// _______________________________________________________________________________________________

  /* constructor */
  animate_type::
animate_type( QObject * p_parent /* = 0 */)
  : holder_base_type       ( p_parent)
  , p_timer_               ( 0)
  , tick_last_             ( date_time::get_invalid_tick_pt( ))
  , animators_             ( )
  , is_animatable_         ( false)
  , start_start_durations_ ( 32)
  , move_count_            ( 0)
  , step_limit_            ( 0)
{
    // Create the timer. We use it for the life of this object.
    p_timer_ = new QTimer( this);
    d_assert( p_timer_);
    d_assert( ! p_timer_->isSingleShot( ));
    d_assert( ! p_timer_->isActive( ));

    // Set the interval to zero, which means run the animation as fast as possible.
    p_timer_->setInterval( 0);

    // Tell the timer to poke us when it is running.
    // Weakness. This hooks the timer signal directly to the wakeup() signal. But we should be
    // intercepting this signal and tweaking, in case things are running fast or slow, or in case
    // something else is driving animation, like auto-solve.
    d_verify( connect( p_timer_, SIGNAL( timeout( )), this, SIGNAL( wakeup( )) ));
}

// _______________________________________________________________________________________________

  /* private non-slot */
  animator_base_type *
  animate_type::
find_animator( holder_base_type * p_animated__look_for) const
{
    // The following is easier than using std::find_if(..) since we'd have to write a
    // custom functor (unless boost has a functor that fits?)
    list_type::const_iterator       iter     = animators_.begin( );
    list_type::const_iterator const iter_end = animators_.end( );
    while ( iter_end != iter ) {
        animator_base_type * p_animator__look_at = (* iter);
        if ( p_animator__look_at->get_animated( ) == p_animated__look_for ) {
            return p_animator__look_at;
        }
        ++ iter;
    }
    return 0;
}

  /* protected non-slot */
  void
  animate_type::
add_animator( animator_base_type * p_animator)
  //
  // This is called in the ctor for animator_base_type.
{
    d_assert( p_animator);

    // The new animator cannot change is_animatable( ) because it is not switched on.
    d_assert( ! p_animator->is_on( ));

    // Connect the animator so when it gets switched on/off we correct is_animatable( ).
    d_verify( connect(
        p_animator, SIGNAL( has_changed__is_on( bool)),
        this, SLOT( recalc_is_animatable( bool))
    ));

    // Add to the list.
    animators_.push_back( p_animator);
}

  /* protected non-slot */
  void
  animate_type::
remove_animator( animator_base_type * p_animator)
  //
  // This is called in the dtor for animator_base_type.
{
    d_assert( p_animator);

    // This is often called when the process is shutting down and deleting all the QObjects.
    // In that case we probably would like to skip the rest and just make this method a no-op.

    // Setting switched_on to false can signal and cause recalc_is_animatable(..).
    // Which in turn can call stop( ).
    p_animator->set_on( false);

    // Remove from the list and delete.
    d_verify( animators_.removeAll( p_animator) == 1);
}

// _______________________________________________________________________________________________

  /* public non-slot */
  void
  animate_type::
request_frames_per_second_log_0_1000( int log_value_0_1000)
  //
  // 0 means the slowest possible (about a frame every 5 seconds).
  // 1000 means the fastest possible for the system, with no delay between the frames.
{
    frames_per_second_type const log_value_01 = log_value_0_1000 / 1000.0;
    request_frames_per_second_log_01( log_value_01);
}

  /* public non-slot */
  void
  animate_type::
request_frames_per_second_log_01( frames_per_second_type log_value)
  //
  // This is an easier way to call request_frames_per_second(..).
  // This formula maps log-value to the following fps values:
  //
  //   log_value   fps -> exp((7*log_value)-1.5)
  //   =========   =============================
  //        0          .22 (a frame every 5 seconds)
  //       .1          .44 (a frame every 2 seconds)
  //       .2          .90 (a frame every second)
  //       .3         1.8  (2 fps)
  //       .4         3.7  (4 fps)
  //       .5         7.4  (7 fps)
  //       .6        15    (15 fps, which is pretty smooth)
  //       .7        30
  //       .8        60
  //       .9       121
  //      1.0       244
  //
  // At 0.95 (172 fps) and above this sets a delay of zero, asking for the fastest
  // speed the system can handle. So although with request_frames_per_second(..)
  // you can ask for 1000 fps, this never asks for more than 172.4 before just asking
  // for the maximum.
{
    frames_per_second_type frames_per_sec = -1.0;
    if ( log_value < 0 ) {
        frames_per_sec = 0; // slowest, a frame every 5 seconds in this case
    } else
    if ( log_value >= 0.95 ) {
        frames_per_sec = 1000.0; // as fast as possible for the system
    } else
    /* intermediate values */ {
        frames_per_sec = std::exp( (7.0 * log_value) - 1.5);
        d_assert( (0.2 <= frames_per_sec) && (frames_per_sec < 173.0));
    }
    d_assert( (0 <= frames_per_sec) && (frames_per_sec <= 1000.0));
    request_frames_per_second( frames_per_sec);
}

  /* public non-slot */
  void
  animate_type::
request_frames_per_second( frames_per_second_type frames_per_sec)
  //
  // frames_per_sec should be between these values:
  //   1/5 <= fps <= 1000
  //
  // We use this to timer.setInterval( msecs) but we don't need to store this value.
  // It is set-only, and changing it does not emit any signal.
{
    // We allow a min of zero instead of one because zero means a timer will fire whenever
    // there are no other higher-priority messages in the queue.
    int const  min_msecs_per_frame = 0;
    int const  max_msecs_per_frame = 5000; /* a frame every 5 seconds */

    frames_per_second_type const  min_frames_per_sec = 1000.0 / max_msecs_per_frame;
    frames_per_second_type const  max_frames_per_sec = 1000.0;

    int msecs_per_frame = -1;
    if ( frames_per_sec <= min_frames_per_sec ) {
        msecs_per_frame = max_msecs_per_frame;
    } else
    if ( frames_per_sec >= max_frames_per_sec ) {
        msecs_per_frame = min_msecs_per_frame;
    } else {
        msecs_per_frame = static_cast< int >( (1000.0 / frames_per_sec) + 0.5);
    }
    d_assert( (min_msecs_per_frame <= msecs_per_frame) && (msecs_per_frame <= max_msecs_per_frame));

    d_assert( p_timer_);
    p_timer_->setInterval( msecs_per_frame);

    // emit changed_frames_per_second( )?
}

  /* public */
  animate_type::frames_per_second_type
  animate_type::
get_requested_frames_per_second( ) const
  //
  // Calculates the requested fps from the interval stored with the timer.
  // So this is not exactly the same as the requested fps, due to rounding and the
  // millisecond granularity of the interval.
  //
  // Returns zero if no limit is requested.
  // Otherwise returns a value in the range of 0.2 (a frame every 5 seconds) to 1000.
{
    d_assert( p_timer_);
    int const interval_msecs = p_timer_->interval( );
    if ( interval_msecs <= 0 ) {
        return 0;
    }
    frames_per_second_type const
        frames_per_sec =
            1000.0 / static_cast< frames_per_second_type >( interval_msecs);
    d_assert( (0.2 <= frames_per_sec) && (frames_per_sec <= 1000.0));
    return frames_per_sec;
}

// _______________________________________________________________________________________________

  /* public */
  bool
  animate_type::
is_animating( ) const
{
    // The timer isActive flag is the same as is_animating.
    d_assert( p_timer_);
    if ( p_timer_->isActive( ) ) {
        d_assert( is_animatable( ));
        return true;
    }
    return false;
}

  /* protected slot */
  void
  animate_type::
recalc_is_animatable( bool is_animator_on)
{
    // If we're animating we must also be animatable.
    d_assert( implies( is_animating( ), is_animatable( )));

    // If an animator was just turned off it must have been on before.
    // When any animator is on, is_animatable( ) is true.
    d_assert( implies( (! is_animator_on), is_animatable( )));

    if ( is_animatable( ) && is_animator_on ) {
        // If we've just switched on an animator but is_animatable( ) is already true,
        // we don't have to change anything.
    } else {
        bool const new__is_animatable = calc_is_animatable( );
        d_assert( (new__is_animatable == true) || (new__is_animatable == false));
        d_assert( (is_animatable_     == true) || (is_animatable_     == false));
        if ( new__is_animatable != is_animatable_ ) {
            if ( is_animating( ) && ! new__is_animatable ) {
                stop( );
            }

            is_animatable_ = new__is_animatable;
            emit has_changed__is_animatable( is_animatable_);
            emit has_changed( );
        }
    }

    // Same assumption as at beginning of this method.
    d_assert( implies( is_animating( ), is_animatable( )));
}

  /* protected non-slot */
  bool
  animate_type::
calc_is_animatable( ) const
  //
  // This tells us that at least one of the animators is switched on.
  // This can change whenever:
  //   We create an animator
  //   We delete an animator
  //   We switch an animator on or off
{
    // Return true if any of the animators is switched on.
    list_type::const_iterator       iter     = animators_.begin( );
    list_type::const_iterator const iter_end = animators_.end( );
    while ( iter_end != iter ) {
        animator_base_type * p_look_at = (* iter);
        if ( p_look_at->is_on( ) ) {
            return true;
        }
        ++ iter;
    }

    // No animators are switched on.
    return false;
}

// _______________________________________________________________________________________________

  void
  animate_type::
start( )
{
    if ( ! is_animating( ) ) {
        if ( is_animatable( ) ) {

            // Start the timer.
            d_assert( p_timer_);
            p_timer_->start( );
            d_assert( is_animating( ));

            // Tell the start/stop buttons to disable/enable.
            emit has_changed__is_animating( true);
            if ( 1 != step_limit_ ) {
                // The has_changed__is_animating_extended(..) signal is almost the same as
                // has_changed__is_animating(..) except it does not fire "true" when step_limit_
                // is only 1. It is used to enable/disable the is-animating buttons in the UI.
                // We could use the has_changed__is_animating(..) signal instead, but then the
                // is-animating button(s) flash on/off when you just push the single-step button.
                // It's a minor glitch, but easy to fix with this separate "extended" signal.
                emit has_changed__is_animating_extended( true);
            }
            emit has_changed( );

            // Do the first animation frame immediately.
            emit wakeup( );
        }
    }
}

  void
  animate_type::
stop( )
{
    if ( is_animating( ) ) {

        // Stop the timer.
        d_assert( p_timer_);
        p_timer_->stop( );
        d_assert( ! is_animating( ));

        // Reset the last tick so it's ready for the next start.
        tick_last_ = date_time::get_invalid_tick_pt( );

        // I'm not sure how this will be used yet, except during single_step.
        // Clearing it here may or may not be appropriate.
        step_limit_ = 0;

        // Tell the start/stop buttons to disable/enable.
        emit has_changed__is_animating( false);
        emit has_changed__is_animating_extended( false);
        emit has_changed( );
    }
}

  /* public slot */
  void
  animate_type::
start_stop( bool is_start)
{
    if ( is_start ) {
        start( );
    } else {
        stop( );
    }
}

  /* public slot */
  void
  animate_type::
single_step( )
{
    if ( is_animatable( ) ) {
        step_limit_ = 1;
        start( );
    }
}

// _______________________________________________________________________________________________

  void
  animate_type::
attach__on_off_button( QAbstractButton * p_button)
{
    d_assert( p_button && p_button->isCheckable( ));

    // Setup the enabled and checked values.
    if ( is_animatable( ) ) {
        p_button->setEnabled( true);
        //if ( init_value_from_holder ) {
            p_button->setChecked( is_animating( ));
        //} else {
        //  start_stop( p_button->isChecked( ));
        //}
    } else {
        d_assert( ! is_animating( ));
        p_button->setEnabled( false);
        p_button->setChecked( false);
    }

    // Button click that starts and stops the animation.
    d_verify( connect(
        p_button, SIGNAL( toggled( bool)),
        this, SLOT( start_stop( bool))
    ));

    // Signals from animate telling us whether to enable or disable the buttons.
    // Setup the start button. Get the initial state from the animate object.
    d_verify( connect(
        this , SIGNAL( has_changed__is_animating_extended( bool)),
        p_button, SLOT( setChecked( bool))
    ));
    d_verify( connect(
        this , SIGNAL( has_changed__is_animatable( bool)),
        p_button, SLOT( setEnabled( bool))
    ));

    // The button and this ctrl should be sync'ed.
    d_assert( is_animatable( ) == p_button->isEnabled( ));
    d_assert( is_animating(  ) == p_button->isChecked( ));
}

  void
  animate_type::
attach__step_button( QAbstractButton * p_button)
{
    d_assert( p_button);

    // The button should be enabled whenever we are animatable.
    p_button->setEnabled( is_animatable( ));
    d_verify( connect(
        this , SIGNAL( has_changed__is_animatable( bool)),
        p_button, SLOT( setEnabled( bool))
    ));

    // Button click that steps the animation.
    d_verify( connect(
        p_button, SIGNAL( clicked( )),
        this, SLOT( single_step( ))
    ));

    // The button and this ctrl should be sync'ed.
    d_assert( is_animatable( ) == p_button->isEnabled( ));
}

// _______________________________________________________________________________________________

  /* non-slot public method */
  void
  animate_type::
maybe_move( )
  //
  // Call this before paint.
  // This is called from the painting widget, just before drawing. It moves all the animated
  // objects to where they should be right now, based on the current time.
  //
  // While animate_type is animating, this records all start-to-start draw durations, not just those
  // triggered by animation. So it is possible to have an animation rate that's higher than the
  // requested rate. For example, if auto-solve is running very fast this could drive a high
  // animation rate.
  //
  // This should probably keep track of the frequency of external updates even when we're not
  // animating. We can use that info when we start, and have a longer history as we tweak.
  // Of course start is triggered by the user, so it's probably appropriate to always update
  // at that time. On the other hand it isn't appropriate if nothing has moved.
{
    // Do nothing is we are not animating.
    if ( is_animating( ) ) {
        bool stop_at_end = false;

        // Calc the time that has passed and advance the tick.
        tick_point_type const  tick_now                 = date_time::get_tick_now( );
        second_type     const  seconds_since_last_move  = calc_seconds_since_last_move( tick_now);
        d_assert( seconds_since_last_move >= 0);

        // Do nothing if the animation is already up-to-date.
        if ( seconds_since_last_move ) {

            // Keep some stats.
            move_count_ += 1;
            if ( move_count_ < 0 ) {
                // After 2 billion increments this wraps around negative.
                move_count_ = 0;
            }
            // We only record duration when the clock isn't broken, and this is not the first move
            // in this animation cycle, and when the clock hasn't wrapped around.
            // If we draw 5 frames in the animation, we will record 4 start-to-start durations.
            // We only record the previous duration when we start the next animation draw, so we
            // never record the last draw duration.
            if ( date_time::is_valid_tick_pt( tick_last_) && (tick_now >= tick_last_) ) {
                start_start_durations_.record_next( tick_now - tick_last_);
            }

            // Tell all the animators to animate if they are switched on.
            list_type::const_iterator       iter     = animators_.begin( );
            list_type::const_iterator const iter_end = animators_.end( );
            while ( iter_end != iter ) {
                animator_base_type * p_look_at = (* iter);
                if ( p_look_at->is_on( ) ) {
                    p_look_at->move( seconds_since_last_move);
                }
                ++ iter;
            }

            // Check to see if we're watching the step limit.
            // We could also set up a time limit.
            if ( step_limit_ > 0 ) {
                if ( 1 == step_limit_ ) {
                    stop_at_end = true;
                }
                -- step_limit_;
            }
        }
        tick_last_ = tick_now;

        // Stop if we've run into a limit.
        if ( stop_at_end ) {
            stop( );
        }
    }
}

  /* protected method */
  animate_type::second_type
  animate_type::
calc_seconds_since_last_move( tick_point_type tick_now) const
  //
  // Calculate (and return) how many seconds have passed since the last animation move.
{
    second_type const millisecond = 0.001;

    // If we don't know how much time has passed ...
    if ( date_time::is_invalid_tick_pt( tick_last_) || (tick_now < tick_last_) ) {
        // We don't know how much time has passed.
        // Either the clock is broken, or this is the first tick in the animation,
        // or the clock has wrapped around back to zero.
        // In either case just advance by the default interval.

        d_assert( p_timer_);
        int const interval_msecs = p_timer_->interval( );
        if ( interval_msecs <= 0 ) {
            // We see this when we've requested that the animation run as fast as possible.
            // If the unlikely event that the clock is broken, we should return some small positive
            // time step.
            // If this is the first tick of the animation we could return zero and skip the initial
            // draw of the animation, or we can return a small positive time stop.

            // Return a small, but not too small, unit of time.
            return millisecond;
        }

        // Return the number of milliseconds in the interval.
        return date_time::convert_milliseconds_to_seconds( interval_msecs);
    }

    // If it looks like no time has passed since the last draw of the animation ...
    if ( tick_now == tick_last_ ) {
        // There are two cases to consider:
        //   We are animating as fast as possible, or
        //   We are refreshing early for some other reason than the animation
        // In the first case we should return some small animation value.
        // In the 2nd case we should return zero.

        d_assert( p_timer_);
        int const interval_msecs = p_timer_->interval( );
        if ( interval_msecs <= 0 ) {
            // We are animating as fast as possible.
            // Return a millisecond since it is probably not larger than a tick.
            // We could also return a fraction of a tick, and even remember how far into the fraction we are.

            // We still cannot be sure we are not painting for another reason, but it doesn't matter because
            // when the interval is zero we paint as much as possible and every paint is an animation paint.

            // Return a small positive value. A millisecond.
            return millisecond;
        }

        // We are animating normally, but we are painting twice in the same tick.
        // This seems unlikely unless painting is very fast or the tick-time is very long.
        // We could progress a fraction of a tick, which might yield a smoother animation,
        // but instead we'll just return zero.
        return 0;
    }

    // Normal mid-animation case.
    // The clock is not broken.
    // This is not the first tick of the animation.
    // At least one tick has passed since the last animation frame.
    d_assert( tick_now > tick_last_);
    return date_time::convert_ticks_to_seconds( tick_now - tick_last_);
}

// _______________________________________________________________________________________________

  bool
  animate_type::
is_available_duration( ) const
{
    return ! start_start_durations_.is_empty( );
}

  animate_type::second_type
  animate_type::
get_average_duration_seconds( ) const
{
    return start_start_durations_.get_average__seconds( );
}

  animate_type::millisecond_type
  animate_type::
get_average_duration_mseconds( ) const
{
    return
        date_time::convert_seconds_to_milliseconds(
            get_average_duration_seconds( ));
}

  void
  animate_type::
clear_duration_stats( )
{
    move_count_ = 0;
    start_start_durations_.set_empty( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// animator_base_type -- class
//
// This provides the basic interface used by the parent animate_type.
// This is also a holder since you can attach it to a UI button (a button that is checkable,
// usually a checkbox).
//
// Suggestion: The bool is_on_ value could be replaced by a bool_holder.
// _______________________________________________________________________________________________

  /* ctor */
  animator_base_type::
animator_base_type( animate_type * p_parent)
  : holder_base_type( p_parent)
  , is_on_( false)
{
    d_assert( p_parent && (get_parent( ) == p_parent));
    p_parent->add_animator( this);
}

  /* dtor */
  animator_base_type::
~animator_base_type( )
{
    animate_type * p_parent = get_parent( );
    // The parent will not exist if this is being deleted because the parent is.
    // When deleting hierarchy, Qt deletes the parent first, leaving the children
    // briefly orphaned.
    if ( p_parent ) {
        p_parent->remove_animator( this);
    }
}

  animate_type *
  animator_base_type::
get_parent( ) const
{
    return qobject_cast< animate_type * >( parent( ));
}

  bool
  animator_base_type::
is_parent_animating( ) const
{
    animate_type * const p_parent = get_parent( );
    return p_parent && p_parent->is_animating( );
}

  /* slot */
  void
  animator_base_type::
set_on( bool is /* = true */)
{
    d_assert( (true == is) || (false == is));
    set_value_base( this, is_on_, is, & animator_base_type::has_changed__is_on);
}

  void
  animator_base_type::
attach__is_on( QAbstractButton * p_button)
{
    // See attach_widget_bool(..) in the supertype.

    d_assert( p_button && p_button->isCheckable( ));

    //if ( init_value_from_holder ) {
        p_button->setChecked( is_on( ));
    //} else {
    //  set_on( p_button->isChecked( ));
    //}

    d_verify( connect( p_button, SIGNAL( toggled( bool)), this, SLOT( set_on( bool))));
    d_verify( connect( this, SIGNAL( has_changed__is_on( bool)), p_button, SLOT( setChecked( bool))));
}

  void
  animator_base_type::
detach__is_on( QAbstractButton * p_button)
{
    d_assert( p_button);

    d_verify( disconnect( this, SIGNAL( has_changed__is_on( bool)), p_button, SLOT( setChecked( bool))));
    d_verify( disconnect( p_button, SIGNAL( toggled( bool)), this, SLOT( set_on( bool))));
    p_button->setChecked( false);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// callout_wrap -- helper class for delta_animator_type
// _______________________________________________________________________________________________

  class
callout_wrap
  {
    public:
      callout_wrap( delta_animator_type * p)
        : p_animator_( p)
        {
          d_assert( p_animator_);
          d_assert( ! p_animator_->is_calling_out_);
          p_animator_->is_calling_out_ = true;
        }

      ~callout_wrap( )
        {
          d_assert( p_animator_->is_calling_out_);
          p_animator_->is_calling_out_ = false;
        }

        delta_animator_type * const p_animator_;
  };

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// delta_animator_type -- class
//
// This is the basic animator class that interacts with a holder.
//
// It is also like an int_range_steps_holder or double_slide_holder in that it attaches to
// a slider and holds range and step value. In fact this class maintains a private
// double_slide_holder.
// _______________________________________________________________________________________________

  /* constructor */
  delta_animator_type::
delta_animator_type
 (  animate_type *         p_parent
  , bool                   is_wrapping
  , delta_per_second_type  init_delta_per_sec
  , delta_per_second_type  min_delta_per_sec
  , delta_per_second_type  max_delta_per_sec
  , delta_per_step_type    max_delta_per_step  /* = 0 */
 )
  : animator_base_type  ( p_parent)
  , p_is_wrapping_      ( 0)
  , p_is_reversed_      ( 0)
  , extra_bump_state_   ( e_smooth)
  , is_calling_out_     ( false)
  , p_delta_per_second_ ( 0)
  , max_delta_per_step_ ( 0)
  , prev_value_         ( 0)
  , leftover_           ( 0)
  , p_auto_on_holder_0_ ( 0)
  , p_auto_on_holder_1_ ( 0)
  , id_auto_on_0_       ( 0)
  , id_auto_on_1_       ( 0)
{
    // Create/attach is_wrapping holder.
    // Either wrapping or reflecting.
    p_is_wrapping_ = new bool_holder( this, is_wrapping);
    d_verify( connect(
        p_is_wrapping_, SIGNAL( has_changed( bool)),
        this, SIGNAL( has_changed__is_wrapping( bool))
    ));
    d_verify( connect(
        p_is_wrapping_, SIGNAL( has_changed( )),
        this, SIGNAL( has_changed( ))
    ));

    // Create/attach is_reverse holder.
    p_is_reversed_ = new bool_holder( this, false);
    d_verify( connect(
        p_is_reversed_, SIGNAL( has_changed( bool)),
        this, SLOT( on_changed__is_reversed( bool))
    ));
    d_verify( connect(
        p_is_reversed_, SIGNAL( has_changed( )),
        this, SIGNAL( has_changed( ))
    ));

    // Create the dps holder.
    if ( 0 == init_delta_per_sec ) {
        init_delta_per_sec =
            calc_candidate_delta_per_second( min_delta_per_sec, max_delta_per_sec);
    }
    d_assert( 0 != init_delta_per_sec);
    p_delta_per_second_ =
        new double_slide_holder(
                this, init_delta_per_sec, min_delta_per_sec, max_delta_per_sec);
    d_verify( connect(
        p_delta_per_second_, SIGNAL( has_changed( double)),
        this, SLOT( on_changed__delta_per_second( double))
    ));
    d_verify( connect(
        p_delta_per_second_, SIGNAL( has_changed( )),
        this, SIGNAL( has_changed( ))
    ));

    // max_delta_per_step is the max delta we can apply in any one animation step.
    // It must be > 0. If the calculated animation step is less than zero,
    // it will be limited by (- max_delta_per_step) instead of (+ max_delta_per_step).
    max_delta_per_step_ =
        calc_candidate_max_per_step(
            max_delta_per_step, min_delta_per_sec, max_delta_per_sec);

    d_verify( connect(
        this, SIGNAL( has_changed__is_on( bool)),
        this, SLOT( on_changed__is_on( bool))
    ));
    if ( p_parent ) {
        d_verify( connect(
            p_parent, SIGNAL( has_changed__is_animating( bool)),
            this, SLOT( on_changed__is_parent_animating( bool))
        ));
    }
}

// _______________________________________________________________________________________________

  /* slot (protected) */
  void
  delta_animator_type::
on_changed__is_parent_animating( bool is_parent_now_animating)
{
    d_assert( ! is_calling_out_);
    d_assert( is_parent_animating( ) == is_parent_now_animating);

    // Clear leftover_ whenever we start an animation.
    if ( is_parent_now_animating ) {
        leftover_ = 0;
    }

    // Take care of auto-on.
    if ( is_on( ) ) {
        inform_auto_on__is_animating( is_parent_now_animating);
    }

    // We don't emit any signals when the parent is_animating( ) state changes.
    // We COULD emit a signal when animation starts/stops for this particular animator.
}

  /* slot (protected) */
  void
  delta_animator_type::
on_changed__is_on( bool is_now_on)
{
    d_assert( ! is_calling_out_);
    d_assert( is_on( ) == is_now_on);

    // Clear leftover_ whenever we start an animation.
    if ( is_now_on ) {
        leftover_ = 0;
    }

    // Take care of auto-on.
    if ( is_parent_animating( ) ) {
        inform_auto_on__is_animating( is_now_on);
    }

    // We don't have to emit has_changed__is_on( is_now_on) because we are catching our own signal
    // here, from the supertype.
}

  /* slot (protected) */
  void
  delta_animator_type::
on_changed__is_reversed( bool is)
{
    d_assert( ! is_calling_out_);

    // Should we clear this or leave it alone?
    // I think sometimes we get kind of a jump when we reverse this because negative doubles
    // round differently from positive doubles, but I'm not sure of that. The safest bet is
    // to just clear this.
    leftover_ = 0;

    emit has_changed__is_reversed( is);
}

  /* slot (protected) */
  void
  delta_animator_type::
on_changed__delta_per_second( delta_per_second_type new_dps)
{
    d_assert( ! is_calling_out_);

    // I'd like to clear this when the value crosses from negative to positive or vice versa.
    // Since the old value is not easily available I'll just always clear.
    leftover_ = 0;

    emit has_changed__delta_per_second( new_dps);
}

// _______________________________________________________________________________________________

  /* static, protected */
  delta_animator_type::delta_per_step_type
  delta_animator_type::
calc_candidate_max_per_step
 (  delta_per_step_type    new_mps
  , delta_per_second_type  min_dps
  , delta_per_second_type  max_dps
 )
{
    // This is not ideal. We want a value that ensures that a single animation step
    // is not more than the max-min of the animated holder, the the dps holder.

    // If we pass in a zero "max delta per step" we need to come up with a reasonable value.
    if ( new_mps == 0 ) {
        // Make the new "max per step" the average of the min and max "per second" values.
        // This assumes several animation steps are taken every second, at at least about
        // once a second when the animation is running the slowest.
        new_mps = (max_dps - min_dps) / 2;
    }
    d_assert( new_mps != 0);

    // Make sure the "max delta per step" value is positive.
    if ( new_mps < 0 ) {
        new_mps = - new_mps;
    }
    d_assert( new_mps > 0);

    return new_mps;
}

  /* not a slot, but could be if we wanted to track this value */
  void
  delta_animator_type::
set_max_delta_per_step( delta_per_step_type new_mps)
  //
  // This clamps the change to not be too big.
  // Think of animating an angle and rotating a drawing. We don't want 360 degree rotations --
  // in fact it's probably best to keep rotations below 90 degrees, especially if we're rotating
  // something square.
{
    d_assert( ! is_calling_out_);
    d_assert( p_delta_per_second_);

    // max_delta_per_step_ must always be positive.
    new_mps =
        calc_candidate_max_per_step
         (  new_mps
          , p_delta_per_second_->get_min_value( )
          , p_delta_per_second_->get_max_value( )
         );
    d_assert( new_mps > 0);

    // How to set this value if it isn't trackable.
    max_delta_per_step_ = new_mps;

    // How to set this if it's trackable.
  # if 0
    set_value_base
     (  this
      , max_delta_per_step_
      , new_mps
      , & delta_animator_type::has_changed__max_delta_per_step
     );
  # endif
}

# if 0
  /* not a slot, but could be if we wanted to track this value */
  void
  delta_animator_type::
set_extra_bump_count( int count)
  //
  // This value takes the place of max_delta_per_step if you specify a
  // very large max_delta_per_step.
  //
  // These configurations make sense:
  //   When is_wrapping( ), extra_bump_count can be:
  //     -1 - Stop at the max side when we bump, and in the next step stop at the low side.
  //          And then animate up from there. With large steps the older alternates between
  //          min and max.
  //          This is used for checkboxes (bool_holder).
  //     0  - During the animation pause every time you wrap around. May look jerky.
  //          We use this for click animation.
  //     1  - Allow a smooth wrap around, but stop if we try to wrap around again.
  //          This is the most common wrapping config for smooth values, like spin angles.
  //     2+ - Allow several wraps. This may not make much sense to the user, but in that
  //          case you should probably set max_delta_per_step to something less than (max-min).
  //
  //   When is_reflecting( ) (not wrapping), extra_bump_count can be:
  //     -1 - Stops for two beats at each end. Not very useful.
  //          If attached to a checkbox, this will leave it true for 2 beats and then false
  //          for two beats. But is_wrapping is supposed to always be true for bool_holders.
  //     0  - Stops at the end points, first one end and then the other.
  //          This would be good for animating checkboxes (bool_holders) except we usually
  //          set these up as is_wrapping( ) and not is_reflecting( ).
  //          This would also look right for click animation.
  //     1  - Stop at the 2nd bump. This is the most common config for smooth values like
  //          rise angles. If steps are very large this will keep the holder stuck at min
  //          (or at max if is_reversed), so not ideal.
  //     2+ - Allow lots of wrapping. This may be confusing. But probably the best for rise angle
  //          since a very large value will bounce back and forth between min/max. Except that
  //          usually max_delta_per_step will keep that from happening.
  //
  // Ideal values:
  //   Checkbox (bool_holder)  - -1, wrapping, 0 if not wrapping
  //   Click animation         - 0, wrapping, 0 if not wrapping
  //   Spin angle              - 1, wrapping, 2 if not wrapping
  //   Rise angle              - 2, not wrapping, 1 if wrapping
{
    // All negative values will trigger extra-slow mode.
    d_assert( count >= -1);

    // How big do we want to let this get? This limit is arbitrary, although any
    // value bigger than 2 is questionable.
    d_assert( count < 5);

    extra_bump_count_ = count;

    // If we're not in extra-slow mode we don't have to clear the leftover. In this case
    // leftover_ only stores rounding errors except during move(..) (before move returns).
    // Buf if we're in extra-slow mode leftover_ can be large if we've just bumped bu haven't
    // corrected yet, because we spread bumping out over two cycles.
    //
    // We only need to clear this if we were in extra-slow mode [(extra_bump_count_ < 0) before
    // being set above], and if we bumped in the last cycle but are waiting for the next cycle
    // to respond.
    leftover_ = 0;
}
# endif

  int
  delta_animator_type::
get_extra_bump_count( ) const
{
    return (e_click == extra_bump_state_) ? 0 : (
        is_wrapping( ) ?
            ((e_check == extra_bump_state_) ? -1 : +1) :
            ((e_check == extra_bump_state_) ?  0 : +2) );
}

// _______________________________________________________________________________________________

  /* static, protected */
  delta_animator_type::delta_per_second_type
  delta_animator_type::
calc_candidate_delta_per_second
 (  delta_per_second_type  min_dps
  , delta_per_second_type  max_dps
 )
{
    // This should never be zero. It can be negative.

    // Try to get close to zero, but not zero.
    if ( min_dps > 0 ) {
        return min_dps;
    }
    if ( max_dps < 0 ) {
        return max_dps;
    }
    delta_per_second_type const width = max_dps - min_dps;
    d_assert( width != 0);
    delta_per_second_type const new_dps = width / 1000.0;
    d_assert( 0 != new_dps);
    return new_dps;
}

  delta_animator_type::delta_per_second_type
  delta_animator_type::
get_delta_per_second( ) const
{
    d_assert( p_delta_per_second_);
    delta_per_second_type dps = p_delta_per_second_->get_value( );

    // dps can be zero because it is set by the user (the slider).
    // We don't filter that value.
    if ( dps == 0 ) {
        dps = calc_candidate_delta_per_second
               (  p_delta_per_second_->get_min_value( )
                , p_delta_per_second_->get_max_value( )
               );
    }
    d_assert( dps != 0);
    return dps;
}

  delta_animator_type::delta_per_second_type
  delta_animator_type::
get_min_delta_per_second( ) const
{
    d_assert( p_delta_per_second_);
    delta_per_second_type const dps = p_delta_per_second_->get_min_value( );
    d_assert( dps != 0);
    return dps;
}

  delta_animator_type::delta_per_second_type
  delta_animator_type::
get_max_delta_per_second( ) const
{
    d_assert( p_delta_per_second_);
    delta_per_second_type const dps = p_delta_per_second_->get_max_value( );
    d_assert( dps != 0);
    return dps;
}

  /* slot (public) */
  void
  delta_animator_type::
set_delta_per_second( delta_per_second_type new_dps)
{
    d_assert( ! is_calling_out_);
    d_assert( p_delta_per_second_);

    // Avoid zero values. If this is zero then the animation cannot move.
    if ( 0 == new_dps ) {
        new_dps =
            calc_candidate_delta_per_second
             (  p_delta_per_second_->get_min_value( )
              , p_delta_per_second_->get_max_value( )
             );
    }
    d_assert( 0 != new_dps);

    p_delta_per_second_->set_value( new_dps);
}

// _______________________________________________________________________________________________

  bool
  delta_animator_type::
is_wrapping( ) const
{
    d_assert( p_is_wrapping_);
    return p_is_wrapping_->get_value( );
}

  /* not a slot */
  void
  delta_animator_type::
set_is_wrapping( bool new_is)
{
    d_assert( ! is_calling_out_);
    d_assert( p_is_wrapping_);
    p_is_wrapping_->set_value( new_is);
}

// _______________________________________________________________________________________________

  bool
  delta_animator_type::
is_reversed( ) const
{
    d_assert( p_is_reversed_);
    return p_is_reversed_->get_value( );
}

  /* not a slot */
  void
  delta_animator_type::
set_is_reversed( bool new_is)
{
    d_assert( ! is_calling_out_);
    d_assert( p_is_reversed_);
    p_is_reversed_->set_value( new_is);
}

// _______________________________________________________________________________________________

  /* virtual (overridden, base is pure with no implementation) */
  void
  delta_animator_type::
move( second_type seconds_since_last_move)
{
    d_assert( ! is_calling_out_);

    // This cannot move backwards in time.
    d_assert( seconds_since_last_move >= 0);

    // Only call this when we are ready to move.
    d_assert( is_on( ));

    // Only move if time has passed since the last move.
    if ( seconds_since_last_move > 0 ) {
        value_type next_animating_value_requested = 0;
        if ( calc_next_animating_value( seconds_since_last_move, next_animating_value_requested) ) {
            request_animating_value( next_animating_value_requested);
        }
    }
}

// _______________________________________________________________________________________________

  /* protected */
  void
  delta_animator_type::
request_animating_value( value_type next_animating_value_requested)
{
    d_assert( ! is_calling_out_);

    // Test to see if we're in extra-slow-bump mode. We use this when animating checkboxes (bool_holders)
    // so that very big steps will cause the value to jump back and forth between min and max
    // when is_wrapping( ).
    if ( get_extra_bump_count( ) < 0 ) {

        // If we're already bumped, just reflect or wrap but don't move further.
        if ( bump_before_move__extra_slow( ) ) {
            emit has_bumped( );
        } else {

            // This will not move you past min/max. It might leave us with a big leftover if
            // we're bumping, but if we're bumping the leftover will be discarded in the next cycle.
            set_animating_value__save_leftover( next_animating_value_requested);

            // We may have just bumped. If so leftover_ may be extra large because it won't just
            // contain rounding errors. We could check if we're bumped here, and clear leftover_ if we
            // are, but instead we'll just wait to clear leftover_ in the next cycle.
        }
    } else {

        // We could make the recursive instead of looping.
        // Remember we could end up recursing (or looping) forever if the min and max values
        // in the holder are equal to each other.

        // We always want to allow at least one bump.
        int bump_countdown = get_extra_bump_count( ) + 1;
        d_assert( bump_countdown > 0);
        do {
            // Move the animating value. This may bump into the min/max values.
            // Save the leftover because we may use it during the next animation step.
            set_animating_value__save_leftover( next_animating_value_requested);

            // We now have a leftover_ value that reflects how much of our requested move was not honored.
            // There are two reasons leftover_ may be non-zero:
            //   Rounding error, or
            //   The holder bumped into a min or max value.

            // If the holder bumped into a min/max, we may want to use up the leftover, either after
            // wrapping around (from max->min or min->max), or after reflecting.
            if ( bump_after_move( ) ) {
                emit has_bumped( );
            } else {
                // We did not bump, so we're done. Any unused part of the move is stored in leftover_.
                break;
            }
            // We bumped, and leftover_ has any extra move we didn't use up.
            // The leftover may be bigger than usual because the final value will probably have been
            // truncated to fit in its min/max. Usually leftover only contains rounding errors.

            // Get tne next value after the bump in case we want to use it up.
            if ( ! calc_next_animating_value_from_leftover( next_animating_value_requested) ) {
                // We did not have any leftover after the last bump.
                d_assert( 0 == leftover_);
                break;
            }
            // All the leftover has been moved into next_animating_value_requested.
            d_assert( 0 == leftover_);

            // We're done with the bump, with a value waiting for us in next_animating_value_requested.
            // If bump_countdown goes to zero, we discard the leftover folded into next_animating_value_requested.
            -- bump_countdown;
        }
        while ( bump_countdown > 0 );
    }
}

// _______________________________________________________________________________________________

  /* protected */
  bool
  delta_animator_type::
bump_before_move__extra_slow( )
  //
  // Returns true if we are already bumped into min or max.
{
    d_assert( ! is_calling_out_);

    // If we are moving min -> max ...
    if ( is_moving_in_positive_direction( ) ) {
        // If the holder is bumped into the max position ...
        if ( is_bumping_max__protected( ) ) {
            if ( is_reflecting( ) ) {
                set_is_reversed( ! is_reversed( ));
            } else
            /* if is_wrapping( ) */ {
                d_assert( is_wrapping( ));
                wrap_to_min__fixup( );
            }
            leftover_ = 0;
            return true;
        }
    } else
    /* if ( ! is_moving_in_positive_direction( ) ) */ {
        // If the holder is bumped into the min position ...
        if ( is_bumping_min__protected( ) ) {
            if ( is_reflecting( ) ) {
                set_is_reversed( ! is_reversed( ));
            } else
            /* if is_wrapping( ) */ {
                d_assert( is_wrapping( ));
                wrap_to_max__fixup( );
            }
            leftover_ = 0;
            return true;
        }
    }
    return false;
}

  /* protected */
  bool
  delta_animator_type::
bump_after_move( )
  //
  // Returns true if we bumped into min or max.
{
    d_assert( ! is_calling_out_);

    // If we are moving min -> max ...
    if ( is_moving_in_positive_direction( ) ) {

        // If we had to stop before we got to our requested next value ...
        if ( leftover_ > 0 ) {

            // If the holder bumped into the max position ...
            if ( is_bumping_max__protected( ) ) {

                if ( is_reflecting( ) ) {
                    value_type const new_leftover = - leftover_;
                    set_is_reversed( ! is_reversed( ));
                    leftover_ = new_leftover;
                } else
                /* if is_wrapping( ) */ {
                    d_assert( is_wrapping( ));
                    wrap_to_min__fixup( );
                }
                return true;
            }
        }
    } else
    /* if ( ! is_moving_in_positive_direction( ) ) */ {

        // If we had to stop before moving as far in the negative direction as requested ...
        if ( leftover_ < 0 ) {

            // If the holder bumped into the min position ...
            if ( is_bumping_min__protected( ) ) {

                if ( is_reflecting( ) ) {
                    value_type const new_leftover = - leftover_;
                    set_is_reversed( ! is_reversed( ));
                    leftover_ = new_leftover;
                } else
                /* if is_wrapping( ) */ {
                    d_assert( is_wrapping( ));
                    wrap_to_max__fixup( );
                }
                return true;
            }
        }
    }
    // We did not bump.
    return false;
}

// _______________________________________________________________________________________________

  bool
  delta_animator_type::
is_moving_in_positive_direction( ) const
{
    bool const is_negative_delta = (get_delta_per_second( ) < 0);

    d_assert( (is_reversed( )    == true) || (is_reversed( )    == false));
    d_assert( (is_negative_delta == true) || (is_negative_delta == false));
    return is_reversed( ) == is_negative_delta;
}

// _______________________________________________________________________________________________

  /* protected */
  bool
  delta_animator_type::
calc_next_animating_value_from_leftover( value_type & next_value)
{
    d_assert( ! is_calling_out_);

    next_value = prev_value_;
    if ( leftover_ != 0 ) {
        next_value += leftover_;
        leftover_ = 0;
        return true;
    }
    return false;
}

// _______________________________________________________________________________________________

  /* protected */
  bool
  delta_animator_type::
calc_next_animating_value
 (  second_type   seconds_since_last_move
  , value_type &  next_value
 )
{
    d_assert( ! is_calling_out_);
    d_assert( seconds_since_last_move > 0);

    // Find out where we are now.
    next_value = get_animating_value__maybe_clear_leftover( );
    // You have to call get_animating_value__maybe_clear_leftover( ) above before
    // refering to leftover_ below.

    // Find out how far we should move.
    // Correct by the last leftover, but only if the value is what we last set it to.
    value_type const delta = calc_delta_from_seconds( seconds_since_last_move) + leftover_;

    // Clear leftover now that we've used it (although it may already be clear).
    leftover_ = 0;

    // Return true (and the next requested value) if we want the animating value to change.
    if ( delta != 0 ) {
        next_value += delta;
        return true;
    }
    // Return false if we don't want to change the animating value.
    return false;
}

// _______________________________________________________________________________________________

  /* protected */
  delta_animator_type::value_type
  delta_animator_type::
calc_delta_from_seconds( second_type seconds_since_last_move) const
{
    // Usually seconds_since_last_move is < 1, but it could be anything.
    d_assert( seconds_since_last_move > 0);

    // Assume we are set up correctly.
    d_assert( max_delta_per_step_ > 0);

    // Figure out how far to move. It should not be zero.
    value_type delta = get_delta_per_second( ) * seconds_since_last_move;
    d_assert( delta != 0);

    // Correct the sign.
    if ( is_reversed( ) ) {
        delta = - delta;
    }

    // Clamp to the max value.
    if ( delta < 0 ) {
        if ( delta < - get_max_delta_per_step( ) ) {
            delta = - get_max_delta_per_step( );
        }
    } else {
        if ( delta > + get_max_delta_per_step( ) ) {
            delta = + get_max_delta_per_step( );
        }
    }

    d_assert( 0 != delta);
    return delta;
}

// _______________________________________________________________________________________________

  /* protected */
  delta_animator_type::value_type
  delta_animator_type::
get_animating_value__maybe_clear_leftover( )
{
    // We call out of this class thru an overridden virtual in this method.
    // All calls out of this class thru virtual methods are wrapped in callout_wrap.
    callout_wrap cw( this);

    // Call the pure virtual, overridden by the subtype.
    // Do not allow the virtual method to call back into here.
    value_type value = get_animating_value( ); /* overridden virtual */

    // Instead of saving prev_value_ we could clear leftover_ whenever the
    // value changes in the holder.
    if ( value != prev_value_ ) {
        leftover_ = 0;
    }
    return value;
}

// _______________________________________________________________________________________________

  /* protected */
  void
  delta_animator_type::
set_animating_value__save_leftover( value_type next_value_requested)
{
    // We should have used up leftover_ before we got here. We set it below.
    d_assert( 0 == leftover_);

    // We call out of this class thru overridden virtuals in this method.
    // Do not allow these virtual methods to call back into here.
    callout_wrap cw( this);

    set_animating_value( next_value_requested); /* overridden virtual */
    value_type const next_value_achieved = get_animating_value( ); /* overridden virtual */

    leftover_ = next_value_requested - next_value_achieved;
    prev_value_ = next_value_achieved;
}

// _______________________________________________________________________________________________

  /* protected */
  bool
  delta_animator_type::
is_bumping_min__protected( )
{
    callout_wrap cw( this);
    bool const is = is_bumping_min( );
    return is;
}

  /* protected */
  bool
  delta_animator_type::
is_bumping_max__protected( )
{
    callout_wrap cw( this);
    bool const is = is_bumping_max( );
    return is;
}

  /* protected */
  void
  delta_animator_type::
wrap_to_min__fixup( )
{
    callout_wrap cw( this);
    wrap_to_min( );
    prev_value_ = get_animating_value( );
}

  /* protected */
  void
  delta_animator_type::
wrap_to_max__fixup( )
{
    callout_wrap cw( this);
    wrap_to_max( );
    prev_value_ = get_animating_value( );
}

// _______________________________________________________________________________________________

  void
  delta_animator_type::
attach__is_wrapping( QAbstractButton * p_button)
{
    d_assert( p_is_wrapping_);
    p_is_wrapping_->attach( p_button);
}

  void
  delta_animator_type::
attach__is_reversed( QAbstractButton * p_button)
{
    d_assert( p_is_reversed_);
    p_is_reversed_->attach( p_button);
}

  void
  delta_animator_type::
attach__delta_per_second( QAbstractSlider * p_slider)
{
    d_assert( p_delta_per_second_);
    p_delta_per_second_->attach( p_slider);
}

// _______________________________________________________________________________________________

  void
  delta_animator_type::
detach__is_wrapping( QAbstractButton * p_button)
{
    d_assert( p_is_wrapping_);
    p_is_wrapping_->detach( p_button);
}

  void
  delta_animator_type::
detach__is_reversed( QAbstractButton * p_button)
{
    d_assert( p_is_reversed_);
    p_is_reversed_->detach( p_button);
}

  void
  delta_animator_type::
detach__delta_per_second( QAbstractSlider * p_slider)
{
    d_assert( p_delta_per_second_);
    p_delta_per_second_->detach( p_slider);
}

// _______________________________________________________________________________________________

  delta_animator_type *
  delta_animator_type::
set_auto_on( bool_holder * p_holder0, bool_holder * p_holder1)
  //
  // p_holder can be zero.
{
    // Either both holders are zero, or they are not equal.
    d_assert(
        ((0 == p_holder0) && (0 == p_holder1)) ||
        (p_holder0 != p_holder1) );

    p_auto_on_holder_0_ = p_holder0;
    p_auto_on_holder_1_ = p_holder1;
    id_auto_on_0_ = 0;
    id_auto_on_1_ = 0;
    return this;
}

  void
  delta_animator_type::
inform_auto_on__is_animating( bool is_animating)
{
    if ( is_animating ) {
        // This animator has just started animating.
        if ( p_auto_on_holder_0_ ) {
            id_auto_on_0_ = p_auto_on_holder_0_->request_auto_on( );
        }
        if ( p_auto_on_holder_1_ ) {
            id_auto_on_1_ = p_auto_on_holder_1_->request_auto_on( );
        }
    } else {
        // This animator has just stopped animating.
        if ( p_auto_on_holder_1_ ) {
            p_auto_on_holder_1_->release_auto_on( id_auto_on_1_);
        }
        if ( p_auto_on_holder_0_ ) {
            p_auto_on_holder_0_->release_auto_on( id_auto_on_0_);
        }
        id_auto_on_1_ = 0;
        id_auto_on_0_ = 0;
    }
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// animate.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
