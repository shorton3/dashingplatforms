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
// ../../../TAO/TAO_IDL/be/be_codegen.cpp:135

#ifndef _TAO_IDL_CPPSOURCE_PLATFORMCONFIGC_H_
#define _TAO_IDL_CPPSOURCE_PLATFORMCONFIGC_H_


#include /**/ "ace/config-all.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */


#include "tao/AnyTypeCode/AnyTypeCode_methods.h"
#include "tao/ORB.h"
#include "tao/SystemException.h"
#include "tao/Basic_Types.h"
#include "tao/ORB_Constants.h"
#include "tao/Object.h"
#include "tao/String_Manager_T.h"
#include "tao/Sequence_T.h"
#include "tao/Objref_VarOut_T.h"
#include "tao/Seq_Var_T.h"
#include "tao/Seq_Out_T.h"
#include "tao/VarOut_T.h"
#include /**/ "tao/Versioned_Namespace.h"

#if defined (TAO_EXPORT_MACRO)
#undef TAO_EXPORT_MACRO
#endif
#define TAO_EXPORT_MACRO 

// TAO_IDL - Generated from 
// ../../../TAO/TAO_IDL/be/be_visitor_root/root_ch.cpp:62
TAO_BEGIN_VERSIONED_NAMESPACE_DECL



namespace TAO
{
  class Collocation_Proxy_Broker;
  template<typename T> class Narrow_Utils;
}
TAO_END_VERSIONED_NAMESPACE_DECL



// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_module/module_ch.cpp:49

namespace platformConfig
{
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_type.cpp:269
  
  struct platformConfigLogLevel;
  
  typedef
    TAO_Var_Var_T<
        platformConfigLogLevel
      >
    platformConfigLogLevel_var;
  
  typedef
    TAO_Out_T<
        platformConfigLogLevel
      >
    platformConfigLogLevel_out;
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_structure/structure_ch.cpp:57
  
  struct  platformConfigLogLevel
  {
    typedef platformConfigLogLevel_var _var_type;
    typedef platformConfigLogLevel_out _out_type;
    
    static void _tao_any_destructor (void *);
    ::CORBA::Short subsystemEnum;
    TAO::String_Manager subsystemName;
    ::CORBA::Short severityLevel;
  };
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_typecode/typecode_decl.cpp:49
  
  extern  ::CORBA::TypeCode_ptr const _tc_platformConfigLogLevel;
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_sequence/sequence_ch.cpp:107

#if !defined (_PLATFORMCONFIG_PLATFORMCONFIGLOGLEVELS_CH_)
#define _PLATFORMCONFIG_PLATFORMCONFIGLOGLEVELS_CH_
  
  class platformConfigLogLevels;
  
  typedef
    TAO_VarSeq_Var_T<
        platformConfigLogLevels
      >
    platformConfigLogLevels_var;
  
  typedef
    TAO_Seq_Out_T<
        platformConfigLogLevels
      >
    platformConfigLogLevels_out;
  
  class  platformConfigLogLevels
    : public
        TAO::unbounded_value_sequence<
            platformConfigLogLevel
          >
  {
  public:
    platformConfigLogLevels (void);
    platformConfigLogLevels ( ::CORBA::ULong max);
    platformConfigLogLevels (
        ::CORBA::ULong max,
        ::CORBA::ULong length,
        platformConfigLogLevel* buffer, 
        ::CORBA::Boolean release = false
      );
    platformConfigLogLevels (const platformConfigLogLevels &);
    virtual ~platformConfigLogLevels (void);
    
    static void _tao_any_destructor (void *);
    
    typedef platformConfigLogLevels_var _var_type;
    typedef platformConfigLogLevels_out _out_type;
    
    
  };

#endif /* end #if !defined */
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_typecode/typecode_decl.cpp:49
  
  extern  ::CORBA::TypeCode_ptr const _tc_platformConfigLogLevels;
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_interface.cpp:638

#if !defined (_PLATFORMCONFIG_PLATFORMCONFIG_I__VAR_OUT_CH_)
#define _PLATFORMCONFIG_PLATFORMCONFIG_I__VAR_OUT_CH_
  
  class platformConfig_I;
  typedef platformConfig_I *platformConfig_I_ptr;
  
  typedef
    TAO_Objref_Var_T<
        platformConfig_I
      >
    platformConfig_I_var;
  
