// This is an implementation file which
// is intended to be included multiple times with different _STLP_ASSOCIATIVE_CONTAINER
// setting

template <class _Key, class _Compare, class _Alloc>
inline bool _STLP_CALL operator==(const _STLP_SET_FLAVOR<_Key,_Compare,_Alloc>& __x, 
                       const _STLP_SET_FLAVOR<_Key,_Compare,_Alloc>& __y) {
  return __x.size() == __y.size() &&
    equal(__x.begin(), __x.end(), __y.begin());
}

template <class _Key, class _Compare, class _Alloc>
inline bool _STLP_CALL operator<(const _STLP_SET_FLAVOR<_Key,_Compare,_Alloc>& __x, 
                      const _STLP_SET_FLAVOR<_Key,_Compare,_Alloc>& __y) {
  return lexicographical_compare(__x.begin(), __x.end(), 
				 __y.begin(), __y.end());
}

#ifdef _STLP_USE_SEPARATE_RELOPS_NAMESPACE

template <class _Key, class _Compare, class _Alloc>
inline bool _STLP_CALL operator!=(const _STLP_SET_FLAVOR<_Key,_Compare,_Alloc>& __x, 
                       const _STLP_SET_FLAVOR<_Key,_Compare,_Alloc>& __y) {
  return !(__x == __y);
}

template <class _Key, class _Compare, class _Alloc>
inline bool _STLP_CALL operator>(const _STLP_SET_FLAVOR<_Key,_Compare,_Alloc>& __x, 
                      const _STLP_SET_FLAVOR<_Key,_Compare,_Alloc>& __y) {
  return __y < __x;
}

template <class _Key, class _Compare, class _Alloc>
inline bool _STLP_CALL operator<=(const _STLP_SET_FLAVOR<_Key,_Compare,_Alloc>& __x, 
                       const _STLP_SET_FLAVOR<_Key,_Compare,_Alloc>& __y) {
  return !(__y < __x);
}

template <class _Key, class _Compare, class _Alloc>
inline bool _STLP_CALL operator>=(const _STLP_SET_FLAVOR<_Key,_Compare,_Alloc>& __x, 
                       const _STLP_SET_FLAVOR<_Key,_Compare,_Alloc>& __y) {
  return !(__x < __y);
}
#endif /* _STLP_USE_SEPARATE_RELOPS_NAMESPACE */

#ifdef _STLP_FUNCTION_TMPL_PARTIAL_ORDER

template <class _Key, class _Compare, class _Alloc>
inline void _STLP_CALL swap(_STLP_SET_FLAVOR<_Key,_Compare,_Alloc>& __x, 
                 _STLP_SET_FLAVOR<_Key,_Compare,_Alloc>& __y) {
  __x.swap(__y);
}

#endif /* _STLP_FUNCTION_TMPL_PARTIAL_ORDER */
