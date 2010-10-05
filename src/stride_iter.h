// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// stride_iter.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef STRIDE_ITER_H
# define STRIDE_ITER_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Iterators in 2 or more dimensions
//
//   The std iterators are designed for 1-dimensional containers.
//   With a 1-dimensional iterator there is one path thru the iteration, and you use (++iter)
//   to reach all the items. This iterator defines one kind of adjacency.
//
//   So how do we abstract iterators for a 2-dimensional container?
//
//   Hierarchy model (iterators for a 2-dimensional container):
//     iter_major = Container.begin( );
//     iter_minor = *iter_major;
//     value = *iter_minor;
//
//     So if we have a hierarchical 2-dimensional array, we'd need a pair of iterator types:
//       Row iterator: iterate thru the rows in the 2D array
//       Element iterator: iterate thru the elements in a row
//
//   Flat model:
//     iter_2d = Container.begin( );
//     iter_2d.go_next( 0 /* first dimension */);
//     iter_2d.go_next( 1 /* second dimension */);
//       or
//     iter2d.go_next<0>( );
//     iter2d.go_next<1>( );
//
//     We end up with one iterator, but you can increment/decrement it in two directions.
//     So it hard to use operators like ++ and +=.
//     And it's hard to define an end tester that looks like what the std library uses.
//     So it's unclear what the navigating operators should do, and what the end tests should be.
//     But it's easy to design an interface that makes sense here. It just doesn't involve the usual operator idioms.
//
//   The flat model only works with a flat container, but if you have a flat container the flat model is more
//   general. From a flat model you can easily define a hierarchical model that's less general but works with
//   the operator idioms.
//
//   sride_iter<..> and stride_range<..>, defined below, do just that. They take a flat model and let you define
//   a hierarchical set of iterators that (mostly) work with the std-lib idioms. And they let you flatten any way
//   you like, so if you have a flat array representing a 2-dimensional sheet, you can define iterators that iterate
//   in column-major order even when the data is arranged in row-major order.
//
//   Flat 2D iterators:
//     (iter_b - iter_a) -> vector of 2 differences
//     iter_a + vector2 -> iter_b
//
// _______________________________________________________________________________________________
// Improvements to the standard iterator model - reset and alignment
//
//   The iterators in the std namespace are careful to be compatible with raw pointers, pointer
//   arithmetic, and operator idioms (which work with raw pointers).
//   Another choice would have been to define trait classes that worked with raw pointers, and
//   then we wouldn't be restricted to 1-dimensional operator-based iterator idioms.
//
//   Template functions to make standard iterators safer and more expressive:
//
//     get_reset< iter_type >( ) -> iter_type
//     get_reset< iter_type >( iter_type const & ) -> iter_type
//     is_reset< iter_type >( iter_type const & ) -> bool
//     reset< iter_type >( iter_type & ) -> void (or iter_type &)
//
//     supports_reset_state< iter_type >::value
//       Trait. Some iterator types may not support any reset state.
//       These iterators should also not provide a default constructor.
//
//     See <boost/type_traits/alignment_of.hpp>
//
//     is_aligned< iter_type >( iter_type const &) -> bool
//       True if the pointer is a legitimate (correctly aligned) pointer.
//       Always true if is_reset< iter_type >( iter).
//       Almost always true and mostly used for debug.
//       If the iterator is a pointer, true iff
//         (0 == ptr) or
//         (0 == static_cast< boost::alignment_of< iter_type >::value_type >( ptr)
//                 % boost::alignment_of< iter_type >::value)
//       There could be an additional test to make sure the iterator appears valid.
//         This could test a lot of things.
//
//     are_aligned< iter_type >( iter1, iter2) -> bool
//       This is a test to make sure it is OK to subtract the two pointers from each other.
//       True if both iters are reset.
//       False if one of the iters is reset and one is not.
//       True if both of the iters are:
//         Not reset
//         Aligned
//         Additional test
//       For a raw pointer there is probably no additional test.
//       Additional tests:
//         Do both iterators appear valid.
//         Do both iterators appear to be in range.
//         Do both iterators appear to come from a container that still exists.
//         Do both iterators appear to come from the same container.
//
//   Iterator types could also support these operators:
//     operator bool( )
//       Syntax error if iter_type does not support reset state.
//       Or some variation using the safe bool idiom.
//       Should be the same as (! is_reset< iter_type >( iter)).
//
//     operator !( )
//       May not have to be defined if operator bool( ) is present.
//       Same as is_reset< iter_type >( iter).
//
//   std::vector< T >::iterator does not fully support a reset state.
//     The default ctor works and default-constructs T.
//     If T is a primitive, std::vector< T >::iterator( ) constructs a pointer full
//       of random bits (IS THIS TRUE? CHECK THIS!).
//
//     Legal:
//       std::vector< float >::iterator iter_a;
//       float * ptr_a = &(*iter_a);
//       if ( iter_a == std::vector< float >::iterator( ) ) { }
//
//     Illegal:
//       std::vector< float >::iterator iter_b( (float*) 0);
//       if ( iter_b ) { }
//       if ( ! iter_b ) { }
//
//     Although you can convert (std::vector< float >::iterator) to a (float*), it is
//     clearly a kludge that should not be used.
//     This won't work if the iterator uses an intermediate ref class instead of a raw ref.
//     For example (std::vector< bool >::iterator) cannot be converted into a (bool*) pointer.
//
// _______________________________________________________________________________________________
// Iterator adaptors:
//   Stride (static and dynamic)
//   Reverse (special case of stride where stride is -1)
//     std::reverse_iterator< ITER_T >
//     boost::reverse_iterator< ITER_T >
//   Scaled
//     See line-walker.
//     All access is multiplied by scale.
//     All assignment is divided by scale?
//   Access only
//   Write only, sum-into only (accumulate only)
//   Const
//     Could be special case of stride where stride is 0.
//   Count
//     The limit is when count_down is zero.
//     (count_down == 0) means past end (at limit) and deref is illegal
//     All count_down==0 iters could test ==.
//   Filter
//   Calculated values
//   Ring
//     A ring that works with constructs/destructors so it can be a ring of refs.
//   Tail (of some max length)
//     Leading and trailing edges
//   Accumulators
//   Inserters
//     std::insert_iterator<T>
//     std::back_insert_iterator<T>
//     std::front_insert_iterator<T>
//
// _______________________________________________________________________________________________
// Iterator structure
//   An iterator has these properties:
//     Are inner objects refs or value. Can they be aliases? Can they change due to actions in
//     this thread? Can they change in other threads?
//
//     Is referenced object stored?
//       No (it's a calculated value)
//       Yes, but only until we increment
//       This iterator doesn't know, ask the source (the underlying container or wrapped iterator)
//
//     Is referenced object stable?
//       Yes, it will never change
//       It will only change on set
//
//     Is the iterator valid?
//       Always, even if the underlying container goes away.
//
//   An iterator can be decomposed into several parts:
//     Reference part:
//         get, set, ==, !=
//         at_end, past_end, at_begin, past_begin
//         reset, is_reset
//       Location
//         Like a getter that you never deref. The std algorithms use limit iterators to mark the
//         end of iteration. These are iterators that do not allow read or write.
//         Comparison like == often operate just on the location
//         There can be several kinds of == test:
//           Test the deref value
//           Test the ref
//           Only == if underlying sources are ==
//           Only == if navigation and calculation parts are ==
//           Any == is really part of a compare set:
//             Is == defined (if so, then so is !=)
//             Is < defined (if so, then so is >)
//             Is (a - b) -> difference defined?
//       Getter (read part)
//         Function is get( )
//       Setter (write part)
//         Many iters are write-only
//         Some may support insert_before, insert_after, remove_reset, remove_before, remove_after
//
//     Navigator part:
//       Provides functions like go_next( ) (operator ++) and go( N).
//
//     Calculation part:
//       Operators like < and (a - b).
//       Functions like is_adjacent?
//       Operator < is not always easily available and may not be defined (or might return 'unknown')
//       The sense of operator < is reversed if the stride is < 0.
//
//     Source part:
//       Underlying source. This part is often missing, but you can use it to stack iterators.
//     Filter part:
//       The source and filter usually make up the reference part described above.
//
// _______________________________________________________________________________________________
// Iterator typedefs
//   Iterators define these type names:
//     difference_type      - signed integer
//                              we need to be able to move backwards and forwards with this
//                              we could have special values for:
//                                (reset - normal)
//                                (normal - reset)
//                                (is_past_end - normal)
//                                  .. etc, 15 in total ..
//                                  .. (normal - normal) is the normal case
//                                  .. (reset - reset), (is_post - is_post), and
//                                      (is_pre - is_pre) could all be zero
//                                  .. (is_post - normal) should be legal, as should the inverse
//                                  .. (normal - is_pre) should be legal, as should the inverse
//     value_type           - value returned by (*iter) or iter.get( )
//     pointer              - value_type *
//     reference            - value_type &
//
//   An iterator can have these special values (that cannot be de-ref'ed):
//     reset (null pointer)
//     one past the end
//     one before the beginning
//   These special values can overlap (so that is_past_end is the same as is_before_begin).
//
//   How inconvenient is it to have to prove you have an is_post iter every time you inc?
//   It seems like a bad idea. There's a few cases that come to mind:
//     You are inc'ing thru several equal-sized collections at the same time, and you only
//       have one is_post iter for testing.
//     You have size information instead of an is_post iter.
//     After you inc you KNOW it's ok to dec, because you just inc'ed. This is a kind of count.
//
//   If your pre/post iters are pointers past the ends, you have to guarantee they don't wrap
//   around the address space. This is more of a problem with stride iters since they can be
//   way past the ends. Using min/max instead of min/limit pointers may be a better idea, although
//   you'd have to add special tests to inc to make sure you didn't overflow.
//
//   Suggestions:
//     diff_type          - better name for difference_type
//     value_ref_type     - never const, only used by non-const iters
//     value_param_type   - only in param lists, either value_type or (value_type const &)
//     value_pass_type    - alt name for value_param_type
//     value_return_type  - probably same as value_param_type. way to return internal value and maybe skip a copy.
//                          can always use value_type instead. must be backed by internal copy.
//     value_back_type    - better name for value_return_type
//     value_type         - never const, never a ref
//
// _______________________________________________________________________________________________
// Const iterator
//   If we have a const_iterator for a hierarchical iter, what is const?
//
//     The final value in the ultimate collection?
//       This is probably usually what we mean, although you can get this easily enough:
//         stride_iter< varia_inner_iter_type, 3 >  - not const
//         stride_iter< const_inner_iter_type, 3 >  - const
//
//     The next range?
//       This is more consistent with iterators.
//       You can still get a non-const iterator out of the range.
//       You can still inc/dec the iterator and get another range that way.
//
//   What if the iter object itself is "const"?
//     That means you cannot inc or dec the iterator itself.
//     It does not affect the value that you get thru the iterator.
//
//   What if the range object is "const"?
//     That means you cannot change the begin/end of the range.
//     And the iters you get from the range should be const-iterators (not allowed to change the range).
//
//   Think of "const" as something that propagates thru certain kinds of rlns.
//   For a container, "const" means all the things in the container. But you can copy anything
//   out of the const container and still change the copy.
//
//   For an iterator, it is a rln that can propagate either const or varia.
//
//   For our iterators here, we probably never have to change the range inside the iter.
//   So we can treat all our iterators as propagating const.
//
//   If an iter propagates const so you cannot change the range, then the range is "const".
//   If a container is "const" then it only produces const-propagating iters.
//   But you can copy the "const" container and change that.
//
//   How can you change a range?
//     Change the begin / end positions.
//     Change the count (same as changing the end position).
//
//     An iter contains a stride and range. The count in the range must be <= the stride.
//
// _______________________________________________________________________________________________
// Change iterator resolution
//   The line_walker_type object gives us a way to change the resolution of an iterator.
//   It is not an iterator itself. It is an object that fits between a src and trg iterator,
//   changing the resolution of the src to fit the trg.
//
// _______________________________________________________________________________________________
// Ganged iterators
//   Sometimes you want to walk several iterators at the same time.
//   They usually have the same count, but if they didn't you could use line_walker_type to change
//   the resolution, or you could use a min-length.
//   This would be part of the solution to a window iterator group.
//
//   Kinds of iterator grouping:
//     Ganged iterators, incrementing the overall object increments every child iterator.
//     Hierarchy (nested) iterators, where one fits inside the other.
//     Flat multi-dimensional iterators.
//     Window iterators, that keep track of leading/trailing values.
//     Resolution changing iterators. See line_walker_type.
//     Flattening (hierarchy) iters:
//       Flattens a nested series of iterators into a single iterator.
//       The stride iters work like this. The top layer returns a range, which gives you a begin/end
//       iter into the next layer, which gives you another range, etc. For each higher iter you need
//       a way to get a begin/end iter to the next level.
//
//   It would be easy to implement a ganged iterator pair to experiment with.
//   The size of the gang is often known at compile time, and is often just 2 (a pair).
//
// _______________________________________________________________________________________________
// Window iterator
//
//   With a window iterator, you remember trailing iterators (after they are passed), and you
//   can look at leading values (that are coming up next).
//
//   You could use a circular buffer to implement this.
//   You could make the leading and trailing lengths fixed at compile, or runtime-settable.
//     2-wide windows are the most common, either {previous, current} or {current, next}.
//
//   The following is like scan_children(..) except it scans a 2-wide window.
//   Instead of just one child, you have two, called lo and hi. Although trailing and leading
//   are probably better names.
//
//   scan_children_2_window( range_type const & range, funct)
//   {
//     size_type count = range.get_count( );
//     d_assert( count >= 2);
//
//     iter_type iter_lo = range.get_iter_lo( );
//     for ( ; ; ) {
//       iter_type const iter_hi = iter_lo + 1;
//
//       funct( *iter_lo, *iter_hi);
//
//       if ( 2 == count ) break;
//       count -= 1;
//       iter_lo = iter_hi;
//     }
//   }
//
// _______________________________________________________________________________________________
// Other functions to consider:
//
//   get_reset< iter_type >( )     -> iter_type
//   is_reset< iter_type >( iter)  -> bool
//   compare( iter_a, iter_b)      -> int
//   swap( iter_a, iter_b)         -> void
//   hash_value( iter)             -> std::size_t
//   min( iter_a, iter_b)          -> iter_type
//   max( iter_a, iter_b)          -> iter_type
//
// _______________________________________________________________________________________________

