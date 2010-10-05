// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_shader.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef GL_SHADER_H
# define GL_SHADER_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "all.h"
# include "gl_env_global.h"
# include <boost/variant.hpp>

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

namespace gl_env {
namespace shader {

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// shader_object_base_type< id_type >
//
// Maybe this should also store:
//   GL_VERTEX_SHADER
//   GL_VERTEX_SHADER_ARB
//   GL_VERTEX_SHADER_EXT
//
//   GL_FRAGMENT_SHADER
//   GL_FRAGMENT_SHADER_ARB
//   GL_FRAGMENT_SHADER_ATI
//
//   GL_GEOMETRY_SHADER  - this is not defined (yet)
//   GL_GEOMETRY_SHADER_ARB
//   GL_GEOMETRY_SHADER_EXT
//
//   GL_PROGRAM_OBJECT  - this is not defined
//   GL_PROGRAM_OBJECT_ARB
// _______________________________________________________________________________________________

  template< typename ID_TYPE >
  class
shader_object_base_type
{
  // Typedef
  public:
    typedef ID_TYPE  id_type;
    // Assumes id_type has a fast copy. Otherwise we'd want to use "id_type const &" for
    // params and return values instead of just "id_type".

    static bool const  is_shader_object_type  = true;

  // Ctor/dtor
  protected:
    /* ctor */  shader_object_base_type( )            : id_( 0 ) { }
    /* ctor */  shader_object_base_type( id_type id)  : id_( id) { }

    // Default dtor
    // Default copy ctor
    // Default copy assignment

  // Reset
  public:
    bool        is_reset( )                     const { return 0 == get_id( ); }
    bool        not_reset( )                    const { return 0 != get_id( ); }

    void        reset( )                              { set_id( 0); }

  // Getter and Setter
  public:
    id_type     get_id( )                       const { return id_; }
  protected:
    void        set_id( id_type id)                   { id_ = id; }

  // Member var, set in supertype
  private:
    id_type  id_ ;
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

namespace normal {

// _______________________________________________________________________________________________

class program_owner_type  ;
class program_holder_type ;

class stage_owner_type    ;
class stage_holder_type   ;

// We will declare:
//   typedef ..variant-of-classes-above.. as .._variant_type;
// Too bad we cannot forward declare it somehow, maybe like this:
//   typename program_holder_variant_type;

// _______________________________________________________________________________________________

  inline
  bool
is_supported( )
{
    return global::are_shaders_supported( );
}

  inline
  bool
is_explicitly_supported( )
{
    return global::are_shaders_explicitly_supported( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// stage_holder_type
// stage_owner_type
// _______________________________________________________________________________________________

# define THIS_TYPE stage_holder_type
  class
THIS_TYPE
  : public shader_object_base_type< uint_type >
{
  // Typedefs
  private:
    typedef shader_object_base_type< uint_type >  base_type    ;
    typedef THIS_TYPE                             this_type    ;
  public:
    typedef stage_owner_type                      other_type   ;

    typedef program_holder_type                   program_type ;
    typedef other_type                            owner_type   ;
    typedef this_type                             holder_type  ;
    typedef base_type::id_type                    id_type      ; /* uint_type */

    static bool const  is_stage_type    = true ;
    static bool const  is_program_type  = false;
    static bool const  is_holder_type   = true ;
    static bool const  is_owner_type    = false;

  // Ctor/Copy/Dtor
  public:
    /* ctor */   THIS_TYPE( )                      { d_assert( is_reset( )); }
    // Default dtor

    // Default copy ctor
    // Default copy assignment

    explicit     THIS_TYPE(  owner_type const &)   ; /* defined below in macros */
    this_type &  operator =( owner_type const &)   ; /* defined below in macros */

    // These will assert if id is not not a legal shader-stage.
    explicit     THIS_TYPE(  id_type id)           { set_id( id); }
    this_type &  operator =( id_type id)           { return set_id( id); }

  // Error checking wrapper on setter
  public:
    this_type &  set_id( id_type id)               { d_assert( (0 == id) || global::is_shader( id));
                                                     base_type::set_id( id);
                                                     return *this;
                                                   }
};
# undef THIS_TYPE

// _______________________________________________________________________________________________

# define THIS_TYPE stage_owner_type
  class
THIS_TYPE
{
  // Typedefs
  private:
    typedef THIS_TYPE             this_type    ;
  public:
    typedef stage_holder_type     other_type   ;

    typedef program_owner_type    program_type ;
    typedef this_type             owner_type   ;
    typedef other_type            holder_type  ;
    typedef holder_type::id_type  id_type      ; /* uint_type */

    BOOST_MPL_ASSERT(( boost::is_same< this_type, holder_type::owner_type >));

    static bool const  is_stage_type    = true ;
    static bool const  is_program_type  = false;
    static bool const  is_holder_type   = false;
    static bool const  is_owner_type    = true ;

  // Ctor/Dtor
  public:
    /* ctor */  THIS_TYPE( )                      { d_assert( is_reset( )); }
    /* dtor */  ~THIS_TYPE( )                     { reset( ); }

  // Disable copy -- we don't support multiple owners
  // No, unfortunately we have to choose between providing copy or not using variants.
  // So provide special highly-restricted copy that should only be used by the variant
  // implementation.
  public:
    /* copy */  THIS_TYPE(  this_type const & b)  { maybe_used_only_for_debug( b);
                                                    d_assert( b.is_reset( ));
                                                    d_assert( is_reset( ));
                                                  }
    void        operator =( this_type const & b)  { maybe_used_only_for_debug( b);
                                                    d_assert( b.is_reset( ));
                                                    reset( );
                                                  }

  // Reset ctor
  // This was an attempt to get out of having to implement copy, above.
  // Unfortunately, it's not enough to satisfy boost variant.
  public:
  // /* ctor */  THIS_TYPE(  reset_type)           { d_assert( is_reset( )); }
  // void        operator =( reset_type)           { reset( ); }

  // Reset
  public:
    bool        is_reset( )                 const { return holder_.is_reset( ); }
    bool        not_reset( )                const { return holder_.not_reset( ); }
    void        reset( )                          { if ( holder_.not_reset( ) ) {
                                                      global::delete_shader( get_id( ));
                                                      holder_.reset( );
                                                    }
                                                    d_assert( is_reset( ));
                                                  }

  // Getters
  public:
    id_type     get_id( )                   const { return holder_.get_id( ); }
    holder_type const &
                get_holder( )               const { return holder_; }

  // Setter
  public:
    void        create_stage( enum_type tag, char_type const * p_src_string)
                                                  { reset( );
                                                    holder_.set_id(
                                                      global::create_shader(
                                                        tag, p_src_string));
                                                  }
    void        create_vertex_stage( char_type const * p_src_string)
                                                  ;
    void        create_fragment_stage( char_type const * p_src_string)
                                                  ;

  // Private member var
  private:
    holder_type  holder_ ;
};
# undef THIS_TYPE

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// program_holder_type
// program_owner_type
// _______________________________________________________________________________________________

# define THIS_TYPE program_holder_type
  class
THIS_TYPE
  : public shader_object_base_type< uint_type >
{
  // Typedefs
  private:
    typedef shader_object_base_type< uint_type >  base_type   ;
    typedef THIS_TYPE                             this_type   ;
  public:
    typedef program_owner_type                    other_type  ;

    typedef stage_holder_type                     stage_type  ;
    typedef other_type                            owner_type  ;
    typedef this_type                             holder_type ;
    typedef base_type::id_type                    id_type     ; /* uint_type */

    static bool const  is_stage_type    = false;
    static bool const  is_program_type  = true ;
    static bool const  is_holder_type   = true ;
    static bool const  is_owner_type    = false;

  // Ctor/Copy/Dtor
  public:
    /* ctor */   THIS_TYPE( )                     { d_assert( is_reset( )); }
    // Default dtor

    // Default copy ctor
    // Default copy assignment

    explicit     THIS_TYPE(  owner_type const &)  ; /* defined below in macros */
    this_type &  operator =( owner_type const &)  ; /* defined below in macros */

    explicit     THIS_TYPE(  id_type id)          { set_id( id); }
    this_type &  operator =( id_type id)          { set_id( id); return *this; }

  // Error checking wrapper on setter
  public:
    this_type &  set_id( id_type id)              { d_assert( (0 == id) || global::is_shader_program( id));
                                                    base_type::set_id( id);
                                                    return *this;
                                                  }

  // Make this the current shader program.
  public:
    this_type    activate( )                const { this_type const
                                                      previously_active_program =
                                                        this_type( global::get_active_shader_program( ));
                                                    global::use_shader_program( get_id( ));
                                                    d_assert( is_active( )); /* assumes you've already checked this is valid */
                                                    return previously_active_program;
                                                  }

    // is_active( ) also returns true if is_reset( ) and there is no active program (the default pipeline is active).
    bool         is_active( )               const { return get_id( ) == global::get_active_shader_program( ); }

  // Find and set uniforms
  public:
    int_type     get_uniform_location( char_type const * p_uniform_name)
                                            const { return global::get_uniform_location( get_id( ), p_uniform_name); }

  // All of these work on the currently active program, not the one associated with this object.
    void         set_uniform( int_type uniform_id, float_type v0)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform( uniform_id, v0);
                                                  }
    void         set_uniform( int_type uniform_id, float_type v0, float_type v1)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform( uniform_id, v0, v1);
                                                  }
    void         set_uniform( int_type uniform_id, float_type v0, float_type v1, float_type v2)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform( uniform_id, v0, v1, v2);
                                                  }
    void         set_uniform( int_type uniform_id, float_type v0, float_type v1, float_type v2, float_type v3)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform( uniform_id, v0, v1, v2, v3);
                                                  }
    void         set_uniform( int_type uniform_id, int_type v0)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform( uniform_id, v0);
                                                  }
    void         set_uniform( int_type uniform_id, int_type v0, int_type v1)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform( uniform_id, v0, v1);
                                                  }
    void         set_uniform( int_type uniform_id, int_type v0, int_type v1, int_type v2)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform( uniform_id, v0, v1, v2);
                                                  }
    void         set_uniform( int_type uniform_id, int_type v0, int_type v1, int_type v2, int_type v3)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform( uniform_id, v0, v1, v2, v3);
                                                  }
    void         set_uniform( int_type uniform_id, rgb_type< float_type > const & color)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform( uniform_id, color);
                                                  }
    void         set_uniform( int_type uniform_id, rgba_type< float_type > const & color)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform( uniform_id, color);
                                                  }
};
# undef THIS_TYPE

