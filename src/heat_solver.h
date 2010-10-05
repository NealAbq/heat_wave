// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// heat_solver.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef HEAT_SOLVER_H
# define HEAT_SOLVER_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "finite_diff_solver.h"
# include "sheet.h"
# include "date_time.h"

// This uses QT for the following:
//   QObject
//     Manages memory (auto delete).
//     Provides signals/slots (communication between threads, communication with owner object).
//   QThread
//   QSemaphore
//     Provides threads and a simple synch at startup. Easily replaced by <boost/thread.hpp>
//     if you want QT independence.
# include <QtCore/QObject>
# include <QtCore/QThread>
# include <QtCore/QSemaphore>

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
namespace heat_solver {

// _______________________________________________________________________________________________
// Classes declared

// These enums are declared below. Strict c++ (gcc) complains if you forward-declare them here
// like a struct or class name.
//   enum  technique_type           ;
//   enum  method_type              ;
//   enum  last_solve_location_type ;

class input_params_type        ;
class sheet_params_type        ;
class output_params_type       ;

class solver_type              ;
class control_type             ;
class worker_thread_type       ;

// _______________________________________________________________________________________________
// Types used

typedef date_time::tick_point_type     tick_point_type       ;
typedef date_time::tick_duration_type  tick_duration_type    ;

typedef sheet_type::inner_const_iter   src_iter_type         ;
typedef sheet_type::inner_varia_iter   trg_iter_type         ;
typedef sheet_type::value_type         value_type            ;
typedef value_type                     rate_type             ;
typedef sheet_type::size_type          size_type             ;

typedef std::vector< value_type >      buf_type              ;
typedef buf_type::iterator             buf_iter_type         ;

typedef calc_next_1d_functor_super_type
         <  rate_type
          , src_iter_type
          , trg_iter_type
         >                             solve_1d_functor_type ;

// _______________________________________________________________________________________________
// Enum types

// We could also have threading type:
//   Serial
//   Parallel

  enum
technique_type
 {  e_ortho_interleave
  , e_simultaneous_2d
  , e_wave_with_damping
 };

  enum
method_type
 {  e_forward_diff
  , e_backward_diff
  , e_central_diff
 };

  enum
last_solve_location_type
 {  e_not_saved
  , e_in_src
  , e_in_extra
 };

// _______________________________________________________________________________________________
// input_params_type

  class
input_params_type
{
  // -------------------------------------------------------------------------------------------
  public:
    /* default copy ctor and ass-op */

    /* ctor */  input_params_type( )                      ;

    /* ctor */  input_params_type
                 (  technique_type  technique
                  , method_type     method
                  , bool            is_method_parallel
                  , rate_type       damping
                  , rate_type       rate_x
                  , rate_type       rate_y
                  , size_type       extra_pass_count   /* zero means only solve once */
                  , bool            reset_if_not_used  /* reset extra sheet if we don't use it */
                  , bool            copy_for_history   /* copy last result to extra if needed to save history, */
                                                       /*   but only if extra is already correctly allocated */
                  , bool            size_for_history   /* resize extra if necessary to save history */
                 )                                        ;

  // -------------------------------------------------------------------------------------------
  public:
    technique_type
                get_technique( )                    const { return technique_; }
    bool        is_technique__ortho_interleave(  )  const { return technique_ == e_ortho_interleave ; }
    bool        is_technique__simultaneous_2d(   )  const { return technique_ == e_simultaneous_2d  ; }
    bool        is_technique__wave_with_damping( )  const { return technique_ == e_wave_with_damping; }

    method_type get_method( )                       const { return method_; }
    bool        is_method__forward_diff(  )         const { return method_ == e_forward_diff ; }
    bool        is_method__backward_diff( )         const { return method_ == e_backward_diff; }
    bool        is_method__central_diff(  )         const { return method_ == e_central_diff ; }

    bool        is_method_parallel( )               const { return is_method_parallel_; }

    rate_type   get_damping( )                      const { return damping_; }
    rate_type   get_rate_x( )                       const { return rate_x_ ; }
    rate_type   get_rate_y( )                       const { return rate_y_ ; }

    size_type   get_extra_pass_count( )             const { return extra_pass_count_; }
    size_type   get_pass_count( )                   const { return get_extra_pass_count( ) + 1; }
    bool        has_extra_passes( )                 const { return get_extra_pass_count( ) > 0; }
    bool        are_extra_passes_disabled( )        const { return are_extra_passes_disabled_; }

    bool        is_extra_sheet_to_be_reset_if_not_used( )
                                                    const { return reset_if_not_used_; }
    bool        is_saving_history_worth_extra_copy( )
                                                    const { return copy_for_history_     ; }
    bool        is_saving_history_worth_sizing_extra( )
                                                    const { return size_for_history_   ; }

  // -------------------------------------------------------------------------------------------
  protected:
    technique_type  technique_                 ;
    method_type     method_                    ;
    bool            is_method_parallel_        ;

    rate_type       damping_                   ;
    rate_type       rate_x_                    ;
    rate_type       rate_y_                    ;

    size_type       extra_pass_count_          ;
    bool            are_extra_passes_disabled_ ;

    bool            reset_if_not_used_         ;
    bool            copy_for_history_          ;
    bool            size_for_history_          ;
};

// _______________________________________________________________________________________________
// settable_input_params_type

