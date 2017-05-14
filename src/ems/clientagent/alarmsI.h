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
// be/be_codegen.cpp:887

#ifndef CPPSOURCE_ALARMSI_H_
#define CPPSOURCE_ALARMSI_H_

#include "ems/idls/cppsource/alarmsS.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
#pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

class  alarms_alarmInterface_i : public virtual POA_alarms::alarmInterface, public virtual PortableServer::RefCountServantBase
{
public:
  //Constructor 
  alarms_alarmInterface_i (void);
  
  //Destructor 
  virtual ~alarms_alarmInterface_i (void);
  
  virtual
  void getOutstandingAlarms (
      alarms::AlarmNotificationSequence_out outstandingAlarms
    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ));
  
  virtual
  void getAlarmInventory (
      alarms::AlarmInformationSequence_out alarmInventory
    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ));
};


#endif /* CPPSOURCE_ALARMSI_H_  */
