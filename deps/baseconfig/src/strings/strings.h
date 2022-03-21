
#ifndef BASECONFIG_STRINGS_H
#define BASECONFIG_STRINGS_H

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef HAVE_STRCASECMP
#ifdef HAVE__STRICMP
#define strcasecmp(s1, s2) _stricmp(s1, s2)
#else
#error "strcasecmp equivalent not known for this platform."
#endif
#endif

#ifndef HAVE_STRNCASECMP
#ifdef HAVE__STRNICMP
#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)
#else
#error "strncasecmp equivalent not known for this platform."
#endif
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BASECONFIG_STRINGS_H */