  class
settable_input_params_type
  : public input_params_type
{
  // -------------------------------------------------------------------------------------------
  public:
    bool        set_technique( technique_type tech)       ;
    bool        set_method( method_type m)                ;
    bool        set__is_method_parallel( bool is = true)  ;
    bool        set_damping( rate_type)                   ;
    bool        set_rate_x( rate_type)                    ;
    bool        set_rate_y( rate_type)                    ;
    bool        set_extra_pass_count( size_type c)        ;
    bool        set__are_extra_passes_disabled( bool)     ;

  // -------------------------------------------------------------------------------------------
  // Hide inherited member vars
  private:
    input_params_type::technique_                 ;
    input_params_type::method_                    ;
    input_params_type::is_method_parallel_        ;

    input_params_type::damping_                   ;
    input_params_type::rate_x_                    ;
    input_params_type::rate_y_                    ;

    input_params_type::extra_pass_count_          ;
    input_params_type::are_extra_passes_disabled_ ;

    input_params_type::reset_if_not_used_         ;
    input_params_type::copy_for_history_          ;
    input_params_type::size_for_history_          ;
};

// _______________________________________________________________________________________________
// sheet_params_type

  class
sheet_params_type
{
  // -------------------------------------------------------------------------------------------
  public:
    /* ctor */  sheet_params_type
                 (  sheet_type const &  src_sheet
                  , sheet_type       &  trg_sheet
                  , sheet_type       &  extra_sheet
                 )                                        : src_sheet_   ( src_sheet   )
                                                          , trg_sheet_   ( trg_sheet   )
                                                          , extra_sheet_ ( extra_sheet )
                                                          { d_assert( 0 != (& src_sheet  ));
                                                            d_assert( 0 != (& trg_sheet  ));
                                                            d_assert( 0 != (& extra_sheet));
                                                            d_assert( (& src_sheet) != (& extra_sheet));
                                                            d_assert( (& trg_sheet) != (& extra_sheet));
                                                            d_assert( src_sheet.get_x_count( ) == trg_sheet.get_x_count( ));
                                                            d_assert( src_sheet.get_y_count( ) == trg_sheet.get_y_count( ));
                                                            d_assert( src_sheet.get_x_count( ) > 0);
                                                            d_assert( src_sheet.get_y_count( ) > 0);
                                                          }

  // -------------------------------------------------------------------------------------------
  public:
    sheet_type const &  ref_src_sheet(   )          const { return src_sheet_  ; }
    sheet_type       &  ref_trg_sheet(   )          const { return trg_sheet_  ; }
    sheet_type       &  ref_extra_sheet( )          const { return extra_sheet_; }

    bool                are_src_trg_sheets_same( )  const { return (& src_sheet_) == (& trg_sheet_); }
    size_type           get_x_count( )              const { return trg_sheet_.get_x_count( ); }
    size_type           get_y_count( )              const { return trg_sheet_.get_y_count( ); }

    bool                is_extra_sheet_sized( )     const { return
                                                              (extra_sheet_.get_x_count( ) == get_x_count( )) &&
                                                              (extra_sheet_.get_y_count( ) == get_y_count( )) ;
                                                          }

    bool                maybe_size_extra_sheet( )   const { // Make sure the extra_sheet is correctly sized.
                                                            // extra_sheet is usually already set up, so we don't resize it often.
                                                            if ( ! is_extra_sheet_sized( ) ) {
                                                                extra_sheet_.set_xy_counts( get_x_count( ), get_y_count( ), 0);
                                                                d_assert( is_extra_sheet_sized( ));
                                                                return true;
                                                            }
                                                            return false;
                                                          }

  // -------------------------------------------------------------------------------------------
  private:
    sheet_type const &  src_sheet_   ;
    sheet_type       &  trg_sheet_   ;
    sheet_type       &  extra_sheet_ ;
};

// _______________________________________________________________________________________________
// output_params_type