# include "all.h"
# include <iterator>

// _______________________________________________________________________________________________
// Iterator adaptor classes defined in this file

template< typename LEAF_ITER_T, size_t DEPTH = 0 > class stride_range ;
template< typename LEAF_ITER_T, size_t DEPTH = 0 > class stride_iter  ;

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// stride_range< iter_type, depth >
//
//   If we specialize the depth==0 implementation we can provide:
//     is any count zero
//     get_leaf_range - self when depth is zero, otherwise get_next_range( ).get_leaf_range( )
//     ref_leaf_range
//   Maybe the word "leaf" isn't ideal because it already means the wrapped iter.
//   get_term_range, get_terminal_range, get_zero_range, get_innermost_range, ...
//
//   We need the == and != operators.
//     Maybe also is_overlap and is_enclosed_by.
//     Look at the functions for a rectangle (or cube). They are ranges too.

  template< typename LEAF_ITER_T, size_t DEPTH >
  class
stride_range
{
  // These friend classes are allowed access to the inner workings.
  // We include stride_range<..> here to include stride_range< other_leaf_iter, other_depth >.
  template < typename, size_t > friend class stride_range;
  template < typename, size_t > friend class stride_iter_base;

  // -------------------------------------------------------------------------------------------
  // Typedefs
  public:
    static size_t const depth = DEPTH;

  public:
    typedef LEAF_ITER_T                              leaf_iter_type   ;
    typedef stride_iter<  leaf_iter_type, depth >    stride_iter_type ;
  private:
    typedef stride_range< leaf_iter_type, depth >    this_type        ;

  private:
    struct private_null { };
  public:
    typedef
      typename
      boost::mpl::if_c
       <  (depth == 0)
        , private_null
        , stride_range< leaf_iter_type, depth - 1 >
       >::type                                       next_range_type  ;

  // Types from stride_iter_type
  public:
    typedef typename stride_iter_type::diff_type     diff_type        ; // signed
    typedef typename stride_iter_type::val_type      val_type         ;
    typedef typename stride_iter_type::val_ptr_type  val_ptr_type     ;
    typedef typename stride_iter_type::val_ref_type  val_ref_type     ;

  // Standard-library typedefs for containers
  // This is missing the following std-lib container typedefs:
  //   const_pointer
  //   const_reference
  //   const_iterator
  //   const_reverse_iterator
  //   allocator_type
  public:
    typedef size_t                                   size_type        ; // unsigned
    typedef diff_type                                difference_type  ; // signed

    typedef stride_iter_type                         iterator         ;
    typedef stride_iter_type                         reverse_iterator ;

    typedef val_type                                 value_type       ;
    typedef val_ptr_type                             pointer          ; // unnecessary
    typedef val_ref_type                             reference        ;

  // -------------------------------------------------------------------------------------------
  // Constructors
  //   Change this when we get variadic template params and r-values.
  //   If the trailing param is a tuple it will be expanded.
  //
  //   We use setters for construction which simplifies some things but also means leaf_iter_type
  //   cannot be const and cannot be a ref. And leaf_iter_type is always default constructed, and
  //   always assigned with operator=().
  public:
    /* ctor */        stride_range( )           { set( ); }

                        template< typename A0 >
    /* ctor */        stride_range( A0 const & a0)
                                                { set( a0); }

                        template< typename A0, typename A1 >
    /* ctor */        stride_range( A0 const & a0, A1 const & a1)
                                                { set( a0, a1); }

                        template< typename A0, typename A1, typename A2 >
    /* ctor */        stride_range( A0 const & a0, A1 const & a1, A2 const & a2)
                                                { set( a0, a1, a2); }

                        template< typename A0, typename A1, typename A2, typename A3 >
    /* ctor */        stride_range( A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3)
                                                { set( a0, a1, a2, a3); }

                        template< typename A0, typename A1, typename A2, typename A3, typename A4 >
    /* ctor */        stride_range( A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4)
                                                { set( a0, a1, a2, a3, a4); }

                        template< typename A0, typename A1, typename A2, typename A3, typename A4, typename A5 >
    /* ctor */        stride_range( A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5)
                                                { set( a0, a1, a2, a3, a4, a5); }

                        template< typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6 >
    /* ctor */        stride_range( A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6)
                                                { set( a0, a1, a2, a3, a4, a5, a6); }

  // -------------------------------------------------------------------------------------------
  // Assignment operator
  //   This works with tuples, so you can assign 0..6 params with operator=(a).
  public:
                        template< typename ARG_T >
    this_type &       operator =( ARG_T const & a)
                                                { set( a); return *this; }

  // -------------------------------------------------------------------------------------------
  // Copy setter
  public:
                        template< typename ALT_LEAF_ITER_T >
    void              set( stride_range< ALT_LEAF_ITER_T, depth > const & copy)
                                                { set( copy.get_count( ), copy.get_iter( )); }

  // -------------------------------------------------------------------------------------------
  // set( ), no params
  public:
    void              set( )                    { set_count( ); ref_iter( ).set( ); }
    void              set( tuple< > const &)    { set( ); }

  // -------------------------------------------------------------------------------------------
  // set(..), one param
  public:
    // We want "const &" here so this works with literals.
    // Should we overload all the signed/unsigned integer types?
    void              set( size_type const & s) { set_count( s); ref_iter( ).set( ); }

                        template< typename A0 >
    void              set( A0 const & a0)       { set_count( ); ref_iter( ).set( a0); }

                        template< typename A0 >
    void              set( tuple< A0 > const & tu)
                                                { set( tu.get<0>( )); }

  // -------------------------------------------------------------------------------------------
  // set(..), 2 params
  public:
                        template< typename A0 >
    void              set( size_type const & s, A0 const & a0)
                                                { set_count( s); ref_iter( ).set( a0); }

                        template< typename A0, typename A1 >
    void              set( A0 const & a0, A1 const & a1)
                                                { set_count( ); ref_iter( ).set( a0, a1); }

                        template< typename A0, typename A1 >
    void              set( tuple< A0, A1 > const & tu)
                                                { set( tu.get<0>( ), tu.get<1>( )); }


  // -------------------------------------------------------------------------------------------
  // set(..), 3 params
  public:
                        template< typename A0, typename A1 >
    void              set( size_type const & s, A0 const & a0, A1 const & a1)
                                                { set_count( s); ref_iter( ).set( a0, a1); }

                        template< typename A0, typename A1, typename A2 >
    void              set( A0 const & a0, A1 const & a1, A2 const & a2)
                                                { set_count( ); ref_iter( ).set( a0, a1, a2); }

                        template< typename A0, typename A1, typename A2 >
    void              set( tuple< A0, A1, A2 > const & tu)
                                                { set( tu.get<0>( ), tu.get<1>( ), tu.get<2>( )); }

  // -------------------------------------------------------------------------------------------
  // set(..), 4 params
  public:
                        template< typename A0, typename A1, typename A2 >
    void              set( size_type const & s, A0 const & a0, A1 const & a1, A2 const & a2)
                                                { set_count( s); ref_iter( ).set( a0, a1, a2); }

                        template< typename A0, typename A1, typename A2, typename A3 >
    void              set( A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3)
                                                { set_count( ); ref_iter( ).set( a0, a1, a2, a3); }

                        template< typename A0, typename A1, typename A2, typename A3 >
    void              set( tuple< A0, A1, A2, A3 > const & tu)
                                                { set( tu.get<0>( ), tu.get<1>( ), tu.get<2>( ), tu.get<3>( )); }

  // -------------------------------------------------------------------------------------------
  // set(..), 5 params
  public:
                        template< typename A0, typename A1, typename A2, typename A3 >
    void              set( size_type const & s, A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3)
                                                { set_count( s); ref_iter( ).set( a0, a1, a2, a3); }

                        template< typename A0, typename A1, typename A2, typename A3, typename A4 >
    void              set( A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4)
                                                { set_count( ); ref_iter( ).set( a0, a1, a2, a3, a4); }

                        template< typename A0, typename A1, typename A2, typename A3, typename A4 >
    void              set( tuple< A0, A1, A2, A3, A4 > const & tu)
                                                { set( tu.get<0>( ), tu.get<1>( ), tu.get<2>( ), tu.get<3>( ), tu.get<4>( )); }

  // -------------------------------------------------------------------------------------------
  // set(..), 6 params
  public:
                        template< typename A0, typename A1, typename A2, typename A3, typename A4 >
    void              set( size_type const & s, A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4)
                                                { set_count( s); ref_iter( ).set( a0, a1, a2, a3, a4); }

                        template< typename A0, typename A1, typename A2, typename A3, typename A4, typename A5 >
    void              set( tuple< A0, A1, A2, A3, A4, A5 > const & tu)
                                                { set( tu.get<0>( ), tu.get<1>( ), tu.get<2>( ), tu.get<3>( ), tu.get<4>( ), tu.get<5>( )); }

  // -------------------------------------------------------------------------------------------
  // Consistency check
  //   Improvement: Additional methods:
  //     get_min_leaf_count( )    return the max of (range.count * iter.stride)
  //     check that pre- and post-iters will be legal and not overflow the address space
  //       this could be part of is_consistent( )
  public:
    // Checks that every count/stride pair either fits into the stride before/after it, or the
    // before/after count/stride pairs fit into this stride. Checks all included ranges.
    // So maybe this should be called are_counts_stride_properly_nested( ).
    // It'd be better if this could confirm that all pairs conform to this check, and not just
    // adjacent pairs.
    bool              is_consistent( )    const { return get_iter( ).is_consistent( get_count( )); }

  // -------------------------------------------------------------------------------------------
  // Standard-library interface
  public:
    // ---------------------------------------------------------------------------------------
    // We don't define the following because this is not a true container, but rather a
    // slice or reference-generator into another container:
    //
    //   max_size( )
    //   capacity( )
    //   resize( n)
    //   reserve( n)

    bool              empty( )            const { return is_empty( ); }
    size_type         count( )            const { return get_count( ); }

    // ---------------------------------------------------------------------------------------
    // This is not how the std begin/end methods are supposed to be defined, but this works
    // for most templated functions for std containers.
    //
    // The methods are supposed to look like this:
    //
    //   iterator                begin( )         ;
    //   const_iterator          begin( )   const ;
    //   iterator                end( )           ;
    //   const_iterator          end( )     const ;
    //
    //   reverse_iterator        rbegin( )        ;
    //   const_reverse_iterator  rbegin( )  const ;
    //   reverse_iterator        rend( )          ;
    //   const_reverse_iterator  rend( )    const ;

    iterator          begin( )            const { return get_iter_lo( ); }
    iterator          end( )              const { return get_iter_post( ); }

    reverse_iterator  rbegin( )           const { stride_iter_type iter_hi = get_iter_hi( );
                                                  iter_hi.reverse_stride( );
                                                  return iter_hi;
                                                }
    reverse_iterator  rend( )             const { stride_iter_type iter_pre = get_iter_pre( );
                                                  iter_pre.reverse_stride( );
                                                  return iter_pre;
                                                }

    // ---------------------------------------------------------------------------------------
    // The following std container methods can be defined as follows, but these definitions are
    // misleading and can lead to programming errors. They only make real sense when (depth == 0).
    //
    // These signatures are supposed to look like this:
    //
    //   reference        operator []( size_type index)        ;
    //   const_reference  operator []( size_type index)  const ;
    //   reference        at( size_type index)                 ;
    //   const_reference  at( size_type index)           const ;
    //
    //   reference        front( )        ;
    //   const_reference  front( )  const ;
    //   reference        back( )         ;
    //   const_reference  back( )   const ;

  # if 0
    // Return const values to discourage using these as l-values.
    value_type const  operator []( size_type index)
                                          const { d_assert( index < get_count( ));
                                                  return *(begin( ) + index);
                                                }
    value_type const  at( size_type index)
                                          const { if ( index >= get_count( ) ) {
                                                    throw std::out_of_range;
                                                  }
                                                  return (*this)[ index];
                                                }

    // We could get front( ) to work (at all depth levels) because we store an actual copy
    // of the first iterator and the range below it. All the other iterators in this "container"
    // are calculated, so we cannot return refs.
    // But we don't want to return a ref, unless it's a const ref. It reveals too much about
    // the inner workings.
    //reference       front( )            const { return ref_iter_lo( ).ref( ); }
    value_type const  front( )            const { return get_iter_lo( ).get( ); }
    value_type const  back(  )            const { return get_iter_hi( ).get( ); }
  # endif

  // -------------------------------------------------------------------------------------------
  // Iter manipulation methods
  //   We should also have methods for:
  //     set_iter_lo_hold_hi_steady(..)
  //     set_iter_hi_hold_lo_steady(..)
  //     set_iters_lo_hi(..)
  //     set_iters_lo_hi_stride(..)
  //     set_iter_lo_count_stride(..)
  //     set_iter_hi_count_stride(..)
  //     set_count_stride_hold_lo_steady(..)
  //       .. etc ..
  public:
    // This also negates the stride. The size does not change.
    void              swap_lo_hi_iters( )       { ref_iter_lo( ) = get_iter_hi( );
                                                  ref_iter_lo( ).reverse_stride( );
                                                }

    bool              restrict_range_by_index
                       (  size_type  lo
                        , size_type  hi_plus
                       )                        { if ( (lo < hi_plus) && (hi_plus <= get_count( )) ) {
                                                    // Do this part first because inc_iter_lo(..) changes get_count( ).
                                                    if ( hi_plus < get_count( ) ) { dec_iter_hi( get_count( ) - hi_plus); }
                                                    if ( lo      > 0            ) { inc_iter_lo( lo); }
                                                    return true;
                                                  }
                                                  return false;
                                                }

    void              dec_iter_lo( diff_type dist = 1)
                                                { inc_iter_lo( - dist); }
    void              inc_iter_lo( diff_type dist = 1)
                                                { inc_iter_hi( - dist);
                                                  ref_iter_lo( ) += dist;
                                                }
    void              dec_iter_hi( diff_type dist = 1)
                                                { inc_iter_hi( - dist); }
    void              inc_iter_hi( diff_type dist = 1)
                                                { d_assert( (dist >= 0) ?
                                                    (get_count( ) <= (get_count( ) + static_cast< size_type >( dist))) :
                                                    (get_count( ) >= static_cast< size_type >( - dist)) );
                                                  ref_count( ) += dist;
                                                }

  // -------------------------------------------------------------------------------------------
  // Next range - not defined when depth==0
  public:
    next_range_type const &
                      get_next_range( )   const { return get_iter( ).get_range( ); }
    next_range_type & ref_next_range( )         { return ref_iter( ).ref_range( ); }

  // -------------------------------------------------------------------------------------------
  // Leaf iters
  public:
    leaf_iter_type const &
                      get_leaf_iter( )    const { return get_iter( ).get_leaf_iter( ); }
  protected:
    leaf_iter_type &  ref_leaf_iter( )          { return ref_iter( ).ref_leaf_iter( ); }

  // -------------------------------------------------------------------------------------------
  // Iters lo and hi
  protected:
    stride_iter_type &
                      ref_iter_lo( )            { d_assert( not_empty( ));
                                                  return ref_iter( );
                                                }
  public:
    stride_iter_type const &
                      get_iter_lo( )      const { d_assert( not_empty( ));
                                                  return get_iter( );
                                                }
    stride_iter_type  get_iter_hi( )      const { return get_iter_lo( ) + (get_count( ) - 1); }
    stride_iter_type  get_iter_post( )    const { return get_iter_lo( ) + get_count( ); }
    stride_iter_type  get_iter_pre( )     const { return get_iter_lo( ) - 1; }

  // -------------------------------------------------------------------------------------------
  // Iter property
  //   We don't want the user manipulating the inner iterator, although we do let the user
  //   reach past the iterator with ref_next_range( ).
  //   This is an inner-workings prop.
  protected:
    stride_iter_type const &  get_iter( ) const { return iter_; }
    stride_iter_type       &  ref_iter( )       { return iter_; }

  // -------------------------------------------------------------------------------------------
  // Count property
  public:
    bool              is_empty( )         const { return get_count( ) == 0; }
    bool              not_empty( )        const { return get_count( ) != 0; }

    size_type         get_count( )        const { return count_; }
  protected:
    size_type &       ref_count( )              { return count_; }

    void              set_count( size_type c)   { ref_count( ) = c; }
    void              set_count( )              { ref_count( ) = 0; /* zero is the default count */ }

  public:
    void              set_count_hold_lo_steady( size_type c)
                                                { set_count( c); }
    void              set_count_hold_hi_steady( size_type c)
                                                { if ( get_count( ) > c ) {
                                                    inc_iter_lo( get_count( ) - c);
                                                  } else
                                                  if ( c > get_count( ) ) {
                                                    dec_iter_lo( c - get_count( ));
                                                  }
                                                  d_assert( get_count == c);
                                                }

  // -------------------------------------------------------------------------------------------
  // Stride property
  //   Although the stride resides in the iter object underneath, is it a property of this range.
  public:
    diff_type         get_stride( )       const { return get_iter( ).get_stride( ); }

    void              set_stride_hold_lo_steady( diff_type s)
                                                { ref_iter( ).set_stride( s); }
    void              set_stride_hold_hi_steady( diff_type s)
                                                { if ( get_stride( ) != s ) {
                                                    swap_lo_hi_iters( );
                                                    set_stride_hold_lo_steady( -s);
                                                    swap_lo_hi_iters( );
                                                } }

  // -------------------------------------------------------------------------------------------
  // Member vars
  private:
    size_type         count_ ;
    stride_iter_type  iter_  ;

    // When count is zero, either:
    //   leaf_iter is uninitialized (random bits), or
    //   leaf_iter is reset (a zero pointer), or
    //   leaf_iter is a legitimate pointer
    //   leaf_iter is a pre or post pointer
    //
    // Whenever leaf_iter is reset or un-init, count should be zero and we should not let count be changed.
    // If leaf_iter is a legit pointer, we should allow count to change and even be set to zero.
    // If leaf_iter is a post pointer, count should be zero.
    // leaf_iter should never be a pre pointer wrt the stride_iter.
    //   But it can be a pre-ptr wrt the leaf_iter_type when stride_iter has a negative stride.
    //
    // Reset (and junk) pointers should not be inc/dec.
    // Legit pointers can be inc/dec, although they may become pre/post.
    // Pre pointers should not be decremented.
    // Post pointers should not be incremented.
    // A legit pointer should probably not be deref'd after it has been inc'd until after it has been
    //   verified by comparing it to a post iter. You can also verify by looking at the orignal container,
    //   or by comparing to a count.

}; /* end class stride_range< leaf_iter_type, depth > */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// stride_iter_base_base< iter_type, val_type, val_ptr_type, val_ref_type, depth >
//
//   Instead of inheriting from std::iterator<..> this should probably inherit from
//   boost::iterator_facade<..>. Or maybe even from boost::iterator_adaptor<..>.
//   stride_iter< T, 0 > is like an iterator_adaptor.
//   stride_iter< T, D > where D is not zero probably fits iterator_facade<..> the best.

  template
   <  typename LEAF_ITER_T
    , typename VAL_T
    , typename VAL_PTR_T
    , typename VAL_REF_T
    , size_t   DEPTH
   >
  class