// _______________________________________________________________________________________________

# define THIS_TYPE program_owner_type
  class
THIS_TYPE
{
  // Typedefs
  private:
    typedef THIS_TYPE             this_type   ;
  public:
    typedef program_holder_type   other_type  ;

    typedef stage_owner_type      stage_type  ;
    typedef this_type             owner_type  ;
    typedef other_type            holder_type ;
    typedef holder_type::id_type  id_type     ; /* uint_type */

    BOOST_MPL_ASSERT(( boost::is_same< this_type, holder_type::owner_type >));

    static bool const  is_stage_type    = false;
    static bool const  is_program_type  = true ;
    static bool const  is_holder_type   = false;
    static bool const  is_owner_type    = true ;

  // Ctor/Dtor
  public:
    /* ctor */   THIS_TYPE( )                      { d_assert( is_reset( )); }
    /* dtor */   ~THIS_TYPE( )                     { reset( ); }

  // Disable copy -- we don't support multiple owners
  // No, unfortunately we have to choose between providing copy or not using variants.
  // So provide special highly-restricted copy that should only be used by the variant
  // implementation.
  public:
    /* copy */   THIS_TYPE(  this_type const & b)  { maybe_used_only_for_debug( b);
                                                     d_assert( b.is_reset( ));
                                                     d_assert( is_reset( ));
                                                   }
    void         operator =( this_type const & b)  { maybe_used_only_for_debug( b);
                                                     d_assert( b.is_reset( ));
                                                     reset( );
                                                   }

  // Reset ctor
  public:
  //  /* ctor */   THIS_TYPE(  reset_type)           { d_assert( is_reset( )); }
  //  void         operator =( reset_type)           { reset( ); }

  // Reset
  public:
    bool         is_reset( )                 const { return holder_.is_reset( ); }
    bool         not_reset( )                const { return holder_.not_reset( ); }
    void         reset( )                          { if ( holder_.not_reset( ) ) {
                                                       //global::detach_all_shaders( get_id( ));
                                                       global::delete_shader_program( get_id( ));
                                                       holder_.reset( );
                                                     }
                                                     d_assert( is_reset( ));
                                                   }

  // Getter
  public:
    id_type      get_id( )                   const { return holder_.get_id( ); }
    holder_type const &
                 get_holder( )               const { return holder_; }

  // Setter
  public:
    void         create_program
                  (  stage_holder_type const &  vertex_stage
                   , stage_holder_type const &  fragment_stage
                  )                                { reset( );
                                                     holder_.set_id(
                                                       global::create_shader_program
                                                        (  vertex_stage  .get_id( )
                                                         , fragment_stage.get_id( )
                                                        ));
                                                   }

  // Private member var
  private:
    holder_type  holder_ ;
};
# undef THIS_TYPE

