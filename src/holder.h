// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// holder.h
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
# ifndef HOLDER_H
# define HOLDER_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "debug.h"
# include <QtCore/QObject>

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
holder_base_type
  : public QObject
  //
  // Holders coordinate between Qt widgets, the animation engine, the scene manager, and any other
  // part of the program that wants to know certain control values from the UI.
  //
  // Subtypes provide:
  //   typedef value_type
  //   contructor( p_parent, init_value)
  //   get_value( )
  //   set_value(..)
  //
  // It would be nice if we could provide the above minimum as a template, but
  // Qt signals don't work with template classes.
  // Consider changing to boost signals/slots. These signals don't have to cross
  // thread boundaries.
  // If we change to boost signals/slots we could get rid of QObject (and Qt for
  // these classes). Then we'd have to explicitly manage memory or build a simple
  // single-owner strict-hierarchy rln mngr to do it for us.
  //
  // Kinds of subtypes (a lot like any struct):
  //   Value holders: bool, int, double, color etc.
  //     These hook to and keep in sync with UI objects (scrollbars, color buttons, etc).
  //     These can hook up to several UI objects and keep them all in sync.
  //   Enum holders:
  //     These are the "styles", like antialias_style_type.
  //     These attach to a group of radio buttons (UI controls).
  //     These are customized for each set of radio buttons. They could be implemented
  //       as an aggregate of bool holders (one for each radio button).
  //       Or a (templated) simple holder where value_type is the enum type.
  //   Aggregates:
  //     All the controls in a dialog, etc.
  //     An aggregate is the parent/owner of the holders underneath. Occasionally it
  //     would be convenient to let a holder have more than one parent. This will not
  //     work with a simple single-owner hierarchy.
  //
  // Parent -- should it be a QObject?
  //   Maybe we should provide a top-level (parent-less) holder, and build the hierarchy
  //   under that?
{
  Q_OBJECT

  public:
    /* ctor */  holder_base_type( QObject * p_parent)   : QObject( p_parent)
                                                        , is_setting_( false)
                                                        { }
    virtual     ~holder_base_type( )                    { }

  private:
    void        operator =( QObject *)                  ; // no implementation

  // We could also provide before_change( ), which could even return a bool to cancel the change.
  // Or there could be provide all these signals:
  //   bool  before_change_cancel( )  - Return true to cancel. Coulc return a reason instead of bool?
  //                                    The reason could identify the child control that canceled, and maybe
  //                                    the canceler?
  //   void  before_change( )         - Always sent unless first canceled, too late to cancel.
  //   void  after_change( )          - Sent if change was not canceled.
  //   void  after_change_cancel( )   - Sent if change was canceled (only sent to those who were sent
  //                                    before_change_cancel( ) signals and returned false.
  // For nested objects, parent before/after change/cancel methods would wrap around child messages.
  //
  // We could try to provide this who/why information:
  //   Who is requesting the change, and why. Why is often a chain of further who requested higher up.
  //   Who is canceling, and why.
  // Although this is a nice idea, I don't think it's very useful in practice. It's over-designed.
  //
  // We could try to provide before/after values (what/where information):
  //   What the value is changing to (and what it is now). Which value is changing (in case it's a child
  //     buried inside an aggregate).
  //   What the value changed to, and what it was before the change.
  // This sort of thing can be used for undo/redo schemes, scriping, and macro record/playback.
  //
  signals:
    void        has_changed( )                          ; /* after_change( ) */

  public:
    bool        is_setting( )                     const { return is_setting_; }

  public:
      struct
    while_setting_value_wrapper_type
      {
        while_setting_value_wrapper_type( holder_base_type *, bool = true)  ;
        ~while_setting_value_wrapper_type( )                                ;

        void request_signal( bool r = true)                                 { request_signal__has_changed = r; }

        void done_with_no_throws( )                                         ;
        void done_with_no_throws( bool request_signal)                      ;

        private:
            holder_base_type * const p_holder;
            bool is_watching_for_throw;
            bool request_signal__has_changed;
      };

  public:
      template< typename HOLDER_T, typename VALUE_T >
      static
      void
    set_value_base
     (  HOLDER_T *  p_holder
      , VALUE_T &   old_value
      , VALUE_T     new_value
      , void (HOLDER_T :: * p_signal)( VALUE_T) = & HOLDER_T :: has_changed
     )
      // We could make this a CRTP but I don't think templates work with Qt.
      // This emits a value signal like has_changed( new_value), followed by the generic no-arg has_changed( ) signal.
      {
        // Only do something if we have to.
        if ( old_value == new_value ) {
            /* no change, do nothing */
        } else
        if ( p_holder->is_setting( ) ) {
            // We are recursing and the value is shifting!
            // This happens when we send a has_changed() signal and while processing someone else sends a signal back.
            // If we get here we may have a problem. We expect this case to be handled in the == test above.
            d_assert( false);
        } else
        {
            while_setting_value_wrapper_type wrapper( p_holder);
            old_value = new_value;
            emit (p_holder->*p_signal)( new_value); /* this works even tho the type of new_value is templated */
            wrapper.done_with_no_throws( );
        }
      }

      template< typename HOLDER_T, typename VALUE_T >
      static
      void
    set_value_base__no_value_signal
     (  HOLDER_T *  p_holder
      , VALUE_T &   old_value
      , VALUE_T     new_value
     )
      // This only emits the has_changed( ) signal.
      {
        // Only do something if we have to.
        if ( old_value == new_value ) {
            /* no change, do nothing */
        } else
        if ( p_holder->is_setting( ) ) {
            // We are recursing and the value is shifting!
            // This happens when we send a has_changed() signal and while processing someone else sends a signal back.
            // If we get here we may have a problem. We expect this case to be handled in the == test above.
            d_assert( false);
        } else
        {
            while_setting_value_wrapper_type wrapper( p_holder);
            old_value = new_value;
            wrapper.done_with_no_throws( );
        }
      }

  public:
      template< typename HOLDER_T, typename QT_WIDGET_T >
      static
      void
    attach_widget_bool( HOLDER_T p_holder, QT_WIDGET_T * p_widget)
      //
      // Creates two-way attachment so the widget tracks all changes from the holder,
      // and the holder tracks changes from the widget.
      {
        d_assert( p_widget);

        //if ( init_value_from_holder ) {
            p_widget->setChecked( p_holder->get_value( ));
            //p_widget->setValue( p_holder->get_value( ));
        //} else {
        //  p_holder->set_value( p_widget->isChecked( ));
            //p_holder->set_value( p_widget->value( ));
        //}

        d_verify( connect( p_widget, SIGNAL( toggled( bool)), p_holder, SLOT( set_value( bool))));
        d_verify( connect( p_holder, SIGNAL( has_changed( bool)), p_widget, SLOT( setChecked( bool))));

        //d_verify( connect( p_widget, SIGNAL( valueChanged( bool)), p_holder, SLOT( set_value( bool))));
        //d_verify( connect( p_holder, SIGNAL( has_changed( bool)), p_widget, SLOT( setValue( bool))));
      }

      template< typename HOLDER_T, typename QT_WIDGET_T >
      static
      void
    detach_widget_bool( HOLDER_T p_holder, QT_WIDGET_T * p_widget)
      {
        d_assert( p_widget);
        d_verify( disconnect( p_holder, SIGNAL( has_changed( bool)), p_widget, SLOT( setChecked( bool))));
        d_verify( disconnect( p_widget, SIGNAL( toggled( bool)), p_holder, SLOT( set_value( bool))));
        // Should we clear the widget?
        // p_widget->setChecked( false);
      }

  public:
      template< typename HOLDER_T, typename QT_WIDGET_T >
      static
      void
    attach_widget_int( HOLDER_T p_holder, QT_WIDGET_T * p_widget)
      //
      // Creates two-way attachment so the widget tracks all changes from the holder,
      // and the holder tracks changes from the widget.
      {
        d_assert( p_widget);

        //if ( init_value_from_holder ) {
            p_widget->setValue( p_holder->get_value( ));
        //} else {
        //  p_holder->set_value( p_widget->value( ));
        //}

        d_verify( connect( p_widget, SIGNAL( valueChanged( int)), p_holder, SLOT( set_value( int))));
        d_verify( connect( p_holder, SIGNAL( has_changed( int)), p_widget, SLOT( setValue( int))));
      }

      template< typename HOLDER_T, typename QT_WIDGET_T >
      static
      void
    detach_widget_int( HOLDER_T p_holder, QT_WIDGET_T * p_widget)
      {
        d_assert( p_widget);
        d_verify( disconnect( p_holder, SIGNAL( has_changed( int)), p_widget, SLOT( setValue( int))));
        d_verify( disconnect( p_widget, SIGNAL( valueChanged( int)), p_holder, SLOT( set_value( int))));
        // Should we clear the widget? Should we set the range and steps?
        // p_widget->setValue( 0);
      }

  protected:
    bool  is_setting_ ; /* recursion watch - true during set_value(..) */
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef HOLDER_H
//
// holder.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