stride_iter_base_base
  : public
    std::iterator
     <  typename std::iterator_traits< LEAF_ITER_T >::iterator_category
      , VAL_T                                      // value_type
      , typename std::iterator_traits< LEAF_ITER_T >::difference_type
      , VAL_PTR_T                                  // pointer
      , VAL_REF_T                                  // reference
     >
{
  // -------------------------------------------------------------------------------------------
  // Typedefs

  // Inherited from supertype:
  //   iterator_catagory
  //   difference_type
  //   value_type
  //   pointer
  //   reference
  public:
    typedef LEAF_ITER_T                             leaf_iter_type    ;
  private:
    typedef stride_iter_base_base
             <  leaf_iter_type
              , VAL_T
              , VAL_PTR_T
              , VAL_REF_T
              , DEPTH
             >                                      this_type         ;
    typedef std::iterator
             <  typename std::iterator_traits< leaf_iter_type >::iterator_category
              , VAL_T      // value_type
              , typename std::iterator_traits< leaf_iter_type >::difference_type
              , VAL_PTR_T  // pointer
              , VAL_REF_T  // reference
             >                                      super_type        ;
  public:
    typedef typename super_type::difference_type    difference_type   ;
    typedef typename super_type::value_type         value_type        ;
    typedef typename super_type::pointer            pointer           ;
    typedef typename super_type::reference          reference         ;
    typedef typename super_type::iterator_category  iterator_category ;

    typedef size_t                                  size_type         ;
    typedef difference_type                         diff_type         ;
    typedef value_type                              val_type          ;
    typedef pointer                                 val_ptr_type      ;
    typedef reference                               val_ref_type      ;

        BOOST_MPL_ASSERT(( boost::is_same< VAL_T    , val_type     >));
        BOOST_MPL_ASSERT(( boost::is_same< VAL_PTR_T, val_ptr_type >));
        BOOST_MPL_ASSERT(( boost::is_same< VAL_REF_T, val_ref_type >));

  public:
    static size_t    const   depth          = DEPTH;
    static diff_type const   default_stride = 1;

  // -------------------------------------------------------------------------------------------
  // Constructor
  public:
    /* ctor */   stride_iter_base_base( )       { set_stride( ); }

  // -------------------------------------------------------------------------------------------
  // Stride getters/setters
  public:
    bool         is_stride_negative( )    const { return get_stride( ) < 0; }
    bool         is_stride_positive( )    const { return get_stride( ) > 0; }
    bool         is_stride_zero( )        const { return get_stride( ) == 0; }
    bool         not_stride_zero( )       const { return get_stride( ) != 0; }

  public:
    diff_type    get_stride( )            const { return stride_; }

    diff_type &  ref_stride( )                  { return stride_; }
    void         set_stride( diff_type s)       { ref_stride( ) = s; }
    void         set_stride( )                  { ref_stride( ) = default_stride; }
    void         reverse_stride( )              { ref_stride( ) = - get_stride( ); }

  // -------------------------------------------------------------------------------------------
  // Private member var
  private:
    diff_type  stride_ ;

}; /* end class stride_iter_base_base< leaf_iter_type, val_type, val_ptr_type, val_ref_type, depth > */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// stride_iter_base< iter_type, depth >

  template< typename LEAF_ITER_T, size_t DEPTH >
  class
