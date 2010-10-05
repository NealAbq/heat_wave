// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// holder.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Holder redesign:
//   Holders should attach in a directed graph.
//   Like a directed graph, holders have targets. They also have sources although they may not
//     be able to access them.
//   Like a directed graph, holders also need at least one leaf type.
//   The UI objects should be holders that have no targets. They are leaves.
//   All holders can act like a collection of targets, but leaf types will always be an empty collection.
//   When you attach holders, the source value should flow to the target.
//   When you detach all sources, should a target be set to a default state? Disable if a UI ctrl,
//     set values to false or zero or minimum?
//
//   Event holders:
//     Push button.
//     No values. Signals an action, not an event. Like a discrete with only one value (like a void)?
//     A holder because part of the UI and can be animated/scripted.
//     With the other holders, the actions are changing values.
//     Can see this as a counter or flip/flop, where the actual value isn't important and only serves
//       as a source of events when it changes.
//
//     Timers also generate events.
//     Notifiers and cross-thread messages also generate events.
//
//   Narrow/sparse discrete holders:
//     Ctrls:
//       Checkbox, radio buttons, slider (jumps from value to value)
//       Label for display, spin up/downs
//     Values:
//       Bool
//       Enum
//       Small int
//
//   Wide/dense discrete holders:
//     Slider, scrollbar, spinbox, label, progress bar
//     Large int (many values, too many for radio buttons)
//
//   Holders for special values:
//     Doubles - can often be mapped to integers or strings
//     Strings
//     Money, dates/times/durations, colors, coords, widths
//
//   Collections:
//     Can be mapped to group boxes, tabs, pages, etc.
//
//   Basic UI controls:
//     Checkbox, radios, sliders, spins, labels
//     Should be able to map almost any simple value to all of these.
//     Bool -> slider, enum -> slider
// _______________________________________________________________________________________________

# include "all.h"
# include "holder.h"

// _______________________________________________________________________________________________

  /* constructor */
  holder_base_type::while_setting_value_wrapper_type::
while_setting_value_wrapper_type( holder_base_type * p, bool request_signal)
  : p_holder                    ( p)
  , is_watching_for_throw       ( true)
  , request_signal__has_changed ( request_signal)
  {
    d_assert( p_holder && ! p_holder->is_setting( ));
    p_holder->is_setting_ = true;
  }

// _______________________________________________________________________________________________

  /* destructor */
  holder_base_type::while_setting_value_wrapper_type::
~while_setting_value_wrapper_type( )
  {
    d_assert( p_holder->is_setting( ));
    if ( is_watching_for_throw ) {
        // Don't throw has_changed( ) if we're unwinding a throw.
        d_assert( false); /* unexpected */
    } else
    if ( request_signal__has_changed ) {
        try {
            emit p_holder->has_changed( );
        }
        catch ( ... ) {
            // The above "emit signal" caused a throw.
            d_assert( false); /* unexpected */
            p_holder->is_setting_ = false;
            throw;
        }
    }
    p_holder->is_setting_ = false;
}

// _______________________________________________________________________________________________

  void
  holder_base_type::while_setting_value_wrapper_type::
done_with_no_throws( )
  {
    d_assert( is_watching_for_throw);
    is_watching_for_throw = false;
  }

  void
  holder_base_type::while_setting_value_wrapper_type::
done_with_no_throws( bool request_signal)
  {
    request_signal__has_changed = request_signal;

    d_assert( is_watching_for_throw);
    is_watching_for_throw = false;
  }

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// holder.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