  class
output_params_type
{
  // -------------------------------------------------------------------------------------------
  public:
    /* ctor */  output_params_type( )                  { reset( ); }

  // -------------------------------------------------------------------------------------------
  public:
    void       reset( )                                { is_early_exit_       = false;
                                                         was_extra_used_      = false;
                                                         was_extra_sized_     = false;
                                                         solve_count_         = 0;
                                                         last_solve_location_ = e_not_saved;
                                                       }

    bool &     ref_early_exit( )                       { return is_early_exit_; }
    bool       is_early_exit( )                  const { return is_early_exit_; }

    bool       was_extra_used( )                 const { return was_extra_used_; }
    bool       was_extra_sized( )                const { return was_extra_sized_; }
    size_type  get_solve_count( )                const { return solve_count_; }

    void       set__was_extra_used( )                  { was_extra_used_  = true; }
    void       set__was_extra_sized( )                 { was_extra_sized_ = true; }
    void       inc_solve_count( )                      { ++ solve_count_; }

    bool       is_last_solve_not_saved( )        const { return last_solve_location_ == e_not_saved; }
    bool       is_last_solve_saved_in_src( )     const { return last_solve_location_ == e_in_src   ; }
    bool       is_last_solve_saved_in_extra( )   const { return last_solve_location_ == e_in_extra ; }

    void       set__is_last_solve_saved_in_src( )      { last_solve_location_ = e_in_src   ; }
    void       set__is_last_solve_saved_in_extra( )    { last_solve_location_ = e_in_extra ; }

  // -------------------------------------------------------------------------------------------
  private:
    bool                      is_early_exit_       ;
    bool                      was_extra_used_      ;
    bool                      was_extra_sized_     ;
    size_type                 solve_count_         ;  /* zero before solve */
    last_solve_location_type  last_solve_location_ ;  /* is history available, and if so is it in trg_sheet or extra_sheet */
};

// _______________________________________________________________________________________________
// solver_type

  class
solver_type
  //
  // This is an algorithm class.
  // It runs in a worker thread. However the object is owned by an object in the UI thread,
  // and controlled from there. It is ctor'd and dtor'd from the UI thread.
{
  // -------------------------------------------------------------------------------------------
  // Constructor
  public:
    /* ctor */  solver_type( )                          ;
    /* dtor */  ~solver_type( )                         { }

  // -------------------------------------------------------------------------------------------
  // Controls
  public:
    output_params_type const &
                get_output_params( )              const { return output_params_; }

    bool        is_early_exit( )                  const { return output_params_.is_early_exit( ); }
    bool        not_early_exit( )                 const { return ! is_early_exit( ); }
    void        request_early_exit( )                   { output_params_.ref_early_exit( ) = true; }

  // -------------------------------------------------------------------------------------------
  // Solve calculations
  public:
    void        calc_next
                 (  input_params_type const &  input_params
                  , sheet_params_type const &  sheet_params
                 )                                      ;
    void        calc_next_pass
                 (  technique_type      technique
                  , method_type         method
                  , bool                is_parallel_method
                  , rate_type           damping
                  , rate_type           rate_x
                  , rate_type           rate_y
                  , sheet_type const &  src_sheet
                  , sheet_type       &  trg_sheet
                 )                                      ;

  protected:
    solve_1d_functor_type const &
                get_1d_functor
                 (  method_type  method
                  , bool         is_parallel_method
                 )                                const ;

    void        ensure_buffer_size
                 (  solve_1d_functor_type
                                     const &  calc_1d_functor
                  , size_type                 x_size
                  , size_type                 y_size
                 )                                      ;
    void        clear_buffers( )                        ;

  protected:
    void        calc_next_ortho_interleave
                 (  method_type         method
                  , bool                is_parallel_method
                  , rate_type           x_rate
                  , rate_type           y_rate
                  , sheet_type const &  src_sheet
                  , sheet_type       &  trg_sheet
                 )                                      ;
    void        calc_next_simultaneous_2d
                 (  method_type         method
                  , bool                is_parallel_method
                  , rate_type           x_rate
                  , rate_type           y_rate
                  , sheet_type const &  src_sheet
                  , sheet_type       &  trg_sheet
                 )                                      ;
    void        calc_next_wave_with_damping
                 (  method_type         method
                  , bool                is_parallel_method
                  , rate_type           damping
                  , rate_type           x_rate
                  , rate_type           y_rate
                  , sheet_type const &  src_sheet
                  , sheet_type       &  trg_sheet
                 )                                      ;

  protected:
    void        fix_out_of_bounds_if_necessary
                 (  technique_type  technique
                  , method_type     method
                  , rate_type       damping
                  , rate_type       rate_x
                  , rate_type       rate_y
                  , sheet_type &    trg_sheet
                 )                                      ;
    void        fix_severely_out_of_bounds_sheet
                 (  sheet_type &  trg_sheet
                 )                                      ;

  // -------------------------------------------------------------------------------------------
  // Member vars
  private:
    output_params_type  output_params_                  ;

    buf_type            buf_a_                          ;
    buf_type            buf_b_                          ;
    buf_iter_type       buf_iter_a_                     ;
    buf_iter_type       buf_iter_b_                     ;

    calc_next_1d_forward_diff_serial_functor_type
     <  rate_type
      , src_iter_type
      , trg_iter_type
     >                  forward_diff_serial_functor_    ;

    calc_next_1d_forward_diff_parallel_functor_type
     <  rate_type
      , src_iter_type
      , trg_iter_type
     >                  forward_diff_parallel_functor_  ;

    calc_next_1d_backward_diff_serial_functor_type
     <  rate_type
      , src_iter_type
      , trg_iter_type
      , buf_iter_type
     >                  backward_diff_serial_functor_   ;

    calc_next_1d_backward_diff_parallel_functor_type
     <  rate_type
      , src_iter_type
      , trg_iter_type
      , buf_iter_type
     >                  backward_diff_parallel_functor_ ;

    calc_next_1d_central_diff_serial_functor_type
     <  rate_type
      , src_iter_type
      , trg_iter_type
      , buf_iter_type
     >                  central_diff_serial_functor_    ;

    calc_next_1d_central_diff_parallel_functor_type
     <  rate_type
      , src_iter_type
      , trg_iter_type
      , buf_iter_type
     >                  central_diff_parallel_functor_  ;

}; /* end class solver_type */

// _______________________________________________________________________________________________
// control_type

