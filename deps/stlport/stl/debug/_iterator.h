/*
 *
 * Copyright (c) 1997
 * Moscow Center for SPARC Technology
 *
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

#ifndef _STLP_DBG_ITERATOR_H
# define _STLP_DBG_ITERATOR_H

# include <stl/_pair.h>
# include <stl/_alloc.h>

# define _STLP_DBG_ALLOCATOR_SELECT( _Tp ) _STLP_DEFAULT_ALLOCATOR_SELECT( _Tp )

_STLP_BEGIN_NAMESPACE

//============================================================

template <class _Iterator>
void _Decrement(_Iterator& __it, const bidirectional_iterator_tag &) {
  --__it;
}

template <class _Iterator>
void _Decrement(_Iterator& __it, const random_access_iterator_tag &) {
  --__it;
}

template <class _Iterator>
void _Decrement(_Iterator& __it, const forward_iterator_tag &) {
  _STLP_ASSERT(0)
}

template <class _Iterator>
void _Advance(_Iterator&, ptrdiff_t, const forward_iterator_tag &) {
  _STLP_ASSERT(0)
}

template <class _Iterator>
void _Advance(_Iterator& __it, ptrdiff_t, const bidirectional_iterator_tag &) {
  _STLP_ASSERT(0)  
}

template <class _Iterator>
void _Advance(_Iterator& __it, ptrdiff_t __n, const random_access_iterator_tag &) {
  __it += __n;
}

template <class _Iterator>
ptrdiff_t _DBG_distance(const _Iterator& __x, const _Iterator& __y, const random_access_iterator_tag &) {
  return __x - __y;
}

template <class _Iterator>
ptrdiff_t _DBG_distance(const _Iterator&, const _Iterator&, const forward_iterator_tag &) {
  _STLP_ASSERT(0)
  return 0;
}

template <class _Iterator>
ptrdiff_t _DBG_distance(const _Iterator&, const _Iterator&, const bidirectional_iterator_tag &) {
  _STLP_ASSERT(0)  
  return 0;
}

template <class _Iterator>
bool _CompareIt(const _Iterator&, const _Iterator&, const forward_iterator_tag &) {
  _STLP_ASSERT(0)
  return false;
}

template <class _Iterator>
bool _CompareIt(const _Iterator&, const _Iterator&, const bidirectional_iterator_tag &) {
  _STLP_ASSERT(0)  
  return false;
}

template <class _Iterator>
bool _CompareIt(const _Iterator& __x, const _Iterator& __y, const random_access_iterator_tag &) {
  return __x < __y;
}


template <class _Iterator>
bool _Dereferenceable(_Iterator __it) {
  return (__it._Get_container_ptr() !=0) && !(__it._M_iterator == (__it._Get_container_ptr())->end());
}


template <class _Iterator>
bool _Incrementable(const _Iterator& __it, ptrdiff_t __n, const forward_iterator_tag &) {
  return (__n == 1) && _Dereferenceable(__it);
}

template <class _Iterator>
bool _Incrementable(const _Iterator& __it, ptrdiff_t __n, const bidirectional_iterator_tag &) {
  typedef typename _Iterator::_Container_type __container_type;
  __container_type* __c = __it._Get_container_ptr();
  return (__c!=0) && ((__n == 1 && __it._M_iterator != __c->end() ) ||
    (__n == -1 && __it._M_iterator != __c->begin()));
}

template <class _Iterator>
bool _Incrementable(const _Iterator& __it, ptrdiff_t __n, const random_access_iterator_tag &) {
  typedef typename _Iterator::_Container_type __container_type;
  __container_type* __c = __it._Get_container_ptr();
  if (!__c) return false;
  ptrdiff_t __new_pos = (__it._M_iterator - __c->begin()) + __n;
  return  (__new_pos >=0) && (__STATIC_CAST(typename __container_type::size_type,__new_pos) <=__c->size());
}


template <class _Container>
struct _DBG_iter_base : public __owned_link {
public:
  typedef typename _Container::value_type value_type;
  typedef typename _Container::reference  reference;
  typedef typename _Container::pointer    pointer;
  typedef ptrdiff_t difference_type;
  //private:
  typedef typename _Container::iterator        _Nonconst_iterator;
  typedef typename _Container::const_iterator  _Const_iterator;
  typedef _Container                     _Container_type;
    
# ifdef _STLP_CLASS_PARTIAL_SPECIALIZATION
  typedef typename iterator_traits<_Const_iterator>::iterator_category _Iterator_category;
# else
  typedef typename _Container::_Iterator_category  _Iterator_category;
# endif
  typedef _Iterator_category iterator_category;

  _DBG_iter_base() : __owned_link(0)  {}
  _DBG_iter_base(const __owned_list* __c, const _Const_iterator& __it) :
# ifdef __HP_aCC
    __owned_link(__c), _M_iterator(*__REINTERPRET_CAST(const _Nonconst_iterator *, &__it)) {}
# else
    __owned_link(__c), _M_iterator(*(const _Nonconst_iterator*)&__it) {}
# endif  
  _Container* _Get_container_ptr() const { 
    return (_Container*)__stl_debugger::_Get_container_ptr(this); 
  }

  void __increment() {
    _STLP_DEBUG_CHECK(_Incrementable(*this,1,_Iterator_category()))
    ++_M_iterator;
  }
  
  void __decrement() {
    _STLP_DEBUG_CHECK(_Incrementable(*this,-1,_Iterator_category()))
    _Decrement(_M_iterator, _Iterator_category());
  }

  void __advance(difference_type __n) {
    _STLP_DEBUG_CHECK(_Incrementable(*this,__n, _Iterator_category()))
    _Advance(_M_iterator,__n, _Iterator_category());
  }

// protected:
  _Nonconst_iterator _M_iterator;
};

template <class _Container>
ptrdiff_t operator-(const _DBG_iter_base<_Container>& __x,
                    const _DBG_iter_base<_Container>& __y ) {
  typedef typename _DBG_iter_base<_Container>::_Iterator_category  _Iterator_category;
  _STLP_DEBUG_CHECK(__check_same_owner(__x, __y))
  return _DBG_distance(__x._M_iterator,__y._M_iterator, _Iterator_category());
}

template <class _Container, class _Traits>
struct _DBG_iter_mid : public _DBG_iter_base<_Container>
{
  typedef _DBG_iter_mid<_Container, typename _Traits::_Non_const_traits> _Nonconst_self;
  typedef typename _Container::iterator        _Nonconst_iterator;
  typedef typename _Container::const_iterator  _Const_iterator;

  _DBG_iter_mid() {}

  explicit _DBG_iter_mid(const _Nonconst_self& __it) :
      _DBG_iter_base<_Container>(__it) {}
        
  _DBG_iter_mid(const __owned_list* __c, const _Const_iterator& __it) :
      _DBG_iter_base<_Container>(__c, __it) {}
};

template <class _Container, class _Traits>
struct _DBG_iter : public _DBG_iter_mid<_Container, _Traits> {
  typedef _DBG_iter_base<_Container>          _Base;
public:
  typedef typename _Base::value_type value_type;
  typedef typename _Base::difference_type difference_type;    
  typedef typename _Traits::reference  reference;
  typedef typename _Traits::pointer    pointer;

private:
  typedef typename _Base::_Nonconst_iterator _Nonconst_iterator;
  typedef typename _Base::_Const_iterator _Const_iterator;

  typedef _DBG_iter<_Container, _Traits>     _Self;
  typedef _DBG_iter_mid<_Container, typename _Traits::_Non_const_traits> _Nonconst_mid;

public:

# ifdef _STLP_CLASS_PARTIAL_SPECIALIZATION
  typedef typename _Base::iterator_category iterator_category;
# endif
  typedef typename _Base::_Iterator_category  _Iterator_category;
    
public:
  _DBG_iter() {}
    // boris : real type of iter would be nice
  _DBG_iter(const __owned_list* __c, const _Const_iterator& __it) :
    _DBG_iter_mid<_Container, _Traits>(__c, __it) {}

  // This allows conversions from iterator to const_iterator without being
  // redundant with the copy constructor below.
  _DBG_iter(const _Nonconst_mid& __rhs) :
    _DBG_iter_mid<_Container, _Traits>(__rhs) {}

  _DBG_iter(const  _Self& __rhs) :
    _DBG_iter_mid<_Container, _Traits>(__rhs) {}
  
  // This allows conversions from iterator to const_iterator without being
  // redundant with the copy assignment operator below.
  _Self& operator=(const _Nonconst_mid& __rhs)  
  {
    (_Base&)*this = __rhs;
    return *this;
  }

  _Self& operator=(const  _Self& __rhs) 
  {
    (_Base&)*this = __rhs;
    return *this;
  }
  
  reference operator*() const {
    _STLP_DEBUG_CHECK(_Dereferenceable(*this))
    return *this->_M_iterator;
  }

  _STLP_DEFINE_ARROW_OPERATOR
  
  _Self& operator++() {
    this->__increment();
    return *this;
  }
  _Self operator++(int) {
    _Self __tmp = *this;
    this->__increment();
    return __tmp;
  }
  _Self& operator--() {
    this->__decrement();
    return *this;
  }
  _Self operator--(int) {
    _Self __tmp = *this;
    this->__decrement();
    return __tmp;
  }
  
  _Self& operator+=(difference_type __n) {
    this->__advance(__n);
    return *this;
  }
  
  _Self& operator-=(difference_type __n) {
    this->__advance(-__n);
    return *this;
  }
  _Self operator+(difference_type __n) const {
    _Self __tmp(*this);
    __tmp.__advance(__n);
    return __tmp;
  }
  _Self operator-(difference_type __n) const {
    _Self __tmp(*this);
    __tmp.__advance(-__n);
    return __tmp;
  }
  reference operator[](difference_type __n) const { return *(*this + __n); }
};

template <class _Container>
inline bool 
operator==(const _DBG_iter_base<_Container>& __x, const _DBG_iter_base<_Container>& __y) {
  _STLP_DEBUG_CHECK(__check_same_owner_or_null(__x, __y))
  return __x._M_iterator==__y._M_iterator;
}

template <class _Container>
inline bool 
operator<(const _DBG_iter_base<_Container>& __x, const _DBG_iter_base<_Container>& __y) {
  _STLP_DEBUG_CHECK(__check_same_owner_or_null(__x, __y))
  typedef typename _DBG_iter_base<_Container>::_Iterator_category _Category;
  return _CompareIt(__x._M_iterator , __y._M_iterator, _Category());
}

template <class _Container>
inline bool 
operator>(const _DBG_iter_base<_Container>& __x,
	  const _DBG_iter_base<_Container>& __y) { 
  typedef typename _DBG_iter_base<_Container>::_Iterator_category _Category;
  return _CompareIt(__y._M_iterator , __x._M_iterator, _Category());
}

template <class _Container>
inline bool 
operator>=(const _DBG_iter_base<_Container>& __x, const _DBG_iter_base<_Container>& __y) {
  _STLP_DEBUG_CHECK(__check_same_owner_or_null(__x, __y))
  typedef typename _DBG_iter_base<_Container>::_Iterator_category _Category;
  return !_CompareIt(__x._M_iterator , __y._M_iterator, _Category());
}

template <class _Container>
inline bool 
operator<=(const _DBG_iter_base<_Container>& __x,
	  const _DBG_iter_base<_Container>& __y) {
  typedef typename _DBG_iter_base<_Container>::_Iterator_category _Category; 
  return !_CompareIt(__y._M_iterator , __x._M_iterator, _Category());
}

template <class _Container>
inline bool 
operator!=(const _DBG_iter_base<_Container>& __x, 
	   const _DBG_iter_base<_Container>& __y) {
  _STLP_DEBUG_CHECK(__check_same_owner_or_null(__x, __y))
  return __x._M_iterator != __y._M_iterator;
}

//------------------------------------------

template <class _Container, class _Traits>
inline _DBG_iter<_Container, _Traits> 
operator+(ptrdiff_t __n, const _DBG_iter<_Container, _Traits>& __it) {
    _DBG_iter<_Container, _Traits> __tmp(__it);
    return __tmp += __n;
}


# ifdef _STLP_USE_OLD_HP_ITERATOR_QUERIES
# if defined (_STLP_NESTED_TYPE_PARAM_BUG) \
   || ( defined (__SUNPRO_CC) && __SUNPRO_CC < 0x600) \
   || ( defined (_STLP_MSVC) && (_STLP_MSVC < 1100) )
#  define _STLP_DEBUG_USE_DISTINCT_VALUE_TYPE_HELPERS 1
# endif

template <class _Container>
inline ptrdiff_t* 
distance_type(const  _DBG_iter_base<_Container>&) { return (ptrdiff_t*) 0; }

# if !defined (_STLP_DEBUG_USE_DISTINCT_VALUE_TYPE_HELPERS)
template <class _Container>
inline _STLP_TYPENAME_ON_RETURN_TYPE _DBG_iter_base<_Container>::value_type*
value_type(const  _DBG_iter_base<_Container>&) {
  typedef typename _DBG_iter_base<_Container>::value_type _Val;
  return (_Val*)0;
}

template <class _Container>
inline _STLP_TYPENAME_ON_RETURN_TYPE _DBG_iter_base<_Container>::_Iterator_category 
iterator_category(const  _DBG_iter_base<_Container>&) {
  typedef typename _DBG_iter_base<_Container>::_Iterator_category _Category;
  return _Category();
}
# endif

#  endif /* _STLP_USE_OLD_HP_ITERATOR_QUERIES */

# define _Get_iter(__x)   __x
# define _Debug_iter(__x, __y) __y

_STLP_END_NAMESPACE

#endif /* INTERNAL_H */

// Local Variables:
// mode:C++
// End:

