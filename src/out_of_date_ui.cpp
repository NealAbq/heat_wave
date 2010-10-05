// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// out_of_date_ui.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Improve:
//   This was written before aggregate holders, like bristle_properties_style.
//   This should be changed to a holder like that.
//
// We need to know if the controls are hidden (buried by a tab, or because of full-screen or minimize)
// so we can stop updating.
// _______________________________________________________________________________________________

# include "all.h"
# include "out_of_date_ui.h"

# include <QtGui/QRadioButton>
# include <QtGui/QSpinBox>
# include <QtGui/QDoubleSpinBox>

// _______________________________________________________________________________________________

  /* constructor */
  out_of_date_ui_type::
out_of_date_ui_type( )
  : delay_                       (  )
  , p_button_always_             ( 0)
  , p_button_never_              ( 0)
  , p_button_watch_tick_         ( 0)
  , p_button_watch_gen_          ( 0)
  , p_button_watch_tick_and_gen_ ( 0)
  , p_button_watch_tick_or_gen_  ( 0)
  , p_spinbox_seconds_           ( 0)
  , p_spinbox_gen_               ( 0)
{
}

// _______________________________________________________________________________________________
// Button setters:
//
//   set_button_always(..)
//   set_button_never(..)
//   set_button_watch_tick(..)
//   set_button_watch_gen(..)
//   set_button_watch_tick_and_gen(..)
//   set_button_watch_tick_or_gen(..)
//
//   For now we can only connect to one button, and we cannot disconnect.
//   This is easily changed if the need arises.

# define DEFINE_BUTTON_SETTER( NAME)                        \
      void                                                  \
      out_of_date_ui_type::                                 \
    set_button_ ## NAME ( QRadioButton * p_button)          \
    {                                                       \
        d_assert( p_button && ! p_button_ ## NAME ## _);    \
        p_button_ ## NAME ## _ = p_button;                  \
        d_verify( connect(                                  \
            p_button, SIGNAL( toggled( bool)),              \
            this, SLOT( ui_changed_option( bool))));        \
    }                                                       /* end macro */

DEFINE_BUTTON_SETTER( always )
DEFINE_BUTTON_SETTER( never )
DEFINE_BUTTON_SETTER( watch_tick )
DEFINE_BUTTON_SETTER( watch_gen )
DEFINE_BUTTON_SETTER( watch_tick_and_gen )
DEFINE_BUTTON_SETTER( watch_tick_or_gen )

# undef DEFINE_BUTTON_SETTER

// _______________________________________________________________________________________________

  void
  out_of_date_ui_type::
set_spinbox_seconds( QDoubleSpinBox * p_sb)
{
    d_assert( p_sb);
    d_assert( ! p_spinbox_seconds_);

    p_spinbox_seconds_ = p_sb;
    d_verify( connect(
        p_spinbox_seconds_, SIGNAL( valueChanged( double)),
        this, SLOT( ui_changed_tick_max( double))));
}

  void
  out_of_date_ui_type::
set_spinbox_gen( QSpinBox * p_sb)
{
    d_assert( p_sb);
    d_assert( ! p_spinbox_gen_);

    p_spinbox_gen_ = p_sb;
    d_verify( connect(
        p_spinbox_gen_, SIGNAL( valueChanged( int)),
        this, SLOT( ui_changed_gen_max( int))));
}

// _______________________________________________________________________________________________

  /* slot */
  void
  out_of_date_ui_type::
ui_changed_option( bool)
{
    set_delay_option_from_ui( );
}

  /* slot */
  void
  out_of_date_ui_type::
ui_changed_tick_max( double)
{
    set_delay_tick_max_from_ui( );
}

  /* slot */
  void
  out_of_date_ui_type::
ui_changed_gen_max( int)
{
    set_delay_gen_max_from_ui( );
}

// _______________________________________________________________________________________________

  void
  out_of_date_ui_type::
set_ui_from_delay( )
{
  # define CHECK_BUTTON( NAME)                                      \
    if ( p_button_ ## NAME ## _ ) {                                 \
        p_button_## NAME ## _->setChecked( delay_.is_ ## NAME( ));  \
    }                                                               /* end macro */

    CHECK_BUTTON( always             )
    CHECK_BUTTON( never              )
    CHECK_BUTTON( watch_tick         )
    CHECK_BUTTON( watch_gen          )
    CHECK_BUTTON( watch_tick_and_gen )
    CHECK_BUTTON( watch_tick_or_gen  )

  # undef CHECK_BUTTON

    if ( p_spinbox_seconds_ ) {
        // If delay_.is_set__tick_max_out_of_date( ) is false then either we are not watching
        // time-elapsed out-of-date conditions, or we are assuming we are always out-of-date.
        // In the first case it is like seconds_max out_of_date is infinity, in the second it
        // is like it is zero (or slightly negative).
        p_spinbox_seconds_->setValue(
            delay_.is_set__tick_max_out_of_date( ) ? delay_.get_seconds_max_out_of_date( ) : 0);
    }

    if ( p_spinbox_gen_ ) {
        // The spinbox shows something like this:
        //   Update every XXX solves.
        // If XXX is 2, that means every two solves. So if the gen is one out-of-date that is
        // OK, but 2 out-of-date is too many. So the max is 1.
        gen_type const value = delay_.get_gen_max_out_of_date( );
        // What this means:
        //   0 - update every time even if the generation has not changed or is not available
        //   1 - update whenever the generation is out-of-date (every time)
        //   2 - update every other time
        //   3 - update every third time
        //   .. etc ..
        p_spinbox_gen_->setValue( (value < 0) ? 0 : (value + 1));
    }
}

// _______________________________________________________________________________________________

  void
  out_of_date_ui_type::
set_delay_from_ui( )
{
    set_delay_option_from_ui( );
    set_delay_tick_max_from_ui( );
    set_delay_gen_max_from_ui( );
}

  void
  out_of_date_ui_type::
set_delay_option_from_ui( )
{
    delay_.set_option( get_option_from_ui( ));
}

  void
  out_of_date_ui_type::
set_delay_tick_max_from_ui( )
{
    if ( p_spinbox_seconds_ ) {
        delay_.set_seconds_max_out_of_date( p_spinbox_seconds_->value( ));
    }
}

  void
  out_of_date_ui_type::
set_delay_gen_max_from_ui( )
{
    if ( p_spinbox_gen_ ) {
        int const value = p_spinbox_gen_->value( );
        delay_.set_gen_max_out_of_date( (value <= 0) ? -1 : (value - 1));
    }
}

// _______________________________________________________________________________________________

  out_of_date_type::e_option
  out_of_date_ui_type::
get_option_from_ui( ) const
{
  # define MAYBE_RETURN_OPTION( NAME )          \
    if ( p_button_ ## NAME ## _ &&              \
         p_button_ ## NAME ## _->isChecked( ) ) \
    {                                           \
        return out_of_date_type :: NAME ;       \
    }                                           /* end macro */

    MAYBE_RETURN_OPTION( always             )
    MAYBE_RETURN_OPTION( never              )
    MAYBE_RETURN_OPTION( watch_tick         )
    MAYBE_RETURN_OPTION( watch_gen          )
    MAYBE_RETURN_OPTION( watch_tick_and_gen )
    MAYBE_RETURN_OPTION( watch_tick_or_gen  )

  # undef MAYBE_RETURN_OPTION

    d_assert( false);
    return out_of_date_type::always;
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// out_of_date_ui.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
