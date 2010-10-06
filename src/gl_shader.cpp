// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_shader.cpp
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
// Concepts:
//   To use a type in a variant, it has to be a "bounded type", which has these requirements:
//     CopyConstructible [20.1.3].
//     Destructor upholds the no-throw exception-safety guarantee.
//     Complete at the point of variant template instantiation.
//
//   But we don't have a boost concept to capture all that. There is nothing like this:
//     BOOST_CONCEPT_ASSERT(( BoundedType< my_type > ));
//
//   In fact, the boost concepts don't seem to work at all, at least not in boost 1-39.
//   These does not compile (syntax errors with parens) (experiment in global namespace):
//
//     # include <boost/concept/assert.hpp>
//     BOOST_CONCEPT_ASSERT( (EqualityComparable< int >));
//
//     # include <list>
//     BOOST_CONCEPT_ASSERT( (BidirectionalIterator< std::list< int >::iterator >));
//
//   Otherwise we could at least do this:
//     BOOST_CONCEPT_ASSERT(( boost::CopyConstructible< normal::stage_holder_type > ));
//     BOOST_CONCEPT_ASSERT(( boost::CopyConstructible< ARB   ::stage_holder_type > ));
//
//     BOOST_CONCEPT_ASSERT(( boost::CopyConstructible< normal::program_holder_type > ));
//     BOOST_CONCEPT_ASSERT(( boost::CopyConstructible< ARB   ::program_holder_type > ));
//
// Owner objects and variants
//   The owner objects are single-owner, not shared-owner. They should NOT support copy --
//   neither a copy ctor nor a copy assignment op.
//
//   But a class has to support copy -- both ctor and assop -- in order to be used as part
//   of a boost variant. Without copy you can still create a variant instance, but only with
//   the default ctor, which sets the variant to the first class in the variant list.
//
//   Without copy there is no way to change the class inside the variant. So in our case
//   it'll always be stuck as the first type (reset_type).
//
//   I thought I could get around this by creating special ctor/assop members that looked
//   like copy but actually took other classes. Namely the tag classes:
//     stage_owner_type::reset_type
//     program_owner_type::reset_type
//   But the variant code wasn't fooled. It requires an exact copy.
//
//   I don't know if this is because of inherent restrictions in c++ or because the boost::variant
//   implementers did not implement this the way I'd like.
//
//   There are several ways to disable copy. Normally we'd use both (1) and (3) from this list.
//
//     1 Private copy ctor/assop  - compile-time error
//       In-class and friend methods can still access private methods.
//
//     2 Implementation with static assert failure  - compile-time error
//       Only works for template classes.
//
//     3 No implementation  - link-time error
//       Harder to track than a compile-time error.
//
//     4 Implementation with assert failure  - run-time error
//
//     5 Implementation with assert that fails without special incantation.
//       Usually the incantation is a flag set by a wrapper.
//       Elaborate and complicated. Makes code harder to understand.
//
//     6 Unexpected implementation.
//       This is what we do. It's pretty weak. Perhaps it'd be better to not use variant
//       with these classes at all.
// _______________________________________________________________________________________________

# include "all.h"
# include "gl_shader.h"

// _______________________________________________________________________________________________
//
namespace gl_env {
namespace shader {
// _______________________________________________________________________________________________

  void
  normal::stage_owner_type::
create_vertex_stage( char_type const * p_src_string)
{
    create_stage( GL_VERTEX_SHADER, p_src_string);
}

  void
  normal::stage_owner_type::
create_fragment_stage( char_type const * p_src_string)
{
    create_stage( GL_FRAGMENT_SHADER, p_src_string);
}

  void
  ARB::stage_owner_type::
create_vertex_stage( char_type const * p_src_string)
{
    create_stage( GL_VERTEX_SHADER_ARB, p_src_string);
}

  void
  ARB::stage_owner_type::
create_fragment_stage( char_type const * p_src_string)
{
    create_stage( GL_FRAGMENT_SHADER_ARB, p_src_string);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

namespace /* anonymous */ {

  /* private visitor-functor type */
  struct
get_id_variant__visitor_functor_type
  : public boost::static_visitor< uint_type >
{
    uint_type
  operator ()( tag::reset_type) const
    {
      return 0;
    }

    template< typename INNER_TYPE >
    uint_type
  operator ()( INNER_TYPE const & inner) const
    {
      // Owner types auto-cast to matching non-owner (holder) types.
      return inner.get_id( );
    }
};

  /* private visitor-functor type */
  struct
set_id__visitor_functor_type
  : public boost::static_visitor< void >
{
    /* ctor */
  set_id__visitor_functor_type( uint_type id) : id_( id) { }
  uint_type const id_;

    void
  operator ()( tag::reset_type) const
    {
      d_assert( 0 == id_);
    }

    template< typename INNER_TYPE >
    void
  operator ()( INNER_TYPE & inner) const
    {
      inner.set_id( id_);
    }
};

  /* private */
  template< typename OUTER_TYPE >
  uint_type
get_id_( OUTER_TYPE const & outer)
{
    get_id_variant__visitor_functor_type functor;
    return boost::apply_visitor( functor, outer);
}

  /* private */
  template< typename OUTER_TYPE >
  void
set_id_( OUTER_TYPE & outer, uint_type id)
{
    set_id__visitor_functor_type functor( id);
    boost::apply_visitor( functor, outer);
}

} /* end namespace anonymous */

// _______________________________________________________________________________________________
// Externs:

uint_type  get_id( stage_holder_variant_type   const & outer)  { return get_id_( outer); }
uint_type  get_id( stage_owner_variant_type    const & outer)  { return get_id_( outer); }
uint_type  get_id( program_holder_variant_type const & outer)  { return get_id_( outer); }
uint_type  get_id( program_owner_variant_type  const & outer)  { return get_id_( outer); }

void  set_id( stage_holder_variant_type   & outer, uint_type id)  { set_id_( outer, id); }
//void  set_id( stage_owner_variant_type    & outer, uint_type id)  { set_id_( outer, id); }
void  set_id( program_holder_variant_type & outer, uint_type id)  { set_id_( outer, id); }
//void  set_id( program_owner_variant_type  & outer, uint_type id)  { set_id_( outer, id); }

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

namespace /* anonymous */ {

  /* private visitor-functor type */
  template< typename HOLDER_TYPE >
  struct
get_holder__visitor_functor_type
  //
  // Visitor functors are used with variant types.
  // This one extracts a non-owner-variant from a similar owner-variant.
  // A non-owner-type is also called a holder-type.
  //
  : public boost::static_visitor< HOLDER_TYPE >
{
    HOLDER_TYPE
  operator ()( tag::reset_type) const
    {
      return tag::reset_type( );
    }

    template< typename OWNER_TYPE >
    HOLDER_TYPE
  operator ()( OWNER_TYPE const & owner) const
    {
      return owner.get_holder( );
    }
}; /* end struct get_holder__visitor_functor_type */

  template< typename HOLDER_TYPE, typename OWNER_TYPE >
  HOLDER_TYPE
get_holder_( OWNER_TYPE const & owner)
{
    get_holder__visitor_functor_type< HOLDER_TYPE > functor;
    return boost::apply_visitor( functor, owner);
}

} /* end namespace anonymous */

// _______________________________________________________________________________________________

  /* extern */
  stage_holder_variant_type
get_holder( stage_owner_variant_type const & stage_owner)
{
    return get_holder_< stage_holder_variant_type >( stage_owner);
}

  /* extern */
  program_holder_variant_type
get_holder( program_owner_variant_type const & program_owner)
{
    return get_holder_< program_holder_variant_type >( program_owner);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

namespace /* anonymous */ {

  /* private visitor-functor type */
  struct
create_stage__visitor_functor_type
  : public boost::static_visitor< void >
{
    /* ctor */
  create_stage__visitor_functor_type( enum_type tag, char_type const * p_src_string)
    : tag_( tag), p_src_string_( p_src_string) { }
  enum_type         const  tag_          ;
  char_type const * const  p_src_string_ ;

    void
  operator ()( tag::reset_type) const
    {
      /* this is legal */
    }

    template< typename STAGE_OWNER_TYPE >
    void
  operator ()( STAGE_OWNER_TYPE & stage_owner) const
    {
      stage_owner.create_stage( tag_, p_src_string_);
    }
}; /* end struct create_stage__visitor_functor_type */

  /* private visitor-functor type */
  struct
create_vertex_stage__visitor_functor_type
  : public boost::static_visitor< void >
{
    /* ctor */
  create_vertex_stage__visitor_functor_type( char_type const * p_src_string)
    : p_src_string_( p_src_string) { }
  char_type const * const  p_src_string_ ;

    void
  operator ()( tag::reset_type) const
    {
      /* this is legal */
    }

    template< typename STAGE_OWNER_TYPE >
    void
  operator ()( STAGE_OWNER_TYPE & stage_owner) const
    {
      stage_owner.create_vertex_stage( p_src_string_);
    }
}; /* end struct create_vertex_stage__visitor_functor_type */

  /* private visitor-functor type */
  struct
create_fragment_stage__visitor_functor_type
  : public boost::static_visitor< void >
{
    /* ctor */
  create_fragment_stage__visitor_functor_type( char_type const * p_src_string)
    : p_src_string_( p_src_string) { }
  char_type const * const  p_src_string_ ;

    void
  operator ()( tag::reset_type) const
    {
      /* this is legal */
    }

    template< typename STAGE_OWNER_TYPE >
    void
  operator ()( STAGE_OWNER_TYPE & stage_owner) const
    {
      stage_owner.create_fragment_stage( p_src_string_);
    }
}; /* end struct create_fragment_stage__visitor_functor_type */

} /* end namespace anonymous */

// _______________________________________________________________________________________________

  /* extern */
  void
create_stage
 (  stage_owner_variant_type &  stage_owner
  , enum_type                   tag
  , char_type const *           p_src_string
 )
{
    init_type_binding( stage_owner);
    create_stage__visitor_functor_type functor( tag, p_src_string);
    boost::apply_visitor( functor, stage_owner);
}

  /* extern */
  void
create_vertex_stage
 (  stage_owner_variant_type &  stage_owner
  , char_type const *           p_src_string
 )
{
    init_type_binding( stage_owner);
    create_vertex_stage__visitor_functor_type functor( p_src_string);
    boost::apply_visitor( functor, stage_owner);
}

  /* extern */
  void
create_fragment_stage
 (  stage_owner_variant_type &  stage_owner
  , char_type const *           p_src_string
 )
{
    init_type_binding( stage_owner);
    create_fragment_stage__visitor_functor_type functor( p_src_string);
    boost::apply_visitor( functor, stage_owner);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

namespace /* anonymous */ {

  /* private visitor-functor type */
  struct
create_program__visitor_functor_type
  //
  // This is a binary visitor functor, on the types of the stage holders.
  // Although it could be implemented as a uniary one one stage (or on the program), followed by
  // boost::get<..> calls to extract the other parts.
  //
  // This does not require the program-owner-variant to be typed ahead of time, and does not look
  // at that type if it is set.
  //
  : public boost::static_visitor< void >
{
    /* ctor */
  create_program__visitor_functor_type( program_owner_variant_type & program)
    : program_owner_variant_( program) { }
  program_owner_variant_type & program_owner_variant_;

    void
  operator ()( tag::reset_type, tag::reset_type) const
    {
      // This is legal.
      // The following is probably not necessary since the program-owner is probably
      // already deep-reset.
      deep_reset( program_owner_variant_);
    }

    template< typename T >
    void
  operator ()( T const &, tag::reset_type) const
    {
      d_static_assert( T::is_holder_type && T::is_stage_type);
      // This shouldn't happen. If one stage is deep-reset, the other should be also.
      d_assert( false);
      deep_reset( program_owner_variant_);
    }

    template< typename T >
    void
  operator ()( tag::reset_type, T const &) const
    {
      d_static_assert( T::is_holder_type && T::is_stage_type);
      // This shouldn't happen. If one stage is deep-reset, the other should be also.
      d_assert( false);
      deep_reset( program_owner_variant_);
    }

    template< typename STAGE_HOLDER_TYPE >
    void
  operator ()
   (  STAGE_HOLDER_TYPE const &  vertex_stage
    , STAGE_HOLDER_TYPE const &  fragment_stage
   ) const
    {
      // Normal case: stages are the same type.
      typedef typename STAGE_HOLDER_TYPE::program_type::owner_type program_owner_type;
      program_owner_variant_ = program_owner_type( );

      program_owner_type * const
        p_program_owner =
          boost::get< program_owner_type >( & program_owner_variant_);
      d_assert( p_program_owner && p_program_owner->is_reset( ));

      // Link the program. If this fails *p_program_owner will still be reset afterwards,
      // but not deep-reset.
      p_program_owner->create_program( vertex_stage, fragment_stage);
      // If the stages are both reset (but not deep-reset), should it be normal to
      // "compile" those to a reset (but not deep-reset) program?
      // Maybe we should do this:
      //   if ( is_reset( program_owner_variant_) ) {
      //     if ( not_reset( vertex_stage) || not_reset( fragment_stage) ) {
      //       deep_reset( program_owner_variant_);
      //     }
      //   }
    }

    template< typename T0, typename T1 >
    void
  operator ()( T0 const &, T1 const &) const
    {
      // Incompatible stage types.
      d_static_assert( T0::is_holder_type && T0::is_stage_type);
      d_static_assert( T1::is_holder_type && T0::is_stage_type);
      d_assert( false);
      deep_reset( program_owner_variant_);
    }
}; /* end struct create_program__visitor_functor_type */

} /* end namespace anonymous */

  /* extern */
  void
create_program
 (  program_owner_variant_type      &  program_owner
  , stage_holder_variant_type const &  vertex_stage
  , stage_holder_variant_type const &  fragment_stage
 )
  // If all goes well, this sets up program_owner with a linked shader program:
  //   not_reset( program_owner)
  //
  // If this runs into typing trouble, this deep-resets:
  //   is_deep_reset( program_owner)
  //
  // If this runs into linking trouble, this shallow-resets:
  //   not_deep_reset( program_owner) &&
  //   is_reset( program_owner)
{
    create_program__visitor_functor_type functor( program_owner);
    boost::apply_visitor( functor, vertex_stage, fragment_stage); /* binary visitor functor */
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

namespace /* anonymous */ {

  /* private visitor-functor type */
  struct
is_active__visitor_functor_type
  : public boost::static_visitor< bool >
{
    bool
  operator ()( tag::reset_type) const
    {
      d_assert( false);
      return false;
    }

    template< typename PROGRAM_HOLDER_TYPE >
    bool
  operator ()( PROGRAM_HOLDER_TYPE const & program_holder) const
    {
      return program_holder.is_active( );
    }
}; /* end struct activate__visitor_functor_type */

  /* private visitor-functor type */
  struct
activate__visitor_functor_type
  : public boost::static_visitor< program_holder_variant_type >
{
    program_holder_variant_type
  operator ()( tag::reset_type) const
    {
      d_assert( false);
      return tag::reset_type( );
    }

    template< typename PROGRAM_HOLDER_TYPE >
    program_holder_variant_type
  operator ()( PROGRAM_HOLDER_TYPE const & program_holder) const
    {
      // This will activate the default (pipeline) shader if program_holder.is_reset( ).
      return program_holder.activate( );
    }
}; /* end struct activate__visitor_functor_type */

  /* private visitor-functor type */
  struct
get_uniform_location__visitor_functor_type
  : public boost::static_visitor< int_type >
{
    /* ctor */
  get_uniform_location__visitor_functor_type( char_type const * p_uniform_name)
    : p_uniform_name_( p_uniform_name ) { }
  char_type const * const  p_uniform_name_ ;

    int_type
  operator ()( tag::reset_type) const
    {
      // Should this be legal?
      d_assert( false);
      return -1;
    }

    template< typename PROGRAM_HOLDER_TYPE >
    int_type
  operator ()( PROGRAM_HOLDER_TYPE const & program_holder) const
    {
      // Returns negative (-1) if uniform does not exist.
      return program_holder.get_uniform_location( p_uniform_name_);
    }
}; /* end struct get_uniform_location__visitor_functor_type */

} /* end namespace anonymous */

// _______________________________________________________________________________________________

  /* extern */
  bool
is_active( program_holder_variant_type const & program_holder)
{
    is_active__visitor_functor_type functor;
    return boost::apply_visitor( functor, program_holder);
}

  /* extern */
  program_holder_variant_type
activate( program_holder_variant_type const & program_holder)
{
    activate__visitor_functor_type functor;
    return boost::apply_visitor( functor, program_holder);
}

  /* extern */
  int_type
get_uniform_location
 (  program_holder_variant_type const &  program_holder
  , char_type const *                    p_uniform_name
 )
{
    get_uniform_location__visitor_functor_type functor( p_uniform_name);
    return boost::apply_visitor( functor, program_holder);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

namespace /* anonymous */ {

  /* private visitor-functor type */
  template< typename VALUE_TYPE >
  struct
set_uniform__visitor_functor_type
  : public boost::static_visitor< void >
{
  // This should be re-written when variadic templates are supported.

    /* value members */
    int_type           const  uniform_id_ ;
    size_type          const  count_      ;
    VALUE_TYPE const * const  p_v0_       ;
    VALUE_TYPE const * const  p_v1_       ;
    VALUE_TYPE const * const  p_v2_       ;
    VALUE_TYPE const * const  p_v3_       ;

    /* ctor - 1 arg */
  # if 0
  set_uniform__visitor_functor_type
   (  int_type   const &  uniform_id
    , VALUE_TYPE const &  v0
   )
    : uniform_id_( uniform_id), count_( 1)
    , p_v0_( & v0), p_v1_( 0), p_v2_( 0), p_v3_( 0)
    { }
  # endif

    /* ctor - 2 arg */
  set_uniform__visitor_functor_type
   (  int_type   const &  uniform_id
    , VALUE_TYPE const &  v0
    , VALUE_TYPE const &  v1
   )
    : uniform_id_( uniform_id), count_( 2)
    , p_v0_( & v0), p_v1_( & v1), p_v2_( 0), p_v3_( 0)
    { }

    /* ctor - 3 arg */
  set_uniform__visitor_functor_type
   (  int_type   const &  uniform_id
    , VALUE_TYPE const &  v0
    , VALUE_TYPE const &  v1
    , VALUE_TYPE const &  v2
   )
    : uniform_id_( uniform_id), count_( 3)
    , p_v0_( & v0), p_v1_( & v1), p_v2_( & v2), p_v3_( 0)
    { }

    /* ctor - 4 arg */
  set_uniform__visitor_functor_type
   (  int_type   const &  uniform_id
    , VALUE_TYPE const &  v0
    , VALUE_TYPE const &  v1
    , VALUE_TYPE const &  v2
    , VALUE_TYPE const &  v3
   )
    : uniform_id_( uniform_id), count_( 4)
    , p_v0_( & v0), p_v1_( & v1), p_v2_( & v2), p_v3_( & v3)
    { }

    void
  operator ()( tag::reset_type) const
    {
      d_assert( false);
    }

    template< typename PROGRAM_HOLDER_TYPE >
    void
  operator ()( PROGRAM_HOLDER_TYPE const & program_holder) const
    {
      switch ( count_ ) {
      //case 1: program_holder.set_uniform( uniform_id_, *p_v0_                        ); break;
        case 2: program_holder.set_uniform( uniform_id_, *p_v0_, *p_v1_                ); break;
        case 3: program_holder.set_uniform( uniform_id_, *p_v0_, *p_v1_, *p_v2_        ); break;
        case 4: program_holder.set_uniform( uniform_id_, *p_v0_, *p_v1_, *p_v2_, *p_v3_); break;
        default:
          d_assert( false);
      }
    }
};

  /* private visitor-functor type */
  template< typename VALUE_TYPE >
  struct
set_uniform_single__visitor_functor_type
  : public boost::static_visitor< void >
{
  // This should be re-written when variadic templates are supported.

    /* value members */
    int_type           const  uniform_id_ ;
    VALUE_TYPE const * const  p_v0_       ;

    /* ctor - 1 arg */
  set_uniform_single__visitor_functor_type
   (  int_type   const &  uniform_id
    , VALUE_TYPE const &  v0
   )
    : uniform_id_( uniform_id), p_v0_( & v0) { }

    void
  operator ()( tag::reset_type) const
    {
      d_assert( false);
    }

    template< typename PROGRAM_HOLDER_TYPE >
    void
  operator ()( PROGRAM_HOLDER_TYPE const & program_holder) const
    {
      program_holder.set_uniform( uniform_id_, *p_v0_);
    }
};

} /* end namespace anonymous */

// _______________________________________________________________________________________________

  /* extern */
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , rgb_type< float_type > const &       color
 )
{
    set_uniform_single__visitor_functor_type< rgb_type< float_type > > functor( uniform_id, color);
    boost::apply_visitor( functor, program_holder);
}

  /* extern */
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , rgba_type< float_type > const &      color
 )
{
    set_uniform_single__visitor_functor_type< rgba_type< float_type > > functor( uniform_id, color);
    boost::apply_visitor( functor, program_holder);
}

  /* extern */
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , float_type                           v0
 )
{
    set_uniform_single__visitor_functor_type< float_type > functor( uniform_id, v0);
    boost::apply_visitor( functor, program_holder);
}

  /* extern */
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , float_type                           v0
  , float_type                           v1
 )
{
    set_uniform__visitor_functor_type< float_type > functor( uniform_id, v0, v1);
    boost::apply_visitor( functor, program_holder);
}

  /* extern */
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , float_type                           v0
  , float_type                           v1
  , float_type                           v2
 )
{
    set_uniform__visitor_functor_type< float_type > functor( uniform_id, v0, v1, v2);
    boost::apply_visitor( functor, program_holder);
}