// _______________________________________________________________________________________________

} /* end namespace normal */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

namespace ARB {

// _______________________________________________________________________________________________

class program_owner_type  ;
class program_holder_type ;

class stage_owner_type    ;
class stage_holder_type   ;

// _______________________________________________________________________________________________

  inline
  bool
is_supported( )
{
    return global::are_shaders_supported__ARB( );
}

  inline
  bool
is_explicitly_supported( )
{
    return global::are_shaders_explicitly_supported__ARB( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// stage_holder_type
// stage_owner_type
// _______________________________________________________________________________________________

# define THIS_TYPE stage_holder_type
  class
THIS_TYPE
  : public shader_object_base_type< GLhandleARB >
{
  // Typedefs
  private:
    typedef shader_object_base_type< GLhandleARB >  base_type    ;
    typedef THIS_TYPE                               this_type    ;
  public:
    typedef stage_owner_type                        other_type   ;

    typedef program_holder_type                     program_type ;
    typedef other_type                              owner_type   ;
    typedef this_type                               holder_type  ;
    typedef base_type::id_type                      id_type      ; /* GLhandleARB */

    static bool const  is_stage_type    = true ;
    static bool const  is_program_type  = false;
    static bool const  is_holder_type   = true ;
    static bool const  is_owner_type    = false;

  // Ctor/Copy/Dtor
  public:
    /* ctor */   THIS_TYPE( )                     { d_assert( is_reset( )); }
    // Default dtor

    // Default copy ctor
    // Default copy assignment

    explicit     THIS_TYPE(  owner_type const &)  ; /* defined below in macros */
    this_type &  operator =( owner_type const &)  ; /* defined below in macros */

  // Problem: Because set_id(..) cannot check to make sure the id is OK, these are not safe.
  // Solution: Make these protected instead of public. Declare owner as friend.
  protected:
    explicit     THIS_TYPE(  id_type id)          { set_id( id); }
    this_type &  operator =( id_type id)          { set_id( id); return *this; }

  // Error checking wrapper on setter
  // Solution: Make these protected instead of public. Declare owner as friend.
  //   No, that messes up the set_id(..) template function. We just have to risk it.
  public:
    // Problem: These are not safe because there is no global::is_shader__ARB( id) predicate.
    this_type &  set_id( id_type id)              { //d_assert( (0 == id) || global::is_shader__ARB( id));
                                                    base_type::set_id( id);
                                                    return *this;
                                                  }
  friend class stage_owner_type;
  // g++ objects if the above friend is "class owner_type" instead of "class stage_owner_type".
  // Is it because it is a typedef?
};
# undef THIS_TYPE

// _______________________________________________________________________________________________

# define THIS_TYPE stage_owner_type
  class
THIS_TYPE
{
  // Typedefs
  private:
    typedef THIS_TYPE             this_type    ;
  public:
    typedef stage_holder_type     other_type   ;

    typedef program_owner_type    program_type ;
    typedef this_type             owner_type   ;
    typedef other_type            holder_type  ;
    typedef holder_type::id_type  id_type      ; /* GLhandleARB */

    BOOST_MPL_ASSERT(( boost::is_same< this_type, holder_type::owner_type >));

    static bool const  is_stage_type    = true ;
    static bool const  is_program_type  = false;
    static bool const  is_holder_type   = false;
    static bool const  is_owner_type    = true ;

  // Ctor/Dtor
  public:
    /* ctor */  THIS_TYPE( )                      { d_assert( is_reset( )); }
    /* dtor */  ~THIS_TYPE( )                     { reset( ); }

  // Disable copy -- we don't support multiple owners
  // No, unfortunately we have to choose between providing copy or not using variants.
  // So provide special highly-restricted copy that should only be used by the variant
  // implementation.
  public:
    /* copy */  THIS_TYPE(  this_type const & b)  { maybe_used_only_for_debug( b);
                                                    d_assert( b.is_reset( ));
                                                    d_assert( is_reset( ));
                                                  }
    void        operator =( this_type const & b)  { maybe_used_only_for_debug( b);
                                                    d_assert( b.is_reset( ));
                                                    reset( );
                                                  }

  // Reset ctor
  public:
  // /* ctor */  THIS_TYPE(  reset_type)           { d_assert( is_reset( )); }
  // void        operator =( reset_type)           { reset( ); }

  // Reset
  public:
    bool        is_reset( )                 const { return holder_.is_reset( ); }
    bool        not_reset( )                const { return holder_.not_reset( ); }
    void        reset( )                          { if ( holder_.not_reset( ) ) {
                                                      //global::detach_all_shaders__ARB( get_id( ));
                                                      global::delete_shader_object__ARB( get_id( ));
                                                      holder_.reset( );
                                                    }
                                                    d_assert( is_reset( ));
                                                  }

  // Getter
  public:
    id_type     get_id( )                   const { return holder_.get_id( ); }
    holder_type const &
                get_holder( )               const { return holder_; }

  // Setter
  public:
    void        create_stage( enum_type tag, char_type const * p_src_string)
                                                  { reset( );
                                                    holder_.set_id(
                                                      global::create_shader__ARB(
                                                        tag, p_src_string));
                                                  }
    void        create_vertex_stage( char_type const * p_src_string)
                                                  ;
    void        create_fragment_stage( char_type const * p_src_string)
                                                  ;

  // Private member var
  private:
    holder_type  holder_ ;
};
# undef THIS_TYPE

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// program_holder_type
// program_owner_type
// _______________________________________________________________________________________________

# define THIS_TYPE program_holder_type
  class
THIS_TYPE
  : public shader_object_base_type< GLhandleARB >
{
  // Typedefs
  private:
    typedef shader_object_base_type< GLhandleARB >  base_type   ;
    typedef THIS_TYPE                               this_type   ;
  public:
    typedef program_owner_type                      other_type  ;

    typedef stage_holder_type                       stage_type  ;
    typedef other_type                              owner_type  ;
    typedef this_type                               holder_type ;
    typedef base_type::id_type                      id_type     ; /* GLhandleARB */

    static bool const  is_stage_type    = false;
    static bool const  is_program_type  = true ;
    static bool const  is_holder_type   = true ;
    static bool const  is_owner_type    = false;

  // Ctor/Copy/Dtor
  public:
    /* ctor */   THIS_TYPE( )                     { d_assert( is_reset( )); }
    // Default dtor

    // Default copy ctor
    // Default copy assignment

    explicit     THIS_TYPE(  owner_type const &)  ; /* defined below in macros */
    this_type &  operator =( owner_type const &)  ; /* defined below in macros */

    explicit     THIS_TYPE(  id_type id)          { set_id( id); }
    this_type &  operator =( id_type id)          { set_id( id); return *this; }

  // Error checking wrapper on setter
  public:
    this_type &  set_id( id_type id)              { d_assert( (0 == id) || global::is_shader_program__ARB( id));
                                                    base_type::set_id( id);
                                                    return *this;
                                                  }

  // Make this the current shader program.
  public:
    this_type    activate( )                const { this_type const
                                                      previously_active_program =
                                                        this_type( global::get_active_shader_program__ARB( ));
                                                    global::use_shader_program__ARB( get_id( ));
                                                    d_assert( is_active( ));
                                                    return previously_active_program;
                                                  }

    // is_active( ) also returns true if is_reset( ) and there is no active program (the default pipeline is active).
    bool         is_active( )               const { return get_id( ) == global::get_active_shader_program__ARB( ); }

  // Find and set uniforms
  public:
    int_type     get_uniform_location( char_type const * p_uniform_name)
                                            const { return global::get_uniform_location__ARB( get_id( ), p_uniform_name); }

  // All of these work on the currently active program, not the one associated with this object.
    void         set_uniform( int_type uniform_id, float_type v0)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform__ARB( uniform_id, v0);
                                                  }
    void         set_uniform( int_type uniform_id, float_type v0, float_type v1)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform__ARB( uniform_id, v0, v1);
                                                  }
    void         set_uniform( int_type uniform_id, float_type v0, float_type v1, float_type v2)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform__ARB( uniform_id, v0, v1, v2);
                                                  }
    void         set_uniform( int_type uniform_id, float_type v0, float_type v1, float_type v2, float_type v3)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform__ARB( uniform_id, v0, v1, v2, v3);
                                                  }
    void         set_uniform( int_type uniform_id, int_type v0)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform__ARB( uniform_id, v0);
                                                  }
    void         set_uniform( int_type uniform_id, int_type v0, int_type v1)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform__ARB( uniform_id, v0, v1);
                                                  }
    void         set_uniform( int_type uniform_id, int_type v0, int_type v1, int_type v2)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform__ARB( uniform_id, v0, v1, v2);
                                                  }
    void         set_uniform( int_type uniform_id, int_type v0, int_type v1, int_type v2, int_type v3)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform__ARB( uniform_id, v0, v1, v2, v3);
                                                  }
    void         set_uniform( int_type uniform_id, rgb_type< float_type > const & color)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform__ARB( uniform_id, color);
                                                  }
    void         set_uniform( int_type uniform_id, rgba_type< float_type > const & color)
                                            const { d_assert( is_active( ));
                                                    global::set_uniform__ARB( uniform_id, color);
                                                  }
};
# undef THIS_TYPE

