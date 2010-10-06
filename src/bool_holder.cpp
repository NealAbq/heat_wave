// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bool_holder.cpp
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
// _______________________________________________________________________________________________

# include "all.h"
# include "bool_holder.h"
# include "pack_holder.h"
# include <QtGui/QAbstractButton>

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// bool_holder - class
//
// A bool_holder is a basic holder for two values: true and false.
// It can serve as a template for an enum holder, to be mapped to radio buttons.
// _______________________________________________________________________________________________

  /* constructor */
  bool_holder::
bool_holder( QObject * p_parent, value_type default_value /* = false */)
  : holder_base_type ( p_parent)
  , value_           ( default_value)
  , auto_on_count_   ( 0)
  , auto_on_id_      ( 0)
{
}

// _______________________________________________________________________________________________

  /* public member */
  void
  bool_holder::
attach( QAbstractButton * p_button)
{
    d_assert( p_button && p_button->isCheckable( ));
    attach_widget_bool( this, p_button);
}

  void
  bool_holder::
detach( QAbstractButton * p_button)
{
    d_assert( p_button);
    detach_widget_bool( this, p_button);
}

// _______________________________________________________________________________________________

  /* slot */
  void
  bool_holder::
set_value( value_type new_value)
{
    if ( new_value != value_ ) {
        auto_on_count_ = 0;
        set_value_base( this, value_, new_value);
    }
}

// _______________________________________________________________________________________________

  int
  bool_holder::
request_auto_on( )
{
    d_assert( auto_on_count_ >= 0 );
    if ( 0 == auto_on_count_ ) {
        if ( ! get_value( ) ) {
            // If this is off, turn it on and generate a new id.
            set_value( true);
            auto_on_count_ = 1;

            ++ auto_on_id_;
            if ( 0 == auto_on_id_ ) {
                auto_on_id_ = 1;
            }
            return auto_on_id_;
        }
        // If this is already on but not auto-on, return an invalid id.
        return 0;
    }
    // The holder must be on if auto_on_count_ != 0;
    d_assert( get_value( ));

    // If this is on AND is auto-on, add to the support count and return the id.
    ++ auto_on_count_;
    d_assert( 0 != auto_on_id_);
    return auto_on_id_;
}

  void
  bool_holder::
release_auto_on( int id)
{
    // The count is only ever > 0 when the holder is on.
    d_assert( auto_on_count_ >= 0 );
    if ( auto_on_count_ > 0 ) {
        d_assert( get_value( ));

        // The id should never be zero once a non-zero id has been returned.
        d_assert( 0 != auto_on_id_);
        if ( id == auto_on_id_ ) {
            // The id is fresh, so we need to unwind it.
            if ( auto_on_count_ == 1 ) {
                // This is the last support, so turn off the holder.
                set_value( false); // this resets auto_on_count_
                d_assert( auto_on_count_ == 0);
            } else {
                d_assert( auto_on_count_ > 1);
                -- auto_on_count_;
            }
        }
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// click_animator_type - class
//
// Supertype is delta_animator_type. We'd like to disable:
//   is_wrapping  - always true
//   is_reversed  - always false
// We'd also like to disable range (min/max) changes to supertype delta_per_second, but
// to do that we'd need to have to work with (private member):
//   delta_animator_type::p_delta_per_second_
// _______________________________________________________________________________________________

  click_animator_type::
click_animator_type
 (  animate_type *  p_parent
  , double          init_delta_per_sec
 )
  : delta_animator_type
     (  p_parent
      , true    /* is_wrapping, always true ) */
      , init_delta_per_sec
      , 0.0001  /* min change-per-second, should never change? */
      , 2.0     /* max change-per-second, should never change? */
      , 1.0     /* max change-per-step */
     )
  , p_moving_value_( new double_slide_holder( this, 0.0, 0.0, 1.0))
{
    d_assert( p_moving_value_);

    set_extra_bump__for_click( );

    // Listen to the moving value, which may be connected to a slider.
    // Relay the changed signal.
    d_verify( connect(
        p_moving_value_, SIGNAL( has_changed( double)),
        this, SIGNAL( has_changed__moving_value( double))
    ));

    d_verify( connect(
        this, SIGNAL( has_bumped( )),
        this, SIGNAL( has_clicked( ))
    ));

    // Should we relay (or intercept) changes from the supertype?
    //    d_verify( connect(
    //        this, SIGNAL( delta_animator_type::has_changed__delta_per_second( double)),
    //        this, SIGNAL(                      has_changed__delta_per_second( double))
    //    ));

    // Should we intercept these signal from supertype and d_assert( false):
    //   has_changed__is_wrapping( bool)
    //   has_changed__is_reversed( bool)
    // We'd also like to disable from the supertype (if it existed):
    //   changes to range
    // The range in the supertype (rates) is related to the range in moving-value (values affected
    // by the rates).
    //
    // From moving-value we'd like to disable changes to the range.
    //
    // Both moving-value and delta-per-second (in the supertype) are implemented as double_slide_holder
    // objects, which in turn use int_range_steps_holder as their inner object.
    // So both allow changes to their ranges thru attached sliders.
    // We'd like to disable this, but still allow attached sliders and value changes.
}

// _______________________________________________________________________________________________

  void
  click_animator_type::
attach__click( QAbstractButton * p_button)
{
    d_assert( p_button);
    d_verify( connect( this, SIGNAL( has_clicked( )), p_button, SLOT( click( )) ));
}

  void
  click_animator_type::
detach__click( QAbstractButton * p_button)
{
    d_assert( p_button);
    d_verify( disconnect( this, SIGNAL( has_clicked( )), p_button, SLOT( click( )) ));
}

// _______________________________________________________________________________________________

  void
  click_animator_type::
attach__moving_value( QAbstractSlider *  p_slider)
{
    d_assert( p_moving_value_);
    p_moving_value_->attach( p_slider);
}

  void
  click_animator_type::
detach__moving_value( QAbstractSlider * p_slider)
{
    d_assert( p_moving_value_);
    p_moving_value_->detach( p_slider);
}

// _______________________________________________________________________________________________

  /* slot */
  void
  click_animator_type::
set_moving_value( double new_value)
{
    p_moving_value_->set_value( new_value);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// bool_animator_type - class
// _______________________________________________________________________________________________

  bool_animator_type::
bool_animator_type
 (  animate_type        *  p_parent
  , default_holder_type *  p_holder
  , double                 init_delta_per_sec
 )
  : click_animator_type( p_parent, init_delta_per_sec)
  , p_holder_     ( p_holder)
  , p_transition_ ( new double_slide_holder( this, 0.5, 0.0, 1.0))
{
    d_assert( p_holder_);
    d_assert( p_transition_);

    set_extra_bump__for_check( );

    // Set moving_value to agree with the holder.
    set_value__bool( p_holder_->get_value( ));

    // Listen to the holder so when it changes we can keep in sync.
    d_verify( connect(
        p_holder_, SIGNAL( has_changed( bool)),
        this, SLOT( intercept__changed_holder( bool))
    ));

    // Listen to the moving value, which may be connected to a slider.
    // Relay the changed signal.
    d_verify( connect(
        this, SIGNAL( has_changed__moving_value( double)),
        this, SLOT( on_changed__moving_value( double))
    ));

    // Listen to the translation value, which may be connected to a slider.
    // Relay the changed signal.
    d_verify( connect(
        p_transition_, SIGNAL( has_changed( double)),
        this, SLOT( intercept__changed_transition( double))
    ));
}

// _______________________________________________________________________________________________

  void
  bool_animator_type::
attach__transition( QAbstractSlider * p_slider)
{
    // We must init the slider from the holder values because the slider min/max have to be [0,1].
    // Although we could take the value from the slider I suppose.
    p_transition_->attach( p_slider);
}

  void
  bool_animator_type::
detach__transition( QAbstractSlider * p_slider)
{
    p_transition_->detach( p_slider);
}

  void
  bool_animator_type::
attach__bool( QAbstractButton * p_button)
{
    d_assert( p_button && p_button->isCheckable( ));
    d_verify( connect(
        this, SIGNAL( has_changed__bool( bool)),
        p_button, SLOT( setChecked( bool))
    ));
    d_verify( connect(
        p_button, SIGNAL( toggled( bool)),
        this, SLOT( set_value__bool( bool))
    ));
}

  void
  bool_animator_type::
detach__bool( QAbstractButton * p_button)
{
    d_assert( p_button && p_button->isCheckable( ));
    d_verify( disconnect(
        this, SIGNAL( has_changed__bool( bool)),
        p_button, SLOT( setChecked( bool))
    ));
    d_verify( disconnect(
        p_button, SIGNAL( toggled( bool)),
        this, SLOT( set_value__bool( bool))
    ));
}

// _______________________________________________________________________________________________

  bool
  bool_animator_type::
get_value__bool( ) const
{
    // Always false if moving_value is against the min value.
    // Always true if moving_value is against the max value.
    // True if values are == (and not the min moving value).
    d_assert( get_max_moving_value( ) >  get_min_moving_value( ));
    d_assert( get_moving_value(     ) >= get_min_moving_value( ));
    double const mv = get_moving_value( );
    return (mv >= get_transition( )) && (mv > get_min_moving_value( ));
}

  void
  bool_animator_type::
set_value__bool( bool new_value)
{
    d_assert( (new_value == true) || (new_value == false));
    bool const old_value = get_value__bool( );
    d_assert( (old_value == true) || (old_value == false));

    if ( new_value != old_value ) {
        if ( new_value ) {
            wrap_to_max( ); // always true
        } else {
            wrap_to_min( ); // always false
        }
    }
    d_assert( get_value__bool( ) == new_value);
}

// _______________________________________________________________________________________________

  double
  bool_animator_type::
get_transition( ) const
{
    return p_transition_->get_value( );
}

  void
  bool_animator_type::
set_transition( double new_transition)
{
    p_transition_->set_value( new_transition);
}

// _______________________________________________________________________________________________

  /* protected slot */
  void
  bool_animator_type::
on_changed__moving_value( double new_moving_value)
{
    maybe_used_only_for_debug( new_moving_value);
    d_assert( get_moving_value( ) == new_moving_value);
    p_holder_->set_value( get_value__bool( ));
}

  /* protected slot */
  void
  bool_animator_type::
intercept__changed_transition( double new_transition)
{
    d_assert( get_transition( ) == new_transition);
    p_holder_->set_value( get_value__bool( ));
    emit has_changed__transition( new_transition);
}

  /* protected slot */
  void
  bool_animator_type::
intercept__changed_holder( bool new_value)
  //
  // This changes moving_value to make our bool value agree with the holder bool value.
{
    double const min_mv       = get_min_moving_value( );
    double const max_mv       = get_max_moving_value( );

    double const moving_value = get_moving_value( );
    double const transition   = get_transition( );

    d_assert( (min_mv <= moving_value) && (moving_value <= max_mv));
    d_assert( (min_mv <= transition  ) && (transition   <= max_mv));

    bool const old_value = get_value__bool( );
    if ( new_value ) {
        if ( ! old_value ) {
            // Old value is false, new value is true.
            d_assert( moving_value <= transition);
            // The only way the two values can be == if is they are min_mv.
            if ( (moving_value <= min_mv) || (max_mv <= transition) ) {
                // Setting moving_value to max will flip it true.
                wrap_to_max( );
            } else {
                d_assert(
                    (min_mv < moving_value) &&
                             (moving_value < transition) &&
                                            (transition < max_mv));
                double const diff = transition - moving_value;
                d_assert( (min_mv < diff) && (diff < max_mv));
                double const new_moving_value = transition + ((max_mv - transition) * (diff / transition));
                if ( (new_moving_value >= max_mv) ||
                     (new_moving_value <= min_mv) ||
                     (new_moving_value <  transition) )
                {
                    d_assert( false);
                    wrap_to_max( );
                } else {
                    set_moving_value( new_moving_value);
                }
            }
            // Value (as bool) is now true.
            d_assert( get_value__bool( ));
        }
    } else {
        if ( old_value ) {
            // Old value is false, new value is true.
            d_assert( moving_value >= transition);
            if ( (moving_value >= max_mv) ||
                 (transition   <= min_mv) ||
                 (moving_value == transition) )
            {
                // Setting moving_value to min_mv will flip it false.
                wrap_to_min( );
            } else {
                d_assert(
                    (max_mv > moving_value) &&
                             (moving_value > transition) &&
                                            (transition > min_mv));
                double const diff = moving_value - transition;
                d_assert( (min_mv < diff) && (diff < max_mv));
                double const new_moving_value = transition * (max_mv - (diff / (max_mv - transition)));
                if ( (new_moving_value >= max_mv) ||
                     (new_moving_value <= min_mv) ||
                     (new_moving_value >= transition) )
                {
                    d_assert( false);
                    wrap_to_min( );
                } else {
                    set_moving_value( new_moving_value);
                }
            }
            d_assert( ! get_value__bool( ));
        }
    }
    d_assert( get_value__bool( ) == new_value);
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bool_holder.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