  class
control_type
  : public QObject
  //
  // QObject is a Q_DISABLE_COPY(..) class.
{
  Q_OBJECT

  // -------------------------------------------------------------------------------------------
  // Construct/destruct
  public:
    explicit    control_type( QObject * p_parent)          ;
    /* dtor */  ~control_type( )                           ;

  // -------------------------------------------------------------------------------------------
  // Solve calculation
  public:
    // This is the method that is called from sheet_control_type.
    void        calc_next
                 (  sheet_type const &  src_sheet
                  , sheet_type       &  trg_sheet
                  , sheet_type       &  extra_sheet
                  , bool                are_extra_passes_disabled
                 )                                         ;

  // -------------------------------------------------------------------------------------------
  // Status and statistics
  public:
    // -1 if not recorded
    double      get_last_duration__seconds( )        const { return last_duration_; }

    output_params_type const *
                get_output_params( )                 const ; /* can return zero */

    // Mostly for debug
    bool        is_busy( )                           const { return is_busy_; }
    bool        not_busy( )                          const { return ! is_busy( ); }

    bool        is_going_down( )                     const ; /* exiting or is_early_exit */

  // -------------------------------------------------------------------------------------------
  // Param getters
  public:
    technique_type
                get_technique( )                     const { return input_params_.get_technique( ); }
    bool        is_technique__ortho_interleave(  )   const { return get_technique( ) == e_ortho_interleave ; }
    bool        is_technique__simultaneous_2d(   )   const { return get_technique( ) == e_simultaneous_2d  ; }
    bool        is_technique__wave_with_damping( )   const { return get_technique( ) == e_wave_with_damping; }

    method_type get_method( )                        const { return input_params_.get_method( ); }
    bool        is_method__forward_diff(  )          const { return get_method( ) == e_forward_diff ; }
    bool        is_method__backward_diff( )          const { return get_method( ) == e_backward_diff; }
    bool        is_method__central_diff(  )          const { return get_method( ) == e_central_diff ; }

    bool        is_method_parallel( )                const { return input_params_.is_method_parallel( ); }

    rate_type   get_damping( )                       const { return input_params_.get_damping( ); }
    rate_type   get_rate_x( )                        const { return input_params_.get_rate_x( ); }
    rate_type   get_rate_y( )                        const { return input_params_.get_rate_y( ); }

    size_type   get_extra_pass_count( )              const { return input_params_.get_extra_pass_count( ); }
    size_type   get_pass_count( )                    const { return input_params_.get_pass_count( ); }
    bool        has_extra_passes( )                  const { return input_params_.has_extra_passes( ); }
    bool        are_extra_passes_disabled( )         const { return input_params_.are_extra_passes_disabled( ); }

  // -------------------------------------------------------------------------------------------
  // Param setters
  public:
    void        set_rates( rate_type rx, rate_type ry)     ;
    void        set_technique( technique_type te)          ;
    void        set_method( method_type m)                 ;
  signals:
    void        technique_is_changed( )                    ; /* signal */
    void        method_is_changed( )                       ; /* signal */

  public:
    void        set_technique__ortho_interleave( )         { set_technique( e_ortho_interleave ); }
    void        set_technique__simultaneous_2d( )          { set_technique( e_simultaneous_2d  ); }
    void        set_technique__wave_with_damping( )        { set_technique( e_wave_with_damping); }

    void        set_method__forward_diff( )                { set_method( e_forward_diff ); }
    void        set_method__backward_diff( )               { set_method( e_backward_diff); }
    void        set_method__central_diff( )                { set_method( e_central_diff ); }
  public slots:
    void        set_technique__ortho_interleave(  bool y)  { if ( y ) { set_technique__ortho_interleave(  ); } }
    void        set_technique__simultaneous_2d(   bool y)  { if ( y ) { set_technique__simultaneous_2d(   ); } }
    void        set_technique__wave_with_damping( bool y)  { if ( y ) { set_technique__wave_with_damping( ); } }

    void        set_method__forward_diff(  bool is_chk)    { if ( is_chk ) { set_method__forward_diff(  ); } }
    void        set_method__backward_diff( bool is_chk)    { if ( is_chk ) { set_method__backward_diff( ); } }
    void        set_method__central_diff(  bool is_chk)    { if ( is_chk ) { set_method__central_diff(  ); } }
  public slots:
    void        set__is_method_parallel( bool is)          ;
    void        set_damping( double d)                     ;
    void        set_pass_count( int c)                     ;

  // -------------------------------------------------------------------------------------------
  // Slot
  private slots:
    void        finished__from_worker_thread( double)      ; /* cross thread, from worker to control */

  // -------------------------------------------------------------------------------------------
  // Signal
  signals:
    void        finished( )                                ; /* the outside world listens for this signal */

  // -------------------------------------------------------------------------------------------
  // Private member vars
  private:
    settable_input_params_type  input_params_  ;

    bool                        is_busy_       ;
    bool                        is_exiting_    ;

    double                      last_duration_ ;
    worker_thread_type *        p_worker_      ;

} /* end class control_type */ ;

// _______________________________________________________________________________________________
// worker_thread_type
//
//   This is a private class. It would not have to be exposed in a header file except QT moc
//   requires it for QObject (signal/slot) subclasses.

