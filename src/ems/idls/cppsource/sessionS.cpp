// -*- C++ -*-
//
// $Id$

// ****  Code generated by the The ACE ORB (TAO) IDL Compiler ****
// TAO and the TAO IDL Compiler have been developed by:
//       Center for Distributed Object Computing
//       Washington University
//       St. Louis, MO
//       USA
//       http://www.cs.wustl.edu/~schmidt/doc-center.html
// and
//       Distributed Object Computing Laboratory
//       University of California at Irvine
//       Irvine, CA
//       USA
//       http://doc.ece.uci.edu/
// and
//       Institute for Software Integrated Systems
//       Vanderbilt University
//       Nashville, TN
//       USA
//       http://www.isis.vanderbilt.edu/
//
// Information about TAO is available at:
//     http://www.cs.wustl.edu/~schmidt/TAO.html


// TAO_IDL - Generated from 
// ../../../TAO/TAO_IDL/be/be_codegen.cpp:703

#ifndef _TAO_IDL_CPPSOURCE_SESSIONS_CPP_
#define _TAO_IDL_CPPSOURCE_SESSIONS_CPP_


#include "sessionS.h"
#include "tao/PortableServer/Operation_Table_Perfect_Hash.h"
#include "tao/PortableServer/Upcall_Command.h"
#include "tao/PortableServer/Upcall_Wrapper.h"
#include "tao/TAO_Server_Request.h"
#include "tao/ORB_Core.h"
#include "tao/Profile.h"
#include "tao/Stub.h"
#include "tao/IFR_Client_Adapter.h"
#include "tao/Object_T.h"
#include "tao/AnyTypeCode/TypeCode.h"
#include "tao/AnyTypeCode/DynamicC.h"
#include "tao/CDR.h"
#include "tao/operation_details.h"
#include "tao/PortableInterceptor.h"
#include "tao/PortableServer/Basic_SArguments.h"
#include "tao/PortableServer/Object_SArgument_T.h"
#include "tao/PortableServer/Special_Basic_SArguments.h"
#include "tao/PortableServer/UB_String_SArguments.h"
#include "tao/PortableServer/TypeCode_SArg_Traits.h"
#include "tao/PortableServer/Object_SArg_Traits.h"
#include "tao/PortableServer/get_arg.h"
#include "tao/Special_Basic_Arguments.h"
#include "tao/UB_String_Arguments.h"
#include "tao/Basic_Arguments.h"
#include "tao/Object_Argument_T.h"
#include "ace/Dynamic_Service.h"
#include "ace/Malloc_Allocator.h"

#if !defined (__ACE_INLINE__)
#include "sessionS.inl"
#endif /* !defined INLINE */

// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_arg_traits.cpp:73

TAO_BEGIN_VERSIONED_NAMESPACE_DECL


// Arg traits specializations.
namespace TAO
{
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_arg_traits.cpp:141

#if !defined (_SESSION_SESSION_I__SARG_TRAITS_)
#define _SESSION_SESSION_I__SARG_TRAITS_
  
  template<>
  class  SArg_Traits<session::Session_I>
    : public
        Object_SArg_Traits_T<
            session::Session_I_ptr,
            session::Session_I_var,
            session::Session_I_out,
            TAO::Any_Insert_Policy_Stream <session::Session_I_ptr>
          >
  {
  };

#endif /* end #if !defined */
}

TAO_END_VERSIONED_NAMESPACE_DECL



// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_arg_traits.cpp:73

TAO_BEGIN_VERSIONED_NAMESPACE_DECL


// Arg traits specializations.
namespace TAO
{
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_arg_traits.cpp:141

#if !defined (_SESSION_SESSION_I__ARG_TRAITS_)
#define _SESSION_SESSION_I__ARG_TRAITS_
  
  template<>
  class  Arg_Traits<session::Session_I>
    : public
        Object_Arg_Traits_T<
            session::Session_I_ptr,
            session::Session_I_var,
            session::Session_I_out,
            TAO::Objref_Traits<session::Session_I>,
            TAO::Any_Insert_Policy_Stream <session::Session_I_ptr>
          >
  {
  };

#endif /* end #if !defined */
}

TAO_END_VERSIONED_NAMESPACE_DECL



// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_interface.cpp:1549

class TAO_session_Session_I_Perfect_Hash_OpTable
  : public TAO_Perfect_Hash_OpTable
{
private:
  unsigned int hash (const char *str, unsigned int len);

public:
  const TAO_operation_db_entry * lookup (const char *str, unsigned int len);
};

