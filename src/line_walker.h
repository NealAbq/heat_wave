// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// line_walker.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef LINE_WALKER_H
# define LINE_WALKER_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "util.h"

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// line_walker_type - class
//
//   This is not an iterator. Instead it fits between two iterators, fitting them together when
//   they are not the same size.
// _______________________________________________________________________________________________

  class
line_walker_type
  //
  // Most of this interface is const (or static).
  // The non-const methods fall into these catagories:
  //   Reset
  //   Init/set/assign methods
  //   inc() and dec() iter methods
{
  // -------------------------------------------------------------------------------------------------
  // Typedefs
  private:
    typedef line_walker_type  this_type;
  public:
    typedef signed   int      sint_type ;
    typedef unsigned int      uint_type ;
    d_static_assert( sizeof( sint_type) == sizeof( uint_type));

  // -------------------------------------------------------------------------------------------------
  // Alignment enum
  public:
    enum alignment_selector {
        e_lo
      , e_mid
      , e_hi
    };

  // -------------------------------------------------------------------------------------------------
  // Constructor
  public:
    /* ctor */        line_walker_type
                       (  uint_type  src_count
                        , uint_type  trg_count
                        , alignment_selector = e_mid
                       )                                                        ;
  private:
    static sint_type  calc_dec_pivot( uint_type src_count, uint_type trg_count) ;
    static sint_type  calc_inc_pivot( uint_type src_count, uint_type trg_count) ;

  // -------------------------------------------------------------------------------------------------
  public:
      template // -------------------------------------------------------------------
       <  typename SRC_ITER_T
        , typename TRG_ITER_T
       >
      struct
    iter_types
    {
        typedef typename util::non_integer_calc_type< SRC_ITER_T, TRG_ITER_T >::trg_value_type  return_type ;
        typedef typename util::non_integer_calc_type< SRC_ITER_T, TRG_ITER_T >::value_type      calc_type   ;
        typedef typename util::non_integer_calc_type< SRC_ITER_T, TRG_ITER_T >::value_type      scale_type  ;
    };

  // -------------------------------------------------------------------------------------------------
  // Copy iterator methods
  public:
      template // -------------------------------------------------------------------
       <  typename SRC_ITER_T
        , typename TRG_ITER_T
       >
    static bool       copy_preserve_sum
                       (  SRC_ITER_T src_iter, uint_type const src_count
                        , TRG_ITER_T trg_iter, uint_type const trg_count
                       )                                                        ;

      template // -------------------------------------------------------------------
       <  typename SRC_ITER_T
        , typename TRG_ITER_T
       >
    static bool       copy_preserve_area
                       (  SRC_ITER_T src_iter, uint_type const src_count
                        , TRG_ITER_T trg_iter, uint_type const trg_count
                       )                                                        ;

      template // -------------------------------------------------------------------
       <  typename SRC_ITER_T
        , typename TRG_ITER_T
       >
    static bool       accumulate_preserve_sum
                       (  SRC_ITER_T src_iter, uint_type const src_count
                        , TRG_ITER_T trg_iter, uint_type const trg_count
                       )                                                        ;

      template // -------------------------------------------------------------------
       <  typename SRC_ITER_T
        , typename TRG_ITER_T
       >
    static bool       accumulate_preserve_area
                       (  SRC_ITER_T src_iter, uint_type const src_count
                        , TRG_ITER_T trg_iter, uint_type const trg_count
                       )                                                        ;

      template // -------------------------------------------------------------------
       <  typename SRC_ITER_T
        , typename TRG_ITER_T
       >
    static bool       scaled_copy_preserve_sum
                       (  typename iter_types< SRC_ITER_T, TRG_ITER_T >::scale_type scale_factor
                        , SRC_ITER_T src_iter, uint_type const src_count
                        , TRG_ITER_T trg_iter, uint_type const trg_count
                       )                                                        ;

      template // -------------------------------------------------------------------
       <  typename SRC_ITER_T
        , typename TRG_ITER_T
       >
    static bool       scaled_copy_preserve_area
                       (  typename iter_types< SRC_ITER_T, TRG_ITER_T >::scale_type scale_factor
                        , SRC_ITER_T src_iter, uint_type const src_count
                        , TRG_ITER_T trg_iter, uint_type const trg_count
                       )                                                        ;

      template // -------------------------------------------------------------------
       <  typename SRC_ITER_T
        , typename TRG_ITER_T
       >
    static bool       scaled_accumulate_preserve_sum
                       (  typename iter_types< SRC_ITER_T, TRG_ITER_T >::scale_type scale_factor
                        , SRC_ITER_T src_iter, uint_type const src_count
                        , TRG_ITER_T trg_iter, uint_type const trg_count
                       )                                                        ;

      template // -------------------------------------------------------------------
       <  typename SRC_ITER_T
        , typename TRG_ITER_T
       >
    static bool       scaled_accumulate_preserve_area
                       (  typename iter_types< SRC_ITER_T, TRG_ITER_T >::scale_type scale_factor
                        , SRC_ITER_T src_iter, uint_type const src_count
                        , TRG_ITER_T trg_iter, uint_type const trg_count
                       )                                                        ;

      template // -------------------------------------------------------------------
       <  typename ASSIGN_FUNCTOR_T
        , typename SRC_ITER_T
        , typename TRG_ITER_T
       >
    static bool       copy
                       (  ASSIGN_FUNCTOR_T assign_fn
                        , SRC_ITER_T src_iter, uint_type const src_count
                        , TRG_ITER_T trg_iter, uint_type const trg_count
                       )                                                        ;

      template // -------------------------------------------------------------------
       <  typename ASSIGN_FUNCTOR_T
        , typename SRC_ITER_T
        , typename TRG_ITER_T
       >
    static void       copy_n
                       (  ASSIGN_FUNCTOR_T assign_fn
                        , SRC_ITER_T src_iter, uint_type count
                        , TRG_ITER_T trg_iter
                       )                                                        ;

      template // -------------------------------------------------------------------
       <  typename RETURN_T
        , typename CALC_T
       >
    RETURN_T          get_trg_value_preserve_sum( CALC_T src_value)       const ;

      template // -------------------------------------------------------------------
       <  typename RETURN_T
        , typename CALC_T
       >
    RETURN_T          get_trg_value_preserve_area( CALC_T src_value)      const ;

  // -------------------------------------------------------------------------------------------------
  // inc( ) & dec( )
  public:
    enum src_trg_selector {
        e_src           = 1
      , e_trg           = 2
      , e_src_and_trg   = 3
    };
    static bool       includes_src( src_trg_selector s)         { return (s & 0x1) != 0; }
    static bool       includes_trg( src_trg_selector s)         { return (s & 0x2) != 0; }

    src_trg_selector  inc( )                                    ;
    src_trg_selector  dec( )                                    ;

  // -------------------------------------------------------------------------------------------------
  // Counts
  public:
    uint_type         get_src_count( )                    const { return src_count_; }
    uint_type         get_trg_count( )                    const { return trg_count_; }

  // -------------------------------------------------------------------------------------------------
  // Widths
  public:
    // If the trg-count is 5 and the src-count is 10, then there are two src for every trg, and each
    // src is half the width of each trg. Since the trg width is twice the src-width, and since the
    // src-count is twice the trg-count, we return the TRG-count for the SRC-width, and vice versa.
    // These "widths" don't have any meaning by themselves. They are always divided by other widths
    // to get pure-number ratios.
    uint_type         get_src_width( )                    const { return get_trg_count( ); }
    uint_type         get_trg_width( )                    const { return get_src_count( ); }

    uint_type         get_src_trg_overlap_width( )        const { uint_type const width = get_src_no_spill( );
                                                                  d_assert( width == get_trg_no_spill( ));
                                                                  d_assert( width > 0);
                                                                  d_assert( width <= get_src_width( ));
                                                                  d_assert( width <= get_trg_width( ));
                                                                  return width;
                                                                }
    bool              is_src_width_fully_covered( )       const { return
                                                                    (! is_src_spilling_lo( )) &&
                                                                    (! is_src_spilling_hi( ));
                                                                }
    bool              is_trg_width_fully_covered( )       const { return
                                                                    (! is_trg_spilling_lo( )) &&
                                                                    (! is_trg_spilling_hi( ));
                                                                }

      template < typename RETURN_T >
    RETURN_T          get_src_overlap_ratio( )            const { return
                                                                    static_cast< RETURN_T >( get_src_trg_overlap_width( )) /
                                                                    static_cast< RETURN_T >( get_src_width( ));
                                                                }
      template < typename RETURN_T >
    RETURN_T          get_trg_overlap_ratio( )            const { return
                                                                    static_cast< RETURN_T >( get_src_trg_overlap_width( )) /
                                                                    static_cast< RETURN_T >( get_trg_width( ));
                                                                }

  // -------------------------------------------------------------------------------------------------
  // Alignment
  public:
    bool              is_aligned_lo( )                    const { return dec_pivot_ == modulo_; }
    bool              is_aligned_hi( )                    const { return inc_pivot_ == modulo_; }
    bool              is_aligned_mid( )                   const { return          0 == modulo_; }

    void              set_aligned_lo( )                         { modulo_ = dec_pivot_; }
    void              set_aligned_hi( )                         { modulo_ = inc_pivot_; }
    void              set_aligned_mid( )                        { modulo_ = 0; }
    // If both counts are even, mid is -1/2.
    // If both counts are odd, mid is 0.
    // If one count is odd and one is even, mid is -1/4.

    bool              is_aligned( alignment_selector a)   const { return get_modulo_for_alignment( a) == modulo_; }
    void              set_aligned( alignment_selector a)        { modulo_ = get_modulo_for_alignment( a); }

  private:
    sint_type         get_modulo_for_alignment( alignment_selector a)
                                                          const { switch ( a ) {
                                                                    case e_lo: return dec_pivot_;
                                                                    case e_hi: return inc_pivot_;
                                                                    default  : return 0;
                                                                } }

  // -------------------------------------------------------------------------------------------------
  // Spill query interface
  protected:
    bool              is_trg_spilling_lo( )               const { return dec_pivot_ > modulo_; }
    bool              is_trg_spilling_hi( )               const { return modulo_ > inc_pivot_; }

    uint_type         get_trg_lo_spill( )                 const { return (dec_pivot_ > modulo_) ? (dec_pivot_ - modulo_) : 0; }
    uint_type         get_trg_hi_spill( )                 const { return (modulo_ > inc_pivot_) ? (modulo_ - inc_pivot_) : 0; }
    uint_type         get_trg_no_spill( )                 const { return get_trg_width( ) -
                                                                         (get_trg_lo_spill( ) + get_trg_hi_spill( ));
                                                                }

    bool              is_src_spilling_lo( )               const { return modulo_ > dec_pivot_; }
    bool              is_src_spilling_hi( )               const { return inc_pivot_ > modulo_; }

    uint_type         get_src_lo_spill( )                 const { return (modulo_ > dec_pivot_) ? (modulo_ - dec_pivot_) : 0; }
    uint_type         get_src_hi_spill( )                 const { return (inc_pivot_ > modulo_) ? (inc_pivot_ - modulo_) : 0; }
    uint_type         get_src_no_spill( )                 const { return get_src_width( ) -
                                                                         (get_src_lo_spill( ) + get_src_hi_spill( ));
                                                                }

    void              get_trg_spills
                       (  uint_type &  lo_spill
                        , uint_type &  hi_spill
                        , uint_type &  no_spill  /* the middle part that isn't spilled */
                       )                                  const { lo_spill = get_trg_lo_spill( );
                                                                  hi_spill = get_trg_hi_spill( );
                                                                  no_spill = get_trg_width( ) - (lo_spill + hi_spill);
                                                                  d_assert( no_spill > 0);
                                                                }

    void              get_src_spills
                       (  uint_type &  lo_spill
                        , uint_type &  hi_spill
                        , uint_type &  no_spill  /* the middle part that isn't spilled */
                       )                                  const { lo_spill = get_src_lo_spill( );
                                                                  hi_spill = get_src_hi_spill( );
                                                                  no_spill = get_src_width( ) - (lo_spill + hi_spill);
                                                                  d_assert( no_spill > 0);
                                                                }

  // -------------------------------------------------------------------------------------------------
  // For debugging
  private:
    sint_type         get_modulo_min( )                   const { bool      const is_trg_count_odd = (get_trg_count( ) & 1);
                                                                  sint_type const odd_correction   = is_trg_count_odd ? 0 : +1;
                                                                  sint_type const avg_half_counts  = (get_src_count( ) / 2) +
                                                                                                     (get_trg_count( ) / 2);
                                                                  return odd_correction - avg_half_counts;
                                                                }
    sint_type         get_modulo_max( )                   const { bool      const is_src_count_odd = (get_src_count( ) & 1);
                                                                  sint_type const odd_correction   = is_src_count_odd ? 0 : -1;
                                                                  sint_type const avg_half_counts  = (get_src_count( ) / 2) +
                                                                                                     (get_trg_count( ) / 2);
                                                                  return odd_correction + avg_half_counts;
                                                                }

  // -------------------------------------------------------------------------------------------------
  // Member vars
  //   Improve: We should have an init(..) function so you can reuse this object.
  //     We'll have to take "const" away from these 4 const vars.
  //     We'll also want a reset or invalid state, maybe (0==src_count).
  //   Improve: This should have a copy ctor/ass-op.
  //     The const has to go for copy assignment.
  private:
    uint_type const  src_count_ ;  /* only set during init */
    uint_type const  trg_count_ ;  /* only set during init */

    sint_type const  dec_pivot_ ;  /* calc'd when counts are set (during init) */
    sint_type const  inc_pivot_ ;  /* calc'd when counts are set (during init) */

    sint_type        modulo_    ;  /* changed by inc( ) and dec( ) */
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Iterator Copy helpers:
//
//   Methods:
//     walker.get_trg_value_preserve_sum<  RETURN_T, CALC_T >( CALC_T src_value)
//     walker.get_trg_value_preserve_area< RETURN_T, CALC_T >( CALC_T src_value)
//
//   Functors:
//     preserve_sum_doit_functor<  SRC_ITER_T, TRG_ITER_T >( )::doit( walker, *src_iter)
//     preserve_area_doit_functor< SRC_ITER_T, TRG_ITER_T >( )::doit( walker, *src_iter)
// _______________________________________________________________________________________________

  template
   <  typename RETURN_T
    , typename CALC_T
   >
  RETURN_T
  line_walker_type::
get_trg_value_preserve_sum( CALC_T src_value) const
  //
  // This lets you copy such that the sum of the values in the src ends up the same as the sum of
  // the results stored in the trg. There are two ways to do this:
  //
  //   *trg = *src * (src_no_spill / src_width)
  //   *trg = *src * (src_no_spill / trg_count)
  //
  // and
  //
  //   *trg = *src * (trg_no_spill / trg_width) * (src_count / trg_count)
  //   *trg = *src * (trg_no_spill / src_count) * (src_count / trg_count)
  //   *trg = *src * (trg_no_spill / trg_count)
{
    return
      util::round_if_appropriate< RETURN_T, CALC_T >::doit(
        is_src_width_fully_covered( ) ?
           src_value :
          (src_value * get_src_overlap_ratio< CALC_T >( )));
}

  template
   <  typename RETURN_T
    , typename CALC_T
   >
  RETURN_T
  line_walker_type::
get_trg_value_preserve_area( CALC_T src_value) const
  //
  // This lets you copy such that the sum of all the src values multiplied by the width of the
  // src cell is the same as the sum of the results stored in the trg multiplied by the width
  // of a trg cell. This is what you'd use if you were copying pixels while changing resolution.
  //
  // There are two ways to do this:
  //   *trg = *src * (trg_no_spill / trg_width)
  //   *trg = *src * (trg_no_spill / src_count)
  //
  // and
  //
  //   *trg = *src * (src_no_spill / src_width) * (trg_count / src_count)
  //   *trg = *src * (src_no_spill / trg_count) * (trg_count / src_count)
  //   *trg = *src * (src_no_spill / src_count)
{
    return
      util::round_if_appropriate< RETURN_T, CALC_T >::doit(
        is_trg_width_fully_covered( ) ?
           src_value :
          (src_value * get_trg_overlap_ratio< CALC_T >( )));
}

// _______________________________________________________________________________________________
// Functor types to wrap the above methods
//   See template typenames PRESERVE_FUNCTOR_T

  template
   <  typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  struct
preserve_sum_doit_functor
{
    typedef line_walker_type::iter_types< SRC_ITER_T, TRG_ITER_T >  iter_types  ;
    typedef typename iter_types::return_type                        return_type ;
    typedef typename iter_types::calc_type                          calc_type   ;

    // Use doit(..) instead of operator() so we can make this static.
    static
    return_type
  doit
   (  line_walker_type const &  walker
    , calc_type                 src_value
   )
   {
     return walker.get_trg_value_preserve_sum< return_type >( src_value);
   }
};

  template
   <  typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  struct
preserve_area_doit_functor
{
    typedef line_walker_type::iter_types< SRC_ITER_T, TRG_ITER_T >  iter_types  ;
    typedef typename iter_types::return_type                        return_type ;
    typedef typename iter_types::calc_type                          calc_type   ;

    static
    return_type
  doit
   (  line_walker_type const &  walker
    , calc_type                 src_value
   )
   {
     return walker.get_trg_value_preserve_area< return_type >( src_value);
   }
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Assignment functor
//
//   copy_functor<..>
//   scaled_copy_functor<..>
//   accumulate_functor<..>
//   scaled_accumulate_functor<..>
//
//   This is where we scale iterators.
//   Probably a better solution is to provide an iterator adaptor (wrapper) that scales
//   access and/or assignment.
//
//   These functors are used where you see typename ASSIGN_FUNCTOR_T.
// _______________________________________________________________________________________________

  template
   <  typename PRESERVE_FUNCTOR_T
    , typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  struct
copy_functor
{
    typedef line_walker_type::iter_types< SRC_ITER_T, TRG_ITER_T >  iter_types  ;
    typedef typename iter_types::return_type                        return_type ;

    void
  operator ()
   (  line_walker_type const &  walker
    , SRC_ITER_T             &  src_iter
    , TRG_ITER_T             &  trg_iter
    , bool                   /* is_new_src */
    , bool                      is_new_trg
   )
    {   return_type
            new_trg_value =
                PRESERVE_FUNCTOR_T::doit( walker, *src_iter);
        if ( is_new_trg ) {
            *trg_iter = new_trg_value;
        } else
        /* not is_new_trg */ {
            *trg_iter += new_trg_value;
        }
    }

    void
  operator ()
   (  SRC_ITER_T &  src_iter
    , TRG_ITER_T &  trg_iter
   )
    { *trg_iter = *src_iter; }
};

// _______________________________________________________________________________________________

  template
   <  typename PRESERVE_FUNCTOR_T
    , typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  struct
scaled_copy_functor
{
    typedef line_walker_type::iter_types< SRC_ITER_T, TRG_ITER_T >  iter_types  ;
    typedef typename iter_types::return_type                        return_type ;
    typedef typename iter_types::calc_type                          calc_type   ;

  scaled_copy_functor( calc_type sv) : scale_value( sv) { }
    calc_type const scale_value;

    void
  operator ()
   (  line_walker_type const &  walker
    , SRC_ITER_T             &  src_iter
    , TRG_ITER_T             &  trg_iter
    , bool                   /* is_new_src */
    , bool                      is_new_trg
   )
    {   return_type
            new_trg_value =
                PRESERVE_FUNCTOR_T::doit( walker, (*src_iter) * scale_value);
        if ( is_new_trg ) {
            *trg_iter = new_trg_value;
        } else
        /* not is_new_trg */ {
            *trg_iter += new_trg_value;
        }
    }

    void
  operator ()
   (  SRC_ITER_T &  src_iter
    , TRG_ITER_T &  trg_iter
   )
    { *trg_iter = (*src_iter) * scale_value; }
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  template
   <  typename PRESERVE_FUNCTOR_T
    , typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  struct
accumulate_functor
{
    void
  operator ()
   (  line_walker_type const &  walker
    , SRC_ITER_T             &  src_iter
    , TRG_ITER_T             &  trg_iter
    , bool                   /* is_new_src */
    , bool                   /* is_new_trg */
   )
    { *trg_iter += PRESERVE_FUNCTOR_T::doit( walker, *src_iter); }

    void
  operator ()
   (  SRC_ITER_T &  src_iter
    , TRG_ITER_T &  trg_iter
   )
    { *trg_iter += *src_iter; }
};

// _______________________________________________________________________________________________

  template
   <  typename PRESERVE_FUNCTOR_T
    , typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  struct
scaled_accumulate_functor
{
    typedef line_walker_type::iter_types< SRC_ITER_T, TRG_ITER_T >  iter_types ;
    typedef typename iter_types::calc_type                          calc_type  ;

  scaled_accumulate_functor( calc_type sv) : scale_value( sv) { }
    calc_type const scale_value;

    void
  operator ()
   (  line_walker_type const &  walker
    , SRC_ITER_T             &  src_iter
    , TRG_ITER_T             &  trg_iter
    , bool                   /* is_new_src */
    , bool                   /* is_new_trg */
   )
    { *trg_iter += PRESERVE_FUNCTOR_T::doit( walker, (*src_iter) * scale_value); }

    void
  operator ()
   (  SRC_ITER_T &  src_iter
    , TRG_ITER_T &  trg_iter
   )
    { *trg_iter += (*src_iter) * scale_value; }
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Iterator Copy/Accumulate
//
//   line_walker_type::copy_preserve_sum( src_iter, src_count, trg_iter, trg_count)
//   line_walker_type::copy_preserve_area( src_iter, src_count, trg_iter, trg_count)
//
//   line_walker_type::accumulate_preserve_sum( src_iter, src_count, trg_iter, trg_count)
//   line_walker_type::accumulate_preserve_area( src_iter, src_count, trg_iter, trg_count)
//
//   line_walker_type::scaled_copy_preserve_sum( scale, src_iter, src_count, trg_iter, trg_count)
//   line_walker_type::scaled_copy_preserve_area( scale, src_iter, src_count, trg_iter, trg_count)
//
//   line_walker_type::scaled_accumulate_preserve_sum( scale, src_iter, src_count, trg_iter, trg_count)
//   line_walker_type::scaled_accumulate_preserve_area( scale, src_iter, src_count, trg_iter, trg_count)
// _______________________________________________________________________________________________

  template
   <  typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  /* static method */
  bool
  line_walker_type::
copy_preserve_sum
 (  SRC_ITER_T src_iter, uint_type const src_count
  , TRG_ITER_T trg_iter, uint_type const trg_count
 )
{
    return
        copy
         (  copy_functor
             <  preserve_sum_doit_functor
                 <  SRC_ITER_T
                  , TRG_ITER_T
                 >
              , SRC_ITER_T
              , TRG_ITER_T
             >( )
          , src_iter, src_count
          , trg_iter, trg_count
         );
}

// _______________________________________________________________________________________________

  template
   <  typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  /* static method */
  bool
  line_walker_type::
accumulate_preserve_sum
 (  SRC_ITER_T src_iter, uint_type const src_count
  , TRG_ITER_T trg_iter, uint_type const trg_count
 )
{
    return
        copy
         (  accumulate_functor
             <  preserve_sum_doit_functor
                 <  SRC_ITER_T
                  , TRG_ITER_T
                 >
              , SRC_ITER_T
              , TRG_ITER_T
             >( )
          , src_iter, src_count
          , trg_iter, trg_count
         );
}

// _______________________________________________________________________________________________

  template
   <  typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  /* static method */
  bool
  line_walker_type::
copy_preserve_area
 (  SRC_ITER_T src_iter, uint_type const src_count
  , TRG_ITER_T trg_iter, uint_type const trg_count
 )
{
    return
        copy
         (  copy_functor
             <  preserve_area_doit_functor
                 <  SRC_ITER_T
                  , TRG_ITER_T
                 >
              , SRC_ITER_T
              , TRG_ITER_T
             >( )
          , src_iter, src_count
          , trg_iter, trg_count
         );
}

// _______________________________________________________________________________________________

  template
   <  typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  /* static method */
  bool
  line_walker_type::
accumulate_preserve_area
 (  SRC_ITER_T src_iter, uint_type const src_count
  , TRG_ITER_T trg_iter, uint_type const trg_count
 )
{
    return
        copy
         (  accumulate_functor
             <  preserve_area_doit_functor
                 <  SRC_ITER_T
                  , TRG_ITER_T
                 >
              , SRC_ITER_T
              , TRG_ITER_T
             >( )
          , src_iter, src_count
          , trg_iter, trg_count
         );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Scaled copy/accumulate
//   These are not needed for 1D copy. They're used to copy 2D and 3D grids.

  template
   <  typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  /* static method */
  bool
  line_walker_type::
scaled_copy_preserve_sum
 (  typename iter_types< SRC_ITER_T, TRG_ITER_T >::scale_type scale_factor
  , SRC_ITER_T src_iter, uint_type const src_count
  , TRG_ITER_T trg_iter, uint_type const trg_count
 )
{
    return
        copy
         (  scaled_copy_functor
             <  preserve_sum_doit_functor
                 <  SRC_ITER_T
                  , TRG_ITER_T
                 >
              , SRC_ITER_T
              , TRG_ITER_T
             >( scale_factor)
          , src_iter, src_count
          , trg_iter, trg_count
         );
}

// _______________________________________________________________________________________________

  template
   <  typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  /* static method */
  bool
  line_walker_type::
scaled_accumulate_preserve_sum
 (  typename iter_types< SRC_ITER_T, TRG_ITER_T >::scale_type scale_factor
  , SRC_ITER_T src_iter, uint_type const src_count
  , TRG_ITER_T trg_iter, uint_type const trg_count
 )
{
    return
        copy
         (  scaled_accumulate_functor
             <  preserve_sum_doit_functor
                 <  SRC_ITER_T
                  , TRG_ITER_T
                 >
              , SRC_ITER_T
              , TRG_ITER_T
             >( scale_factor)
          , src_iter, src_count
          , trg_iter, trg_count
         );
}

// _______________________________________________________________________________________________

  template
   <  typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  /* static method */
  bool
  line_walker_type::
scaled_copy_preserve_area
 (  typename iter_types< SRC_ITER_T, TRG_ITER_T >::scale_type scale_factor
  , SRC_ITER_T src_iter, uint_type const src_count
  , TRG_ITER_T trg_iter, uint_type const trg_count
 )
{
    return
        copy
         (  scaled_copy_functor
             <  preserve_area_doit_functor
                 <  SRC_ITER_T
                  , TRG_ITER_T
                 >
              , SRC_ITER_T
              , TRG_ITER_T
             >( scale_factor)
          , src_iter, src_count
          , trg_iter, trg_count
         );
}

// _______________________________________________________________________________________________

  template
   <  typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  /* static method */
  bool
  line_walker_type::
scaled_accumulate_preserve_area
 (  typename iter_types< SRC_ITER_T, TRG_ITER_T >::scale_type scale_factor
  , SRC_ITER_T src_iter, uint_type const src_count
  , TRG_ITER_T trg_iter, uint_type const trg_count
 )
{
    return
        copy
         (  scaled_accumulate_functor
             <  preserve_area_doit_functor
                 <  SRC_ITER_T
                  , TRG_ITER_T
                 >
              , SRC_ITER_T
              , TRG_ITER_T
             >( scale_factor)
          , src_iter, src_count
          , trg_iter, trg_count
         );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Low-level copy/accumulate function templates
//
//   line_walker_type::copy( assign_fn, src_iter, src_count, trg_iter, trg_count)
//   line_walker_type::copy_n( assign_fn, src_iter, count, trg_iter)
// _______________________________________________________________________________________________

  template
   <  typename ASSIGN_FUNCTOR_T
    , typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  /* static method */
  bool
  line_walker_type::
copy
 (  ASSIGN_FUNCTOR_T assign_fn
  , SRC_ITER_T src_iter, uint_type const src_count
  , TRG_ITER_T trg_iter, uint_type const trg_count
 )
  // To be consistent with std::copy(..) and other std functions, the src params are before the trg params.
  //
  // This does not have to copy. It could instead add-to or subtract-from. It is up to the assignment functor.
{
    // Take care of the easy linear-copy case. This is faster.
    if ( src_count == trg_count ) {
        copy_n( assign_fn, src_iter, src_count, trg_iter);
        return true;
    }

    // The line_walker_type class requires the following:
    if ( (src_count > 0) && (trg_count > 0) && ((src_count + trg_count) > 0) ) {
        line_walker_type walker( src_count, trg_count, line_walker_type::e_lo);

        d_assert( walker.is_aligned_lo( ));
        d_assert( ! walker.is_trg_spilling_lo( ));
        d_assert( ! walker.is_src_spilling_lo( ));

        uint_type src_countdown = src_count;
        uint_type trg_countdown = trg_count;
        bool is_new_src = true;
        bool is_new_trg = true;
        do {
            assign_fn( walker, src_iter, trg_iter, is_new_src, is_new_trg);

            src_trg_selector const inc_code = walker.inc( );
            is_new_src = includes_src( inc_code);
            is_new_trg = includes_trg( inc_code);
            d_assert( is_new_src || is_new_trg /* one or both are true */);

            if ( is_new_src ) {
                ++ src_iter;
                -- src_countdown;
            }
            if ( is_new_trg ) {
                ++ trg_iter;
                -- trg_countdown;
            }
        }
        while ( src_countdown && trg_countdown );
        d_assert( is_new_trg);
        d_assert( (0 == src_countdown) && (0 == trg_countdown));

        d_assert( ! walker.is_src_spilling_lo( ));
        d_assert( ! walker.is_trg_spilling_lo( ));
        d_assert( walker.is_aligned_lo( ));

        return true;
    }
    return false;
}

// _______________________________________________________________________________________________

  template
   <  typename ASSIGN_FUNCTOR_T
    , typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  /* static method */
  void
  line_walker_type::
copy_n
 (  ASSIGN_FUNCTOR_T assign_fn
  , SRC_ITER_T src_iter, uint_type count
  , TRG_ITER_T trg_iter
 )
  // Very simple one-to-one linear copy.
{
    // We could express this as
    //   std::copy_n( src_iter, count, trg_iter)
    // But copy_n(..) is not standard. Or we could do:
    //   std::copy( src_iter, src_iter + count, trg_iter)
    // But that assumes src_iter is a RandomAccessIterator and not the more general InputIterator.

    while ( count > 0 ) {
        assign_fn( src_iter, trg_iter);
        ++ src_iter;
        ++ trg_iter;
        -- count;
    }
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef LINE_WALKER_H */
//
// line_walker.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