// _______________________________________________________________________________________________

# define THIS_TYPE program_owner_type
  class
THIS_TYPE
{
  // Typedefs
  private:
    typedef THIS_TYPE             this_type   ;
  public:
    typedef program_holder_type   other_type  ;

    typedef stage_owner_type      stage_type  ;
    typedef this_type             owner_type  ;
    typedef other_type            holder_type ;
    typedef holder_type::id_type  id_type     ; /* GLhandleARB */

    BOOST_MPL_ASSERT(( boost::is_same< this_type, holder_type::owner_type >));

    static bool const  is_stage_type    = false;
    static bool const  is_program_type  = true ;
    static bool const  is_holder_type   = false;
    static bool const  is_owner_type    = true ;

  // Ctor/Dtor
  public:
    /* ctor */   THIS_TYPE( )                      { d_assert( is_reset( )); }
    /* dtor */   ~THIS_TYPE( )                     { reset( ); }

  // Disable copy -- we don't support multiple owners
  // No, unfortunately we have to choose between providing copy or not using variants.
  // So provide special highly-restricted copy that should only be used by the variant
  // implementation.
  public:
    /* copy */   THIS_TYPE(  this_type const & b)  { maybe_used_only_for_debug( b);
                                                     d_assert( b.is_reset( ));
                                                     d_assert( is_reset( ));
                                                   }
    void         operator =( this_type const & b)  { maybe_used_only_for_debug( b);
                                                     d_assert( b.is_reset( ));
                                                     reset( );
                                                   }

  // Reset ctor
  public:
  // /* ctor */   THIS_TYPE(  reset_type)           { d_assert( is_reset( )); }
  // void         operator =( reset_type)           { reset( ); }

  // Reset
  public:
    bool         is_reset( )                 const { return holder_.is_reset( ); }
    bool         not_reset( )                const { return holder_.not_reset( ); }
    void         reset( )                          { if ( holder_.not_reset( ) ) {
                                                       global::delete_shader_object__ARB( get_id( ));
                                                       holder_.reset( );
                                                     }
                                                     d_assert( is_reset( ));
                                                   }

  // Getter
  public:
    id_type      get_id( )                   const { return holder_.get_id( ); }
    holder_type const &
                 get_holder( )               const { return holder_; }

  // Setter
  public:
    void         create_program
                  (  stage_holder_type const &  vertex_stage
                   , stage_holder_type const &  fragment_stage
                  )                                { reset( );
                                                     holder_.set_id(
                                                       global::create_shader_program__ARB
                                                        (  vertex_stage  .get_id( )
                                                         , fragment_stage.get_id( )
                                                        ));
                                                   }

  // Private member var
  private:
    holder_type  holder_ ;
};
# undef THIS_TYPE