/* C++ code produced by gperf version 2.8 (ACE version) */
/* Command-line: gperf -m -M -J -c -C -D -E -T -f 0 -F 0,0 -a -o -t -p -K opname -L C++ -Z TAO_session_Session_I_Perfect_Hash_OpTable -N lookup  */
unsigned int
TAO_session_Session_I_Perfect_Hash_OpTable::hash (const char *str, unsigned int len)
{
  static const unsigned char asso_values[] =
    {
#if defined (ACE_MVS)
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28,  0,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28,  0,
     28, 28,  0,  5, 28,  0, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28,  5,
     28,  0, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28,  0, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28,
#else
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28,  0, 28,  0, 28, 28,
      0,  5, 28,  0, 28, 28, 28, 28, 28, 28,
      5, 28,  0, 28, 28, 28,  0, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28,
#endif /* ACE_MVS */
    };
  return len + asso_values[(int) str[len - 1]] + asso_values[(int) str[0]];
}

const TAO_operation_db_entry *
TAO_session_Session_I_Perfect_Hash_OpTable::lookup (const char *str, unsigned int len)
{
  enum
    {
      TOTAL_KEYWORDS = 8,
      MIN_WORD_LENGTH = 4,
      MAX_WORD_LENGTH = 22,
      MIN_HASH_VALUE = 4,
      MAX_HASH_VALUE = 27,
      HASH_VALUE_RANGE = 24,
      DUPLICATES = 0,
      WORDLIST_SIZE = 12
    };

  static const TAO_operation_db_entry  wordlist[] =
    {
      {"",0,0},{"",0,0},{"",0,0},{"",0,0},
      {"ping", &POA_session::Session_I::ping_skel, 0},
      {"_is_a", &POA_session::Session_I::_is_a_skel, 0},
      {"",0,0},{"",0,0},{"",0,0},{"",0,0},
      {"_component", &POA_session::Session_I::_component_skel, 0},
      {"",0,0},{"",0,0},
      {"_non_existent", &POA_session::Session_I::_non_existent_skel, 0},
      {"_repository_id", &POA_session::Session_I::_repository_id_skel, 0},
      {"_interface", &POA_session::Session_I::_interface_skel, 0},
      {"",0,0},{"",0,0},{"",0,0},{"",0,0},
      {"endSession", &POA_session::Session_I::endSession_skel, 0},
      {"",0,0},{"",0,0},{"",0,0},{"",0,0},{"",0,0},{"",0,0},
      {"_get_associatedSession", &POA_session::Session_I::_get_associatedSession_skel, 0},
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= MIN_HASH_VALUE)
        {
          const char *s = wordlist[key].opname;

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1))
            return &wordlist[key];
        }
    }
  return 0;
}

static TAO_session_Session_I_Perfect_Hash_OpTable tao_session_Session_I_optable;

// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_interface/interface_ss.cpp:958

TAO::Collocation_Proxy_Broker *
session__TAO_Session_I_Proxy_Broker_Factory_function ( ::CORBA::Object_ptr)
{
  return reinterpret_cast<TAO::Collocation_Proxy_Broker *> (0xdead); // Dummy
}

int
session__TAO_Session_I_Proxy_Broker_Factory_Initializer (size_t)
{
  session__TAO_Session_I_Proxy_Broker_Factory_function_pointer = 
    session__TAO_Session_I_Proxy_Broker_Factory_function;
  
  return 0;
}

static int
session__TAO_Session_I_Proxy_Broker_Stub_Factory_Initializer_Scarecrow =
  session__TAO_Session_I_Proxy_Broker_Factory_Initializer (
      reinterpret_cast<size_t> (session__TAO_Session_I_Proxy_Broker_Factory_Initializer)
    );

// TAO_IDL - Generated from 
// ../../../TAO/TAO_IDL/be/be_visitor_interface/interface_ss.cpp:103

POA_session::Session_I::Session_I (void)
  : TAO_ServantBase ()
{
  this->optable_ = &tao_session_Session_I_optable;
}

POA_session::Session_I::Session_I (const Session_I& rhs)
  : TAO_Abstract_ServantBase (rhs),
    TAO_ServantBase (rhs)
{
}

POA_session::Session_I::~Session_I (void)
{
}
namespace POA_session
{
  
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_operation/upcall_command_ss.cpp:136
  
  class _get_associatedSession_Session_I
    : public TAO::Upcall_Command
  {
  public:
    inline _get_associatedSession_Session_I (
      POA_session::Session_I * servant,
      TAO_Operation_Details const * operation_details,
      TAO::Argument * const args[])
      : servant_ (servant)
        , operation_details_ (operation_details)
        , args_ (args)
    {
    }
    
    virtual void execute (void)
    {
      TAO::SArg_Traits< ::session::Session_I>::ret_arg_type retval =
        TAO::Portable_Server::get_ret_arg< ::session::Session_I> (
          this->operation_details_,
          this->args_);
      
      retval =
        this->servant_->associatedSession ();
    }
  
  private:
    POA_session::Session_I * const servant_;
    TAO_Operation_Details const * const operation_details_;
    TAO::Argument * const * const args_;
  };
}


// TAO_IDL - Generated from 
// ../../../TAO/TAO_IDL/be/be_visitor_operation/operation_ss.cpp:190

void POA_session::Session_I::_get_associatedSession_skel (
    TAO_ServerRequest & server_request,
    void * TAO_INTERCEPTOR (servant_upcall),
    void * servant)
{
#if TAO_HAS_INTERCEPTORS == 1
  static ::CORBA::TypeCode_ptr const * const exceptions = 0;
  static ::CORBA::ULong const nexceptions = 0;
#endif /* TAO_HAS_INTERCEPTORS */
  
  TAO::SArg_Traits< ::session::Session_I>::ret_val retval;
  
  TAO::Argument * const args[] =
    {
      &retval
    };
  
  static size_t const nargs = 1;
  
  POA_session::Session_I * const impl =
    static_cast<POA_session::Session_I *> (servant);

  _get_associatedSession_Session_I command (
    impl,
    server_request.operation_details (),
    args);
  
  TAO::Upcall_Wrapper upcall_wrapper;
  upcall_wrapper.upcall (server_request
                         , args
                         , nargs
                         , command
#if TAO_HAS_INTERCEPTORS == 1
                         , servant_upcall
                         , exceptions
                         , nexceptions
#endif  /* TAO_HAS_INTERCEPTORS == 1 */
                         );
}

namespace POA_session
{
  
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_operation/upcall_command_ss.cpp:136
  
  class ping_Session_I
    : public TAO::Upcall_Command
  {
  public:
    inline ping_Session_I (
      POA_session::Session_I * servant)
      : servant_ (servant)
    {
    }
    
    virtual void execute (void)
    {
      this->servant_->ping ();
    }
  
  private:
    POA_session::Session_I * const servant_;
  };
}


// TAO_IDL - Generated from 
// ../../../TAO/TAO_IDL/be/be_visitor_operation/operation_ss.cpp:190

void POA_session::Session_I::ping_skel (
    TAO_ServerRequest & server_request,
    void * TAO_INTERCEPTOR (servant_upcall),
    void * servant)
{
#if TAO_HAS_INTERCEPTORS == 1
  static ::CORBA::TypeCode_ptr const * const exceptions = 0;
  static ::CORBA::ULong const nexceptions = 0;
#endif /* TAO_HAS_INTERCEPTORS */
  
  TAO::SArg_Traits< void>::ret_val retval;
  
  TAO::Argument * const args[] =
    {
      &retval
    };
  
  static size_t const nargs = 1;
  
  POA_session::Session_I * const impl =
    static_cast<POA_session::Session_I *> (servant);

  ping_Session_I command (
    impl);
  
  TAO::Upcall_Wrapper upcall_wrapper;
  upcall_wrapper.upcall (server_request
                         , args
                         , nargs
                         , command
#if TAO_HAS_INTERCEPTORS == 1
                         , servant_upcall
                         , exceptions
                         , nexceptions
#endif  /* TAO_HAS_INTERCEPTORS == 1 */
                         );
}

namespace POA_session
{
  
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_operation/upcall_command_ss.cpp:136
  
  class endSession_Session_I
    : public TAO::Upcall_Command
  {
  public:
    inline endSession_Session_I (
      POA_session::Session_I * servant)
      : servant_ (servant)
    {
    }
    
    virtual void execute (void)
    {
      this->servant_->endSession ();
    }
  
  private:
    POA_session::Session_I * const servant_;
  };
}


// TAO_IDL - Generated from 
// ../../../TAO/TAO_IDL/be/be_visitor_operation/operation_ss.cpp:190

