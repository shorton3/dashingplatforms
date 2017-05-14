
// -*- C++ -*-
// $Id$
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl ProcessController
// ------------------------------
#ifndef PROCESSCONTROLLER_EXPORT_H
#define PROCESSCONTROLLER_EXPORT_H

#include "ace/config-all.h"

#if !defined (PROCESSCONTROLLER_HAS_DLL)
#  define PROCESSCONTROLLER_HAS_DLL 1
#endif /* ! PROCESSCONTROLLER_HAS_DLL */

#if defined (PROCESSCONTROLLER_HAS_DLL) && (PROCESSCONTROLLER_HAS_DLL == 1)
#  if defined (PROCESSCONTROLLER_BUILD_DLL)
#    define ProcessController_Export ACE_Proper_Export_Flag
#    define PROCESSCONTROLLER_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define PROCESSCONTROLLER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* PROCESSCONTROLLER_BUILD_DLL */
#    define ProcessController_Export ACE_Proper_Import_Flag
#    define PROCESSCONTROLLER_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define PROCESSCONTROLLER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* PROCESSCONTROLLER_BUILD_DLL */
#else /* PROCESSCONTROLLER_HAS_DLL == 1 */
#  define ProcessController_Export
#  define PROCESSCONTROLLER_SINGLETON_DECLARATION(T)
#  define PROCESSCONTROLLER_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* PROCESSCONTROLLER_HAS_DLL == 1 */

// Set PROCESSCONTROLLER_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (PROCESSCONTROLLER_NTRACE)
#  if (ACE_NTRACE == 1)
#    define PROCESSCONTROLLER_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define PROCESSCONTROLLER_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !PROCESSCONTROLLER_NTRACE */

#if (PROCESSCONTROLLER_NTRACE == 1)
#  define PROCESSCONTROLLER_TRACE(X)
#else /* (PROCESSCONTROLLER_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define PROCESSCONTROLLER_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (PROCESSCONTROLLER_NTRACE == 1) */

#endif /* PROCESSCONTROLLER_EXPORT_H */

// End of auto generated file.