stride_iter_base
  : public
    stride_iter_base_base
     <  LEAF_ITER_T
      , stride_range< LEAF_ITER_T, DEPTH - 1 > const     // val_type
      , stride_range< LEAF_ITER_T, DEPTH - 1 > const *   // val_ptr_type
      , stride_range< LEAF_ITER_T, DEPTH - 1 > const &   // val_ref_type
      , DEPTH
     >
{
  // -------------------------------------------------------------------------------------------
  // Typedefs
  //   Inner working: the range type that we store and calculate in this iter (when depth!=0).
  private:
    typedef stride_iter_base< LEAF_ITER_T, DEPTH >  this_type         ;
    typedef stride_iter_base_base
             <  LEAF_ITER_T
              , stride_range< LEAF_ITER_T, DEPTH - 1 > const     // val_type
              , stride_range< LEAF_ITER_T, DEPTH - 1 > const *   // val_ptr_type
              , stride_range< LEAF_ITER_T, DEPTH - 1 > const &   // val_ref_type
              , DEPTH
             >                                      super_type        ;
  public:
    typedef typename super_type::difference_type    difference_type   ;
    typedef typename super_type::value_type         value_type        ;
    typedef typename super_type::pointer            pointer           ;
    typedef typename super_type::reference          reference         ;
    typedef typename super_type::iterator_category  iterator_category ;

    typedef typename super_type::leaf_iter_type     leaf_iter_type    ;
    typedef typename super_type::size_type          size_type         ;
    typedef typename super_type::diff_type          diff_type         ;
    typedef typename super_type::val_type           val_type          ;
    typedef typename super_type::val_ptr_type       val_ptr_type      ;
    typedef typename super_type::val_ref_type       val_ref_type      ;

    typedef stride_range
             <  leaf_iter_type
              , super_type::depth - 1
             >                                      range_type        ;

  // Make inherited methods visible.
  public:
    super_type::is_stride_negative  ;
    super_type::is_stride_positive  ;
    super_type::is_stride_zero      ;
    super_type::not_stride_zero     ;
    super_type::get_stride          ;
    super_type::ref_stride          ;
    super_type::set_stride          ;
    super_type::reverse_stride      ;

  // -------------------------------------------------------------------------------------------
  // Setters, set_next(..) with 0 and 1 arg
  protected:
    void         set_next( )                        { ref_next( ).set( ); }

                   template< typename A0 >
    void         set_next( A0 const & a0)           { ref_next( ).set( a0); }

  // -------------------------------------------------------------------------------------------
  // Consistency check
  public:
    bool         is_consistent( size_t count_above)  const
                     { // Addition: We could also make sure (stride * count) fits
                       // in size_t, implying it fits in the address space.
                       size_t const stride_above = std::abs( get_stride( ));
                       size_t const count_below  = get_range( ).get_count( );
                       size_t const stride_below = std::abs( get_range( ).get_iter( ).get_stride( ));
                       return
                         ( ((count_above * stride_above) <= stride_below) ||
                           ((count_below * stride_below) <= stride_above) ) &&
                         get_range( ).is_consistent( );
                     }

  // -------------------------------------------------------------------------------------------
  // Dereference operators
  //
  //   The operator*( ) is not standard as defined here. This iterator doesn't ref back to a
  //   container (except at depth 0), so the notion of "const" vs "const-iterator" is different.
  //
  //   Here are the standard signatures:
  //     reference  operator []( diff_type offset)  const ;
  //     reference  operator *( )                   const ;
  //     pointer    operator ->( )                  const ;
  //
  //   There is a serious flaw with these definitions too. What if you do something like this:
  //     my_stride_iter a( .. );
  //     int count = a->get_count( ); // this is ok
  //     int coun2 = (a + 3)->get_count( ); // this is bad!
  //   Normally this is not a problem if this iter is pointing to an underlying container, but
  //   since that is not the case here this is potentially unsafe.
  public:
  /* get rid of the unsafe definitions */
  # if 0
    range_type const &       operator []( diff_type off)  const { return *((*this) + off); } // very unsafe
    range_type       &       operator []( diff_type off)        { return *((*this) + off); } // very unsafe

    range_type const &       operator *( )                const { return get_range( ); }
    range_type       &       operator *( )                      { return ref_range( ); }

    range_type const *       operator ->( )               const { return & get_range( ); }
    range_type       *       operator ->( )                     { return & ref_range( ); }
  # endif

  /* these are safe although not entirely compatible with std-lib */
  /* return a "const" object to discourage assignment */
    range_type const         operator []( diff_type off)  const { return *((*this) + off); }
    range_type const         operator *( )                const { return get_range( ); }

  // -------------------------------------------------------------------------------------------
  // Primitive getters
  protected:
    range_type const &       get_next( )                  const { return get_range( ); }
    range_type       &       ref_next( )                        { return ref_range( ); }

  public:
    range_type const &       get_range( )                 const { return range_; }
    range_type       &       ref_range( )                       { return range_; }

    leaf_iter_type  const &  get_leaf_iter( )             const { return get_range( ).get_leaf_iter( ); }
    leaf_iter_type        &  ref_leaf_iter( )                   { return ref_range( ).ref_leaf_iter( ); }

  // -------------------------------------------------------------------------------------------
  // Private member var
  private:
    range_type  range_ ;

}; /* end class stride_iter_base< leaf_iter_type, depth > */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// stride_iter_base< leaf_iter_type, 0 >
//
//   Template class specialization.

  template< typename LEAF_ITER_T >
  class