void POA_session::Session_I::endSession_skel (
    TAO_ServerRequest & server_request,
    void * TAO_INTERCEPTOR (servant_upcall),
    void * servant)
{
#if TAO_HAS_INTERCEPTORS == 1
  static ::CORBA::TypeCode_ptr const * const exceptions = 0;
  static ::CORBA::ULong const nexceptions = 0;
#endif /* TAO_HAS_INTERCEPTORS */
  
  TAO::SArg_Traits< void>::ret_val retval;
  
  TAO::Argument * const args[] =
    {
      &retval
    };
  
  static size_t const nargs = 1;
  
  POA_session::Session_I * const impl =
    static_cast<POA_session::Session_I *> (servant);

  endSession_Session_I command (
    impl);
  
  TAO::Upcall_Wrapper upcall_wrapper;
  upcall_wrapper.upcall (server_request
                         , args
                         , nargs
                         , command
#if TAO_HAS_INTERCEPTORS == 1
                         , servant_upcall
                         , exceptions
                         , nexceptions
#endif  /* TAO_HAS_INTERCEPTORS == 1 */
                         );
}



// TAO_IDL - Generated from 
// ../../../TAO/TAO_IDL/be/be_visitor_interface/interface_ss.cpp:169

namespace POA_session
{
  
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_operation/upcall_command_ss.cpp:136
  
  class _is_a_Session_I_Upcall_Command
    : public TAO::Upcall_Command
  {
  public:
    inline _is_a_Session_I_Upcall_Command (
      POA_session::Session_I * servant,
      TAO_Operation_Details const * operation_details,
      TAO::Argument * const args[])
      : servant_ (servant)
        , operation_details_ (operation_details)
        , args_ (args)
    {
    }
    
    virtual void execute (void)
    {
      TAO::SArg_Traits< ::ACE_InputCDR::to_boolean>::ret_arg_type retval =
        TAO::Portable_Server::get_ret_arg< ::ACE_InputCDR::to_boolean> (
          this->operation_details_,
          this->args_);
      
      TAO::SArg_Traits< ::CORBA::Char *>::in_arg_type arg_1 =
        TAO::Portable_Server::get_in_arg< ::CORBA::Char *> (
          this->operation_details_,
          this->args_,
          1);
        
      retval =
        this->servant_-> _is_a (
          arg_1);
    }
  
  private:
    POA_session::Session_I * const servant_;
    TAO_Operation_Details const * const operation_details_;
    TAO::Argument * const * const args_;
  };
}


void POA_session::Session_I::_is_a_skel (
    TAO_ServerRequest & server_request, 
    void * TAO_INTERCEPTOR (servant_upcall),
    void * servant)
{
#if TAO_HAS_INTERCEPTORS == 1
  static ::CORBA::TypeCode_ptr const * const exceptions = 0;
  static ::CORBA::ULong const nexceptions = 0;
#endif /* TAO_HAS_INTERCEPTORS */
  
  TAO::SArg_Traits< ::ACE_InputCDR::to_boolean>::ret_val retval;
  TAO::SArg_Traits< ::CORBA::Char *>::in_arg_val _tao_repository_id;
  
  TAO::Argument * const args[] =
    {
      &retval,
      &_tao_repository_id
    };
  
  static size_t const nargs = 2;
  
  POA_session::Session_I * const impl =
    static_cast<POA_session::Session_I *> (servant);
  
  _is_a_Session_I_Upcall_Command command (
    impl,
    server_request.operation_details (),
    args);
  
  TAO::Upcall_Wrapper upcall_wrapper;
  upcall_wrapper.upcall (server_request
                         , args
                         , nargs
                         , command
#if TAO_HAS_INTERCEPTORS == 1
                         , servant_upcall
                         , exceptions
                         , nexceptions
#endif  /* TAO_HAS_INTERCEPTORS == 1 */
                         );
}

namespace POA_session
{
  
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_operation/upcall_command_ss.cpp:136
  
  class _non_existent_Session_I_Upcall_Command
    : public TAO::Upcall_Command
  {
  public:
    inline _non_existent_Session_I_Upcall_Command (
      POA_session::Session_I * servant,
      TAO_Operation_Details const * operation_details,
      TAO::Argument * const args[])
      : servant_ (servant)
        , operation_details_ (operation_details)
        , args_ (args)
    {
    }
    
    virtual void execute (void)
    {
      TAO::SArg_Traits< ::ACE_InputCDR::to_boolean>::ret_arg_type retval =
        TAO::Portable_Server::get_ret_arg< ::ACE_InputCDR::to_boolean> (
          this->operation_details_,
          this->args_);
      
      retval =
        this->servant_-> _non_existent ();
    }
  
  private:
    POA_session::Session_I * const servant_;
    TAO_Operation_Details const * const operation_details_;
    TAO::Argument * const * const args_;
  };
}