// _______________________________________________________________________________________________

} /* end namespace ARB */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Define mixed-type methods and functions
//
//   stage_holder_type( stage_owner_type)     - copy ctor
//   stage_holder_type = stage_owner_type     - copy assignment
//
//   stage_holder_type == stage_holder_type   - equality operator
//   stage_holder_type  < stage_holder_type   - inequality operator
//
//   stage_owner_type == stage_owner_type     - equality operator
//   stage_owner_type  < stage_owner_type     - inequality operator
//
//   stage_holder_type == stage_owner_type    - equality operator
//   stage_holder_type  < stage_owner_type    - inequality operator
//
//   stage_owner_type == stage_holder_type    - equality operator
//   stage_owner_type  < stage_holder_type    - inequality operator
// _______________________________________________________________________________________________

# define DEFINE_COPY_CTOR_AND_ASSOP_( OWNER_TYPE, HOLDER_TYPE)              \
    inline HOLDER_TYPE::HOLDER_TYPE( OWNER_TYPE const & src)                \
        { set_id( src.get_id( )); }                                         \
    inline HOLDER_TYPE & HOLDER_TYPE::operator =( OWNER_TYPE const & src)   \
        { set_id( src.get_id( )); return *this; }                           /* end macro */

# define DEFINE_COMPARE_OP_( OP, A_TYPE, B_TYPE)                            \
    inline bool operator OP( A_TYPE const & a, B_TYPE const & b)            \
        { return a.get_id( ) OP b.get_id( ); }                              /* end macro */

