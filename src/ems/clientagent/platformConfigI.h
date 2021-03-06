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

#ifndef CPPSOURCE_PLATFORMCONFIGI_H_
#define CPPSOURCE_PLATFORMCONFIGI_H_

#include "ems/idls/cppsource/platformConfigS.h"

class  platformConfig_platformConfig_I_i : public virtual POA_platformConfig::platformConfig_I, public virtual PortableServer::RefCountServantBase
{
public:
  //Constructor 
  platformConfig_platformConfig_I_i (void);
  
  //Destructor 
  virtual ~platformConfig_platformConfig_I_i (void);
  
  virtual
  void getLogLevels (
      platformConfig::platformConfigLogLevels_out logLevels
    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ));
  
  virtual
  void setLogLevel (
      const platformConfig::platformConfigLogLevel & logLevel
    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ));
  
  virtual
  void setLogLevels (
      const platformConfig::platformConfigLogLevels & logLevels
    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ));
};


#endif /* CPPSOURCE_PLATFORMCONFIGI_H_  */