void POA_session::Session_I::_non_existent_skel (
    TAO_ServerRequest & server_request, 
    void * TAO_INTERCEPTOR (servant_upcall),
    void * servant)
{
#if TAO_HAS_INTERCEPTORS == 1
  static ::CORBA::TypeCode_ptr const * const exceptions = 0;
  static ::CORBA::ULong const nexceptions = 0;
#endif /* TAO_HAS_INTERCEPTORS */
  
  TAO::SArg_Traits< ::ACE_InputCDR::to_boolean>::ret_val retval;
  
  TAO::Argument * const args[] =
    {
      &retval
    };
  
  static size_t const nargs = 1;
  
  POA_session::Session_I * const impl =
    static_cast<POA_session::Session_I *> (servant);
  
  _non_existent_Session_I_Upcall_Command command (
    impl,
    server_request.operation_details (),
    args);
  
  TAO::Upcall_Wrapper upcall_wrapper;
  upcall_wrapper.upcall (server_request
                         , args
                         , nargs
                         , command
#if TAO_HAS_INTERCEPTORS == 1
                         , servant_upcall
                         , exceptions
                         , nexceptions
#endif  /* TAO_HAS_INTERCEPTORS == 1 */
                         );
}
namespace POA_session
{
  
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_operation/upcall_command_ss.cpp:136
  
  class _repository_id_Session_I_Upcall_Command
    : public TAO::Upcall_Command
  {
  public:
    inline _repository_id_Session_I_Upcall_Command (
      POA_session::Session_I * servant,
      TAO_Operation_Details const * operation_details,
      TAO::Argument * const args[])
      : servant_ (servant)
        , operation_details_ (operation_details)
        , args_ (args)
    {
    }
    
    virtual void execute (void)
    {
      TAO::SArg_Traits< ::CORBA::Char *>::ret_arg_type retval =
        TAO::Portable_Server::get_ret_arg< ::CORBA::Char *> (
          this->operation_details_,
          this->args_);
      
      retval =
        this->servant_-> _repository_id ();
    }
  
  private:
    POA_session::Session_I * const servant_;
    TAO_Operation_Details const * const operation_details_;
    TAO::Argument * const * const args_;
  };
}


void POA_session::Session_I::_repository_id_skel (
    TAO_ServerRequest & server_request, 
    void * TAO_INTERCEPTOR (servant_upcall),
    void * servant)
{
#if TAO_HAS_INTERCEPTORS == 1
  static ::CORBA::TypeCode_ptr const * const exceptions = 0;
  static ::CORBA::ULong const nexceptions = 0;
#endif /* TAO_HAS_INTERCEPTORS */
  
  TAO::SArg_Traits< ::CORBA::Char *>::ret_val retval;
  
  TAO::Argument * const args[] =
    {
      &retval
    };
  
  static size_t const nargs = 1;
  
  POA_session::Session_I * const impl =
    static_cast<POA_session::Session_I *> (servant);
  
  _repository_id_Session_I_Upcall_Command command (
    impl,
    server_request.operation_details (),
    args);
  
  TAO::Upcall_Wrapper upcall_wrapper;
  upcall_wrapper.upcall (server_request
                         , args
                         , nargs
                         , command
#if TAO_HAS_INTERCEPTORS == 1
                         , servant_upcall
                         , exceptions
                         , nexceptions
#endif  /* TAO_HAS_INTERCEPTORS == 1 */
                         );
}

// TAO_IDL - Generated from 
// ../../../TAO/TAO_IDL/be/be_visitor_interface/interface_ss.cpp:508

void POA_session::Session_I::_interface_skel (
    TAO_ServerRequest & server_request, 
    void * /* servant_upcall */,
    void * servant)
{
  TAO_IFR_Client_Adapter *_tao_adapter =
    ACE_Dynamic_Service<TAO_IFR_Client_Adapter>::instance (
        TAO_ORB_Core::ifr_client_adapter_name ()
      );
    
  if (_tao_adapter == 0)
    {
      throw ::CORBA::INTF_REPOS (::CORBA::OMGVMCID | 1, ::CORBA::COMPLETED_NO);
    }
  
  POA_session::Session_I * const impl =
    static_cast<POA_session::Session_I *> (servant);
  ::CORBA::InterfaceDef_ptr _tao_retval = impl->_get_interface ();
  server_request.init_reply ();
  TAO_OutputCDR &_tao_out = *server_request.outgoing ();
  
  ::CORBA::Boolean const _tao_result =
    _tao_adapter->interfacedef_cdr_insert (_tao_out, _tao_retval);
  
  _tao_adapter->dispose (_tao_retval);
  
  if (_tao_result == false)
    {
      throw ::CORBA::MARSHAL ();
    }
}

