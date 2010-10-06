// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// pair_iter.h
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
# ifndef PAIR_ITER_H
# define PAIR_ITER_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "all.h"
# include <iterator>
# include <boost/iterator/counting_iterator.hpp>

// _______________________________________________________________________________________________
// class pair_counter< A, B >
//
//   This gangs together two countables so you can inc/dec them both at the same time.
//   Eventually this should be generalized like tuple<..>.
//   When we generalize this we should call it tuple_counter<..>
//
//   Bug (flaw) in boost::counting_iterator<..>: The implementation of counting_iterator<..>
//   expects the counter class to be either a primitive integer, or an iterator-like thing.
//   If it's not an integer it looks for iterator typedefs.

  template< typename T0, typename T1 >
  class
pair_counter
  : public tuple< T0, T1 >
  , public std::iterator /* see note above on why we're making this a supertype */
            <  typename std::iterator_traits< T0 >::iterator_category
             , pair_counter< T0, T1 >            // value_type
             , typename std::iterator_traits< T0 >::difference_type
             , pair_counter< T0, T1 > const *    // pointer
             , pair_counter< T0, T1 > const &    // reference
            >
{
  // -------------------------------------------------------------------------------------------
  // Typedefs
  private:
    typedef pair_counter< T0, T1 >                      this_type         ;
    typedef tuple<        T0, T1 >                      super_type        ;
    typedef std::iterator
             <  typename std::iterator_traits< T0 >::iterator_category
              , pair_counter< T0, T1 >            // value_type
              , typename std::iterator_traits< T0 >::difference_type
              , pair_counter< T0, T1 > const *    // pointer
              , pair_counter< T0, T1 > const &    // reference
             >                                          iter_type         ;
  public:
    typedef typename iter_type::difference_type         difference_type   ;
    typedef typename iter_type::pointer                 pointer           ;
    typedef typename iter_type::reference               reference         ;
    typedef typename iter_type::value_type              value_type        ;
    typedef typename iter_type::iterator_category       iterator_category ;

    typedef difference_type                             diff_type         ;

  // -------------------------------------------------------------------------------------------
  // Constructors and copy assignment
  //   This will be a lot cleaner with variadic templates and perfect (rvalue) fowarding.
  //   Template typedefs will also help.
  public:
    /* ctor */   pair_counter( )            : super_type( ) { }

    /* ctor */   pair_counter( typename boost::tuples::access_traits< T0 >::parameter_type a0
                             , typename boost::tuples::access_traits< T1 >::parameter_type a1
                             )              : super_type( a0, a1) { }

    /* ctor */   pair_counter( typename boost::tuples::access_traits< T0 >::parameter_type a0)
                                            : super_type( a0) { }

    /* null */   pair_counter( tuple< > const &)
                                            : super_type( ) { }

    this_type &  operator =( tuple< > const &)
                                            { operator =( this_type( ));
                                              return *this;
                                            }

                   template< typename A0, typename A1 >
    /* copy */   pair_counter( tuple< A0, A1 > const & tup)
                                            : super_type( tup) { }

                   template< typename A0, typename A1 >
    this_type &  operator =( tuple< A0, A1 > const & tup)
                                            { super_type::operator =( tup);
                                              return *this;
                                            }

  // -------------------------------------------------------------------------------------------
  // Equality operators
  //   Only look at the first iterator in the tuple.
  //   The first iterator is the master. You only have to set the first iterator for an end test.
  //
  //   Consider using boost::less_than_comparable<T, U>?
  public:
                   template< typename A0, typename A1 >
    bool         operator ==( tuple< A0, A1 > const & tup)
                                      const { return this->get<0>( ) == tup.get<0>( ); }
                   template< typename A0, typename A1 >
    bool         operator !=( tuple< A0, A1 > const & tup)
                                      const { return ! operator ==( tup); }
                   template< typename A0, typename A1 >
    bool         operator <( tuple< A0, A1 > const & tup)
                                      const { return this->get<0>( ) < tup.get<0>( ); }
                   template< typename A0, typename A1 >
    bool         operator >( tuple< A0, A1 > const & tup)
                                      const { return tup.get<0>( ) < this->get<0>( ); }
                   template< typename A0, typename A1 >
    bool         operator >=( tuple< A0, A1 > const & tup)
                                      const { return ! operator <( tup); }
                   template< typename A0, typename A1 >
    bool         operator <=( tuple< A0, A1 > const & tup)
                                      const { return ! operator >( tup); }

  // -------------------------------------------------------------------------------------------
  // Increment/decrement/+/-
  public:
    // Prefix operators
    this_type &  operator ++( /*prefix*/)   { ++ this->get<0>( ); ++ this->get<1>( ); return *this; }
    this_type &  operator --( /*prefix*/)   { -- this->get<0>( ); -- this->get<1>( ); return *this; }

    // Postfix operators
    // We could define these with boost::incrementable<T> and boost::decrementable<T>.
    this_type    operator ++( int /*post*/) { this_type const save = *this; ++ *this; return save; }
    this_type    operator --( int /*post*/) { this_type const save = *this; -- *this; return save; }

    // In-place random-access
    this_type &  operator +=( diff_type n)  { this->get<0>( ) += n; this->get<1>( ) += n; return *this; }
    this_type &  operator -=( diff_type n)  { this->get<0>( ) -= n; this->get<1>( ) -= n; return *this; }

    // Displace random-access
    // We could define these with boost::addable<T, U> and boost::subtractable<T, U>.
    this_type    operator +( diff_type n)
                                      const { this_type copy = *this; copy += n; return copy; }
    this_type    operator -( diff_type n)
                                      const { this_type copy = *this; copy -= n; return copy; }

    // Difference: (iter - offset)
    // This only looks at the first iterator in the tuple, so it's different than the operators above.
                   template< typename A0, typename A1 >
    diff_type    operator -( tuple< A0, A1 > const & tup)
                                      const { return this->get<0>( ) - tup.get<0>( ); }

    // Missing: (offset + iter)
    // Not a good idea anyway. The form of the +/- operators above implies + is not commutative.
};

