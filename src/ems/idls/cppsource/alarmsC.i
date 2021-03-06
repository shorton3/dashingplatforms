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
// be/be_visitor_interface/interface_ci.cpp:70

#if !defined (_ALARMS_ALARMINTERFACE___CI_)
#define _ALARMS_ALARMINTERFACE___CI_

ACE_INLINE
alarms::alarmInterface::alarmInterface (
    TAO_Stub *objref,
    CORBA::Boolean _tao_collocated,
    TAO_Abstract_ServantBase *servant,
    TAO_ORB_Core *oc
  )
  : ACE_NESTED_CLASS (CORBA, Object) (
        objref,
        _tao_collocated,
        servant,
        oc
      ),
    the_TAO_alarmInterface_Proxy_Broker_ (0)
{
  this->alarms_alarmInterface_setup_collocation (_tao_collocated);
}

ACE_INLINE
alarms::alarmInterface::alarmInterface (
    IOP::IOR *ior,
    TAO_ORB_Core *oc
  )
  : ACE_NESTED_CLASS (CORBA, Object) (ior, oc),
    the_TAO_alarmInterface_Proxy_Broker_ (0)
{
}

#endif /* end #if !defined */

// TAO_IDL - Generated from
// be/be_visitor_enum/cdr_op_ci.cpp:51

ACE_INLINE
CORBA::Boolean operator<< (TAO_OutputCDR &strm, const alarms::Severity &_tao_enumval)
{
  CORBA::ULong _tao_temp = _tao_enumval;
  return strm << _tao_temp;
}

ACE_INLINE
CORBA::Boolean operator>> (TAO_InputCDR &strm, alarms::Severity &_tao_enumval)
{
  CORBA::ULong _tao_temp = 0;
  CORBA::Boolean _tao_result = strm >> _tao_temp;
  
  if (_tao_result == 1)
    {
      _tao_enumval = ACE_static_cast (alarms::Severity, _tao_temp);
    }
  
  return _tao_result;
}

// TAO_IDL - Generated from
// be/be_visitor_structure/cdr_op_ci.cpp:70

ACE_INLINE
CORBA::Boolean operator<< (
    TAO_OutputCDR &strm,
    const alarms::AlarmNotification &_tao_aggregate
  )
{
  return
    (strm << _tao_aggregate.neid.in ()) &&
    (strm << _tao_aggregate.code) &&
    (strm << _tao_aggregate.mObject) &&
    (strm << _tao_aggregate.mObjectInstance) &&
    (strm << _tao_aggregate.sev) &&
    (strm << _tao_aggregate.tStamp.in ()) &&
    (strm << _tao_aggregate.ack.in ());
}

ACE_INLINE
CORBA::Boolean operator>> (
    TAO_InputCDR &strm,
    alarms::AlarmNotification &_tao_aggregate
  )
{
  return
    (strm >> _tao_aggregate.neid.out ()) &&
    (strm >> _tao_aggregate.code) &&
    (strm >> _tao_aggregate.mObject) &&
    (strm >> _tao_aggregate.mObjectInstance) &&
    (strm >> _tao_aggregate.sev) &&
    (strm >> _tao_aggregate.tStamp.out ()) &&
    (strm >> _tao_aggregate.ack.out ());
}

// TAO_IDL - Generated from
// be/be_visitor_sequence/cdr_op_ci.cpp:81

#if !defined _TAO_CDR_OP_alarms_AlarmNotificationSequence_I_
#define _TAO_CDR_OP_alarms_AlarmNotificationSequence_I_

CORBA::Boolean  operator<< (
    TAO_OutputCDR &,
    const alarms::AlarmNotificationSequence &
  );

CORBA::Boolean  operator>> (
    TAO_InputCDR &,
    alarms::AlarmNotificationSequence &
  );

#endif /* _TAO_CDR_OP_alarms_AlarmNotificationSequence_I_ */

// TAO_IDL - Generated from
// be/be_visitor_enum/cdr_op_ci.cpp:51

ACE_INLINE
CORBA::Boolean operator<< (TAO_OutputCDR &strm, const alarms::AlarmCategory &_tao_enumval)
{
  CORBA::ULong _tao_temp = _tao_enumval;
  return strm << _tao_temp;
}

ACE_INLINE
CORBA::Boolean operator>> (TAO_InputCDR &strm, alarms::AlarmCategory &_tao_enumval)
{
  CORBA::ULong _tao_temp = 0;
  CORBA::Boolean _tao_result = strm >> _tao_temp;
  
  if (_tao_result == 1)
    {
      _tao_enumval = ACE_static_cast (alarms::AlarmCategory, _tao_temp);
    }
  
  return _tao_result;
}

// TAO_IDL - Generated from
// be/be_visitor_structure/cdr_op_ci.cpp:70

ACE_INLINE
CORBA::Boolean operator<< (
    TAO_OutputCDR &strm,
    const alarms::AlarmInformation &_tao_aggregate
  )
{
  return
    (strm << _tao_aggregate.code) &&
    (strm << _tao_aggregate.aCategory) &&
    (strm << _tao_aggregate.hWaterMark) &&
    (strm << _tao_aggregate.lWaterMark) &&
    (strm << _tao_aggregate.sev) &&
    (strm << CORBA::Any::from_boolean (_tao_aggregate.autoClearing)) &&
    (strm << CORBA::Any::from_boolean (_tao_aggregate.filterEnabled)) &&
    (strm << _tao_aggregate.desc.in ()) &&
    (strm << _tao_aggregate.res.in ());
}

ACE_INLINE
CORBA::Boolean operator>> (
    TAO_InputCDR &strm,
    alarms::AlarmInformation &_tao_aggregate
  )
{
  return
    (strm >> _tao_aggregate.code) &&
    (strm >> _tao_aggregate.aCategory) &&
    (strm >> _tao_aggregate.hWaterMark) &&
    (strm >> _tao_aggregate.lWaterMark) &&
    (strm >> _tao_aggregate.sev) &&
    (strm >> CORBA::Any::to_boolean (_tao_aggregate.autoClearing)) &&
    (strm >> CORBA::Any::to_boolean (_tao_aggregate.filterEnabled)) &&
    (strm >> _tao_aggregate.desc.out ()) &&
    (strm >> _tao_aggregate.res.out ());
}

// TAO_IDL - Generated from
// be/be_visitor_sequence/cdr_op_ci.cpp:81

#if !defined _TAO_CDR_OP_alarms_AlarmInformationSequence_I_
#define _TAO_CDR_OP_alarms_AlarmInformationSequence_I_

CORBA::Boolean  operator<< (
    TAO_OutputCDR &,
    const alarms::AlarmInformationSequence &
  );

CORBA::Boolean  operator>> (
    TAO_InputCDR &,
    alarms::AlarmInformationSequence &
  );

#endif /* _TAO_CDR_OP_alarms_AlarmInformationSequence_I_ */

// TAO_IDL - Generated from
// be/be_visitor_interface/cdr_op_ci.cpp:72

 CORBA::Boolean operator<< (
    TAO_OutputCDR &,
    const alarms::alarmInterface_ptr
  );

 CORBA::Boolean operator>> (
    TAO_InputCDR &,
    alarms::alarmInterface_ptr &
  );

