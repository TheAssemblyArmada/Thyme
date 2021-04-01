/*
 * Copyright (c) 1999 
 * Boris Fomitchev
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted 
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */

# ifndef _STLP_OUTERMOST_HEADER_ID
#  define _STLP_OUTERMOST_HEADER_ID 0x262
#  include <stl/_prolog.h>
# endif

# ifndef _STLP_WINCE

# include _STLP_NATIVE_C_HEADER_UCRT(stddef.h)

# endif /* WINCE */

# if (_STLP_OUTERMOST_HEADER_ID == 0x262)
#  if ! defined (_STLP_DONT_POP_0x262)
#   include <stl/_epilog.h>
#   undef  _STLP_OUTERMOST_HEADER_ID
#   endif
#   undef  _STLP_DONT_POP_0x262
# endif

// Local Variables:
// mode:C++
// End:
