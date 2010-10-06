// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// int_holder.cpp
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
// Improvements:
//   There should be holders for unsigned-ints, strings, chars, bytes, etc.
//   This should be a template class. How do templates work with the signals/slots? Not well.
//
//   Class double_holder.
//   This would attach to double-spin-boxes.
//   The UI currently has double-spin-boxes in these places:
//     4 out_of_date_ui_type objects (how many seconds to delay).
//     The x and y solve rates (2 spin boxes).
//   Neither of these is able to attach to several (redundant) UI ctrls at once.
//   Nor can they be animated right now.
//   |
//   The solve-rates are a good example of an interacting set of UI objects:
//     Two dspin boxes to set the rates (with color-coded backgrounds).
//     Two labels that show the current rates (not what's in the boxes).
//     A button to make both values in the boxes into the official rates.
//   We could also have:
//     Reset button.
//     Checkbox to lock the two rates together.
//   If we used holders for all this, we would also have an overall holder
//   direct changed values where they need to go.
//
// Notes:
//   There are two sides to a holder:
//   1. UI elements and the values that control them.
//   2. Model or model-view values.
//   To illustrate, think of a zoom-in/out controlled by a slider.
//   The model-view side may want to see the zoom-scale as a float from 0.1 .. 10.0.
//   The slider will deal with integers, which will probably be exponentially mapped to
//   the model-view units.
//
//   The UI widget are usually wrapped in a class with signals/slots using internal widget
//   units. These holder objects should be separate, so they can be attached dynamically
//   to zero or many UI widgets. These holder objects are the glue between two or more
//   control systems.
//
// Addition:
//   A flag to freeze the range. int_range_steps_holder is used as an internal value holder
//   for other holder classes, and it it attached directly to an external slider.
//   Which means anyone can change the range (and steps) of the slider and have it propagate
//   back to the internal int_range_steps_holder object even though it's a private object.
//   Answer:
//     Flags to freeze the range and also the steps.
//     Way to say:
//       The range is frozen and should not be changed by anyone.
//       The value should only come from the UI and not from another holder?
//         I'm not sure we'd ever use that though. It disables animation and overlapping controls.
//         No, this would mean you can attach tracking controls but not those that make changes.
//           Unless maybe the changes came from the UI (user actions)?
//       The value should only come from ctrls that attach thru the owner holder.
// _______________________________________________________________________________________________

# include "all.h"
# include "int_holder.h"
# include "util.h"

# include <QtGui/QSpinBox>
# include <QtGui/QAbstractSlider>

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  namespace /* anonymous */ {
  int
calc_about_an_eighth( int diff)
{
    if ( diff == 0 ) return 1;
    return (diff + ((diff < 0) ? -7 : +7)) / 8;
}
} /* end namespace anonymous */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// int_range_steps_holder - class
// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* ctor */
  int_range_steps_holder::
int_range_steps_holder
 (  QObject *                 p_parent
  , int_range_steps_holder *  p_copy
 )
  : holder_base_type  ( p_parent)
  , attached_widgets_ ( )
  , value_            ( d_verify_not_zero( p_copy)->get_value( ))
  , min_value_        ( p_copy->get_min_value( ))
  , max_value_        ( p_copy->get_max_value( ))
  , single_step_      ( p_copy->get_single_step( ))
  , page_step_        ( p_copy->get_page_step( ))
{
    d_assert( is_valid( ));
}

  /* ctor */
  int_range_steps_holder::
int_range_steps_holder
 (  QObject *     p_parent
  , int           init_value  // = 0
 )
  : holder_base_type  ( p_parent)
  , attached_widgets_ ( )
  , value_            ( init_value)
  , min_value_        ( init_value)
  , max_value_        ( init_value)
  , single_step_      ( 0         )
  , page_step_        ( 0         )
{
    d_assert( is_valid( ));
}

  /* ctor */
  int_range_steps_holder::
int_range_steps_holder
 (  QObject *     p_parent
  , int           init_value
  , int           min_value
  , int           max_value
  , int           single_step  // = 0
  , int           page_step    // = 0
 )
  : holder_base_type  ( p_parent)
  , attached_widgets_ ( )
  , value_            ( init_value )
  , min_value_        ( min_value  )
  , max_value_        ( max_value  )
  , single_step_      ( single_step)
  , page_step_        ( page_step  )
{
    d_assert( is_valid( ));
}

// _______________________________________________________________________________________________

  bool
  int_range_steps_holder::
is_valid( ) const
{
    return
        (get_min_value( ) <= get_value( )) &&
        (get_value( ) <= get_max_value( )) &&
        (get_single_step( ) >= 0) &&
        (get_page_step( ) >= 0) &&
        implies(
            (get_page_step( ) > 0),
            (get_page_step( ) >= get_single_step( )));
}

  bool
  int_range_steps_holder::
is_attached( ) const
{
    return ! attached_widgets_.isEmpty( );
}

  int
  int_range_steps_holder::
get_attachment_count( ) const
{
    return attached_widgets_.count( );
}

// _______________________________________________________________________________________________

  void
  int_range_steps_holder::
move_values_to( QSpinBox * p_spinb) const
{
    d_assert( p_spinb);

    // The single-step value we store isn't always the one we give to the spinboxes.
    int single_step_spinb = get_single_step( );
    if ( 0 == single_step_spinb ) {
        single_step_spinb = 1;
    }

    p_spinb->setSingleStep( single_step_spinb);
    p_spinb->setRange( get_min_value( ), get_max_value( ));
    p_spinb->setValue( get_value( ));
}

  void
  int_range_steps_holder::
move_values_to( QAbstractSlider * p_slider) const
{
    d_assert( p_slider);

    // The step values we store aren't necessarily the ones we give the sliders.
    int page_step_slider   = get_page_step(   );
    int single_step_slider = get_single_step( );
    if ( 0 == page_step_slider ) {
        page_step_slider =
            (0 != single_step_slider) ?
            (10 * single_step_slider) :
            calc_about_an_eighth( get_max_value( ) - get_min_value( ));
    }
    if ( 0 == single_step_slider ) {
        single_step_slider = calc_about_an_eighth( page_step_slider);
    }

    p_slider->setPageStep( page_step_slider);
    p_slider->setSingleStep( single_step_slider);
    p_slider->setRange( get_min_value( ), get_max_value( ));
    p_slider->setValue( get_value( ));
}

// _______________________________________________________________________________________________

  void
  int_range_steps_holder::
set_values_from( QSpinBox * p_spinb)
{
    d_assert( p_spinb);
    set_values
     (  p_spinb->value( )
      , p_spinb->minimum( )
      , p_spinb->maximum( )
      , p_spinb->singleStep( )
      //p_spinb->pageStep( )
     );
}

  void
  int_range_steps_holder::
set_values_from( QAbstractSlider * p_slider)
{
    d_assert( p_slider);
    set_values
     (  p_slider->value( )
      , p_slider->minimum( )
      , p_slider->maximum( )
      , p_slider->singleStep( )
      , p_slider->pageStep( )
     );
}

// _______________________________________________________________________________________________

  void
  int_range_steps_holder::
set_values( int_range_steps_holder const * p_copy)
{
    d_assert( p_copy);
    set_values
     (  p_copy->get_value(       )
      , p_copy->get_min_value(   )
      , p_copy->get_max_value(   )
      , p_copy->get_single_step( )
      , p_copy->get_page_step(   )
     );
}

  void
  int_range_steps_holder::
set_values
 (  value_type  val
  , value_type  min
  , value_type  max
  , value_type  ss   // = -1
  , value_type  ps   // = -1
 )
{
    d_assert( is_valid( ));

    // Use existing values if negative steps specified.
    if ( ss < 0 ) { ss = get_single_step( ); }
    if ( ps < 0 ) { ps = get_page_step(   ); }

    // Make sure the new values are valid. Min and single-step never change here.
    if ( max < min ) { max = min; }
    if ( val < min ) { val = min; }
    if ( val > max ) { val = max; }
    if ( (0 != ps) && (ps < ss) ) { ps = ss; }

    // Find out what's going to change.
    bool const  is_changed__val  = (value_       != val);
    bool const  is_changed__min  = (min_value_   != min);
    bool const  is_changed__max  = (max_value_   != max);
    bool const  is_changed__ss   = (single_step_ != ss );
    bool const  is_changed__ps   = (page_step_   != ps );

    // We're done if nothing is changing.
    if ( is_changed__val ||
         is_changed__min ||
         is_changed__max ||
         is_changed__ss  ||
         is_changed__ps  )
    {
        if ( is_setting( ) ) {
            d_assert( false);
        } else {
            while_setting_value_wrapper_type wrapper( this);

            // Set the values in this holder.
            if ( is_changed__val ) { value_       = val; }
            if ( is_changed__min ) { min_value_   = min; }
            if ( is_changed__max ) { max_value_   = max; }
            if ( is_changed__ss  ) { single_step_ = ss ; }
            if ( is_changed__ps  ) { page_step_   = ps ; }
            d_assert( is_valid( ));

            // Loop thru all the attached widgets (spinboxes and sliders).
            // We don't have to relay our has_changed.. signals to these ctrls because we
            // keep them in sync here.
            list_type::iterator        iter        = attached_widgets_.begin( );
            list_type::iterator const  iter_limit  = attached_widgets_.end( );
            while ( iter != iter_limit ) {
                QWidget * p_widget = *iter;
                d_assert( p_widget);

                QSpinBox * p_spinb = qobject_cast< QSpinBox * >( p_widget);
                if ( p_spinb ) {
                    move_values_to( p_spinb);
                } else {
                    QAbstractSlider * p_slider = qobject_cast< QAbstractSlider * >( p_widget);
                    d_assert( p_slider);
                    move_values_to( p_slider);
                }
                
                ++ iter;
            }

            // Emit the value signals. The has_changed( ) signal with no args will be emitted last.
            if ( is_changed__ss || is_changed__ps ) {
                emit has_changed__steps( get_single_step( ), get_page_step( ));
            }
            if ( is_changed__min || is_changed__max ) {
                emit has_changed__range( get_min_value( ), get_max_value( ));
            }
            if ( is_changed__val ) {
                emit has_changed( get_value( ));
            }

            // The dtor for this wrapper emits the has_changed( ) signal.
            wrapper.done_with_no_throws( );
        }
    }
    d_assert( is_valid( ));
}

// _______________________________________________________________________________________________

  void
  int_range_steps_holder::
set_range( value_type min, value_type max)
{
    set_values( get_value( ), min, max);
}

  void
  int_range_steps_holder::
set_steps( value_type single_step, value_type page_step)
{
    set_values( get_value( ), get_min_value( ), get_max_value( ), single_step, page_step);
}

// _______________________________________________________________________________________________

  void
  int_range_steps_holder::
set_value( value_type new_value)
{
    // set_value_base( this, value_, new_value);
    set_values( new_value, get_min_value( ), get_max_value( ));
}

  void
  int_range_steps_holder::
set_min_value( value_type new_min_value)
{
    // set_value_base( this, min_value_, new_min_value, & int_range_steps_holder::has_changed__min_value);
    set_values( get_value( ), new_min_value, get_max_value( ));
}

  void
  int_range_steps_holder::
set_max_value( value_type new_max_value)
{
    // set_value_base( this, max_value_, new_max_value, & int_range_steps_holder::has_changed__max_value);
    set_values( get_value( ), get_min_value( ), new_max_value);
}

  void
  int_range_steps_holder::
set_single_step( value_type new_single_step)
{
    // set_value_base( this, single_step_, new_single_step, & int_range_steps_holder::has_changed__single_step);
    set_values( get_value( ), get_min_value( ), get_max_value( ), new_single_step);
}

  void
  int_range_steps_holder::
set_page_step( value_type new_page_step)
{
    // set_value_base( this, page_step_, new_page_step, & int_range_steps_holder::has_changed__page_step);
    set_values( get_value( ), get_min_value( ), get_max_value( ), get_single_step( ), new_page_step);
}

// _______________________________________________________________________________________________

  void
  int_range_steps_holder::
attach( QSpinBox * p_spinb)
{
    d_assert( p_spinb);

    // Move the values.
    d_assert( is_valid( ));
    //if ( init_value_from_holder ) {
        move_values_to( p_spinb);
    //} else {
    //  set_values_from( p_spinb);
    //}
    d_assert( is_valid( ));

    // Put the widget on the list.
    d_assert( ! attached_widgets_.contains( p_spinb));
    attached_widgets_.push_front( p_spinb);
    d_verify( connect(
        p_spinb, SIGNAL( destroyed( QObject *)),
        this, SLOT( before_dtor( QObject *))
    ));

    // Watch for a signal from the UI.
    d_verify( connect( p_spinb, SIGNAL( valueChanged( int)), this, SLOT( set_value( int))));

    // We also need to relay changes here back to the UI objects. We could do it by intercepting
    // these signals from this object:
    //   has_changed( int)
    //   has_changed__range( int, int)
    //   has_changed__steps( int, int)
    // But since the ctrl classes don't have the slots to support this, so instead we loop thru
    // our list of attached widgets and set these values explicitly. See move_values_to(..) and
    // set_values(..).
}

  void
  int_range_steps_holder::
detach( QSpinBox * p_spinb)
{
    d_assert( p_spinb);
    d_assert( is_valid( ));
    d_assert( attached_widgets_.contains( p_spinb));

    d_verify( disconnect( p_spinb, SIGNAL( valueChanged( int)), this, SLOT( set_value( int))));
    d_verify( disconnect( p_spinb, SIGNAL( destroyed( QObject *)), this, SLOT( before_dtor( QObject *))));
    d_verify( 1 == attached_widgets_.removeAll( p_spinb));

    d_assert( ! attached_widgets_.contains( p_spinb));
    d_assert( is_valid( ));
}

  void
  int_range_steps_holder::