  typedef
    TAO_Objref_Out_T<
        platformConfig_I
      >
    platformConfig_I_out;

#endif /* end #if !defined */
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_interface/interface_ch.cpp:54

#if !defined (_PLATFORMCONFIG_PLATFORMCONFIG_I_CH_)
#define _PLATFORMCONFIG_PLATFORMCONFIG_I_CH_
  
  class  platformConfig_I
    : public virtual ::CORBA::Object
  {
  public:
    friend class TAO::Narrow_Utils<platformConfig_I>;
    typedef platformConfig_I_ptr _ptr_type;
    typedef platformConfig_I_var _var_type;
    typedef platformConfig_I_out _out_type;
    
    // The static operations.
    static platformConfig_I_ptr _duplicate (platformConfig_I_ptr obj);
    
    static void _tao_release (platformConfig_I_ptr obj);
    
    static platformConfig_I_ptr _narrow (::CORBA::Object_ptr obj);
    static platformConfig_I_ptr _unchecked_narrow (::CORBA::Object_ptr obj);
    static platformConfig_I_ptr _nil (void)
    {
      return static_cast<platformConfig_I_ptr> (0);
    }
    
    static void _tao_any_destructor (void *);
    
    // TAO_IDL - Generated from
    // ../../../TAO/TAO_IDL/be/be_visitor_operation/operation_ch.cpp:46
    
    virtual void getLogLevels (
        ::platformConfig::platformConfigLogLevels_out logLevels);
    
    // TAO_IDL - Generated from
    // ../../../TAO/TAO_IDL/be/be_visitor_operation/operation_ch.cpp:46
    
    virtual void setLogLevel (
        const ::platformConfig::platformConfigLogLevel & logLevel);
    
    // TAO_IDL - Generated from
    // ../../../TAO/TAO_IDL/be/be_visitor_operation/operation_ch.cpp:46
    
    virtual void setLogLevels (
        const ::platformConfig::platformConfigLogLevels & logLevels);
    
    // TAO_IDL - Generated from
    // ../../../TAO/TAO_IDL/be/be_visitor_interface/interface_ch.cpp:216
    
    virtual ::CORBA::Boolean _is_a (const char *type_id);
    virtual const char* _interface_repository_id (void) const;
    virtual ::CORBA::Boolean marshal (TAO_OutputCDR &cdr);
  private:
    TAO::Collocation_Proxy_Broker *the_TAO_platformConfig_I_Proxy_Broker_;
  
  protected:
    // Concrete interface only.
    platformConfig_I (void);
    
    // These methods travese the inheritance tree and set the
    // parents piece of the given class in the right mode.
    virtual void platformConfig_platformConfig_I_setup_collocation (void);
    
    // Concrete non-local interface only.
    platformConfig_I (
        IOP::IOR *ior,
        TAO_ORB_Core *orb_core = 0);
    
    // Non-local interface only.
    platformConfig_I (
        TAO_Stub *objref,
        ::CORBA::Boolean _tao_collocated = false,
        TAO_Abstract_ServantBase *servant = 0,
        TAO_ORB_Core *orb_core = 0);
    
    virtual ~platformConfig_I (void);
  
  private:
    // Private and unimplemented for concrete interfaces.
    platformConfig_I (const platformConfig_I &);
    
    void operator= (const platformConfig_I &);
  };

#endif /* end #if !defined */
  
  // TAO_IDL - Generated from
  // ../../../TAO/TAO_IDL/be/be_visitor_typecode/typecode_decl.cpp:49
  
  extern  ::CORBA::TypeCode_ptr const _tc_platformConfig_I;

// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_module/module_ch.cpp:78

} // module platformConfig

// Proxy Broker Factory function pointer declarations.

// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_root/root.cpp:139

extern 
TAO::Collocation_Proxy_Broker *
(*platformConfig__TAO_platformConfig_I_Proxy_Broker_Factory_function_pointer) (
    ::CORBA::Object_ptr obj
  );

// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_traits.cpp:64

TAO_BEGIN_VERSIONED_NAMESPACE_DECL

// Traits specializations.
namespace TAO
{

#if !defined (_PLATFORMCONFIG_PLATFORMCONFIG_I__TRAITS_)
#define _PLATFORMCONFIG_PLATFORMCONFIG_I__TRAITS_
  
  template<>
  struct  Objref_Traits< ::platformConfig::platformConfig_I>
  {
    static ::platformConfig::platformConfig_I_ptr duplicate (
        ::platformConfig::platformConfig_I_ptr
      );
    static void release (
        ::platformConfig::platformConfig_I_ptr
      );
    static ::platformConfig::platformConfig_I_ptr nil (void);
    static ::CORBA::Boolean marshal (
        const ::platformConfig::platformConfig_I_ptr p,
        TAO_OutputCDR & cdr
      );
  };

#endif /* end #if !defined */
}
TAO_END_VERSIONED_NAMESPACE_DECL



// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_structure/any_op_ch.cpp:53


TAO_BEGIN_VERSIONED_NAMESPACE_DECL

 void operator<<= (::CORBA::Any &, const platformConfig::platformConfigLogLevel &); // copying version
 void operator<<= (::CORBA::Any &, platformConfig::platformConfigLogLevel*); // noncopying version
 ::CORBA::Boolean operator>>= (const ::CORBA::Any &, platformConfig::platformConfigLogLevel *&); // deprecated
 ::CORBA::Boolean operator>>= (const ::CORBA::Any &, const platformConfig::platformConfigLogLevel *&);
TAO_END_VERSIONED_NAMESPACE_DECL



// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_sequence/any_op_ch.cpp:53


TAO_BEGIN_VERSIONED_NAMESPACE_DECL

 void operator<<= ( ::CORBA::Any &, const platformConfig::platformConfigLogLevels &); // copying version
 void operator<<= ( ::CORBA::Any &, platformConfig::platformConfigLogLevels*); // noncopying version
 ::CORBA::Boolean operator>>= (const ::CORBA::Any &, platformConfig::platformConfigLogLevels *&); // deprecated
 ::CORBA::Boolean operator>>= (const ::CORBA::Any &, const platformConfig::platformConfigLogLevels *&);
TAO_END_VERSIONED_NAMESPACE_DECL



// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_interface/any_op_ch.cpp:54



#if defined (ACE_ANY_OPS_USE_NAMESPACE)

namespace platformConfig
{
   void operator<<= ( ::CORBA::Any &, platformConfig_I_ptr); // copying
   void operator<<= ( ::CORBA::Any &, platformConfig_I_ptr *); // non-copying
   ::CORBA::Boolean operator>>= (const ::CORBA::Any &, platformConfig_I_ptr &);
}

#else


TAO_BEGIN_VERSIONED_NAMESPACE_DECL

 void operator<<= (::CORBA::Any &, platformConfig::platformConfig_I_ptr); // copying
 void operator<<= (::CORBA::Any &, platformConfig::platformConfig_I_ptr *); // non-copying
 ::CORBA::Boolean operator>>= (const ::CORBA::Any &, platformConfig::platformConfig_I_ptr &);
TAO_END_VERSIONED_NAMESPACE_DECL



#endif

// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_structure/cdr_op_ch.cpp:54


TAO_BEGIN_VERSIONED_NAMESPACE_DECL

 ::CORBA::Boolean operator<< (TAO_OutputCDR &, const platformConfig::platformConfigLogLevel &);
 ::CORBA::Boolean operator>> (TAO_InputCDR &, platformConfig::platformConfigLogLevel &);

TAO_END_VERSIONED_NAMESPACE_DECL



// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_sequence/cdr_op_ch.cpp:71

#if !defined _TAO_CDR_OP_platformConfig_platformConfigLogLevels_H_
#define _TAO_CDR_OP_platformConfig_platformConfigLogLevels_H_
TAO_BEGIN_VERSIONED_NAMESPACE_DECL



 ::CORBA::Boolean operator<< (
    TAO_OutputCDR &strm,
    const platformConfig::platformConfigLogLevels &_tao_sequence
  );
 ::CORBA::Boolean operator>> (
    TAO_InputCDR &strm,
    platformConfig::platformConfigLogLevels &_tao_sequence
  );
TAO_END_VERSIONED_NAMESPACE_DECL



#endif /* _TAO_CDR_OP_platformConfig_platformConfigLogLevels_H_ */

// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_visitor_interface/cdr_op_ch.cpp:55

TAO_BEGIN_VERSIONED_NAMESPACE_DECL

 ::CORBA::Boolean operator<< (TAO_OutputCDR &, const platformConfig::platformConfig_I_ptr );
 ::CORBA::Boolean operator>> (TAO_InputCDR &, platformConfig::platformConfig_I_ptr &);

TAO_END_VERSIONED_NAMESPACE_DECL



// TAO_IDL - Generated from
// ../../../TAO/TAO_IDL/be/be_codegen.cpp:1228
#if defined (__ACE_INLINE__)
#include "platformConfigC.inl"
#endif /* defined INLINE */

#endif /* ifndef */