namespace POA_session
{
  
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_operation/upcall_command_ss.cpp:136
  
  class _get_component_Session_I_Upcall_Command
    : public TAO::Upcall_Command
  {
  public:
    inline _get_component_Session_I_Upcall_Command (
      POA_session::Session_I * servant,
      TAO_Operation_Details const * operation_details,
      TAO::Argument * const args[])
      : servant_ (servant)
        , operation_details_ (operation_details)
        , args_ (args)
    {
    }
    
    virtual void execute (void)
    {
      TAO::SArg_Traits< ::CORBA::Object>::ret_arg_type retval =
        TAO::Portable_Server::get_ret_arg< ::CORBA::Object> (
          this->operation_details_,
          this->args_);
      
      retval =
        this->servant_-> _get_component ();
    }
  
  private:
    POA_session::Session_I * const servant_;
    TAO_Operation_Details const * const operation_details_;
    TAO::Argument * const * const args_;
  };
}


void POA_session::Session_I::_component_skel (
    TAO_ServerRequest & server_request, 
    void * TAO_INTERCEPTOR (servant_upcall),
    void * servant
  )
{
#if TAO_HAS_INTERCEPTORS == 1
  static ::CORBA::TypeCode_ptr const * const exceptions = 0;
  static ::CORBA::ULong const nexceptions = 0;
#endif /* TAO_HAS_INTERCEPTORS */
  
  TAO::SArg_Traits< ::CORBA::Object>::ret_val retval;
  
  TAO::Argument * const args[] =
    {
      &retval
    };
  
  static size_t const nargs = 1;
  
  POA_session::Session_I * const impl =
    static_cast<POA_session::Session_I *> (servant);
  
  _get_component_Session_I_Upcall_Command command (
    impl,
    server_request.operation_details (),
    args);
  
  TAO::Upcall_Wrapper upcall_wrapper;
  upcall_wrapper.upcall (server_request
                         , args
                         , nargs
                         , command
#if TAO_HAS_INTERCEPTORS == 1
                         , servant_upcall
                         , exceptions
                         , nexceptions
#endif  /* TAO_HAS_INTERCEPTORS == 1 */
                         );
}

::CORBA::Boolean POA_session::Session_I::_is_a (const char* value)
{
  return
    (
      !ACE_OS::strcmp (
          value,
          "IDL:mtnm.tmforum.org/session/Session_I:1.0"
        ) ||
      !ACE_OS::strcmp (
          value,
          "IDL:omg.org/CORBA/Object:1.0"
        )
    );
}

const char* POA_session::Session_I::_interface_repository_id (void) const
{
  return "IDL:mtnm.tmforum.org/session/Session_I:1.0";
}

// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_interface/interface_ss.cpp:902

void POA_session::Session_I::_dispatch (TAO_ServerRequest & req, void * servant_upcall)
{
  this->synchronous_upcall_dispatch (req, servant_upcall, this);
}

// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_interface/interface_ss.cpp:852

session::Session_I *
POA_session::Session_I::_this (void)
{
  TAO_Stub *stub = this->_create_stub ();
  
  TAO_Stub_Auto_Ptr safe_stub (stub);
  ::CORBA::Object_ptr tmp = CORBA::Object_ptr ();
  
  ::CORBA::Boolean const _tao_opt_colloc =
    stub->servant_orb_var ()->orb_core ()->optimize_collocation_objects ();
  
  ACE_NEW_RETURN (
      tmp,
      ::CORBA::Object (stub, _tao_opt_colloc, this),
      0
    );
  
  ::CORBA::Object_var obj = tmp;
  (void) safe_stub.release ();
  
  typedef ::session::Session_I STUB_SCOPED_NAME;
  return
    TAO::Narrow_Utils<STUB_SCOPED_NAME>::unchecked_narrow (
        obj.in (),
        session__TAO_Session_I_Proxy_Broker_Factory_function_pointer
      );
}

#endif /* ifndef */