# define DEFINE_COMPARE_OPS_( OP, A_TYPE, B_TYPE)                           \
    DEFINE_COMPARE_OP_( OP, A_TYPE, A_TYPE)                                 \
    DEFINE_COMPARE_OP_( OP, B_TYPE, B_TYPE)                                 \
    DEFINE_COMPARE_OP_( OP, A_TYPE, B_TYPE)                                 \
    DEFINE_COMPARE_OP_( OP, B_TYPE, A_TYPE)                                 /* end macro */

# define DEFINE_CTOR_ASSOP_OPS( OWNER_TYPE, HOLDER_TYPE)                    \
    DEFINE_COPY_CTOR_AND_ASSOP_( OWNER_TYPE, HOLDER_TYPE)                   \
    DEFINE_COMPARE_OPS_( == , OWNER_TYPE, HOLDER_TYPE)                      \
    DEFINE_COMPARE_OPS_( != , OWNER_TYPE, HOLDER_TYPE)                      \
    DEFINE_COMPARE_OPS_(  < , OWNER_TYPE, HOLDER_TYPE)                      /* end macro */

// _______________________________________________________________________________________________

namespace normal {
DEFINE_CTOR_ASSOP_OPS( stage_owner_type  , stage_holder_type   )
DEFINE_CTOR_ASSOP_OPS( program_owner_type, program_holder_type )
} /* end namespace normal */

namespace ARB {
DEFINE_CTOR_ASSOP_OPS( stage_owner_type  , stage_holder_type   )
DEFINE_CTOR_ASSOP_OPS( program_owner_type, program_holder_type )
} /* end namespace ARB */

// _______________________________________________________________________________________________

# undef DEFINE_CTOR_ASSOP_OPS
# undef DEFINE_COMPARE_OPS_
# undef DEFINE_COMPARE_OP_
# undef DEFINE_COPY_CTOR_AND_ASSOP_

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  typedef
  boost::variant
   <  tag::reset_type
    , normal::program_holder_type
    , ARB::program_holder_type
   >
program_holder_variant_type ;

  typedef
  boost::variant
   <  tag::reset_type
    , normal::program_owner_type
    , ARB::program_owner_type
   >
program_owner_variant_type ;

  typedef
  boost::variant
   <  tag::reset_type
    , normal::stage_holder_type
    , ARB::stage_holder_type
   >
stage_holder_variant_type ;

  typedef
  boost::variant
   <  tag::reset_type
    , normal::stage_owner_type
    , ARB::stage_owner_type
   >
stage_owner_variant_type ;

// _______________________________________________________________________________________________

  template< typename VARIANT_TYPE >
  struct
variant_traits_type
{
    // This static_assert(..) doesn't work with gcc -Wall
    // even though this part of the template struct is never instantiated.
  # ifdef WIN32
    d_static_assert( false);
  # endif
};

  template< >
  struct
variant_traits_type< program_holder_variant_type >
{
    typedef normal::program_holder_type  normal_type ;
    typedef ARB   ::program_holder_type  ARB_type    ;
};

  template< >
  struct
variant_traits_type< program_owner_variant_type >
{
    typedef normal::program_owner_type  normal_type ;
    typedef ARB   ::program_owner_type  ARB_type    ;
};

  template< >
  struct
variant_traits_type< stage_holder_variant_type >
{
    typedef normal::stage_holder_type  normal_type ;
    typedef ARB   ::stage_holder_type  ARB_type    ;
};

  template< >
  struct
variant_traits_type< stage_owner_variant_type >
{
    typedef normal::stage_owner_type  normal_type ;
    typedef ARB   ::stage_owner_type  ARB_type    ;
};

// _______________________________________________________________________________________________

// Not used. Not tested.
# if 0

  template< typename VARIANT_TRAITS_TYPE >
  struct
is_stage_type
{
    // Rely on the normal_type here. The value in the ARB_type should be the same.
    typedef typename VARIANT_TRAITS_TYPE::normal_type normal_type;
    static bool const value = normal_type::is_stage_type;
};

  template< typename VARIANT_TRAITS_TYPE >
  struct
is_program_type
{
    // Rely on the normal_type here. The value in the ARB_type should be the same.
    typedef typename VARIANT_TRAITS_TYPE::normal_type normal_type;
    static bool const value = normal_type::is_program_type;
};

  template< typename VARIANT_TRAITS_TYPE >
  struct
