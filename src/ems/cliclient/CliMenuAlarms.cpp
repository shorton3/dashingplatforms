/******************************************************************************
*
* File name:   CliMenuAlarms.cpp
* Subsystem:   EMS
* Description: Static handler methods for alarm requests.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/


//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <sstream>

#include <tao/ORB_Core.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------

#include "CliMenuAlarms.h"

#include "platform/logger/Logger.h"

#include "platform/common/AlarmsEvents.h"

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// PUBLIC methods.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Method Type: Constructor
// Description:
// Design:
//-----------------------------------------------------------------------------
CliMenuAlarms::CliMenuAlarms() : errorString_("DefaultError")
{
}//end Constructor


//-----------------------------------------------------------------------------
// Method Type: Virtual Destructor
// Description: 
// Design:     
//-----------------------------------------------------------------------------
CliMenuAlarms::~CliMenuAlarms()
{
}//end virtual destructor


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Initialize static variables to be used by the methods
// Design:     
//-----------------------------------------------------------------------------
void CliMenuAlarms::initialize(alarms::alarmInterface_ptr alarmInterface_ptr)
{
   alarmInterface_ptr_ = alarmInterface_ptr;
}//end initialize


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Retrieve all of the Outstanding Alarms from the agent
// Design:
//-----------------------------------------------------------------------------
string CliMenuAlarms::getOutstandingAlarms()
{
   char* exceptionMessage = NULL;

   if (CORBA::is_nil (alarmInterface_ptr_) || alarmInterface_ptr_->_non_existent())
   {
      TRACELOG(DEBUGLOG, CLICLIENTLOG, "alarmInterfacePtr is NULL",0,0,0,0,0,0);
      return errorString_;
   }//end if

   ostringstream ostr;
   try
   {
      exceptionMessage = "While retrieving outstanding alarms";

      alarms::AlarmNotificationSequence_var alarmNotification;

      alarmInterface_ptr_->getOutstandingAlarms(alarmNotification.out());

      TRACELOG(DEBUGLOG, CLICLIENTLOG, "Retrieved (%d) Outstanding Alarms from the Agent",
         alarmNotification.in().length(),0,0,0,0,0);

      ostr <<    "  NEID           AlarmCode      ManagedObject   MOInstance   TimeStamp  Severity  Acknowledgement" << endl;
      ostr <<    "------------   -------------   ---------------  ----------   ---------  --------  ---------------" << endl;
      for (unsigned int i = 0; i < alarmNotification.in().length(); i++)
      {
         string neid = alarmNotification[CORBA::ULong(i)].neid.in();
         // Justify the fields with padding
         neid.resize(9, ' ');

         int alarmCode = alarmNotification[CORBA::ULong(i)].code;
         int mObject = alarmNotification[CORBA::ULong(i)].mObject;
         int mObjectInst = alarmNotification[CORBA::ULong(i)].mObjectInstance;
         int severity = alarmNotification[CORBA::ULong(i)].sev;

         string timeStamp = alarmNotification[CORBA::ULong(i)].tStamp.in();
         string ack = alarmNotification[CORBA::ULong(i)].ack.in();

         ostr << "   " << neid << "   " << alarmCode << "   " << mObject << "   " << mObjectInst 
              << "   " << timeStamp << "   ";
         switch (severity)
         {
            case ALARM_CRITICAL:
               ostr << "CRITICAL";
               break;
            case ALARM_MAJOR: 
               ostr << "MAJOR";
               break;
            case ALARM_MINOR:
               ostr << "MINOR";
               break;
            case ALARM_WARNING:
               ostr << "WARNING";
               break;
            default:
               ostr << "UNKNOWN SEVERITY(" << severity << ")";
               break;
         }//end switch

         ostr << endl << ack << endl;
         ostr << endl;
      }//end for
   }//end try
   catch (CORBA::Exception &exception)
   {
      // See $TAO_ROOT/tao/Exception.h for additional fields and information that can be retrieved
      TRACELOG(ERRORLOG, CLICLIENTLOG, "Application Level Exception - %s. Corba Orb Level Exception - %s",
         exceptionMessage, exception._info().c_str(),0,0,0,0);
   }//end catch
   return ostr.str();
}//end getOutstandingAlarms


//-----------------------------------------------------------------------------
// Method Type: INSTANCE
// Description: Retrieve the Inventory of all of the Alarms in the system from the agent
// Design:
//-----------------------------------------------------------------------------
string CliMenuAlarms::getAlarmInventory()
{
   char* exceptionMessage = NULL;

   if (CORBA::is_nil (alarmInterface_ptr_) || alarmInterface_ptr_->_non_existent())
   {
      TRACELOG(DEBUGLOG, CLICLIENTLOG, "alarmInterfacePtr is NULL",0,0,0,0,0,0);
      return errorString_;
   }//end if

   ostringstream ostr;
   try
   {
      exceptionMessage = "While retrieving alarm inventory";

      alarms::AlarmInformationSequence_var alarmInformation;

      alarmInterface_ptr_->getAlarmInventory(alarmInformation.out());

      TRACELOG(DEBUGLOG, CLICLIENTLOG, "Retrieved Inventory of (%d) Alarms from the Agent",
         alarmInformation.in().length(),0,0,0,0,0);

      ostr <<    " AlarmCode      Category       Severity       Description   Resolution" << endl;
      ostr <<    "------------   -------------   -------------  -----------   ----------" << endl;
      for (unsigned int i = 0; i < alarmInformation.in().length(); i++)
      {
         int alarmCode = alarmInformation[CORBA::ULong(i)].code;
         int category = alarmInformation[CORBA::ULong(i)].aCategory;
         int severity = alarmInformation[CORBA::ULong(i)].sev;
         string description = alarmInformation[CORBA::ULong(i)].desc.in();
         string resolution = alarmInformation[CORBA::ULong(i)].res.in();

         ostr << "   " << alarmCode << "   ";

         switch (category)
         {
            case COMMUNICATIONS_ALARM:
               ostr << "Communications";
               break;
            case QUALITY_OF_SERVICE_ALARM:
               ostr << "Quality of Service";
               break;
            case PROCESSING_ERROR_ALARM:
               ostr << "Processing Error";
               break;
            case EQUIPMENT_ALARM:
               ostr << "Equipment";
               break;
            case ENVIRONMENTAL_ALARM:
               ostr << "Environmental";
               break;
            default:
               ostr << "UnknownCategory(" << category << ")";
               break;
         }//end switch
         ostr << "   ";
         switch (severity)
         {
            case ALARM_CRITICAL:
               ostr << "CRITICAL";
               break;
            case ALARM_MAJOR:
               ostr << "MAJOR";
               break;
            case ALARM_MINOR:
               ostr << "MINOR";
               break;
            case ALARM_WARNING:
               ostr << "WARNING";
               break;
            default:
               ostr << "UNKNOWN SEVERITY(" << severity << ")";
               break;
         }//end switch

         ostr << endl << "Description: " << description;
         ostr << endl << "Resolution: " << resolution << endl;
         ostr << endl;
      }//end for
   }//end try
   catch (CORBA::Exception &exception)
   {
      // See $TAO_ROOT/tao/Exception.h for additional fields and information that can be retrieved
      TRACELOG(ERRORLOG, CLICLIENTLOG, "Application Level Exception - %s. Corba Orb Level Exception - %s",
         exceptionMessage, exception._info().c_str(),0,0,0,0);
   }//end catch
   return ostr.str();
}//end getAlarmInventory


//-----------------------------------------------------------------------------
// PROTECTED methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Nested Class Definitions:
//-----------------------------------------------------------------------------