stride_iter_base< LEAF_ITER_T, 0 >
  : public
    stride_iter_base_base
     <  LEAF_ITER_T
      , typename std::iterator_traits< LEAF_ITER_T >::value_type  // val_type
      , typename std::iterator_traits< LEAF_ITER_T >::pointer     // val_ptr_type
      , typename std::iterator_traits< LEAF_ITER_T >::reference   // val_ref_type
      , 0
     >
{
  // -------------------------------------------------------------------------------------------
  // Typedefs
  //   Inner working: the range type that we store and calculate in this iter (when depth!=0).
  private:
    typedef stride_iter_base< LEAF_ITER_T, 0 >      this_type         ;
    typedef stride_iter_base_base
             <  LEAF_ITER_T
              , typename std::iterator_traits< LEAF_ITER_T >::value_type  // val_type
              , typename std::iterator_traits< LEAF_ITER_T >::pointer     // val_ptr_type
              , typename std::iterator_traits< LEAF_ITER_T >::reference   // val_ref_type
              , 0
             >                                      super_type        ;
  public:
    typedef typename super_type::difference_type    difference_type   ;
    typedef typename super_type::value_type         value_type        ;
    typedef typename super_type::pointer            pointer           ;
    typedef typename super_type::reference          reference         ;
    typedef typename super_type::iterator_category  iterator_category ;

    typedef typename super_type::leaf_iter_type     leaf_iter_type    ;
    typedef typename super_type::size_type          size_type         ;
    typedef typename super_type::diff_type          diff_type         ;
    typedef typename super_type::val_type           val_type          ;
    typedef typename super_type::val_ptr_type       val_ptr_type      ;
    typedef typename super_type::val_ref_type       val_ref_type      ;

  // Make inherited methods visible.
  public:
    super_type::is_stride_negative  ;
    super_type::is_stride_positive  ;
    super_type::is_stride_zero      ;
    super_type::not_stride_zero     ;
    super_type::get_stride          ;
    super_type::ref_stride          ;
    super_type::set_stride          ;
    super_type::reverse_stride      ;

  // -------------------------------------------------------------------------------------------
  // Setters, set_next(..) with 0 and 1 arg
  //   If this doesn't work with your leaf-iter type you can specialize this class.
  //   If that turns out to be common, we should define a traits-like class to wrap it.
  protected:
    // We use operator=(..) when there are no args and depth==0.
    void         set_next( )                        { ref_leaf_iter( ) = leaf_iter_type( ); }

    // We use operator=(..) in the one-arg case when depth==0.
                   template< typename A0 >
    void         set_next( A0 const & a0)           { ref_leaf_iter( ) = a0; }

  // -------------------------------------------------------------------------------------------
  // Consistency check
  public:
    bool         is_consistent( size_t count_above)
                                              const { return true; }

  // -------------------------------------------------------------------------------------------
  // Dereference operators
  //
  //   These operators have the standard signatures when depth==0. They don't when depth!=0.
  public:
    reference    operator [ ]( diff_type offset)
                                              const { return get_leaf_iter( )[ get_stride( ) * offset ]; }
    reference    operator *( )                const { return *get_leaf_iter( ); }
    pointer      operator ->( )               const { return get_leaf_iter( ).operator ->( ); }

  // -------------------------------------------------------------------------------------------
  // Primitive getters
  public:
    leaf_iter_type  const &  get_next( )      const { return get_leaf_iter( ); }
    leaf_iter_type        &  ref_next( )            { return ref_leaf_iter( ); }

    leaf_iter_type  const &  get_leaf_iter( ) const { return leaf_iter_; }
    leaf_iter_type        &  ref_leaf_iter( )       { return leaf_iter_; }

  // -------------------------------------------------------------------------------------------
  // Private member var
  private:
    leaf_iter_type  leaf_iter_ ;

}; /* end class specialization stride_iter_base< leaf_iter_type, 0 > */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// stride_iter< iter_type, depth >
//
//   If the inner type is std::vector< int >::iterator, do these tags mess us up?
//     _Float_iterator_tag
//     _Scalar_ptr_iterator_tag
//     _Ptr_cat
//     typedef _Tptr _Inner_type
//   Make sure copy and move algorithms only move elements on the stride, and not all
//   the intermediate objects too.
//
//   Does std::vector<int>::iterator iter; produce an iter with a zero pointer?
//   Or is the pointer full of random bits?
//   If it is zero'd we may want to add some is_reset( ) or is_zero( ) tests to
//   some of the operators. See the comments about get_reset< iter_type >( ) above.

  template< typename LEAF_ITER_T, size_t DEPTH >
  class