is_holder_type
{
    // Rely on the normal_type here. The value in the ARB_type should be the same.
    typedef typename VARIANT_TRAITS_TYPE::normal_type normal_type;
    static bool const value = normal_type::is_holder_type;
};

  template< typename VARIANT_TRAITS_TYPE >
  struct
is_owner_type
{
    // Rely on the normal_type here. The value in the ARB_type should be the same.
    typedef typename VARIANT_TRAITS_TYPE::normal_type normal_type;
    static bool const value = normal_type::is_owner_type;
};

# endif

// _______________________________________________________________________________________________

  template< typename VARIANT_TYPE >
  void
init_type_binding( VARIANT_TYPE & a)
{
    typedef typename variant_traits_type< VARIANT_TYPE >::normal_type  normal_type ;
    typedef typename variant_traits_type< VARIANT_TYPE >::ARB_type     ARB_type    ;

    if ( normal::is_supported( ) ) {
        a = normal_type( );
    } else
    if ( ARB::is_supported( ) ) {
        a = ARB_type( );
    } else {
        deep_reset( a);
    }
    d_assert( is_reset( a));
}

  template< typename VARIANT_TYPE >
  void
init_type_binding_explicitly( VARIANT_TYPE & a)
{
    typedef typename variant_traits_type< VARIANT_TYPE >::normal_type  normal_type ;
    typedef typename variant_traits_type< VARIANT_TYPE >::ARB_type     ARB_type    ;

    if ( normal::is_explicitly_supported( ) ) {
        a = normal_type( );
    } else
    if ( ARB::is_explicitly_supported( ) ) {
        a = ARB_type( );
    } else {
        deep_reset( a);
    }
    d_assert( is_reset( a));
}

  template< typename T >
  T
init_type_binding( )
{
    T inst;
    init_type_binding( inst);
    return inst;
}

  template< typename T >
  T
init_type_binding_explicitly( )
{
    T inst;
    init_type_binding_explicitly( inst);
    return inst;
}

// _______________________________________________________________________________________________

// Functions to set and get variant id.
// You cannot set the id for a variant that is_deep_reset( ) UNLESS that id == 0.
extern uint_type  get_id( stage_holder_variant_type   const &);
extern uint_type  get_id( stage_owner_variant_type    const &);
extern uint_type  get_id( program_holder_variant_type const &);
extern uint_type  get_id( program_owner_variant_type  const &);

extern void  set_id( stage_holder_variant_type   &, uint_type);
extern void  set_id( stage_owner_variant_type    &, uint_type);
extern void  set_id( program_holder_variant_type &, uint_type);
extern void  set_id( program_owner_variant_type  &, uint_type);

// Template functions for variant types:
template< typename T > inline void  deep_reset(     T       & a)  { a = tag::reset_type( ); }
template< typename T > inline bool  is_deep_reset(  T const & a)  { return 0 != boost::get< tag::reset_type >( & a); }
template< typename T > inline bool  not_deep_reset( T const & a)  { return ! is_deep_reset( a); }

// Shallow reset:
// is_deep_reset( ) --implies--> is_reset( )
template< typename T > inline void  reset(          T       & a)  { set_id( a, 0); }
template< typename T > inline bool  is_reset(       T const & a)  { return 0 == get_id( a); }
template< typename T > inline bool  not_reset(      T const & a)  { return ! is_reset( a); }

// _______________________________________________________________________________________________

  inline
  bool
are_shaders_supported( )
{
    return not_deep_reset( init_type_binding< stage_holder_variant_type >( ));
}

  inline
  bool
are_shaders_explicitly_supported( )
{
    return not_deep_reset( init_type_binding_explicitly< stage_holder_variant_type >( ));
}

// _______________________________________________________________________________________________

extern stage_holder_variant_type    get_holder( stage_owner_variant_type   const &);
extern program_holder_variant_type  get_holder( program_owner_variant_type const &);

// _______________________________________________________________________________________________

  extern
  void
create_stage
 (  stage_owner_variant_type &  stage_owner
  , enum_type                   tag
  , char_type const *           p_src_string
 );

  extern
  void
create_vertex_stage
 (  stage_owner_variant_type &  stage_owner
  , char_type const *           p_src_string
 );

  extern
  void
create_fragment_stage
 (  stage_owner_variant_type &  stage_owner
  , char_type const *           p_src_string
 );

  extern
  void
create_program
 (  program_owner_variant_type      &  program_owner
  , stage_holder_variant_type const &  vertex_stage
  , stage_holder_variant_type const &  fragment_stage
 );

// _______________________________________________________________________________________________

  extern
  int_type
get_uniform_location
 (  program_holder_variant_type const &  program_holder
  , char_type const *                    p_uniform_name
 );

// _______________________________________________________________________________________________
//
// These assume the holder is not deep-reset. It can be shallow-reset though, which means the
// the default pipeline shader.

  extern
  bool
is_active
 (  program_holder_variant_type const &  program_holder  /* assumes not deep-reset */
 );

  extern
  program_holder_variant_type
activate
 (  program_holder_variant_type const &  program_holder  /* assumes not deep-reset */
 );