attach( QAbstractSlider * p_slider)
{
    d_assert( p_slider);

    // Move the values.
    d_assert( is_valid( ));
    //if ( init_value_from_holder ) {
        move_values_to( p_slider);
    //} else {
    //  set_values_from( p_slider);
    //}
    d_assert( is_valid( ));

    // Put the widget on the list.
    d_assert( ! attached_widgets_.contains( p_slider));
    attached_widgets_.push_front( p_slider);
    d_verify( connect(
        p_slider, SIGNAL( destroyed( QObject *)),
        this, SLOT( before_dtor( QObject *))
    ));

    // Watch for a signal from the UI.
    d_verify( connect( p_slider, SIGNAL( valueChanged( int)), this, SLOT( set_value( int))));

    // We don't connect back the other way (from this object to the UI) because the UI classes
    // don't have the right slots. Instead we keep these objects updated in set_values(..).
}

  void
  int_range_steps_holder::
detach( QAbstractSlider * p_slider)
{
    d_assert( p_slider);
    d_assert( is_valid( ));
    d_assert( attached_widgets_.contains( p_slider));

    d_verify( disconnect( p_slider, SIGNAL( valueChanged( int)), this, SLOT( set_value( int))));
    d_verify( disconnect( p_slider, SIGNAL( destroyed( QObject *)), this, SLOT( before_dtor( QObject *))));
    d_verify( 1 == attached_widgets_.removeAll( p_slider));

    d_assert( ! attached_widgets_.contains( p_slider));
    d_assert( is_valid( ));
}

  /* protected slot */
  void
  int_range_steps_holder::
before_dtor( QObject * p_attached_object_about_to_be_destroyed)
{
    QWidget * const
        p_attached_widget_about_to_be_destroyed =
            qobject_cast< QWidget * >( p_attached_object_about_to_be_destroyed);
    d_assert( p_attached_widget_about_to_be_destroyed);
    d_verify( 1 == attached_widgets_.removeAll( p_attached_widget_about_to_be_destroyed));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// int_animator_type - class
// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  namespace /* anonymous */ {
  double
guess_min_delta_per_sec( int_range_steps_holder * p_holder, double init_value, double min_value)
{
    if ( min_value != 0 ) return min_value;

    d_assert( p_holder);
    int const range = p_holder->get_range( );
    d_assert( range > 0);
    double guess = static_cast< double >( range) / 1000;
    d_assert( guess > 0);

    if ( init_value != 0 ) {
        if ( guess > init_value ) {
            guess = init_value;
        }
    }

    return guess;
} } /* end namespace anonymous */

  namespace /* anonymous */ {
  double
guess_max_delta_per_sec( int_range_steps_holder * p_holder, double init_value, double max_value)
{
    if ( max_value != 0 ) return max_value;

    d_assert( p_holder);
    int const range = p_holder->get_range( );
    d_assert( range > 0);
    double guess = (static_cast< double >( range) * 2) / 3;

    if ( init_value != 0 ) {
        if ( guess < init_value ) {
            guess = init_value;
        }
    }

    return guess;
} } /* end namespace anonymous */

// _______________________________________________________________________________________________

  /* constructor */
  int_animator_type::
int_animator_type
 (  animate_type           *  p_parent
  , int_range_steps_holder *  p_holder
  , double                    init_delta_per_sec
  , double                    min_delta_per_sec
  , double                    max_delta_per_sec
 )
  : delta_animator_type (  p_parent
                         , false
                         , init_delta_per_sec
                         , guess_min_delta_per_sec( p_holder, init_delta_per_sec, min_delta_per_sec)
                         , guess_max_delta_per_sec( p_holder, init_delta_per_sec, max_delta_per_sec)
                        )
  , p_holder_           ( p_holder)
  { d_assert( p_holder_); }

// _______________________________________________________________________________________________

  /* overridden virtual */
  holder_base_type *
  int_animator_type::
get_animated( ) const
{
    d_assert( p_holder_);
    return p_holder_;
}

  /* overridden virtual */
  double
  int_animator_type::
get_animating_value( ) const
{
    d_assert( p_holder_);
    // Cast from int to double.
    return util::cast_nearest< double >( p_holder_->get_value( ));
}

  /* overridden virtual */
  void
  int_animator_type::
set_animating_value( double new_value)
{
    d_assert( p_holder_);
    // Set the int as a double.
    int const new_value_i = util::cast_nearest< int >( new_value);
    p_holder_->set_value( new_value_i);
}

  /* overridden virtual */
  bool
  int_animator_type::
is_bumping_min( )
{
    d_assert( p_holder_);
    return p_holder_->is_bumping_min( );
}

  /* overridden virtual */
  bool
  int_animator_type::
is_bumping_max( )
{
    d_assert( p_holder_);
    return p_holder_->is_bumping_max( );
}

  /* overridden virtual */
  void
  int_animator_type::
wrap_to_min( )
{
    d_assert( p_holder_);
    p_holder_->wrap_to_min( );
}

  /* overridden virtual */
  void
  int_animator_type::
wrap_to_max( )
{
    d_assert( p_holder_);
    p_holder_->wrap_to_max( );
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// int_holder.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