stride_iter
  : public stride_iter_base< LEAF_ITER_T, DEPTH >
{
  template < typename, size_t > friend class stride_iter_base;
  template < typename, size_t > friend class stride_iter;

  // -------------------------------------------------------------------------------------------
  // Typedefs
  private:
    typedef stride_iter<      LEAF_ITER_T, DEPTH >     this_type         ;
    typedef stride_iter_base< LEAF_ITER_T, DEPTH >     super_type        ;
  public:
    typedef typename super_type::difference_type       difference_type   ;
    typedef typename super_type::value_type            value_type        ;
    typedef typename super_type::pointer               pointer           ;
    typedef typename super_type::reference             reference         ;
    typedef typename super_type::iterator_category     iterator_category ;

    typedef typename super_type::leaf_iter_type        leaf_iter_type    ;
    typedef typename super_type::size_type             size_type         ;
    typedef typename super_type::diff_type             diff_type         ;
    typedef typename super_type::val_type              val_type          ;
    typedef typename super_type::val_ptr_type          val_ptr_type      ;
    typedef typename super_type::val_ref_type          val_ref_type      ;

  // Make inherited methods visible.
  public:
    super_type::is_stride_negative  ;
    super_type::is_stride_positive  ;
    super_type::is_stride_zero      ;
    super_type::not_stride_zero     ;
    super_type::get_stride          ;
    super_type::ref_stride          ;
    super_type::set_stride          ;
    super_type::reverse_stride      ;

    super_type::is_consistent       ;
    super_type::get_next            ;
    super_type::ref_next            ;
    super_type::get_leaf_iter       ;
    super_type::ref_leaf_iter       ;

  // -------------------------------------------------------------------------------------------
  // Constructors
  //   Change this when we get variadic template params and r-values.
  //   If the trailing param is a tuple it will be expanded.
  public:
    /* ctor */   stride_iter( )                     { set( ); }

                   template< typename A0 >
    /* ctor */   stride_iter( A0 const & a0)        { set( a0); }

                   template< typename A0, typename A1 >
    /* ctor */   stride_iter( A0 const & a0, A1 const & a1)
                                                    { set( a0, a1); }

                   template< typename A0, typename A1, typename A2 >
    /* ctor */   stride_iter( A0 const & a0, A1 const & a1, A2 const & a2)
                                                    { set( a0, a1, a2); }

                   template< typename A0, typename A1, typename A2, typename A3 >
    /* ctor */   stride_iter( A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3)
                                                    { set( a0, a1, a2, a3); }

                   template< typename A0, typename A1, typename A2, typename A3, typename A4 >
    /* ctor */   stride_iter( A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4)
                                                    { set( a0, a1, a2, a3, a4); }

                   template< typename A0, typename A1, typename A2, typename A3, typename A4, typename A5 >
    /* ctor */   stride_iter( A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5)
                                                    { set( a0, a1, a2, a3, a4, a5); }

                   template< typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6 >
    /* ctor */   stride_iter( A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4, A5 const & a5, A6 const & a6)
                                                    { set( a0, a1, a2, a3, a4, a5, a6); }

  // -------------------------------------------------------------------------------------------
  // Assignment operator
  //   This works with tuples, so you can assign 0..6 params with operator=(a).
  public:
                   template< typename ARG_T >
    this_type &  operator =( ARG_T const & a)       { set( a); return *this; }

  // -------------------------------------------------------------------------------------------
  // Reset
  //   is_reset( ) doesn't work when the leaf iterator null-inits with random bits.
  //   Like it does when it's a raw pointer. So don't use it in generic algorithms that you
  //   want to have work with raw pointers.
  //
  //   Because of this we should probably call an is_reset( ) function on the leaf-iter
  //   instead of just comparing it. Then you can only use it with leaf iters that provide
  //   an explicit is_reset( ) function, or a function with an operator bool( ).
  //
  //   Traits:
  //     has bool test
  //       operator safe_bool
  //     has reset
  //       a = my_type( );
  //       a = my_type( tuple<>( ));
  //       a = tuple<>( );
  //       a.set( )
  //     is initializtion consistent
  //       This is not the same as ! has_random_bits.
  //       It means the default ctor should always produce an == and reset object.
  //         my_type a;
  //         my_type b = my_type( );
  //         my_type c = my_type( tuple<>( ));
  //         my_type d; d = a;
  //         my_type e; e = tuple<>( );
  //         my_type g; f.set( );
  //         my_type g; g.set( a);
  //         my_type h; h.set( tuple<>( ));
  //       All these objects must be == (a==b)&&(a==c)&&...
  //       All these objects should test false:
  //         d_assert( (! a) && (a ? false : true));
  public:
  # if 0
    void         reset( )                           { set( ); }
    //bool       is_set( )                    const { return (*this) ? true : false; }
    //bool       is_reset( )                  const { return ! is_set( ); }
    //           operator safe_bool_type( )   const { return (test_compoent_0 && test_component_1) ? (& this_type::private_member_) : 0; }
    //safe_bool_type
    //           operator !( )                const { .. should we define this? .. }
    bool         is_reset( )                  const { return (*this) == this_null_value; }
    bool         not_reset( )                 const { return ! is_reset( ); }
  private:
    static this_type const this_null_value = this_type( ); - must be done outside of class{..} wrapper
  # endif

  // -------------------------------------------------------------------------------------------
  // set_next(..), 0..5 args
  protected:
    void         set_next( )                        { super_type::set_next( ); }

                   template< typename A0 >
    void         set_next( A0 const & a0)           { super_type::set_next( a0); }

                   template< typename A0, typename A1 >
    void         set_next( A0 const & a0, A1 const & a1)
                                                    { ref_next( ).set( a0, a1); }

                   template< typename A0, typename A1, typename A2 >
    void         set_next( A0 const & a0, A1 const & a1, A2 const & a2)
                                                    { ref_next( ).set( a0, a1, a2); }

                   template< typename A0, typename A1, typename A2, typename A3 >
    void         set_next( A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3)
                                                    { ref_next( ).set( a0, a1, a2, a3); }

                   template< typename A0, typename A1, typename A2, typename A3, typename A4 >
    void         set_next( A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4)
                                                    { ref_next( ).set( a0, a1, a2, a3, a4); }

  // -------------------------------------------------------------------------------------------
  // Copy setter
  public:
                   template< typename ALT_LEAF_ITER_T >
    void         set( stride_iter< ALT_LEAF_ITER_T, DEPTH > const & copy)
                                                    { set( copy.get_stride( ), copy.get_next( )); }

  // -------------------------------------------------------------------------------------------
  // set( ), no params
  public:
    void         set( )                             { set_stride( ); set_next( ); }
    void         set( tuple< > const &)             { set( ); }

  // -------------------------------------------------------------------------------------------
  // set(..), one param
  public:
    // We want "const &" here so this works with literals.
    // Should we overload all the signed/unsigned integer types?
    void         set( diff_type const & s)          { set_stride( s); set_next( ); }

                   template< typename A0 >
    void         set( A0 const & a0)                { set_stride( ); set_next( a0); }

                   template< typename A0 >
    void         set( tuple< A0 > const & tu)       { set( tu.get<0>( )); }

  // -------------------------------------------------------------------------------------------
  // set(..), 2 params
  public:
                   template< typename A0 >
    void         set( diff_type const & s, A0 const & a0)
                                                    { set_stride( s); set_next( a0); }

                   template< typename A0, typename A1 >
    void         set( A0 const & a0, A1 const & a1)
                                                    { set_stride( ); set_next( a0, a1); }

                   template< typename A0, typename A1 >
    void         set( tuple< A0, A1 > const & tu)   { set( tu.get<0>( ), tu.get<1>( )); }


  // -------------------------------------------------------------------------------------------
  // set(..), 3 params
  public:
                   template< typename A0, typename A1 >
    void         set( diff_type const & s, A0 const & a0, A1 const & a1)
                                                    { set_stride( s); set_next( a0, a1); }

                   template< typename A0, typename A1, typename A2 >
    void         set( A0 const & a0, A1 const & a1, A2 const & a2)
                                                    { set_stride( ); set_next( a0, a1, a2); }

                   template< typename A0, typename A1, typename A2 >
    void         set( tuple< A0, A1, A2 > const & tu)
                                                    { set( tu.get<0>( ), tu.get<1>( ), tu.get<2>( )); }

  // -------------------------------------------------------------------------------------------
  // set(..), 4 params
  public:
                   template< typename A0, typename A1, typename A2 >
    void         set( diff_type const & s, A0 const & a0, A1 const & a1, A2 const & a2)
                                                    { set_stride( s); set_next( a0, a1, a2); }

                   template< typename A0, typename A1, typename A2, typename A3 >
    void         set( A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3)
                                                    { set_stride( ); set_next( a0, a1, a2, a3); }

                   template< typename A0, typename A1, typename A2, typename A3 >
    void         set( tuple< A0, A1, A2, A3 > const & tu)
                                                    { set( tu.get<0>( ), tu.get<1>( ), tu.get<2>( ), tu.get<3>( )); }

  // -------------------------------------------------------------------------------------------
  // set(..), 5 params
  public:
                   template< typename A0, typename A1, typename A2, typename A3 >
    void         set( diff_type const & s, A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3)
                                                    { set_stride( s); set_next( a0, a1, a2, a3); }

                   template< typename A0, typename A1, typename A2, typename A3, typename A4 >
    void         set( A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4)
                                                    { set_stride( ); set_next( a0, a1, a2, a3, a4); }

                   template< typename A0, typename A1, typename A2, typename A3, typename A4 >
    void         set( tuple< A0, A1, A2, A3, A4 > const & tu)
                                                    { set( tu.get<0>( ), tu.get<1>( ), tu.get<2>( ), tu.get<3>( ), tu.get<4>( )); }

  // -------------------------------------------------------------------------------------------
  // set(..), 6 params
  public:
                   template< typename A0, typename A1, typename A2, typename A3, typename A4 >
    void         set( diff_type const & s, A0 const & a0, A1 const & a1, A2 const & a2, A3 const & a3, A4 const & a4)
                                                    { set_stride( s); set_next( a0, a1, a2, a3, a4); }

                   template< typename A0, typename A1, typename A2, typename A3, typename A4, typename A5 >
    void         set( tuple< A0, A1, A2, A3, A4, A5 > const & tu)
                                                    { set( tu.get<0>( ), tu.get<1>( ), tu.get<2>( ), tu.get<3>( ), tu.get<4>( ), tu.get<5>( )); }

  // -------------------------------------------------------------------------------------------
  // Subtraction support
  //   These should check consistency.
  //   You cannot subtract a zero pointer and a non-zero pointer.
  //     But there is no general way to test for "zero". Some iterators don't even support a zero (reset) state.
  //   The pointers should be properly aligned.
  //     If you are getting a stride difference they should be stride aligned.
  public:
    diff_type    get_leaf_difference_with( leaf_iter_type const & b)
                                              const { return get_leaf_iter( ) - b; }
    diff_type    get_stride_difference_with( leaf_iter_type const & b)
                                              const { if ( get_leaf_iter( ) == b ) return 0;
                                                      d_assert( is_stride_aligned_with( b));
                                                      return get_leaf_difference_with( b) / get_stride( );
                                                    }
    bool         is_stride_aligned_with( leaf_iter_type const & b)
                                              const { if ( get_leaf_iter( ) == b ) return true;
                                                      d_assert( not_stride_zero( ));
                                                      return 0 ==
                                                        (get_leaf_difference_with( b) % get_stride( ));
                                                    }

  // -------------------------------------------------------------------------------------------
  // Increment/decrement operators
  public:
    this_type    operator ++( int)                  { this_type const save = *this; ++ *this; return save; }
    this_type    operator --( int)                  { this_type const save = *this; -- *this; return save; }

    this_type &  operator ++( )                     { ref_leaf_iter( ) += get_stride( ); return *this; }
    this_type &  operator --( )                     { ref_leaf_iter( ) -= get_stride( ); return *this; }

    this_type &  operator +=( diff_type offset)     { ref_leaf_iter( ) += (get_stride( ) * offset); return *this; }
    this_type &  operator -=( diff_type offset)     { ref_leaf_iter( ) -= (get_stride( ) * offset); return *this; }

  // -------------------------------------------------------------------------------------------
  // Comparison operators
  public:
    // Wrong? This is not comparing the entire iter, just the leaf and stride. This is only right when depth==0.

    //bool         operator ==( this_type const & b)
    //                                          const { d_assert( get_stride( ) == b.get_stride( ));
    //                                                  return get_leaf_iter( ) == b.get_leaf_iter( );
    //                                                }

                   template< typename ALT_LEAF_ITER_TYPE >
    bool         operator ==( stride_iter< ALT_LEAF_ITER_TYPE, DEPTH > const & b)
                                              const { d_assert( get_stride( ) == b.get_stride( ));
                                                      return get_leaf_iter( ) == b.get_leaf_iter( );
                                                    }

    //bool         operator !=( this_type const & b)
    //                                          const { return ! ((*this) == b); }

                   template< typename ALT_LEAF_ITER_TYPE >
    bool         operator !=( stride_iter< ALT_LEAF_ITER_TYPE, DEPTH > const & b)
                                              const { return ! ((*this) == b); }

                   template< typename ALT_LEAF_ITER_TYPE >
    bool         operator <=( stride_iter< ALT_LEAF_ITER_TYPE, DEPTH > const & b)
                                              const { d_assert( get_stride( ) == b.get_stride( ));
                                                      return is_stride_negative( ) ?
                                                        (get_leaf_iter( ) >= b.get_leaf_iter( )) :
                                                        (get_leaf_iter( ) <= b.get_leaf_iter( )) ;
                                                    }
                   template< typename ALT_LEAF_ITER_TYPE >
    bool         operator >=( stride_iter< ALT_LEAF_ITER_TYPE, DEPTH > const & b)
                                              const { d_assert( get_stride( ) == b.get_stride( ));
                                                      return is_stride_negative( ) ?
                                                        (get_leaf_iter( ) <= b.get_leaf_iter( )) :
                                                        (get_leaf_iter( ) >= b.get_leaf_iter( )) ;
                                                    }
                   template< typename ALT_LEAF_ITER_TYPE >
    bool         operator <( stride_iter< ALT_LEAF_ITER_TYPE, DEPTH > const & b)
                                              const { d_assert( get_stride( ) == b.get_stride( ));
                                                      return is_stride_negative( ) ?
                                                        (get_leaf_iter( ) > b.get_leaf_iter( )) :
                                                        (get_leaf_iter( ) < b.get_leaf_iter( )) ;
                                                    }
                   template< typename ALT_LEAF_ITER_TYPE >
    bool         operator >( stride_iter< ALT_LEAF_ITER_TYPE, DEPTH > const & b)
                                              const { d_assert( get_stride( ) == b.get_stride( ));
                                                      return is_stride_negative( ) ?
                                                        (get_leaf_iter( ) < b.get_leaf_iter( )) :
                                                        (get_leaf_iter( ) > b.get_leaf_iter( )) ;
                                                    }

}; /* end class stride_iter< leaf_iter_type, depth > */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// stride_iter< LEAF_ITER_T, DEPTH > operators
//
//   (old_iter + offset) -> new_iter.
//   (offset + old_iter) -> new_iter.
//   (old_iter - offset) -> new_iter.
//   (iter_a - iter_b) -> offset

  // --------------------------------------------------------------
  template< typename LEAF_ITER_T, size_t DEPTH >
  inline
  stride_iter< LEAF_ITER_T, DEPTH >
operator +
  ( stride_iter< LEAF_ITER_T, DEPTH > const &     iter
  , typename
    stride_iter< LEAF_ITER_T, DEPTH >::diff_type  offset
  )
  // (old_iter + offset) -> new_iter.
  // new_iter has the same stride as old_iter.
{
    stride_iter< LEAF_ITER_T, DEPTH > copy = iter;
    copy += offset;
    return copy;
}

  // --------------------------------------------------------------
  template< typename LEAF_ITER_T, size_t DEPTH >
  inline
  stride_iter< LEAF_ITER_T, DEPTH >
operator +
  ( typename
    stride_iter< LEAF_ITER_T, DEPTH >::diff_type  offset
  , stride_iter< LEAF_ITER_T, DEPTH > const &     iter
  )
  // (offset + old_iter) -> new_iter.
  // new_iter has the same stride as old_iter.
  //
  // Addition is commutative in this case.
  // (I do not think this function should be provided. -nealabq)
{
    stride_iter< LEAF_ITER_T, DEPTH > copy = iter;
    copy += offset;
    return copy;
}

  // --------------------------------------------------------------
  template< typename LEAF_ITER_T, size_t DEPTH >
  inline
  stride_iter< LEAF_ITER_T, DEPTH >
operator -
  ( stride_iter< LEAF_ITER_T, DEPTH > const &     iter
  , typename
    stride_iter< LEAF_ITER_T, DEPTH >::diff_type  offset
  )
  // (old_iter - offset) -> new_iter.
  // new_iter has the same stride as old_iter.
  //
  // We do not provide (offset - iter) because what would that mean?
  // Nor do we provide (- iter) or (+ iter).
{
    stride_iter< LEAF_ITER_T, DEPTH > copy = iter;
    copy -= offset;
    return copy;
}

  // --------------------------------------------------------------
  template< typename LEAF_ITER_T0, typename LEAF_ITER_T1, size_t DEPTH0, size_t DEPTH1 >
  inline
  typename stride_iter< LEAF_ITER_T0, 0 >::diff_type
