// STLport configuration file
// It is internal STLport header - DO NOT include it directly
// Microsoft Visual C++ 4.0, 4.1, 4.2, 5.0, ICL


// Common features for VC++ 4.0 and higher
# ifdef _M_IA64
#  define _STLP_NATIVE_HEADER(x) <../crt/##x>
#  define _STLP_NATIVE_C_HEADER(x) <../crt/##x>
#  define _STLP_NATIVE_CPP_C_HEADER(x) <../crt/##x>
#  define _STLP_NATIVE_OLD_STREAMS_HEADER(x) <../crt/##x>
#  define _STLP_NATIVE_CPP_RUNTIME_HEADER(header) <../crt/##header>
#  define _STLP_GLOBAL_NEW_HANDLER
# else
#  define _STLP_NATIVE_HEADER(x) <../include/##x>
#  define _STLP_NATIVE_C_HEADER(x) <../include/##x>
#  define _STLP_NATIVE_CPP_C_HEADER(x) <../include/##x>
#  define _STLP_NATIVE_OLD_STREAMS_HEADER(x) <../include/##x>
#  define _STLP_NATIVE_CPP_RUNTIME_HEADER(header) <../include/##header>
# endif

# define _STLP_CALL __cdecl

# ifndef _STLP_LONG_LONG
#  define _STLP_LONG_LONG  __int64
# endif

// these switches depend on compiler flags
# ifndef _CPPUNWIND
#  define _STLP_HAS_NO_EXCEPTIONS 1
# endif

# define _STLP_VENDOR_UNEXPECTED_STD

# if defined ( _MT ) && !defined (_STLP_NO_THREADS)  && !defined (_REENTRANT)
#   define _REENTRANT 1
# endif

# define _STLP_WCHAR_T_IS_USHORT      1
# define _STLP_MINIMUM_IMPORT_STD

# ifdef _STLP_MSVC

# ifndef _STLP_MSVC50_COMPATIBILITY
#  define _STLP_MSVC50_COMPATIBILITY   1
# endif

// # define _STLP_STATIC_CONST_INIT_BUG   1
# define _STLP_DEFAULT_CONSTRUCTOR_BUG 1

#  define _STLP_DLLEXPORT_NEEDS_PREDECLARATION 1

// # ifndef __BUILDING_STLPORT
// #  define _STLP_USE_TEMPLATE_EXPORT 1
// # endif

#  define _STLP_HAS_SPECIFIC_PROLOG_EPILOG
#  define _STLP_NO_TYPENAME_IN_TEMPLATE_HEADER
// fails to properly resolve call to sin() from within sin()
#  define _STLP_SAME_FUNCTION_NAME_RESOLUTION_BUG

#  if (_STLP_MSVC > 1100)
     typedef char __stl_char;
#   define _STLP_DEFAULTCHAR __stl_char
#  endif /* (_STLP_MSVC < 1100 ) */

 #  if (_STLP_MSVC < 1900)
#  define _STLP_NO_TYPENAME_ON_RETURN_TYPE 1
 #  endif
//  using ::func_name results in ambiguity

# if (_STLP_MSVC <= 1300) 
// boris : not defining this macro for SP5 causes other problems
// #  if !defined (_MSC_FULL_VER) || (_MSC_FULL_VER < 12008804 )
#   define _STLP_NO_USING_FOR_GLOBAL_FUNCTIONS 1
//#  endif
#  define _STLP_NO_FUNCTION_TMPL_PARTIAL_ORDER 1
#  define _STLP_NO_CLASS_PARTIAL_SPECIALIZATION 1
#  define _STLP_NO_FRIEND_TEMPLATES
#  define _STLP_STATIC_CONST_INIT_BUG   1
//  these work, as long they are inline
#   define _STLP_INLINE_MEMBER_TEMPLATES 1
// VC++ cannot handle default allocator argument in template constructors
#   define _STLP_NEEDS_EXTRA_TEMPLATE_CONSTRUCTORS
// there is no partial spec, and MSVC breaks on simulating it for iterator_traits queries
#   define _STLP_USE_OLD_HP_ITERATOR_QUERIES
# endif

// #  define _STLP_NO_EXPLICIT_FUNCTION_TMPL_ARGS 1
#  define _STLP_NO_MEMBER_TEMPLATE_KEYWORD 1
#  define _STLP_NO_MEMBER_TEMPLATE_CLASSES 1

#  define _STLP_NO_QUALIFIED_FRIENDS    1
#  define _STLP_DONT_USE_BOOL_TYPEDEF 1

# endif /* _STLP_MSVC */


# if (_MSC_VER <= 1300) 
#  define _STLP_VENDOR_GLOBAL_CSTD
// They included the necessary coding,
// but the beta still has an issue with template classes
// ok:    class a { static const int v = 2; };
// error: template &lt;class _Tp> class a { static const int v = 2; };
# endif /* (_MSC_VER <= 1300) */