// _______________________________________________________________________________________________
// class pair_iter< T0, T1 >
//
//   Bug (flaw) in boost::counting_iterator<..>: Even with iterator_catagory defined in the
//   counter_type we still have to specify boost::random_access_traversal_tag as the 2nd
//   template param.

  template< typename T0, typename T1 >
  class
pair_iter
  : public boost::counting_iterator
            <  pair_counter< T0, T1 >
             , boost::random_access_traversal_tag
            >
  //
  // This would be better as:
  //     template< typename T0, typename T1 >
  //   typedef counting_iterator< pair_counter< T0, T1 > >  pair_iter;
  //
  // The class boost::counting_iterator<..> could also be called:
  //   self-deref
  //   self-evaluate
  // It turns an integer-like type into an iterator-like type by adding operator* and having
  // it return itself. This works because the iterator pattern is just the integer pattern with
  // with operator* (and operator[] and operator->) added. (Which is, btw, an argument to add
  // operator bool() (or safe_bool) to the iterator pattern.)
  //
  // To use counting_iterator, provide a counter class with:
  //   ctor( )
  //   ctor( copy)
  //   operator =( copy)
  //   operator ==
  //   operator <
  //   operator ++() - prefix only
  //   operator --() - prefix only
  //   operator +=(n)
  //   operator -( other_value) -> difference_type
  //
  // Counting iterator gives you a self-evaluating iterator (except the type changes in an
  // way that cannot be reversed with a cast, although it can with a ctor).
  // Another possible return from the deref operator would be pair_counter< T0::value_type, T1::value_type >.
  // This would be a deep deref'ing.
  //
  // Additional types and methods:
  //   deref(         ) -> deref_type< 0 >
  //   deref( infinity) -> deref_type< infinity >
  //   deref( 1)        -> pair_iter< T0::value_type, T1::value_type >::value_type
  //   |
  //   Since deref-getting is very different from deref-setting, they should have different methods.
  //   deref(..) as above is getter-only. It returns values, or const-refs when appropriate, but
  //   not l-value refs.
  //
  //   val_type
  //   val_ref_type  val_const_ref_type  val_varia_ref_type  val_return_type  val_param_type
{
  // -------------------------------------------------------------------------------------------
  // Typedefs
  public:
    typedef pair_counter< T0, T1 >                  counter_type ;
  private:
    typedef pair_iter< T0, T1 >                     this_type    ;
    typedef boost::counting_iterator
             <  counter_type
              , boost::random_access_traversal_tag
             >                                      super_type   ;

  // -------------------------------------------------------------------------------------------
  // Constructors required by super_type
  public:
    /* ctor */   pair_iter( )                               : super_type(  ) { }
    /* copy */   pair_iter( this_type const & b)            : super_type( b) { }
    /* ctor */   pair_iter( counter_type const & c)         : super_type( c) { }

  // -------------------------------------------------------------------------------------------
  // Constructors and assignment from counter_type
  public:
    /* ctor */   pair_iter( typename boost::tuples::access_traits< T0 >::parameter_type a0
                          , typename boost::tuples::access_traits< T1 >::parameter_type a1
                          )                                 : super_type( counter_type( a0, a1)) { }

    /* ctor */   pair_iter( typename boost::tuples::access_traits< T0 >::parameter_type a0)
                                                            : super_type( counter_type( a0)) { }

    /* null */   pair_iter( tuple< > const &)               : super_type( ) { }
    this_type &  operator =( tuple< > const &)              { operator =( this_type( ));
                                                              return *this;
                                                            }

                   template< typename A0, typename A1 >
    /* ctor */   pair_iter( tuple< A0, A1 > const & tup)    : super_type( counter_type( tup)) { }

                   template< typename A0, typename A1 >
    this_type &  operator =( tuple< A0, A1 > const & tup)
                                                            { super_type::operator =( this_type( tup));
                                                              return *this;
                                                            }
}; /* end class pair_iter< T0, T1 > */

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef PAIR_ITER_H */
//
// pair_iter.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
