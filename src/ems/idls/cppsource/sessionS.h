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
// ../../../TAO/TAO_IDL/be/be_codegen.cpp:487

#ifndef _TAO_IDL_CPPSOURCE_SESSIONS_H_
#define _TAO_IDL_CPPSOURCE_SESSIONS_H_


#include "sessionC.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "tao/Collocation_Proxy_Broker.h"
#include "tao/PortableServer/PortableServer.h"
#include "tao/PortableServer/Servant_Base.h"

// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_module/module_sh.cpp:49

namespace POA_session
{
  
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_interface/interface_sh.cpp:87
  
  class Session_I;
  typedef Session_I *Session_I_ptr;
  
  class  Session_I
    : public virtual PortableServer::ServantBase
  {
  protected:
    Session_I (void);
  
  public:
    // Useful for template programming.
    typedef ::session::Session_I _stub_type;
    typedef ::session::Session_I_ptr _stub_ptr_type;
    typedef ::session::Session_I_var _stub_var_type;
    
    Session_I (const Session_I& rhs);
    virtual ~Session_I (void);
    
    virtual ::CORBA::Boolean _is_a (const char* logical_type_id);
    
    static void _is_a_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
      );
    
    static void _non_existent_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
      );
    
    static void _interface_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
      );
    
    static void _component_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
      );
    
    static void _repository_id_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant);
    
    virtual void _dispatch (
        TAO_ServerRequest & req,
        void * servant_upcall);
    
    ::session::Session_I *_this (void);
    
    virtual const char* _interface_repository_id (void) const;
    
    // TAO_IDL - Generated from
    // ../../../TAO/TAO_IDL/be/be_visitor_operation/operation_sh.cpp:45
    
    virtual ::session::Session_I_ptr associatedSession (
        void) = 0;
    
    static void _get_associatedSession_skel (
        TAO_ServerRequest & server_request,
        void * servant_upcall,
        void * servant
      );
    
    // TAO_IDL - Generated from
    // ../../../TAO/TAO_IDL/be/be_visitor_operation/operation_sh.cpp:45
    
    virtual void ping (
        void) = 0;
    
    static void ping_skel (
        TAO_ServerRequest & server_request,
        void * servant_upcall,
        void * servant
      );
    
    // TAO_IDL - Generated from
    // ../../../TAO/TAO_IDL/be/be_visitor_operation/operation_sh.cpp:45
    
    virtual void endSession (
        void) = 0;
    
    static void endSession_skel (
        TAO_ServerRequest & server_request,
        void * servant_upcall,
        void * servant
      );
  };

// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_module/module_sh.cpp:80

} // module session

// TAO_IDL - Generated from 
// ../../../TAO/TAO_IDL/be/be_codegen.cpp:1290


#if defined (__ACE_INLINE__)
#include "sessionS.inl"
#endif /* defined INLINE */

#endif /* ifndef */