# if (_MSC_VER <= 1200)  // including MSVC 6.0
#  define _STLP_GLOBAL_NEW_HANDLER
# endif /* (_MSC_VER <= 1200) */

# if ( _MSC_VER<=1010 )
// "bool" is reserved in MSVC 4.1 while <yvals.h> absent, so :
// #    define _STLP_USE_ABBREVS           1
#  define _STLP_NO_BAD_ALLOC
#  define _STLP_HAS_NO_NEW_C_HEADERS 1
#  define _STLP_NO_NEW_NEW_HEADER 1
# elif (_MSC_VER < 1100)
// VC++ 4.2 and higher
#  define _STLP_YVALS_H 1
#  define _STLP_HAS_NO_NEW_IOSTREAMS 1
# endif /* 1010 */

# if defined (_STLP_MSVC) && ( _STLP_MSVC < 1200 ) /* VC++ 6.0 */
// #  define _STLP_NO_MEMBER_TEMPLATES 1
// #  define _STLP_NO_EXPLICIT_FUNCTION_TMPL_ARGS 1
#  define _STLP_NON_TYPE_TMPL_PARAM_BUG 1 
#  define _STLP_THROW_RETURN_BUG 1
# endif

# if defined (_STLP_MSVC) && ( _STLP_MSVC < 1100 )
#  ifndef _STLP_NO_OWN_IOSTREAMS
#   define _STLP_NO_OWN_IOSTREAMS
#   undef  _STLP_OWN_IOSTREAMS
#  endif
// #  define _STLP_NESTED_TYPE_PARAM_BUG 1
// Debug mode does not work for 4.2
#  ifdef _STLP_DEBUG
#   pragma message ("STLport debug mode does not work for VC++ 4.2, turning _STLP_DEBUG off ...")
#    undef _STLP_DEBUG
#  endif
#  define _STLP_NO_BOOL            1
#  define _STLP_NEED_TYPENAME      1
#  define _STLP_NEED_EXPLICIT      1
#   define _STLP_NEED_MUTABLE       1
#   define _STLP_NO_PARTIAL_SPECIALIZATION_SYNTAX
#   define _STLP_LIMITED_DEFAULT_TEMPLATES 1

// up to 4.2, library is in global namespace
#   define _STLP_VENDOR_GLOBAL_STD
#   define _STLP_NONTEMPL_BASE_MATCH_BUG 1
#   define _STLP_BROKEN_USING_DIRECTIVE  1
#   define _STLP_NO_ARROW_OPERATOR
#   define _STLP_NO_SIGNED_BUILTINS 1
#   define _STLP_NO_EXCEPTION_SPEC 1
#   undef  _STLP_DEFAULT_TYPE_PARAM
#   undef  _STLP_HAS_NO_NAMESPACES
#   undef  _STLP_NO_AT_MEMBER_FUNCTION
#   undef  _STLP_NO_MEMBER_TEMPLATES
#   undef  _STLP_NO_MEMBER_TEMPLATE_CLASSES
#   define  _STLP_HAS_NO_NAMESPACES 1
#   define  _STLP_NO_AT_MEMBER_FUNCTION 1
#  define  _STLP_NO_MEMBER_TEMPLATES
#  define  _STLP_NO_MEMBER_TEMPLATE_CLASSES
# endif /* 1100 */

// If we are under Windows CE, include appropriate config

# ifdef UNDER_CE
#   include <config/stl_wince.h>
# endif

# ifdef __ICL
#  define _STLP_LIB_BASENAME "stlport_icl"
# else
# if (_MSC_VER >= 1300) 
#   define _STLP_LIB_BASENAME "stlport_vc7"
# elif (_MSC_VER >= 1200)
//#   ifdef _UNICODE
//#    define _STLP_LIB_BASENAME "stlport_vc6_unicode"
//#   else
#    define _STLP_LIB_BASENAME "stlport_vc6"
//#   endif
#  elif (_MSC_VER >= 1100)
//#   ifdef _UNICODE
//#    define _STLP_LIB_BASENAME "stlport_vc5_unicode"
//#   else
#    define _STLP_LIB_BASENAME "stlport_vc5"
//#   endif
#  endif /* (_MSC_VER >= 1200) */
# endif /* __ICL */


#    if (defined (__ICL) && (__ICL < 450)) || (_MSC_VER < 1200)
//    only static STLport lib now works for ICL and VC 5.0
#     undef  _STLP_USE_STATIC_LIB
#     define _STLP_USE_STATIC_LIB
//    disable hook which makes template symbols to be searched for in the library
#     undef _STLP_NO_CUSTOM_IO
#    endif

# if (_MSC_VER >= 1900)
#  define _STLP_NATIVE_C_HEADER_UCRT(x) <../ucrt/##x>
# else
#  define _STLP_NATIVE_C_HEADER_UCRT _STLP_NATIVE_C_HEADER
# endif

#   include <config/vc_select_lib.h>