operator -
 (  stride_iter< LEAF_ITER_T0, DEPTH0 > const &  iter_a
  , stride_iter< LEAF_ITER_T1, DEPTH1 > const &  iter_b
 )
  // (iter_a - iter_b) -> offset
  //
  // Answer implies the following, assuming + is commutative.
  //   (iter_a - offset) == iter_b
  //   (iter_a + (- offset)) == iter_b
  //   ((- offset) + iter_a) == iter_b
  // The follow also hold, provided iter_b is the same type as iter_a and
  // (iter_a.get_stride( ) == iter_b.get_stride( )):
  //   (iter_b - iter_a) == (- offset)
  //   (iter_b + offset) == iter_a
  //   (offset + iter_b) == iter_a  // assuming commutative
  //   (iter_b - (- offset)) == iter_a
  //
  // This uses the stride in iter_a and ignores the stride in iter_b.
  // It even works if iter_b is the leaf-iter type, which means its stride is 1.
  // So this does not act like normal subtraction because you can only assume the
  // following when the strides are equal:
  //   (iter_a - iter_b) == - (iter_b - iter_a)
{
    d_assert( iter_a.get_stride( ) == iter_b.get_stride( ));
    return iter_a.get_stride_difference_with( iter_b.get_leaf_iter( ));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Functions, customized with functors
//
//   We should provide lots of options here. We should be able to specify:
//     Iterator windows - leading and trailing values
//     Window overlap
//     Window edge conditions
//     Ordering (unordered, lo->hi, hi->lo, row/col major)
//     If unordered, parallel or serial
//     If parallel, shared memory or message-based (?)
//     Early exit
//
//   We also need to know:
//     Are we working on a frozen snapshot or shifting ground.
//     Can we freely navigate from our current location.
//     Change underlying value(s)? Insert/remove? Is the functor read-only?
//
//   When should functors be ref-types and when should they be copies?
//     It might be a good idea to always assume copies.
//     Refs are useful during scan.
//     Copies are better for parallel operations. Which brings up the question, how do
//       you implement parallel scanning? For example, how do you write a parallel get_max()?
//       Usually you state it like a reduce or filter, although in parallel cases you need a
//       2-level reduce/filter where the leaf functions reduce the end values and the inner
//       reduction functions look at intermediate values or mixtures of intermediate and leaf
//       values. In any case you need to identify the return/intermediate values and not bury
//       them in the functor.
//       You have to think about parallelism any time a functor is mutable.
//       Functors that carry return state are passed around by ref and may have parallel issues.
//       Functors that carry mutable refs (so they can be copied instead of being passed around
//       by ref) may also have parallel issues.
//
//   Our scanning/transforming template functions are sometimes treated like functors, when
//   we are using them with hierarchical or nested iterators.
//
//   When can you pass a function like it's a functor? When it's only used as a function
//   (no members besides operator() are used, it is never ctor'd). Are there restrictions on
//   passing it as a pointer/ref or const/non-const?

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// scan_children(..)
//   This can be a transforming (in-place map) operation, or a reduce, or a select.
//   Improve: We need a way to run this in parallel.
//
//   We pass the functor around by ref here. Mistake?

  template< typename LEAF_ITER_T, size_t DEPTH, typename FUNCT_T >
  void
scan_children
 (  stride_range< LEAF_ITER_T, DEPTH > const &  range
  , FUNCT_T                                  &  funct  // void (& funct)( stride_range<LEAF_ITER_T,DEPTH-1> const &)
 )
{
    if ( range.get_count( ) > 0 ) {
        stride_iter< LEAF_ITER_T, DEPTH >        iter_lo  = range.get_iter_lo( );
        stride_iter< LEAF_ITER_T, DEPTH > const  iter_hi  = range.get_iter_hi( );
        for ( ; ; ) {
            d_assert( iter_lo <= iter_hi);
            funct( *iter_lo);
            if ( iter_lo == iter_hi ) break;
            ++ iter_lo;
        }
    }
}

  template< typename LEAF_ITER_T, typename FUNCT_T >
  void
scan_children
 (  stride_range< LEAF_ITER_T, 0 > const &  range
  , FUNCT_T                              &  funct  // void (&funct)( leaf_iter::reference)
 )
{
    if ( range.get_count( ) > 0 ) {
        stride_iter< LEAF_ITER_T, 0 >        iter_lo  = range.get_iter_lo( );
        stride_iter< LEAF_ITER_T, 0 > const  iter_hi  = range.get_iter_hi( );
        for ( ; ; ) {
            d_assert( iter_lo <= iter_hi);
            funct( *iter_lo);
            if ( iter_lo == iter_hi ) break;
            ++ iter_lo;
        }
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// scan_children_early_exit(..)

  template< typename LEAF_ITER_T, size_t DEPTH, typename FUNCT_T >
  bool /* false if early exit, true otherwise */
scan_children_early_exit
 (  stride_range< LEAF_ITER_T, DEPTH > const &  range
  , FUNCT_T                                  &  funct  // bool (& funct)( stride_range<LEAF_ITER_T,DEPTH-1> const &)
 )
{
    if ( range.get_count( ) > 0 ) {
        stride_iter< LEAF_ITER_T, DEPTH >        iter_lo  = range.get_iter_lo( );
        stride_iter< LEAF_ITER_T, DEPTH > const  iter_hi  = range.get_iter_hi( );
        for ( ; ; ) {
            d_assert( iter_lo <= iter_hi);
            if ( ! funct( *iter_lo) ) return false;
            if ( iter_lo == iter_hi ) break;
            ++ iter_lo;
        }
    }
    return true;
}

  template< typename LEAF_ITER_T, typename FUNCT_T >
  bool /* false if early exit, true otherwise */
scan_children_early_exit
 (  stride_range< LEAF_ITER_T, 0 > const &  range
  , FUNCT_T                              &  funct  // bool (&funct)( leaf_iter::reference)
 )
{
    if ( range.get_count( ) > 0 ) {
        stride_iter< LEAF_ITER_T, 0 >        iter_lo  = range.get_iter_lo( );
        stride_iter< LEAF_ITER_T, 0 > const  iter_hi  = range.get_iter_hi( );
        for ( ; ; ) {
            d_assert( iter_lo <= iter_hi);
            if ( ! funct( *iter_lo) ) return false;
            if ( iter_lo == iter_hi ) break;
            ++ iter_lo;
        }
    }
    return true;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// scan_leaves(..) - stateful functor

  template< typename LEAF_ITER_T, size_t DEPTH, typename FUNCTOR_T >
  struct /* functor */
scan_leaves_functor
  : public scan_leaves_functor< LEAF_ITER_T, DEPTH - 1, FUNCTOR_T >
{
    /* constructor */
  scan_leaves_functor( FUNCTOR_T & f)
    : scan_leaves_functor< LEAF_ITER_T, DEPTH - 1, FUNCTOR_T >( f) { }

    /* function */
    void
  operator ()( stride_range< LEAF_ITER_T, DEPTH > const & range)
    { scan_children( range, *this); }
};

  template< typename LEAF_ITER_T, typename FUNCTOR_T >
  struct /* functor */
scan_leaves_functor< LEAF_ITER_T, 0, FUNCTOR_T >
{
    /* constructor */
  scan_leaves_functor( FUNCTOR_T & f)
    : funct_( f) { }
    FUNCTOR_T & funct_;

    /* function */
    void
  operator ()( stride_range< LEAF_ITER_T, 0 > const & range)
    { scan_children( range, funct_); }
};

  template< typename LEAF_ITER_T, size_t DEPTH, typename FUNCTOR_T >
  void
scan_leaves
 (  stride_range< LEAF_ITER_T, DEPTH > const &  range
  , FUNCTOR_T                                &  funct  // void (&funct)( leaf_iter::reference)
 )
{
    scan_leaves_functor< LEAF_ITER_T, DEPTH, FUNCTOR_T > top_functor( funct);
    top_functor( range);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// scan_leaves_early_exit(..)

  template< typename LEAF_ITER_T, size_t DEPTH, typename FUNCTOR_T >
  struct /* functor */
scan_leaves_ee_functor
  : public scan_leaves_ee_functor< LEAF_ITER_T, DEPTH - 1, FUNCTOR_T >
{
    /* constructor */
  scan_leaves_ee_functor( FUNCTOR_T & f)
    : scan_leaves_ee_functor< LEAF_ITER_T, DEPTH - 1, FUNCTOR_T >( f) { }

    /* function */
    bool
  operator ()( stride_range< LEAF_ITER_T, DEPTH > const & range)
    { return scan_children_early_exit( range, *this); }
};

  template< typename LEAF_ITER_T, typename FUNCTOR_T >
  struct /* functor */
scan_leaves_ee_functor< LEAF_ITER_T, 0, FUNCTOR_T >
{
    /* constructor */
  scan_leaves_ee_functor( FUNCTOR_T & f)
    : funct_( f) { }
    FUNCTOR_T & funct_;

    /* function */
    bool
  operator ()( stride_range< LEAF_ITER_T, 0 > const & range)
    { return scan_children_early_exit( range, funct_); }
};

  template< typename LEAF_ITER_T, size_t DEPTH, typename FUNCTOR_T >
  bool
scan_leaves_early_exit
 (  stride_range< LEAF_ITER_T, DEPTH > const &  range
  , FUNCTOR_T                                &  funct  // bool (&funct)( leaf_iter::reference)
 )
{
    scan_leaves_ee_functor< LEAF_ITER_T, DEPTH, FUNCTOR_T > top_functor( funct);
    return top_functor( range);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// scan_children_with_index(..)

  template< typename LEAF_ITER_T, typename FUNCTOR_T >
  struct /* functor */
scan_children_with_index_functor
{
    FUNCTOR_T &  funct_ ;
    int          index_ ;
    int          inc_   ;

    /* constructor */
  scan_children_with_index_functor( FUNCTOR_T & f, int index = 0, int inc = 1)
    : funct_( f)
    , index_( index)
    , inc_  ( inc)
    { }

    /* function */
    void
  operator ()( typename std::iterator_traits< LEAF_ITER_T >::reference a)
    { funct_( a, index_);
      index_ += inc_;
    }
};

  template< typename LEAF_ITER_T, typename FUNCTOR_T >
  void
scan_children_with_index
 (  stride_range< LEAF_ITER_T, 0 > const &  range
  , FUNCTOR_T                            &  funct // void (&funct)( leaf_iter::reference, index)
  , int                                     init_index  = 0
  , int                                     inc         = 1
 )
{
    scan_children_with_index_functor< LEAF_ITER_T, FUNCTOR_T > wrap_funct( funct, init_index, inc);
    scan_children( range, wrap_funct);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// scan_children_with_index_early_exit(..)

  template< typename LEAF_ITER_T, typename FUNCTOR_T >
  struct /* functor */
scan_children_with_index_ee_functor
{
    FUNCTOR_T &  funct_ ;
    int          index_ ;
    int          inc_   ;

    /* constructor */
  scan_children_with_index_ee_functor( FUNCTOR_T & f, int index = 0, int inc = 1)
    : funct_( f)
    , index_( index)
    , inc_  ( inc)
    { }

    /* function */
    bool
  operator ()( typename std::iterator_traits< LEAF_ITER_T >::reference a)
    { if ( ! funct_( a, index_) ) return false;
      index_ += inc_;
      return true;
    }
};

  template< typename LEAF_ITER_T, typename FUNCTOR_T >
  bool
scan_children_with_index_early_exit
 (  stride_range< LEAF_ITER_T, 0 > const &  range
  , FUNCTOR_T                            &  funct // void (&funct)( leaf_iter::reference, index)
  , int                                     init_index  = 0
  , int                                     inc         = 1
 )
{
    scan_children_with_index_ee_functor< LEAF_ITER_T, FUNCTOR_T > wrap_funct( funct, init_index, inc);
    return scan_children_early_exit( range, wrap_funct);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// scan_leaves_with_2d_index(..)

  template< typename LEAF_ITER_T, typename FUNCTOR_T >
  struct /* multi-functor */
scan_leaves_with_2d_index_functor
{
    /* constructor */
  scan_leaves_with_2d_index_functor
   (  FUNCTOR_T & f
    , int         minor_index_init = 0
    , int         major_index_init = 0
    , int         minor_inc        = 1
    , int         major_inc        = 1
   )
    : funct_            ( f)
    , minor_index_init_ ( minor_index_init)
    , minor_index_      ( minor_index_init)
    , major_index_      ( major_index_init)
    , minor_inc_        ( minor_inc  )
    , major_inc_        ( major_inc  )
    { }

    /* members */
    FUNCTOR_T &  funct_            ;
    int const    minor_index_init_ ;
    int          minor_index_      ;
    int          major_index_      ;
    int const    minor_inc_        ;
    int const    major_inc_        ;

    /* function */
    void
  operator ()( stride_range< LEAF_ITER_T, 0 > const & range)
    { scan_children( range, *this);
      major_index_ += major_inc_;
      minor_index_ = minor_index_init_;
    }

    /* function */
    void
  operator ()( typename std::iterator_traits< LEAF_ITER_T >::reference a)
    { funct_( a, minor_index_, major_index_);
      minor_index_ += minor_inc_;
    }
};

  template< typename LEAF_ITER_T, typename FUNCTOR_T >
  void
scan_leaves_with_2d_index
 (  stride_range< LEAF_ITER_T, 1 > const &  range
  , FUNCTOR_T                            &  funct // void (&funct)( leaf_iter::reference, minor_index, major_index)
  , int                                     init_minor_index  = 0
  , int                                     init_major_index  = 0
  , int                                     inc_minor_index   = 1
  , int                                     inc_major_index   = 1
 )
{
    scan_leaves_with_2d_index_functor< LEAF_ITER_T, FUNCTOR_T >
        wrap_funct( funct,
            init_minor_index, init_major_index,
            inc_minor_index , inc_major_index );
    scan_children( range, wrap_funct);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// scan_leaves_with_2d_index_early_exit(..)

  template< typename LEAF_ITER_T, typename FUNCTOR_T >
  struct /* multi-functor */
scan_leaves_with_2d_index_ee_functor
{
    /* constructor */
  scan_leaves_with_2d_index_ee_functor
   (  FUNCTOR_T & f
    , int         minor_index_init = 0
    , int         major_index_init = 0
    , int         minor_inc        = 1
    , int         major_inc        = 1
   )
    : funct_            ( f)
    , minor_index_init_ ( minor_index_init)
    , minor_index_      ( minor_index_init)
    , major_index_      ( major_index_init)
    , minor_inc_        ( minor_inc  )
    , major_inc_        ( major_inc  )
    { }

    /* members */
    FUNCTOR_T &  funct_            ;
    int const    minor_index_init_ ;
    int          minor_index_      ;
    int          major_index_      ;
    int const    minor_inc_        ;
    int const    major_inc_        ;

    /* function */
    bool
  operator ()( stride_range< LEAF_ITER_T, 0 > const & range)
    { if ( ! scan_children_early_exit( range, *this) ) return false;
      major_index_ += major_inc_;
      minor_index_ = minor_index_init_;
      return true;
    }

    /* function */
    bool
  operator ()( typename std::iterator_traits< LEAF_ITER_T >::reference a)
    { if ( ! funct_( a, minor_index_, major_index_) ) return false;
      minor_index_ += minor_inc_;
      return true;
    }
};

  template< typename LEAF_ITER_T, typename FUNCTOR_T >
  bool
scan_leaves_with_2d_index_early_exit
 (  stride_range< LEAF_ITER_T, 1 > const &  range
  , FUNCTOR_T                            &  funct // bool (&funct)( leaf_iter::reference, minor_index, major_index)
  , int                                     init_minor_index  = 0
  , int                                     init_major_index  = 0
  , int                                     inc_minor_index   = 1
  , int                                     inc_major_index   = 1
 )
{
    scan_leaves_with_2d_index_ee_functor< LEAF_ITER_T, FUNCTOR_T >
        wrap_funct( funct,
            init_minor_index, init_major_index,
            inc_minor_index , inc_major_index );
    return scan_children_early_exit( range, wrap_funct);
}

// _______________________________________________________________________________________________

  template< typename LEAF_ITER_T, size_t DEPTH >
  void
rotate_left_in_place
 (  stride_range< LEAF_ITER_T, DEPTH >               &  range
  , typename stride_range< LEAF_ITER_T, 0 >::size_type  top_count
  , typename stride_range< LEAF_ITER_T, 0 >::diff_type  top_stride
 )
  // Helper function. Implements rotate_left(..), below.
  // Sets the stride and count of this range from the next range down.
{
    range.set_count_hold_lo_steady(  range.get_next_range( ).get_count(  ));
    range.set_stride_hold_lo_steady( range.get_next_range( ).get_stride( ));
    rotate_left_in_place( range.ref_next_range( ), top_count, top_stride);
}

  template< typename LEAF_ITER_T >
  void
rotate_left_in_place
 (  stride_range< LEAF_ITER_T, 0 >                   &  range
  , typename stride_range< LEAF_ITER_T, 0 >::size_type  top_count
  , typename stride_range< LEAF_ITER_T, 0 >::diff_type  top_stride
 )
  // Helper function overload (like a specialization). Implements rotate_left(..), below.
  // The lowest-level range gets the count and stride from the top-most range.
{
    range.set_count_hold_lo_steady(  top_count );
    range.set_stride_hold_lo_steady( top_stride);
}

  template< typename LEAF_ITER_T, size_t DEPTH >
  void
rotate_left_in_place( stride_range< LEAF_ITER_T, DEPTH > & range)
  //
  // Does nothing if DEPTH==0.
{
    rotate_left_in_place( range, range.get_count( ), range.get_stride( ));
}

  template< typename LEAF_ITER_T, size_t DEPTH >
  stride_range< LEAF_ITER_T, DEPTH >
rotate_left( stride_range< LEAF_ITER_T, DEPTH > const & src)
  //
  // Returns copy if DEPTH==0.
{
    stride_range< LEAF_ITER_T, DEPTH > copy = src;
    rotate_left_in_place( copy);
    return copy;
}

// _______________________________________________________________________________________________

  template< typename LEAF_ITER_T, size_t DEPTH >
  tuple
   <  typename stride_range< LEAF_ITER_T, 0 >::size_type
    , typename stride_range< LEAF_ITER_T, 0 >::diff_type
   >
rotate_right_in_place
 (  stride_range< LEAF_ITER_T, DEPTH >               &  range
  , typename stride_range< LEAF_ITER_T, 0 >::size_type  above_count
  , typename stride_range< LEAF_ITER_T, 0 >::diff_type  above_stride
 )
  // Helper function. Implements rotate_right_in_place(..), below.
  // Sets the stride and count of this range from the one directly above.
{
    range.set_count_hold_lo_steady(  above_count );
    range.set_stride_hold_lo_steady( above_stride);
    return rotate_right_in_place( range.ref_next_range( ), range.get_count( ), range.get_stride( ));
}

  template< typename LEAF_ITER_T >
  tuple
   <  typename stride_range< LEAF_ITER_T, 0 >::size_type
    , typename stride_range< LEAF_ITER_T, 0 >::diff_type
   >
rotate_right_in_place
 (  stride_range< LEAF_ITER_T, 0 >                   &  range
  , typename stride_range< LEAF_ITER_T, 0 >::size_type  above_count
  , typename stride_range< LEAF_ITER_T, 0 >::diff_type  above_stride
 )
  // Helper function overload (like a specialization). Implements rotate_right_in_place(..), below.
{
    tuple
     <  typename stride_range< LEAF_ITER_T, 0 >::size_type
      , typename stride_range< LEAF_ITER_T, 0 >::diff_type
     > const
    return_tuple( range.get_count( ), range.get_stride( ));
    range.set_count_hold_lo_steady(  above_count );
    range.set_stride_hold_lo_steady( above_stride);
    return return_tuple;
}

  template< typename LEAF_ITER_T, size_t DEPTH >
  void
rotate_right_in_place( stride_range< LEAF_ITER_T, DEPTH > & range)
  //
  // This could be somewhat simpler if we ever implement get_leaf_range( ).
{
    tuple
     <  typename stride_range< LEAF_ITER_T, 0 >::size_type
      , typename stride_range< LEAF_ITER_T, 0 >::diff_type
     > const
    leaf_tuple =
    rotate_right_in_place( range, range.get_count( ), range.get_stride( ));
    range.set_count_hold_lo_steady(  leaf_tuple.get<0>( ));
    range.set_stride_hold_lo_steady( leaf_tuple.get<1>( ));
}

  template< typename LEAF_ITER_T, size_t DEPTH >
  stride_range< LEAF_ITER_T, DEPTH >
rotate_right( stride_range< LEAF_ITER_T, DEPTH > const & src)
  //
  // Returns copy if DEPTH==0.
{
    stride_range< LEAF_ITER_T, DEPTH > copy = src;
    rotate_right_in_place( copy);
    return copy;
}

// _______________________________________________________________________________________________

  template< typename LEAF_ITER_T >
  void
swap_xy_in_place( stride_range< LEAF_ITER_T, 1 > & range)
{
    rotate_left_in_place( range);
}

  template< typename LEAF_ITER_T >
  stride_range< LEAF_ITER_T, 1 >
swap_xy( stride_range< LEAF_ITER_T, 1 > const & src)
{
    return rotate_left( src);
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef STRIDE_ITER_H
//
// stride_iter.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