  class
worker_thread_type
  : public QThread
{
  Q_OBJECT

  // This is a private class, used only by control_type.
  friend class control_type;

  // -------------------------------------------------------------------------------------------
  // Ctor, only used by friend class
  protected:
    /* ctor */  worker_thread_type( QObject * p_parent) ;

  // Dtor, always from the supertype
  private:
    virtual     ~worker_thread_type( )                  ;

  // -------------------------------------------------------------------------------------------
  // Overridden from QThread
  protected:
    virtual
    void        run( )                                  ;

  // -------------------------------------------------------------------------------------------
  // Kick function, only used by control_type class.
  protected:
    void        start_run__from_master_thread
                 (  input_params_type const &  input_params
                  , sheet_type        const &  src_sheet
                  , sheet_type              &  trg_sheet
                  , sheet_type              &  extra_sheet
                 )                                      ;

  // -------------------------------------------------------------------------------------------
  protected:
    output_params_type const &
                get_output_params( )              const { return solver_.get_output_params( ); }

    void        request_early_exit( )                   { solver_.request_early_exit( ); }

  // -------------------------------------------------------------------------------------------
  // Slot
  protected slots:
    void        run__in_worker_thread( )                ; /* cross-thread, from control to worker */

  // -------------------------------------------------------------------------------------------
  // Signals
  signals:
    void        start__master_to_worker( )              ; /* cross-thread, from control to worker */
    void        finished__worker_to_master( double)     ; /* cross-thread, from worker to control */

  // -------------------------------------------------------------------------------------------
  // Private member vars
  private:
    QSemaphore          init_wait_     ;
    solver_type         solver_        ;
    input_params_type   input_params_  ;

    sheet_type const *  p_src_sheet_   ;
    sheet_type       *  p_trg_sheet_   ;
    sheet_type       *  p_extra_sheet_ ;

} /* end class worker_thread_type */ ;

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
} /* end namespace heat_solver */

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef HEAT_SOLVER_H
//
// heat_solver.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