  /* extern */
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , float_type                           v0
  , float_type                           v1
  , float_type                           v2
  , float_type                           v3
 )
{
    set_uniform__visitor_functor_type< float_type > functor( uniform_id, v0, v1, v2, v3);
    boost::apply_visitor( functor, program_holder);
}

// _______________________________________________________________________________________________

  /* extern */
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , int_type                             v0
 )
{
    set_uniform_single__visitor_functor_type< int_type > functor( uniform_id, v0);
    boost::apply_visitor( functor, program_holder);
}

  /* extern */
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , int_type                             v0
  , int_type                             v1
 )
{
    set_uniform__visitor_functor_type< int_type > functor( uniform_id, v0, v1);
    boost::apply_visitor( functor, program_holder);
}

  /* extern */
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , int_type                             v0
  , int_type                             v1
  , int_type                             v2
 )
{
    set_uniform__visitor_functor_type< int_type > functor( uniform_id, v0, v1, v2);
    boost::apply_visitor( functor, program_holder);
}

  /* extern */
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , int_type                             v0
  , int_type                             v1
  , int_type                             v2
  , int_type                             v3
 )
{
    set_uniform__visitor_functor_type< int_type > functor( uniform_id, v0, v1, v2, v3);
    boost::apply_visitor( functor, program_holder);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// This is more than just an "owner". It is a collective owner, that owns:
//   The vertex stage
//   The fragment stage
//   The linked program

# ifndef NDEBUG
  void
  program_owner_any_type::
assert_invariants( ) const
{
    // Starting and ending invariants:
    d_assert( implies( is_init_successful_, is_init_attempted_));
    d_assert( is_init_successful_ == not_deep_reset( program_));
    d_assert( is_init_successful_ == not_deep_reset( vertex_stage_));
    d_assert( is_init_successful_ == not_deep_reset( fragment_stage_));

    // Should we make assumptions about:
    //   p_vertex_stage_src_
    //   p_fragment_stage_src_
}
# endif

  /* ctor */
  program_owner_any_type::
program_owner_any_type
 (  char const *  p_src_vertex_stage
  , char const *  p_src_fragment_stage
 )
  : is_init_attempted_    ( false)
  , is_init_successful_   ( false)
  , p_vertex_stage_src_   ( p_src_vertex_stage)
  , vertex_stage_         ( )
  , p_fragment_stage_src_ ( p_src_fragment_stage)
  , fragment_stage_       ( )
  , program_              ( )
{
    assert_invariants( );
}

// _______________________________________________________________________________________________

  /* public virtual */
  void
  program_owner_any_type::
init( )
{
    if ( ! is_init_attempted( ) ) {
        init_private( );
    }
    d_assert( is_init_attempted( ));
}

  /* protected method */
  void
  program_owner_any_type::
init_private( )
  //
  // This sets the shader-program id, for either a "normal" or "ARB" program.
  // If this fails (maybe due to a compile error) this clears the type-id of the program variant.
  // This never leaves the program reset but not deep-reset (although perhaps it should if the
  // source string is empty).
{
    assert_invariants( );

    is_init_attempted_ = true;
    is_init_successful_ = false;

    // Compile the fragment and vertex stages.
    // Assumes that the desired result is never the default (pipeline) shader program unless
    // shaders are not supported.
    create_fragment_stage( fragment_stage_, p_fragment_stage_src_);
    if ( not_reset( fragment_stage_) ) {
        create_vertex_stage( vertex_stage_, p_vertex_stage_src_);
        if ( not_reset( vertex_stage_) ) {
            // Link the stages to make the shader program.
            create_program( program_, get_holder( vertex_stage_), get_holder( fragment_stage_));
            if ( not_reset( program_) ) {
                is_init_successful_ = true;
            }
        }
    }

    // If this fails we deep-reset instead of just reset (aka shallow-reset).
    if ( ! is_init_successful_ ) {
        deep_reset( program_       );
        deep_reset( vertex_stage_  );
        deep_reset( fragment_stage_);
    }

    assert_invariants( );
}

  void
  program_owner_any_type::
uninit( )
{
    assert_invariants( );

    is_init_successful_ = false;
    is_init_attempted_  = false;

    deep_reset( program_       );
    deep_reset( vertex_stage_  );
    deep_reset( fragment_stage_);

    assert_invariants( );
}

// _______________________________________________________________________________________________

  int_type
  program_owner_any_type::
get_uniform_location( char_type const * p_uniform_name) const
{
    d_assert( p_uniform_name && *p_uniform_name);
    d_assert( is_init_attempted_);
    d_assert( is_init_successful_);
    d_assert( not_reset( program_));
    // Maybe we should return -1 (an illegal value) if (! is_init_successful_)?
    return gl_env::shader::get_uniform_location( get_holder( program_), p_uniform_name);
}

// _______________________________________________________________________________________________

  program_holder_variant_type
  program_owner_any_type::
activate( ) const
{
    // Assumes compile is finished and program is valid.
    d_assert( is_init_attempted_);
    d_assert( is_init_successful_);
    d_assert( not_reset( program_));
    return gl_env::shader::activate( get_holder( program_));
}

// _______________________________________________________________________________________________

  void
  program_owner_any_type::
set_uniform
 (  int_type                        uniform_id
  , rgb_type< float_type > const &  color
 ) const
{
    d_assert( is_init_attempted_);
    // Should we allow this (and just ignore it) if (! is_init_successful_)? No.
    d_assert( is_init_successful_);
    d_assert( not_reset( program_));
    gl_env::shader::set_uniform( get_holder( program_), uniform_id, color);
}

  void
  program_owner_any_type::
set_uniform
 (  int_type                         uniform_id
  , rgba_type< float_type > const &  color
 ) const
{
    d_assert( is_init_attempted_);
    // Should we allow this (and just ignore it) if (! is_init_successful_)? No.
    d_assert( is_init_successful_);
    d_assert( not_reset( program_));
    gl_env::shader::set_uniform( get_holder( program_), uniform_id, color);
}

  void
  program_owner_any_type::
set_uniform
 (  int_type    uniform_id
  , float_type  v0
 ) const
{
    d_assert( is_init_attempted_);
    // Should we allow this (and just ignore it) if (! is_init_successful_)? No.
    d_assert( is_init_successful_);
    d_assert( not_reset( program_));
    gl_env::shader::set_uniform( get_holder( program_), uniform_id, v0);
}

  void
  program_owner_any_type::
set_uniform
 (  int_type    uniform_id
  , float_type  v0
  , float_type  v1
 ) const
{
    d_assert( is_init_attempted_);
    d_assert( is_init_successful_);
    d_assert( not_reset( program_));
    gl_env::shader::set_uniform( get_holder( program_), uniform_id, v0, v1);
}

  void
  program_owner_any_type::
set_uniform
 (  int_type    uniform_id
  , float_type  v0
  , float_type  v1
  , float_type  v2
 ) const
{
    d_assert( is_init_attempted_);
    d_assert( is_init_successful_);
    d_assert( not_reset( program_));
    gl_env::shader::set_uniform( get_holder( program_), uniform_id, v0, v1, v2);
}

  void
  program_owner_any_type::
set_uniform
 (  int_type    uniform_id
  , float_type  v0
  , float_type  v1
  , float_type  v2
  , float_type  v3
 ) const
{
    d_assert( is_init_attempted_);
    d_assert( is_init_successful_);
    d_assert( not_reset( program_));
    gl_env::shader::set_uniform( get_holder( program_), uniform_id, v0, v1, v2, v3);
}

  void
  program_owner_any_type::
set_uniform
 (  int_type  uniform_id
  , int_type  v0
 ) const
{
    d_assert( is_init_attempted_);
    d_assert( is_init_successful_);
    d_assert( not_reset( program_));
    gl_env::shader::set_uniform( get_holder( program_), uniform_id, v0);
}

  void
  program_owner_any_type::
set_uniform
 (  int_type  uniform_id
  , int_type  v0
  , int_type  v1
 ) const
{
    d_assert( is_init_attempted_);
    d_assert( is_init_successful_);
    d_assert( not_reset( program_));
    gl_env::shader::set_uniform( get_holder( program_), uniform_id, v0, v1);
}

  void
  program_owner_any_type::
set_uniform
 (  int_type  uniform_id
  , int_type  v0
  , int_type  v1
  , int_type  v2
 ) const
{
    d_assert( is_init_attempted_);
    d_assert( is_init_successful_);
    d_assert( not_reset( program_));
    gl_env::shader::set_uniform( get_holder( program_), uniform_id, v0, v1, v2);
}

  void
  program_owner_any_type::
set_uniform
 (  int_type  uniform_id
  , int_type  v0
  , int_type  v1
  , int_type  v2
  , int_type  v3
 ) const
{
    d_assert( is_init_attempted_);
    d_assert( is_init_successful_);
    d_assert( not_reset( program_));
    gl_env::shader::set_uniform( get_holder( program_), uniform_id, v0, v1, v2, v3);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

// The with_program wrapper is no longer used.
# if 0

  bool
  with_program::
forget__no_restore( )
{
    if ( has_restore( ) ) {
        deep_reset( previous_program_);
        d_assert( ! has_restore( ));
        return true;
    }
    return false;
}

  bool
  with_program::
restore( )
{
    if ( has_restore( ) ) {
        gl_env::shader::activate( previous_program_); /* discard return */
        deep_reset( previous_program_);
        d_assert( ! has_restore( ));
        return true;
    }
    return false;
}

// _______________________________________________________________________________________________

  /* ctor */
  with_program::
with_program( )
  : previous_program_( /* deep-reset */ )
{
    d_assert( ! has_restore( ));
}

  /* ctor */
  with_program::
with_program( program_owner_any_type & program_owner)
  : previous_program_( /* deep-reset */ )
{
    d_assert( ! has_restore( ));
    activate( program_owner);
}

  /* ctor */
  with_program::
with_program( program_owner_any_type const & program_owner)
  : previous_program_( /* deep-reset */ )
{
    d_assert( ! has_restore( ));
    activate__no_init( program_owner);
}

  /* ctor */
  with_program::
with_program( program_holder_variant_type const & program)
  : previous_program_( /* deep-reset */ )
{
    d_assert( ! has_restore( ));
    activate( program);
}

// _______________________________________________________________________________________________

  void
  with_program::
activate__no_init( program_owner_any_type const & program_owner)
  //
{
    // Assumes the program is already compiled and linked.
    d_assert( program_owner.is_init_attempted( ));
    activate( program_owner.get_program__no_init( ));
}

  void
  with_program::
activate( program_owner_any_type & program_owner)
{
    // This will init( ) (compile and link) the program if it's not already.
    activate( program_owner.get_program( ));
}

  void
  with_program::
activate( program_holder_variant_type const & program)
  //
  // This usually ends with has_restore( ) set true, unless is_deep_reset( program).
{
    // Do nothing if shaders are not supported.
    if ( not_deep_reset( program) ) {

        // Save the last shader if this is the first time we are activating with this wrapper.
        if ( ! has_restore( ) ) {
            previous_program_ = gl_env::shader::activate( program);
            d_assert( has_restore( ));
        } else {
            // Don't save the returned value, the previously activated program.
            // When we restore we'll use previous_program_, the previously active
            // program we encountered first.
            gl_env::shader::activate( program);
        }
    }
}

# endif

// _______________________________________________________________________________________________
//
} /* end namespace shader */
} /* end namespace gl_env */
// _______________________________________________________________________________________________

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_shader.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
