// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// heat_solver.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// solver_type is an algorithm class. Maybe it should be a functor [defining operator()].
// control_type could be more generic, a wrapper around any functor class.
//
// solver_type stores the (heat diffusion) rate values.
// Really we need a more complete model:
//   Heat capacity for each cell
//   Values for rate for each edge (including outer edges)
//
// Can the thread class use QThread::idealThreadCount( )?
//
// Improvement:
//   Scale class (uniform_scalar). Can replace the floats or doubles in the simulation.
//     This is an integer, maybe a 32-bit unsigned, that represents the interval [-1..+1].
//     But it cannot represent outside that range so you have to use it carefully, more carefully
//     than we do here. Floats and doubles are sloppier, but more forgiving when you're just
//     experimenting.
//
//   float is great if you're going to swing wildly to very large and very small numbers.
//   But float is not uniform. It is very dense around zero and very sparse at the outer edges.
//   If we limit some float values to [0..1] we are throwing away 2 bits (exp sign and matissa sign).
//   And accuracy at near 1 is a lot less than accuracy near zero.
//
//   So a scale class is sometimes preferable to float because it:
//     Lets you use all the bits.
//     Distributes the numbers uniformly across the entire range.
//
//   This is not a limited (ranged) integer. That's another class.
//
//      template
//       <  typename INT_INNER_T   /* int */
//        , typename REAL_LIMIT_T  /* float or double */
//        , LIMIT_T  LO            /* -1.0f */
//        , LIMIT_T  HI            /* +1.0f */
//       >
//      class
//    uniform_scalar
//      //
//      // Convert to/from real (float, double, rational) values.
//      // Provide operators + - *
//      // Provide division with various rounding options
//      // Provide +-* functions that tell you about overflow
//      // Provide +-* functions that return corrections that you can pass to the next operation
//      //
//      // Inner type is an integer type.
//      //   Could we make this a vector (x, y, z)? We could overload the template params to watch
//      //   for boost::integer_traits< int_inner_type >::is_integral being true.
//      //
//      // Limit type is probably float, double, or rational.
//      //   Although maybe limit_type could be another uniform type.
//    {
//      public:
//        typedef INT_INNER_T           int_inner_type ;
//        static int_inner_type const   int_inner_min  = boost::integer_traits< int_inner_type >::const_min;
//        static int_inner_type const   int_inner_max  = boost::integer_traits< int_inner_type >::const_max;
//      private:
//        int_inner_type  int_inner_value_ ;
//    };
//
//   Perhaps it's better to provide fixed intervals:
//     0<=x<1   - easiest
//     0<=x<=1  - representing exactly 1 complicates things
//     0<=x<2   - easy
//     0<=x<=2
//     -1<x<1   - easy if you don't mind a signed zero
//     -1<=x<1  - easy
//     -1<=x<=1
//   Perhaps also a 1/x bit for the 0..1 and -1..1 intervals? Like float this throws away a bit,
//   although the 0<=x<1 interval is still uniform.
// _______________________________________________________________________________________________

# include "all.h"
# include "heat_solver.h"
# include "util.h"

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
namespace heat_solver {

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// input_params_type

  /* ctor */
  input_params_type::
input_params_type( )
  : technique_                 ( e_simultaneous_2d )
  , method_                    ( e_forward_diff    )
  , is_method_parallel_        ( true              )
  , damping_                   ( 0                 )
  , rate_x_                    ( 0.2               )
  , rate_y_                    ( 0.2               )
  , extra_pass_count_          ( 0                 )
  , are_extra_passes_disabled_ ( false             )
  , reset_if_not_used_         ( false             )
  , copy_for_history_          ( false             )
  , size_for_history_          ( false             )
{ }

  /* ctor */
  input_params_type::
input_params_type
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
 )
  : technique_                 ( technique          )
  , method_                    ( method             )
  , is_method_parallel_        ( is_method_parallel )
  , damping_                   ( damping            )
  , rate_x_                    ( rate_x             )
  , rate_y_                    ( rate_y             )
  , extra_pass_count_          ( extra_pass_count   )
  , are_extra_passes_disabled_ ( false              )
  , reset_if_not_used_         ( reset_if_not_used  )
  , copy_for_history_          ( copy_for_history   )
  , size_for_history_          ( size_for_history   )
{ }

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// settable_input_params_type

  bool
  settable_input_params_type::
set_technique( technique_type tech)
{
    if ( tech == technique_ ) return false;

    if ( e_ortho_interleave  == tech ) {
        technique_         = e_ortho_interleave;
        copy_for_history_  = true;
        size_for_history_  = false;
        reset_if_not_used_ = true;
    } else
    if ( e_simultaneous_2d   == tech ) {
        technique_         = e_simultaneous_2d;
        copy_for_history_  = true;
        size_for_history_  = false;
        reset_if_not_used_ = false;
    } else
    if ( e_wave_with_damping == tech ) {
        technique_         = e_wave_with_damping;
        copy_for_history_  = true;
        size_for_history_  = true;
        reset_if_not_used_ = false;
    } else {
        d_assert( false);
    }
    return true;
}

  bool
  settable_input_params_type::