// _______________________________________________________________________________________________

  extern
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , rgb_type< float_type > const &       color
 );

  extern
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , rgba_type< float_type > const &      color
 );

  extern
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , float_type                           v0
 );

  extern
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , float_type                           v0
  , float_type                           v1
 );

  extern
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , float_type                           v0
  , float_type                           v1
  , float_type                           v2
 );

  extern
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , float_type                           v0
  , float_type                           v1
  , float_type                           v2
  , float_type                           v3
 );

  extern
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , int_type                             v0
 );

  extern
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , int_type                             v0
  , int_type                             v1
 );

  extern
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , int_type                             v0
  , int_type                             v1
  , int_type                             v2
 );

  extern
  void
set_uniform
 (  program_holder_variant_type const &  program_holder
  , int_type                             uniform_id
  , int_type                             v0
  , int_type                             v1
  , int_type                             v2
  , int_type                             v3
 );

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* owner class for holding a shader program and it's associated vertex & fragment stages */
  class
program_owner_any_type
{
  // Ctor and dtor.
  public:
    /* ctor */    program_owner_any_type
                   (  char const *  p_src_vertex_stage
                    , char const *  p_src_fragment_stage
                   )                                    ;

    // Should the dtor uninit( )? No, because uninit( ) should only be used when
    // the associated gl-context still exists.
    virtual       ~program_owner_any_type( )            { }

    // Copy is disabled because member vars do not support it.

  # ifdef NDEBUG
    void          assert_invariants( )            const { }
  # else
    void          assert_invariants( )            const ;
  # endif

  // Getters
  public:
    bool          is_init_attempted( )            const { return is_init_attempted_; }
    bool          is_init_successful( )           const { return is_init_successful_; }

    // This will init( ) if necessary.
    program_holder_variant_type
                  get_program( )        /* NOT const */ { init( );
                                                          return get_program__no_init( );
                                                        }

    // This does NOT init( ). The returned variant may be is_deep_reset( program_holder_variant).
    program_holder_variant_type
                  get_program__no_init( )         const { return get_holder( program_); }

  // Init
  public:
    virtual void  init( )                               ;
    void          uninit( )                             ;

  protected:
    void          init_private( )                       ;

  public:
    int_type      get_uniform_location( char_type const * p_uniform_name)
                                                  const ;

    program_holder_variant_type /* returns the previously active program */
                  activate( )                     const ; /* assumes program is compiled and valid */

    void          set_uniform( int_type uniform_id, rgb_type< float_type > const &)
                                                  const ;
    void          set_uniform( int_type uniform_id, rgba_type< float_type > const &)
                                                  const ;
    void          set_uniform( int_type uniform_id, float_type v0)
                                                  const ;
    void          set_uniform( int_type uniform_id, float_type, float_type)
                                                  const ;
    void          set_uniform( int_type uniform_id, float_type, float_type, float_type)
                                                  const ;
    void          set_uniform( int_type uniform_id, float_type, float_type, float_type, float_type)
                                                  const ;

    void          set_uniform( int_type uniform_id, int_type v0)
                                                  const ;
    void          set_uniform( int_type uniform_id, int_type, int_type)
                                                  const ;
    void          set_uniform( int_type uniform_id, int_type, int_type, int_type)
                                                  const ;
    void          set_uniform( int_type uniform_id, int_type, int_type, int_type, int_type)
                                                  const ;

  // Private member vars
  private:
    bool                        is_init_attempted_    ;
    bool                        is_init_successful_   ;

    char const * const          p_vertex_stage_src_   ;
    stage_owner_variant_type    vertex_stage_         ;

    char const * const          p_fragment_stage_src_ ;
    stage_owner_variant_type    fragment_stage_       ;

    program_owner_variant_type  program_              ;
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

// No longer used. Good for a generic shader with no custom uniforms or attribs.
# if 0
  /* wrapper class for activating shader programs */
  class
with_program
  //
  // Avoid using this to push an ARB shader on top of a normal shader, and vice versa.
{
  // Wrapper ctor/dtors
  public:
    /* ctor */  with_program( )                                              ;
    /* ctor */  with_program( program_owner_any_type            &)           ; /* compiles program if necessary, warns user */
    /* ctor */  with_program( program_owner_any_type      const &)           ; /* assumes program already compiled */
    /* ctor */  with_program( program_holder_variant_type const &)           ; /* assumes not reset */
    /* dtor */  ~with_program( )                                             { restore( ); }

  // Disable copy
  private:
    /* copy */  with_program( with_program const &)                          ; // no implementation
    void        operator =(   with_program const &)                          ; // no implementation

  public:
    bool        has_restore( )                                         const { return not_deep_reset( previous_program_); }
    bool        restore( )                                                   ; /* clears previous_program_ */
    bool        forget__no_restore( )                                        ; /* clears previous_program_ */

    // These will save the previously active program so it can be restored, but only if
    // previous_program_ is not set.
    void        activate(          program_owner_any_type            &)      ;
    void        activate__no_init( program_owner_any_type      const &)      ;
    void        activate(          program_holder_variant_type const &)      ;

  // Member vars
  private:
    program_holder_variant_type  previous_program_ ;
};
# endif

// _______________________________________________________________________________________________

} /* end namespace shader */
} /* end namespace gl_env */

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef GL_SHADER_H
//
// gl_shader.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
