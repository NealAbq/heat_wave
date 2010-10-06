// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// animate.h
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
# ifndef ANIMATE_H
# define ANIMATE_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// _______________________________________________________________________________________________
// Classes defined

class animate_type;
class animator_base_type;
class delta_animator_type;
class default_animator_type;

// _______________________________________________________________________________________________
// Classes used

# include "all.h"
# include "debug.h"
# include "util.h"
# include "date_time.h"
# include "holder.h"
# include "moving_sum.h"
# include <QtCore/QObject>
# include <QtCore/QList>
# include <QtCore/QTimer>
# include <QtGui/QAbstractButton>
# include <QtGui/QAbstractSlider>

class bool_holder;
class double_slide_holder;

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
animate_type
  : public holder_base_type
{
  Q_OBJECT

  // -----------------------------------------------------------------------------------------
  // Typedefs
  public:
    typedef date_time::tick_point_type     tick_point_type    ;
    typedef date_time::tick_duration_type  tick_duration_type ;
    typedef date_time::second_type         second_type        ; /* seconds duration type */
    typedef date_time::millisecond_type    millisecond_type   ; /* milliseconds duration type */

    // frames per second types:
    //   linear, log_01, log_0_1000
    typedef second_type                    frames_per_second_type
                                                            ;

    typedef QList< animator_base_type * >  list_type        ;

  // -----------------------------------------------------------------------------------------
  // Constructor
  public:
    /* ctor */    animate_type( QObject * p_parent = 0)           ;

  // -----------------------------------------------------------------------------------------
  // Predicates
  public:
    bool          is_animating( )                           const ;
    bool          is_animatable( )                          const { return is_animatable_; }

  // -----------------------------------------------------------------------------------------
  // Private use slot
  protected slots:
    void          recalc_is_animatable( bool)                     ;
  private:
    bool          calc_is_animatable( )                     const ;

  // -----------------------------------------------------------------------------------------
  // Frames per second
  public:
    frames_per_second_type
                  get_requested_frames_per_second( )        const ;
    void          request_frames_per_second_log_0_1000( int log_value_0_1000)
                                                                  ;
    void          request_frames_per_second_log_01( frames_per_second_type)
                                                                  ;
    void          request_frames_per_second( frames_per_second_type)
                                                                  ;

  // -----------------------------------------------------------------------------------------
  // UI control, slots and methods
  public:
    void          start( )                                        ;
    void          stop( )                                         ;
  public slots:
    void          start_stop( bool)                               ;
    void          single_step( )                                  ;

  public:
    void          attach__on_off_button( QAbstractButton *)       ;
    void          attach__step_button( QAbstractButton *)         ;

  // -----------------------------------------------------------------------------------------
  // Signals
  signals:
    void          wakeup( )                                       ; /* signal */
    void          has_changed( )                                  ; /* signal */
    void          has_changed__is_animating( bool)                ; /* signal */
    void          has_changed__is_animating_extended( bool)       ; /* signal */
    void          has_changed__is_animatable( bool)               ; /* signal */

  // -----------------------------------------------------------------------------------------
  // Animation
  public:
    // Call this just before painting.
    void          maybe_move( )                                   ;
  protected:
    second_type   calc_seconds_since_last_move( tick_point_type)
                                                            const ;

  // -----------------------------------------------------------------------------------------
  // Duration stats
  //   These are start-to-start (cycle) durations only recorded mid-animation.
  public:
    int           get_move_count( )                         const { return move_count_; }
    bool          is_available_duration( )                  const ;
    second_type   get_average_duration_seconds( )           const ;
    millisecond_type
                  get_average_duration_mseconds( )          const ;
    void          clear_duration_stats( )                         ;

  // -----------------------------------------------------------------------------------------
  // Animators
  public:
    animator_base_type *
                  find_animator( holder_base_type * p_in)   const ;
  protected:
    friend class animator_base_type;
    void          add_animator( animator_base_type *)             ;
    void          remove_animator( animator_base_type *)          ;

  // -----------------------------------------------------------------------------------------
  // Member vars
  private:
    QTimer *         p_timer_               ;
    tick_point_type  tick_last_             ;
    list_type        animators_             ;
    bool             is_animatable_         ;

    moving_sum< tick_duration_type >
                     start_start_durations_ ;
    int              move_count_            ;
    int              step_limit_            ;
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
animator_base_type
  : public holder_base_type
{
  Q_OBJECT

  // -----------------------------------------------------------------------------------------
  // Typedefs
  public:
    typedef date_time::second_type  second_type ;

  // -------------------------------------------------------------------------------------
  // Constructor
  public:
    /* ctor */         animator_base_type( animate_type * p_parent)  ;
    virtual            ~animator_base_type( )                        ;

  // -------------------------------------------------------------------------------------
  // Pure base virtuals
  public:
    virtual void       move( second_type seconds_since_last_move)    = 0;
    virtual holder_base_type *
                       get_animated( )                         const { return 0; }

  // -------------------------------------------------------------------------------------
  public:
    void               attach__is_on( QAbstractButton *)             ;
    void               detach__is_on( QAbstractButton *)             ;

  // -------------------------------------------------------------------------------------
  // Getters
  public:
    animate_type *     get_parent( )                           const ;
    bool               is_parent_animating( )                  const ;

    bool               is_on( )                                const { return is_on_; }
    bool               is_off( )                               const { return ! is_on( ); }

  // -------------------------------------------------------------------------------------
  // Slots
  public slots:
    void               set_on( bool is = true)                       ;
  public:
    void               set_off( )                                    { set_on( false); }

  // -------------------------------------------------------------------------------------
  // Signals
  signals:
    void               has_changed__is_on( bool)                     ; /* signal */

  // -------------------------------------------------------------------------------------
  // Member vars
  private:
    // Should this be a bool_holder?
    bool  is_on_ ;
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
delta_animator_type
  : public animator_base_type
{
  Q_OBJECT
  friend class holder_base_type;

  // -----------------------------------------------------------------------------------------
  // Typedefs
  public:
    typedef double      value_type            ;
    typedef value_type  delta_per_second_type ; /* delta value per second */
    typedef value_type  delta_per_step_type   ; /* delta value per step */

  public:
    /* ctor */         delta_animator_type
                        (  animate_type *         p_parent
                         , bool                   is_wrapping
                         , delta_per_second_type  init_delta_per_sec  /* never zero */
                         , delta_per_second_type  min_delta_per_sec   /* min != max */
                         , delta_per_second_type  max_delta_per_sec
                         , delta_per_step_type    max_delta_per_step  = 0.0  /* always > 0, default is one-second of movement */
                        )                                           ;

  public:
    bool               is_wrapping( )                         const ;
    bool               is_reflecting( )                       const { return ! is_wrapping( ); }
    bool               is_reversed( )                         const ;
    delta_per_second_type
                       get_delta_per_second( )                const ;
    delta_per_second_type
                       get_min_delta_per_second( )            const ;
    delta_per_second_type
                       get_max_delta_per_second( )            const ;

    int                get_extra_bump_count( )                const ;
    delta_per_step_type
                       get_max_delta_per_step( )              const { d_assert( max_delta_per_step_ > 0);
                                                                      return max_delta_per_step_;
                                                                    }

  public:
    void               set_is_wrapping( )                           { set_is_wrapping( true); }
    void               set_is_reflecting( bool is = true)           { set_is_wrapping( ! is); }

    void               set_max_delta_per_step( delta_per_step_type) ;

                       // is_wrapping( ) should be true for both click and check
    void               set_extra_bump__for_click( )                 { extra_bump_state_ = e_click; }
    void               set_extra_bump__for_check( )                 { extra_bump_state_ = e_check; }
    void               set_extra_bump__for_smooth( )                { extra_bump_state_ = e_smooth; }

  public slots:
    void               set_is_wrapping( bool)                       ;
    void               set_is_reversed( bool)                       ;
    void               set_delta_per_second( double)                ;

  signals:
    void               has_changed__is_wrapping( bool)              ; /* signal */
    void               has_changed__is_reversed( bool)              ; /* signal */
    void               has_changed__delta_per_second( double)       ; /* signal */
    void               has_bumped( )                                ;

  protected:
    static
    delta_per_step_type
                       calc_candidate_max_per_step
                        (  delta_per_step_type    new_mps
                         , delta_per_second_type  min_dps
                         , delta_per_second_type  max_dps
                        )                                           ;
    static
    delta_per_second_type
                       calc_candidate_delta_per_second
                        (  delta_per_second_type  min_dps
                         , delta_per_second_type  max_dps
                        )                                           ;

  // -------------------------------------------------------------------------------------
  public:
    void               attach__is_wrapping( QAbstractButton *)      ;
    void               attach__is_reversed( QAbstractButton *)      ;
    void               attach__delta_per_second( QAbstractSlider *) ;

    void               detach__is_wrapping( QAbstractButton *)      ;
    void               detach__is_reversed( QAbstractButton *)      ;
    void               detach__delta_per_second( QAbstractSlider *) ;

  public:
    delta_animator_type *  /* self return */
                       set_auto_on
                        (  bool_holder * = 0
                         , bool_holder * = 0
                        )                                           ;
  protected:
    void               inform_auto_on__is_animating( bool)          ;

  public:
    virtual void       move( second_type seconds_since_last_move)   ; /* overridden virtual */

  protected:
    virtual value_type get_animating_value( )                 const = 0;
    virtual void       set_animating_value( value_type)             = 0;

    virtual bool       is_bumping_min( )                            = 0;
    virtual bool       is_bumping_max( )                            = 0;

    // Implement these if is_wrapping( ) is ever true.
    virtual void       wrap_to_min( )                               { d_assert( false); }
    virtual void       wrap_to_max( )                               { d_assert( false); }

  protected:
    void               request_animating_value( value_type)         ;
    bool               bump_before_move__extra_slow( )              ;
    bool               bump_after_move( )                           ;
    bool               is_moving_in_positive_direction( ) const     ;
    bool               calc_next_animating_value_from_leftover( value_type &)
                                                                    ;
    bool               calc_next_animating_value( second_type, value_type &)
                                                                    ;
    value_type         calc_delta_from_seconds( second_type seconds_since_last_move)
                                                              const ;
    value_type         get_animating_value__maybe_clear_leftover( ) ;
    void               set_animating_value__save_leftover( value_type)
                                                                    ;
    bool               is_bumping_min__protected( )                 ;
    bool               is_bumping_max__protected( )                 ;
    void               wrap_to_min__fixup( )                        ;
    void               wrap_to_max__fixup( )                        ;

  protected slots:
    void               on_changed__is_parent_animating( bool)       ;
    void               on_changed__is_on( bool)                     ;
    void               on_changed__is_reversed( bool)               ;
    void               on_changed__delta_per_second( double)        ;

  friend class callout_wrap;
  private:
    bool_holder         *  p_is_wrapping_      ; /* or reflecting */
    bool_holder         *  p_is_reversed_      ;
    enum {
        e_click
      , e_check
      , e_smooth
    }                      extra_bump_state_   ;
    bool                   is_calling_out_     ; /* recursion watch */

    double_slide_holder *  p_delta_per_second_ ; // animation value change per second
    delta_per_second_type  max_delta_per_step_ ; // max animating value change in a single animation STEP, not per SECOND
    value_type             prev_value_         ; // previous achieved animating value
    value_type             leftover_           ; // amount of animation value leftover from the last animation step

    bool_holder         *  p_auto_on_holder_0_ ;
    bool_holder         *  p_auto_on_holder_1_ ;
    int                    id_auto_on_0_       ;
    int                    id_auto_on_1_       ;
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  template< typename HOLDER_TYPE >
  class
default_animator_type
  : public delta_animator_type
{
  public:
    typedef HOLDER_TYPE                               default_holder_type ;
    typedef typename default_holder_type::value_type  holder_value_type   ;
    typedef delta_animator_type::value_type           animator_value_type ;

  public:
    /* ctor */      default_animator_type
                     (  animate_type        *  p_parent
                      , default_holder_type *  p_holder
                      , bool                   is_wrapping
                      , animator_value_type    delta_per_sec       /* never zero */
                      , animator_value_type    min_delta_per_sec   /* min != max */
                      , animator_value_type    max_delta_per_sec
                      , animator_value_type    max_delta_per_step  = 0.0
                     )                                       : delta_animator_type
                                                                (  p_parent
                                                                 , is_wrapping
                                                                 , delta_per_sec
                                                                 , min_delta_per_sec
                                                                 , max_delta_per_sec
                                                                 , max_delta_per_step
                                                                )
                                                             , p_holder_( p_holder)
                                                             { d_assert( p_holder_); }

  // Overridden virtuals, used by supertype
  protected:
    virtual animator_value_type
                    get_animating_value( )             const { return
                                                                 util::cast_nearest< animator_value_type >(
                                                                   p_holder_->get_value( ));
                                                             }
    virtual void    set_animating_value( animator_value_type new_value)
                                                             { p_holder_->set_value(
                                                                 util::cast_nearest< holder_value_type >( new_value));
                                                             }

    virtual bool    is_bumping_min( )                        { return p_holder_->is_bumping_min( ); }
    virtual bool    is_bumping_max( )                        { return p_holder_->is_bumping_max( ); }

    virtual void    wrap_to_min( )                           { p_holder_->wrap_to_min( ); }
    virtual void    wrap_to_max( )                           { p_holder_->wrap_to_max( ); }

    virtual holder_base_type *
                    get_animated( )                    const { return p_holder_; }

  // Member var
  private:
    default_holder_type * const  p_holder_ ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef ANIMATE_H */
//
// animate.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