set_method( method_type new_method)
{
    return util::maybe_assign( method_, new_method);
}

  bool
  settable_input_params_type::
set__is_method_parallel( bool new_value /* = true */)
{
    d_assert( (true == new_value) || (false == new_value));
    return util::maybe_assign( is_method_parallel_, new_value);
}

  bool
  settable_input_params_type::
set_damping( rate_type new_damping)
{
    return util::maybe_assign( damping_, new_damping);
}

  bool
  settable_input_params_type::
set_rate_x( rate_type new_rate_x)
{
    return util::maybe_assign( rate_x_, new_rate_x);
}

  bool
  settable_input_params_type::
set_rate_y( rate_type new_rate_y)
{
    return util::maybe_assign( rate_y_, new_rate_y);
}

  bool
  settable_input_params_type::
set_extra_pass_count( size_type new_extra_pass_count)
{
    return util::maybe_assign( extra_pass_count_, new_extra_pass_count);
}

  bool
  settable_input_params_type::
set__are_extra_passes_disabled( bool new_value)
{
    d_assert( (true == new_value) || (false == new_value));
    return util::maybe_assign( are_extra_passes_disabled_, new_value);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// solver_type

  /* constructor */
  solver_type::
solver_type( )

  // Early exit is used when we shut down the program, to finish/abort an off-thread solve quickly.
  : output_params_                  ( )

  // Buffers used by backward and central diff. Buffers are bigger when solving parallel.
  , buf_a_                          ( )
  , buf_b_                          ( )
  , buf_iter_a_                     ( )
  , buf_iter_b_                     ( )

  // Solving functors. We don't have a functor for 2d forward-diff solves -- we just call a function.
  , forward_diff_serial_functor_    ( output_params_.ref_early_exit( ))
  , forward_diff_parallel_functor_  ( output_params_.ref_early_exit( ))
  , backward_diff_serial_functor_   ( output_params_.ref_early_exit( ), buf_iter_a_, buf_iter_b_)
  , backward_diff_parallel_functor_ ( output_params_.ref_early_exit( ), buf_iter_a_, buf_iter_b_)
  , central_diff_serial_functor_    ( output_params_.ref_early_exit( ), buf_iter_a_, buf_iter_b_)
  , central_diff_parallel_functor_  ( output_params_.ref_early_exit( ), buf_iter_a_, buf_iter_b_)
{
}

// _______________________________________________________________________________________________

  /* main method */
  void
  solver_type::
calc_next
 (  input_params_type const &  input_params
  , sheet_params_type const &  sheet_params
 )
  // Instead of dealing with sheet_type this could start with these src/trg objects:
  //   stride_range< src_iter_type, 1 >  src_range
  //   stride_range< trg_iter_type, 1 >  trg_range
  // See swap_xy( range_xy) -> range_yx
{
    // Initialize the output params. We will set them as we go along.
    output_params_.reset( );

    sheet_type const &  src_sheet    = sheet_params.ref_src_sheet( );
    sheet_type       &  trg_sheet    = sheet_params.ref_trg_sheet( );
    sheet_type       &  extra_sheet  = sheet_params.ref_extra_sheet( );

    // Interpret input_params and sheet_params as intuitive choices.
    bool const  is_multi_pass               = input_params.has_extra_passes( ) && ! input_params.are_extra_passes_disabled( );
    bool const  is_one_pass                 = ! is_multi_pass;
    bool const  is_in_place_requested       = sheet_params.are_src_trg_sheets_same( );
    bool const  is_in_place_possible        = input_params.is_technique__ortho_interleave( );
    bool const  is_extra_pre_sized          = sheet_params.is_extra_sheet_sized( );
    bool const  is_history_vital            = input_params.is_saving_history_worth_sizing_extra( );
    bool const  is_history_nice             = is_history_vital || input_params.is_saving_history_worth_extra_copy( );
    bool const  is_extra_vital_for_solve    = is_multi_pass && ! is_in_place_possible; /* or is_in_place_requested and not possible */
    bool const  is_extra_vital_for_history  = is_multi_pass || is_in_place_requested;
    bool const  is_extra_vital              = is_extra_vital_for_solve || (is_history_vital && is_extra_vital_for_history);
    bool const  is_extra_nice               = is_extra_vital || (is_history_nice && is_extra_vital_for_history);
    bool const  is_extra_used               = is_extra_vital || (is_extra_nice && is_extra_pre_sized);

    // The two sheets must be different (unless technique == e_ortho_interleave).
    // We could make in_place work for e_simultaneous_2d too (but not for e_wave_with_damping)
    // by using extra_sheet like this (one pass):
    //   copy src -> extra
    //   solve extra -> trg
    // For two passes:
    //   solve src -> extra
    //   solve extra -> trg
    // Three passes:
    //   copy src -> extra
    //   solve extra -> trg
    //   solve trg -> extra
    //   solve extra -> trg
    // In this scenario, extra_sheet always ends up with history.
    d_assert( implies( is_in_place_requested, is_in_place_possible));

    if ( is_extra_vital && ! is_extra_pre_sized ) {
        // We need an extra trg sheet. Make sure it's properly sized.
        d_verify( sheet_params.maybe_size_extra_sheet( ));
        output_params_.set__was_extra_sized( );
    }

    if ( is_extra_used ) {
        // Even if extra_sheet wasn't pre-sized, it's sized now.
        d_assert( sheet_params.is_extra_sheet_sized( ));
        output_params_.set__was_extra_used( );
    } else
    if ( input_params.is_extra_sheet_to_be_reset_if_not_used( ) ) {
        extra_sheet.reset( );
    }

    // If we are solving the wave equation using the extra sheet we have to set up history.
    //
    // We need the following assumtion because we are testing is_odd( extra_pass_count):
    //   d_assert( extra_pass_count >= 0)
    // This is always true because size_type is unsigned.
    d_static_assert( boost::is_unsigned< size_type >::value);
    if ( is_multi_pass && input_params.is_technique__wave_with_damping( ) ) {
        d_assert( is_extra_used);
        if ( is_early_exit( ) ) return;

        // If we are wave solving we need to setup the extra sheet with values so we get
        // the history right. Remember with waves we have a kludge where trg_sheet also
        // holds the src-values from one generation back.
        if ( util::is_odd( input_params.get_extra_pass_count( )) ) {
            // If extra_pass_count is odd the solves will look like:
            //   src -> extra  -- first set (extra <- trg) so history is right
            //   extra -> trg  -- first set (trg <- src) so history is right
            //   trg -> extra  -- ok
            //   extra -> trg  -- ok
            extra_sheet = trg_sheet;
            trg_sheet   = src_sheet;
        } else {
            // If extra_pass_count is even (and not zero) the solves look like this:
            //   src -> trg           -- ok
            //   trg -> extra  -- first set (extra <- src) so history is right
            //   extra -> trg  -- ok
            //   trg -> extra  -- ok
            //   extra -> trg  -- ok
            extra_sheet = src_sheet;
        }
    }

    // Solve repeatedly if extra solve passes are requested.
    // Alternatively solve to trg_sheet and extra_sheet, assuming extra_sheet is available.
    sheet_type const * p_src_sheet = & src_sheet;
    for ( size_type
            countdown = is_multi_pass ? input_params.get_extra_pass_count( ) : 0
          ; countdown > 0
          ; -- countdown )
    {
        if ( is_early_exit( ) ) return;
        output_params_.inc_solve_count( );

        bool const    is_this_pass_targeting_extra  = is_extra_used && util::is_odd( countdown);
        sheet_type &  trg_sheet_this_pass           = is_this_pass_targeting_extra ? extra_sheet : trg_sheet;
        calc_next_pass
         (  input_params.get_technique( )
          , input_params.get_method( )
          , input_params.is_method_parallel( )
          , input_params.get_damping( )
          , input_params.get_rate_x( )
          , input_params.get_rate_y( )
          , *p_src_sheet
          , trg_sheet_this_pass
         );
        p_src_sheet = & trg_sheet_this_pass;
    }

    // We're about to do the last solve. The 2nd-to-last solution is in *p_src_sheet, which is:
    //   src_sheet if this is not just the last solve, but also the only solve.
    //   extra_sheet if we were using extra_sheet in the loop above.
    //   trg_sheet if in_place solving is possible and history is not required.
    if ( is_one_pass ) {
        d_assert( (& src_sheet) == p_src_sheet);
        if ( is_in_place_requested ) {
            // There is one unusual case where we have to copy src_sheet to preserve history.
            if ( is_history_nice && is_extra_used ) {
                if ( is_early_exit( ) ) return;
                extra_sheet = src_sheet;
                output_params_.set__is_last_solve_saved_in_extra( );
            } else {
                /* we are doing one in-place solve */
                /* history is not saved */
            }
        } else {
            output_params_.set__is_last_solve_saved_in_src( );
        }
    } else
    if ( is_extra_used ) {
        // Multi-pass and we had the extra sheet available. In that case we always solve into it.
        // In this case history is always returned in extra_sheet, even if not requested.
        d_assert( (& extra_sheet) == p_src_sheet);
        output_params_.set__is_last_solve_saved_in_extra( );
    } else {
        // Multi-pass and we don't have extra_sheet. We must be solving in-place.
        d_assert( (& trg_sheet) == p_src_sheet);
        d_assert( is_in_place_possible);
        d_assert( ! is_history_vital);
        d_assert( ! (is_history_nice && is_extra_pre_sized));
        /* history is not saved */
    }

    // Do the last-pass solve.
    // For the last pass we always solve into the trg_sheet.
    if ( is_early_exit( ) ) return;
    output_params_.inc_solve_count( );
    calc_next_pass
     (  input_params.get_technique( )
      , input_params.get_method( )
      , input_params.is_method_parallel( )
      , input_params.get_damping( )
      , input_params.get_rate_x( )
      , input_params.get_rate_y( )
      , *p_src_sheet
      , trg_sheet
     );
}

// _______________________________________________________________________________________________

  void
  solver_type::
calc_next_pass
 (  technique_type      technique
  , method_type         method
  , bool                is_parallel_method
  , rate_type           damping
  , rate_type           rate_x
  , rate_type           rate_y
  , sheet_type const &  src_sheet
  , sheet_type       &  trg_sheet
 )
  // Instead of dealing with sheet_type this could start with these src/trg objects:
  //   stride_range< src_iter_type, 1 >  src_range
  //   stride_range< trg_iter_type, 1 >  trg_range
  // See swap_xy( range_xy) -> range_yx
{
    d_assert( 0 != & src_sheet);
    d_assert( 0 != & trg_sheet);
    d_assert( src_sheet.get_x_count( ) == trg_sheet.get_x_count( ));
    d_assert( src_sheet.get_y_count( ) == trg_sheet.get_y_count( ));

    // The two sheets must be different (unless technique == e_ortho_interleave).
    d_assert( implies( (technique != e_ortho_interleave), ((& src_sheet) != (& trg_sheet)) ));

    if ( not_early_exit( ) ) {
        if ( technique == e_ortho_interleave ) {
            // This works even if src_sheet and trg_sheet are the same.
            calc_next_ortho_interleave
             (  method, is_parallel_method
              , rate_x, rate_y
              , src_sheet, trg_sheet
             );
        } else
        if ( technique == e_simultaneous_2d ) {
            calc_next_simultaneous_2d
             (  method, is_parallel_method
              , rate_x, rate_y
              , src_sheet, trg_sheet
             );
        } else
        /* technique == e_wave_with_damping */ {
            d_assert( technique == e_wave_with_damping);
            calc_next_wave_with_damping
             (  method, is_parallel_method
              , damping, rate_x, rate_y
              , src_sheet, trg_sheet
             );
        }
    }

    if ( not_early_exit( ) ) {
        fix_out_of_bounds_if_necessary( technique, method, damping, rate_x, rate_y, trg_sheet);
    }
}

// _______________________________________________________________________________________________

  solve_1d_functor_type const &
  solver_type::
get_1d_functor( method_type method, bool is_parallel_method) const
{
    if ( is_parallel_method ) {
        switch ( method ) {
          case e_central_diff  : return central_diff_parallel_functor_;
          case e_backward_diff : return backward_diff_parallel_functor_;
          case e_forward_diff  : return forward_diff_parallel_functor_;
        }
    } else {
        switch ( method ) {
          case e_central_diff  : return central_diff_serial_functor_;
          case e_backward_diff : return backward_diff_serial_functor_;
          case e_forward_diff  : return forward_diff_serial_functor_;
        }
    }
    d_assert( false);
    return forward_diff_serial_functor_;
}

  void
  solver_type::
ensure_buffer_size
 (  solve_1d_functor_type
                     const &  calc_1d_functor
  , size_type                 x_size
  , size_type                 y_size
 )
  // The 1d functors know how big a buffer they need.
{
    // Make sure the buffers are big enough to handle the solve.
    // And shrink them or release them if they are too big.
    if ( not_early_exit( ) ) {
        size_type const now_buf_size = buf_a_.size( );
        size_type const min_buf_size = calc_1d_functor.get_min_buf_count( x_size, y_size);
        if ( (min_buf_size > now_buf_size) ||
             (min_buf_size < (now_buf_size / 2)) )
        {
            if ( min_buf_size == 0 ) {
                clear_buffers( );
            } else {
                buf_a_.resize( min_buf_size); buf_iter_a_ = buf_a_.begin( );
                buf_b_.resize( min_buf_size); buf_iter_b_ = buf_b_.begin( );
            }
        }
        d_assert( buf_a_.size( ) >= min_buf_size);
        d_assert( buf_b_.size( ) >= min_buf_size);
    }
}

  void
  solver_type::
clear_buffers( )
  //
  // Frees all the memory allocated for the buffers.
{
    if ( not_early_exit( ) && (buf_a_.size( ) > 0) ) {
        // Kludge to actually free memory when you are using the microsoft std library.
        util::apply_dtor( & buf_a_);
        util::apply_dtor( & buf_b_);
        util::apply_default_ctor( & buf_b_);
        util::apply_default_ctor( & buf_a_);
    }
}

// _______________________________________________________________________________________________

  void
  solver_type::
calc_next_ortho_interleave
 (  method_type         method
  , bool                is_parallel_method
  , rate_type           x_rate
  , rate_type           y_rate
  , sheet_type const &  src_sheet
  , sheet_type       &  trg_sheet
 )
  //
  // Solve the 2D sheet as two perpendicular 1D solves.
  //
  // This is used with forward, backward, and central diff, both serial and parellel.
  // We only use this to solve the heat equation, not the wave equation.
{
    // Get the appropriate 1d functor.
    solve_1d_functor_type const & calc_1d_functor = get_1d_functor( method, is_parallel_method);
    ensure_buffer_size( calc_1d_functor, src_sheet.get_x_count( ), src_sheet.get_y_count( ));

    // Damping is never used because this only solves heat, not wave.
    // We pass in a special damping value to ensure damping is ignored, and we always use
    // set (=) (as opposed to sum or +=) when setting values in the trg.
    rate_type const damping = finite_difference::get_no_init_damping_set_value< rate_type >( );

    if ( x_rate ) {
        calc_1d_functor( damping, x_rate, src_sheet.get_range_yx( ), trg_sheet.get_range_yx( ));
        if ( not_early_exit( ) && y_rate ) {
            // The 2nd calc above must be trg->trg because trg holds the results of the first calculation.
            calc_1d_functor( damping, y_rate, trg_sheet.get_range_xy( ), trg_sheet.get_range_xy( ));
        }
    } else
    if ( y_rate ) {
        calc_1d_functor( damping, y_rate, src_sheet.get_range_xy( ), trg_sheet.get_range_xy( ));
    } else
    if ( (& src_sheet) != (& trg_sheet) ) {
        // Both x- and y-rate are zero.
        // Simple copy from src to trg, unless they are the same sheet.
        trg_sheet = src_sheet;
    }
}

  void
  solver_type::
calc_next_simultaneous_2d
 (  method_type         method
  , bool                is_parallel_method
  , rate_type           x_rate
  , rate_type           y_rate
  , sheet_type const &  src_sheet
  , sheet_type       &  trg_sheet
 )
{
    // This is the same as solving the wave with full damping.
    rate_type const full_damping = 1;
    calc_next_wave_with_damping
     (  method, is_parallel_method
      , full_damping, x_rate, y_rate
      , src_sheet, trg_sheet
     );
}

  void
  solver_type::
calc_next_wave_with_damping
 (  method_type         method
  , bool                is_parallel_method
  , rate_type           damping
  , rate_type           x_rate
  , rate_type           y_rate
  , sheet_type const &  src_sheet
  , sheet_type       &  trg_sheet
 )
{
    // Damping cannot be one of the special values. Those are only used to make the
    // mixing algorithms work.
    d_assert( damping != finite_difference::get_no_init_damping_set_value< rate_type >( ));
    d_assert( damping != finite_difference::get_no_init_damping_sum_value< rate_type >( ));

    // We solve forward-diff with 2-d functions instead the 1-d functors for all the others.
    if ( method == e_forward_diff ) {
        clear_buffers( ); // forward-diff doesn't need buffers -- free their alloc'd memory

        if ( is_parallel_method ) {
            // We don't use a functor for 2d forward diff. Instead we have two functions:
            // one for serial and one for parallel.
            // We have to pass in all the state since we don't have a functor to wrap it up for us.
            calc_next_2d_forward_diff_parallel
             (  output_params_.ref_early_exit( )
              , damping, x_rate, y_rate
              , src_sheet.get_range_yx( )
              , trg_sheet.get_range_yx( )
             );
        } else {
            // The serial (not parallel) 2d forward-diff function.
            calc_next_2d_forward_diff_serial
             (  output_params_.ref_early_exit( )
              , damping, x_rate, y_rate
              , src_sheet.get_range_yx( )
              , trg_sheet.get_range_yx( )
             );
        }
    } else
    /* central or backward diff */ {
        // We use the 1d functors. The damping params tell them how we're using them.
        d_assert( (method == e_backward_diff) || (method == e_central_diff));

        // Get the appropriate 1d functor.
        solve_1d_functor_type const & calc_1d_functor = get_1d_functor( method, is_parallel_method);
        ensure_buffer_size( calc_1d_functor, src_sheet.get_x_count( ), src_sheet.get_y_count( ));

        // This is a 2-pass algorithm that uses different damping values for each pass.
        rate_type const damping_1st_pass = damping;
        rate_type const damping_2nd_pass = finite_difference::get_no_init_damping_sum_value< rate_type >( );

        // Two pass algorithms:
        //   The 1st pass calculates values from src_sheet and writes them to trg_sheet.
        //   The 2nd pass calculates other values from the original src_sheet and sums them into trg_sheet.

        // Improve? Since the first pass does slightly more work, perhaps we should do the narrow
        // dimension first? Although it probably doesn't matter at all.

        // We could just copy if (damping==1) and (x_rate==0) and (y_rate==0).
        calc_1d_functor( damping_1st_pass, x_rate, src_sheet.get_range_yx( ), trg_sheet.get_range_yx( ));
        if ( not_early_exit( ) ) {
            // 2nd-pass damping says use +=.
            calc_1d_functor( damping_2nd_pass, y_rate, src_sheet.get_range_xy( ), trg_sheet.get_range_xy( ));
        }
    }
}

// _______________________________________________________________________________________________

  void
  solver_type::
fix_out_of_bounds_if_necessary
 (  technique_type  technique
  , method_type     method
  , rate_type       damping
  , rate_type       rate_x
  , rate_type       rate_y
  , sheet_type &    trg_sheet
 )
  // This tries to keep the sheet from exploding when we're experimenting with unstable
  // solvers. Some of the solvers are unstable when you give them rates outside of a specific
  // narrow range.
  //
  // This is not perfect, particularly with the wave equation which can probably produce very
  // large sheet values even under somewhat normal conditions.
{
    bool needs_correction = false;

    // Look for the obvious cases where the values in the sheet might grow very large.
    // There are probably some cases I missed. Maybe limits should be built into the solve
    // functions themselves and always applied. That way they could be run in parallel.
    if ( (rate_x < 0) || (rate_y < 0) ) {
        needs_correction = true;
    } else
    if ( (technique == e_wave_with_damping) &&
         ( (damping < 0) /* illegal damping (accelerating) */ ||
           (damping > 1) /* illegal damping (negative momentum?) */ ||
           (damping < 0.3) /* kludge rule because we can get the sheet to blow up */
           //((damping <= 0.01) && (rate_x <= 0.05) && (rate_y <= 0.05))
       ) )
    {
        // The above test is mostly right, but I can construct scenarios where even normal
        // damping can cause blow ups. And the open-gl drawing stuff seems sensitive to
        // abnormal values, at least some with drivers.
        // Blow ups happen when we set rates/damping very low when we have momentum, and after
        // recovering from other blow-up situations.
        // Later: 
        needs_correction = true;
    } else
    if ( (method != e_backward_diff) &&
         ( (rate_x >= 0.5) || (rate_y >= 0.5) ||
           ( (technique != e_ortho_interleave) && ((rate_x + rate_y) >= 0.5)
       ) ) )
    {
        // The central-diff solver doesn't blow up as readily as the forward-diff.
        // I've never seen it blow up with the ortho-interleave technique, but I have with
        // the simultaneous technique.
        needs_correction = true;
    }

    // Clamp the values in the sheet to some large +/- value.
    if ( needs_correction ) {
        fix_severely_out_of_bounds_sheet( trg_sheet);
    }
}

  void
  solver_type::
fix_severely_out_of_bounds_sheet( sheet_type & trg_sheet)
  //
  // This is like a sheet-transforming functor, and should probably be integrated into
  // the solvers so it can calculate off-thread (and maybe even in parallel).
{
    // We need a way to do this in a worker thread (and ideally parallel) without
    // incrementing the generation count. So this either needs to be integrated into
    // the solvers, or we need to have a general way to queue worker-thread instructions.

    value_type const normal_min = -1;
    value_type const normal_max = +1;
    value_type const normal_mid =  0;

    // We don't flatten the sheet unless it gets out of this range (except in the
    // special case where the sheet is completely flat):
    value_type const out_of_bounds_min = -100;
    value_type const out_of_bounds_max = +100;

    // After we're done none of the sheet values should be outside this range:
    value_type const back_in_bounds_min = -50;
    value_type const back_in_bounds_max = +50;

    value_type min_value = 0;
    value_type max_value = 0;
    d_verify( trg_sheet.get_min_max_values( min_value, max_value));
    d_assert( min_value <= max_value);

    // If the sheet is flat ..
    if ( min_value == max_value ) {
        // If the flat value is outside the normal range ..
        if ( (min_value < normal_min) || (max_value > normal_max) ) {
            trg_sheet.fill_sheet( normal_mid);
        }
    } else
    if ( min_value < out_of_bounds_min ) {
        trg_sheet.normalize(
            back_in_bounds_min,
            std::max( normal_mid, std::min( back_in_bounds_max, max_value)));
    } else
    if ( max_value > out_of_bounds_max ) {
        trg_sheet.normalize(
            std::min( normal_mid, std::max( back_in_bounds_min, min_value)),
            back_in_bounds_max);
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// worker_thread_type
//
//   Private class.

  /* constructor */
  worker_thread_type::
worker_thread_type( QObject *  p_parent)
  // This manages the worker thread that does all the solving work away from the UI thread.
  // The serial solvers do all their work in this thread.
  // The parallel solvers use this thread to launch many other worker threads, and then gather
  // up the results.
  // When this thread finishes a solve it posts a message to the UI thread and waits for the
  // UI thread to use the results and maybe request further actions.
  //
  // This thread should be generalized to not just handle solves, but also handle almost any
  // of the sheet thransforms and scans.
  //
  // This class is not used outside this file. Its decl is exposed in the header file, but only
  // because Qt requires it.
  //
  : QThread                ( p_parent)
  , init_wait_             ( 0)
  , solver_                ( )
  , input_params_          ( )
  , p_src_sheet_           ( 0)
  , p_trg_sheet_           ( 0)
  , p_extra_sheet_         ( 0)
{
    // The constructor runs in the master thread.
    d_assert( currentThread( ) != this);

    // Although this is the worker thread object, it is now owned by the master thread.
    // The worker thread (this object) isn't even running yet.
    d_assert( thread( ) != this);
    d_assert( ! isRunning( ));

    // Start the worker thread (represented by this object).
    start( );

    // Wait until the worker is done connecting (listening for messages from the master thread)
    // before continuing.
    init_wait_.acquire( 1);
    // If we had a bunch of threads they could all share this semaphore, but it would have to
    // be protected by a mutex or another semaphore in that case.

    // The worker thread is now running, awaiting instructions.
    d_assert( isRunning( ));
}

// _______________________________________________________________________________________________

  /* overridden virtual */
  void
  worker_thread_type::
run( )
{
    // This runs in the worker thread.
    d_assert( currentThread( ) == this);

    // Tell the master thread that we are now listening.
    init_wait_.release( 1);

    // The supertype run( ) starts exec( ).
    QThread::run( );
}

// _______________________________________________________________________________________________

  /* destructor */
  worker_thread_type::
~worker_thread_type( )
{
    // Which thread runs the destructor?
    // d_assert( currentThread( ) != this) ?

    // We should not be in the middle of a run.
    d_assert( 0 == p_src_sheet_  );
    d_assert( 0 == p_trg_sheet_  );
    d_assert( 0 == p_extra_sheet_);
}

// _______________________________________________________________________________________________

  void
  worker_thread_type::
start_run__from_master_thread
 (  input_params_type const &  input_params
  , sheet_type        const &  src_sheet
  , sheet_type              &  trg_sheet
  , sheet_type              &  extra_sheet
 )
  // Start running the worker thread.
  // The worker thread will signal when it is done.
{
    // This is called from the master thread.
    d_assert( currentThread( ) != this);

    // This is only called if we are not running.
    d_assert( 0 == p_src_sheet_  );
    d_assert( 0 == p_trg_sheet_  );
    d_assert( 0 == p_extra_sheet_);

    // Setup the params for the solver.
    input_params_  = input_params  ;
    p_src_sheet_   = & src_sheet   ;
    p_trg_sheet_   = & trg_sheet   ;
    p_extra_sheet_ = & extra_sheet ;

    // This signal should be picked up by the worker thread.
    emit start__master_to_worker( );
}

// _______________________________________________________________________________________________

  // slot
  void
  worker_thread_type::
run__in_worker_thread( )
{
    // This runs in the worker thread.
    d_assert( currentThread( ) == this);

    // Start the timer.
    // We'd use boost::timer here if it offered any advantage.
    tick_point_type const start_tick = date_time::get_tick_now( );

    // Set up the sheet params.
    sheet_params_type sheet_params( *p_src_sheet_, *p_trg_sheet_, *p_extra_sheet_);

    // Run the solver. This might be slow.
    // And if it's parallel it might use a bunch of other worker threads.
    solver_.calc_next( input_params_, sheet_params);

    // We're done with the simulation. Clear the state vars.
    p_extra_sheet_ = 0;
    p_trg_sheet_   = 0;
    p_src_sheet_   = 0;

    // Calculate duration.
    tick_duration_type duration_in_ticks = date_time::get_invalid_tick_dur( );
    if ( date_time::is_valid_tick_pt( start_tick) ) {
        tick_point_type const finish_tick = date_time::get_tick_now( );
        if ( date_time::is_valid_tick_pt( finish_tick) && (finish_tick >= start_tick) ) {
            duration_in_ticks = finish_tick - start_tick;
        }
    }

    // Convert the duration (tick_duration_type) to a double value that we can easily send.
    double const
      duration_in_seconds =
        double(
          date_time::is_invalid_tick_dur( duration_in_ticks) ?
            -1.0 :
            date_time::convert_ticks_to_seconds( duration_in_ticks));

    // This signal is picked up by the master thread.
    emit finished__worker_to_master( duration_in_seconds);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Class control_type

  /* ctor */
  /* explicit */
  control_type::
control_type( QObject * p_parent)
  //
  // This is not a thread object. This manages the solving worker thread. You use instances of this
  // class from the UI thread to control and communicate with the worker thread.
  //
  : QObject              ( p_parent)
  , input_params_        ( )
  , is_busy_             ( false)
  , is_exiting_          ( false)
  , last_duration_       ( 0)
  , p_worker_            ( 0)
{
}

  /* dtor */
  control_type::
~control_type( )
{
    // We only exit once, when the program is done.
    d_assert( ! is_exiting_);
    is_exiting_ = true;

    // Get rid of the worker-thread object. We have to wait for the worker thread to stop running first.
    // The worker thread object has no parent so it is not auto-deleted.
    if ( p_worker_ ) {
        d_assert( p_worker_->isRunning( ));

        // If the solver is running in the worker thread, this will tell it to stop as soon as it can.
        p_worker_->request_early_exit( );

        p_worker_->quit( ); /* queue a quit message */
        p_worker_->wait( ); /* wait for the thread to finish */
        d_assert( p_worker_->isFinished( ));

        delete p_worker_;
        p_worker_ = 0;
    }
}

// _______________________________________________________________________________________________

  bool
  control_type::
is_going_down( ) const
{
    return is_exiting_ ||
        (p_worker_ && p_worker_->get_output_params( ).is_early_exit( ));
}

  output_params_type const *
  control_type::
get_output_params( ) const
{
    d_assert( not_busy( ));
    if ( p_worker_ && ! is_going_down( ) ) {
        return & (p_worker_->get_output_params( ));
    }
    return 0;
}

// _______________________________________________________________________________________________

  void
  control_type::
set_rates( rate_type rx, rate_type ry)
{
    input_params_.set_rate_x( rx);
    input_params_.set_rate_y( ry);
}

  void
  control_type::
set_technique( technique_type new_technique)
{
    if ( input_params_.set_technique( new_technique) ) {
        emit technique_is_changed( );
    }
}

  void
  control_type::
set_method( method_type new_method)
{
    if ( input_params_.set_method( new_method) ) {
        emit method_is_changed( );
    }
}

  /* slot */
  void
  control_type::
set__is_method_parallel( bool new_is)
{
    input_params_.set__is_method_parallel( new_is);
}

  /* slot */
  void
  control_type::
set_damping( double new_damping)
{
    input_params_.set_damping( static_cast< rate_type >( new_damping));
}

  /* slot */
  void
  control_type::
set_pass_count( int new_pass_count)
{
    size_type const
        new_extra_pass_count =
            (new_pass_count <= 1) ? 0 : static_cast< size_type >( new_pass_count - 1);
    input_params_.set_extra_pass_count( new_extra_pass_count);
}

// _______________________________________________________________________________________________

  void
  control_type::
calc_next
 (  sheet_type const &  src_sheet
  , sheet_type       &  trg_sheet
  , sheet_type       &  extra_sheet
  , bool                are_extra_passes_disabled
)
{
    // Runs in the master thread.
    // The worker thread may not even be created yet.
    d_assert( QThread::currentThread( ) != p_worker_);

    // We should not be processing this message after we have exited.
    if ( is_going_down( ) ) {
        d_assert( false);
        return;
    }

    // We will be busy until we process the "finish" message from the worker thread.
    d_assert( not_busy( ));
    is_busy_ = true;

    // If p_worker_ is zero then we still have to create the worker thread object.
    if ( 0 == p_worker_ ) {
        // Create the thread.
        // We give the worker-thread object no parent (zero pointer) so that signals sent between
        // the master and worker threads are queued instead of dispatched.
        // The ctor for the worker thread runs in the master thread, during creation below.
        // And that ctor starts the worker thread and waits for it to settle.
        p_worker_ = new worker_thread_type( 0);

        // Worker is now running.
        d_assert( p_worker_ && p_worker_->isRunning( ));

        // The parent serves these purposes:
        //   It auto-deletes the object at the end of its life.
        //   When you send a message to an object, Qt has to decide in which thread the message should run.
        //     If the target object has a parent, we use that thread that owns the target object.
        //     If the target object has no parent, we use p_trg->thread( ), which is initially the thread
        //       where the target was created. But we set it below.
        //
        // Since we want messages to p_worker_ to process in that thread, we have to create p_worker_ with
        // no parent (or set it to zero later) and then set the thread.
        //
        // Instead of setting the thread, we could try setting the worker-thread object to be its own parent.
        // It'd look like this:
        //   p_worker_->setParent( p_worker_);
        // This compiles and runs, tho it's not clear what it means for auto-delete.
        // And this doesn't work, even if you follow it with:
        //   p_worker_->moveToThread( p_worker_);
        // The messages to p_worker_ still get dispatched in the master thread.

        // Right now worker_->thread( ) is this (master) thread. And the parent is not set.
        d_assert( p_worker_->thread( ) == QThread::currentThread( ));
        d_assert( p_worker_->parent( ) == 0);

        // Change the thread. You MUST do this from the thread that currently owns the object (this master thread).
        // This will let the signal/slot mechanism work across threads.
        p_worker_->moveToThread( p_worker_);
        // This does not set the parent, so auto-delete is disabled. The worker thread does not delete itself
        // after we call p_worker_->quit( ).
        d_assert( p_worker_->parent( ) == 0);
        // Now messages sent to p_worker_ from the master thread are queued and processed in the worker thread.

        // The worker thread is now awaiting instructions.
        // We use the following 2 async connections, plus ->quit( ), to control it.

        // This is a Qt::QueuedConnection. It is sent from the master and processed in the worker thread.
        // The target (p_worker_ in this case) determines where the message is processed.
          d_verify(
        connect(
            p_worker_, SIGNAL( start__master_to_worker( )),
            p_worker_, SLOT( run__in_worker_thread( )))
          );

        // This is a Qt::QueuedConnection. It is sent from the worker to the master thread.
          d_verify(
        connect(
            p_worker_, SIGNAL( finished__worker_to_master( double)),
            this, SLOT( finished__from_worker_thread( double)))
          );
    }
    d_assert( p_worker_->isRunning( ));

    // Tell the worker thread to start working.
    input_params_.set__are_extra_passes_disabled( are_extra_passes_disabled);
    p_worker_->start_run__from_master_thread
     (  input_params_
      , src_sheet
      , trg_sheet
      , extra_sheet
     );
}

// _______________________________________________________________________________________________

  // private slot
  void
  control_type::
finished__from_worker_thread( double duration_seconds)
{
    // Runs in the master thread. Sent from the worker thread.
    d_assert( p_worker_ && (p_worker_ != QThread::currentThread( )));

    // We are no longer busy and are ready for another solve.
    d_assert( is_busy( ));
    is_busy_ = false;

    // We should not be processing this message after we have exited.
    if ( is_going_down( ) ) {
        d_assert( false);
        return;
    }

    // duration_seconds will be negative if no duration was calculated.
    //
    // Duration is often zero the first time around. This is because there is nothing going on
    // during the first solve, so it's fast. After that there are often messages flying
    // around and redraws happening.
    last_duration_ = duration_seconds;

    // Tell the outside world that the next solve generation is finished being calculated.
    emit finished( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
} /* end namespace heat_solver */

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// heat_solver.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
