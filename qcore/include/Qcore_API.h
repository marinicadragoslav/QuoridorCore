#ifndef QCORE_API_H
#define QCORE_API_H

#ifdef WIN32

#ifdef QCORE_API_EXPORT
#define QCODE_API __declspec( dllexport )
#else
#define QCODE_API __declspec( dllimport )
#endif

#else
#define QCODE_API
#endif

#endif

