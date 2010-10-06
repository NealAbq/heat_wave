// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bool_holder.h
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
# ifndef BOOL_HOLDER_H
# define BOOL_HOLDER_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "holder.h"
# include "pack_holder.h"
# include "animate.h"
class QAbstractButton;

// _______________________________________________________________________________________________

class bool_holder;
class click_animator_type;
class bool_animator_type;

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
bool_holder
  : public holder_base_type
{
  Q_OBJECT
  friend class holder_base_type;

  // ------------------------------------------------------------------------------------------
  public:
    typedef bool                value_type            ;
    typedef bool_animator_type  default_animator_type ;

  // ------------------------------------------------------------------------------------------
  public:
    /* ctor */  bool_holder
                 (  QObject *   p_parent
                  , value_type  default_value  = false
                 )                                    ;

  // ------------------------------------------------------------------------------------------
  public:
    void        attach( QAbstractButton *)            ;
    void        detach( QAbstractButton *)            ;

    int         request_auto_on( )                    ;
    void        release_auto_on( int id)              ;

  // ------------------------------------------------------------------------------------------
  public:
    value_type  get_value( )                    const { return value_; }

  // ------------------------------------------------------------------------------------------
  public slots:
    void        set_value( bool /*value_type*/)       ;
  signals:
    void        has_changed( bool /*value_type*/)     ; /* signal */
                                                      ;

  // ------------------------------------------------------------------------------------------
  // Member vars
  private:
    value_type  value_         ;
    int         auto_on_count_ ;
    int         auto_on_id_    ;
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
click_animator_type
  : public delta_animator_type
{
  Q_OBJECT

  public:
    /* ctor */         click_animator_type
                        (  animate_type *  p_parent
                         , double          init_delta_per_sec  = 0.5 /* never zero */
                        )                                 ;

  // From supertype:
  //   attach__is_wrapping( QAbstractButton *)
  //   attach__is_reversed( QAbstractButton *)
  //   attach__delta_per_second( QAbstractSlider *)
  //   detach__is_wrapping( QAbstractButton *)
  //   detach__is_reversed( QAbstractButton *)
  //   detach__delta_per_second( QAbstractSlider *)
  public:
    void               attach__click( QAbstractButton *)  ;
    void               detach__click( QAbstractButton *)  ;

    void               attach__moving_value( QAbstractSlider *)
                                                          ;
    void               detach__moving_value( QAbstractSlider *)
                                                          ;

  // From supertype:
  //   is_wrapping( )               -> bool
  //   is_reflecting( )             -> bool
  //   is_reversed( )               -> bool
  //   get_delta_per_second( )      -> double
  //   get_min_delta_per_second( )  -> double
  //   get_max_delta_per_second( )  -> double
  //   set_is_wrapping( )
  //   set_is_reflecting( bool is = true)
  //   set_is_wrapping( bool)                  - slot
  //   set_is_reversed( bool)                  - slot
  //   set_delta_per_second( double)           - slot
  //   has_changed__is_wrapping( bool)         - signal
  //   has_changed__is_reversed( bool)         - signal
  //   has_changed__delta_per_second( double)  - signal
  public:
    double             get_moving_value( )          const { return p_moving_value_->get_value( ); }
    double             get_min_moving_value( )      const { return p_moving_value_->get_min_value( ); }
    double             get_max_moving_value( )      const { return p_moving_value_->get_max_value( ); }
  public slots:
    void               set_moving_value( double)          ;
  signals:
    void               has_clicked( )                     ;
    void               has_changed__moving_value( double) ;

  // Overridden virtuals, used by supertype
  protected:
    virtual double     get_animating_value( )       const { return get_moving_value( ); }
    virtual void       set_animating_value( double v)     { set_moving_value( v); }

    virtual bool       is_bumping_min( )                  { return p_moving_value_->is_bumping_min( ); }
    virtual bool       is_bumping_max( )                  { return p_moving_value_->is_bumping_max( ); }

    virtual void       wrap_to_min( )                     { p_moving_value_->wrap_to_min( ); }
    virtual void       wrap_to_max( )                     { p_moving_value_->wrap_to_max( ); }

    virtual holder_base_type *
                       get_animated( )              const { return 0; }

  // Member var
  private:
    double_slide_holder * const  p_moving_value_ ; // [0,1]
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
bool_animator_type
  : public click_animator_type
{
  Q_OBJECT

  public:
    typedef bool_holder  default_holder_type ;

  public:
    /* ctor */         bool_animator_type
                        (  animate_type        *  p_parent
                         , default_holder_type *  p_holder
                         , double                 init_delta_per_sec  = 0.5 /* never zero */
                        )                                       ;

  // From supertype:
  //   attach__is_wrapping( QAbstractButton *)
  //   attach__is_reversed( QAbstractButton *)
  //   attach__delta_per_second( QAbstractSlider *)
  //   detach__is_wrapping( QAbstractButton *)
  //   detach__is_reversed( QAbstractButton *)
  //   detach__delta_per_second( QAbstractSlider *)
  //
  //   attach__click(..)
  //   detach__click(..)
  //   attach__moving_value(..)
  //   detach__moving_value(..)
  public:
    void               attach__transition( QAbstractSlider *)   ;
    void               detach__transition( QAbstractSlider *)   ;

    void               attach__bool( QAbstractButton *)         ;
    void               detach__bool( QAbstractButton *)         ;

  // From supertype:
  //   is_wrapping( )                       -> bool
  //   is_reflecting( )                     -> bool
  //   is_reversed( )                       -> bool
  //   get_delta_per_second( )              -> double
  //   get_min_delta_per_second( )          -> double
  //   get_max_delta_per_second( )          -> double
  //   set_is_wrapping( )
  //   set_is_reflecting( bool is = true)
  //   set_is_wrapping( bool)                  - slot
  //   set_is_reversed( bool)                  - slot
  //   set_delta_per_second( double)           - slot
  //   has_changed__is_wrapping( bool)         - signal
  //   has_changed__is_reversed( bool)         - signal
  //   has_changed__delta_per_second( double)  - signal
  //
  //   get_moving_value( )                     -> double
  //   get_min_moving_value( )                 -> double
  //   get_max_moving_value( )                 -> double
  //   set_moving_value( double)               - slot
  //   has_clicked( )                          - signal
  //   has_changed__moving_value( double)      - signal
  public:
    bool               get_value__bool( )                 const ;
    double             get_transition( )                  const ;
  public slots:
    void               set_value__bool( bool)                   ;
    void               set_transition( double)                  ;
  protected slots:
    void               on_changed__moving_value( double)        ;
    void               intercept__changed_holder( bool)         ;
    void               intercept__changed_transition( double)   ;
  signals:
    void               has_changed__bool( double)               ;
    void               has_changed__transition( double)         ;

  // Overridden virtual
  protected:
    virtual holder_base_type *
                       get_animated( )                    const { return p_holder_; }

  // Member vars
  private:
    default_holder_type * const  p_holder_     ;
    double_slide_holder * const  p_transition_ ; // [0,1]
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef BOOL_HOLDER_H */
//
// bool_holder.H - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
