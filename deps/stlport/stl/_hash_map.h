/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
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

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef _STLP_INTERNAL_HASH_MAP_H
#define _STLP_INTERNAL_HASH_MAP_H

#ifndef _STLP_INTERNAL_HASHTABLE_H
# include <stl/_hashtable.h>
#endif

_STLP_BEGIN_NAMESPACE

# define  hash_map      __WORKAROUND_RENAME(hash_map)
# define  hash_multimap __WORKAROUND_RENAME(hash_multimap)

#  define _STLP_KEY_PAIR pair< const _Key, _Tp >
#  define _STLP_HASHTABLE hashtable \
      < pair < const _Key, _Tp >, _Key, _HashFcn, \
      _STLP_SELECT1ST( _STLP_KEY_PAIR,  _Key ), _EqualKey, _Alloc >

template <class _Key, class _Tp, __DFL_TMPL_PARAM(_HashFcn,hash<_Key>),
          __DFL_TMPL_PARAM(_EqualKey,equal_to<_Key>),
          _STLP_DEFAULT_PAIR_ALLOCATOR_SELECT(const _Key, _Tp) >
class hash_map
{
private:
  typedef _STLP_HASHTABLE _Ht;
  typedef hash_map<_Key, _Tp, _HashFcn, _EqualKey, _Alloc> _Self;
public:
  typedef typename _Ht::key_type key_type;
  typedef _Tp data_type;
  typedef _Tp mapped_type;
  typedef typename _Ht::value_type _value_type;
  typedef typename _Ht::value_type value_type;
  typedef typename _Ht::hasher hasher;
  typedef typename _Ht::key_equal key_equal;
  
  typedef typename _Ht::size_type size_type;
  typedef typename _Ht::difference_type difference_type;
  typedef typename _Ht::pointer pointer;
  typedef typename _Ht::const_pointer const_pointer;
  typedef typename _Ht::reference reference;
  typedef typename _Ht::const_reference const_reference;

  typedef typename _Ht::iterator iterator;
  typedef typename _Ht::const_iterator const_iterator;

  typedef typename _Ht::allocator_type allocator_type;

  hasher hash_funct() const { return _M_ht.hash_funct(); }
  key_equal key_eq() const { return _M_ht.key_eq(); }
  allocator_type get_allocator() const { return _M_ht.get_allocator(); }

private:
  _Ht _M_ht;
public:
  hash_map() : _M_ht(100, hasher(), key_equal(), allocator_type()) {}
  explicit hash_map(size_type __n)
    : _M_ht(__n, hasher(), key_equal(), allocator_type()) {}
  hash_map(size_type __n, const hasher& __hf)
    : _M_ht(__n, __hf, key_equal(), allocator_type()) {}
  hash_map(size_type __n, const hasher& __hf, const key_equal& __eql,
           const allocator_type& __a = allocator_type())
    : _M_ht(__n, __hf, __eql, __a) {}

#ifdef _STLP_MEMBER_TEMPLATES
  template <class _InputIterator>
  hash_map(_InputIterator __f, _InputIterator __l)
    : _M_ht(100, hasher(), key_equal(), allocator_type())
    { _M_ht.insert_unique(__f, __l); }
  template <class _InputIterator>
  hash_map(_InputIterator __f, _InputIterator __l, size_type __n)
    : _M_ht(__n, hasher(), key_equal(), allocator_type())
    { _M_ht.insert_unique(__f, __l); }
  template <class _InputIterator>
  hash_map(_InputIterator __f, _InputIterator __l, size_type __n,
           const hasher& __hf)
    : _M_ht(__n, __hf, key_equal(), allocator_type())
    { _M_ht.insert_unique(__f, __l); }
# ifdef _STLP_NEEDS_EXTRA_TEMPLATE_CONSTRUCTORS
  template <class _InputIterator>
  hash_map(_InputIterator __f, _InputIterator __l, size_type __n,
           const hasher& __hf, const key_equal& __eql)
    : _M_ht(__n, __hf, __eql, allocator_type())
    { _M_ht.insert_unique(__f, __l); }
# endif
  template <class _InputIterator>
  hash_map(_InputIterator __f, _InputIterator __l, size_type __n,
           const hasher& __hf, const key_equal& __eql,
           const allocator_type& __a _STLP_ALLOCATOR_TYPE_DFL)
    : _M_ht(__n, __hf, __eql, __a)
    { _M_ht.insert_unique(__f, __l); }

#else
  hash_map(const value_type* __f, const value_type* __l)
    : _M_ht(100, hasher(), key_equal(), allocator_type())
    { _M_ht.insert_unique(__f, __l); }
  hash_map(const value_type* __f, const value_type* __l, size_type __n)
    : _M_ht(__n, hasher(), key_equal(), allocator_type())
    { _M_ht.insert_unique(__f, __l); }
  hash_map(const value_type* __f, const value_type* __l, size_type __n,
           const hasher& __hf)
    : _M_ht(__n, __hf, key_equal(), allocator_type())
    { _M_ht.insert_unique(__f, __l); }
  hash_map(const value_type* __f, const value_type* __l, size_type __n,
           const hasher& __hf, const key_equal& __eql,
           const allocator_type& __a = allocator_type())
    : _M_ht(__n, __hf, __eql, __a)
    { _M_ht.insert_unique(__f, __l); }

  hash_map(const_iterator __f, const_iterator __l)
    : _M_ht(100, hasher(), key_equal(), allocator_type())
    { _M_ht.insert_unique(__f, __l); }
  hash_map(const_iterator __f, const_iterator __l, size_type __n)
    : _M_ht(__n, hasher(), key_equal(), allocator_type())
    { _M_ht.insert_unique(__f, __l); }
  hash_map(const_iterator __f, const_iterator __l, size_type __n,
           const hasher& __hf)
    : _M_ht(__n, __hf, key_equal(), allocator_type())
    { _M_ht.insert_unique(__f, __l); }
  hash_map(const_iterator __f, const_iterator __l, size_type __n,
           const hasher& __hf, const key_equal& __eql,
           const allocator_type& __a = allocator_type())
    : _M_ht(__n, __hf, __eql, __a)
    { _M_ht.insert_unique(__f, __l); }
#endif /*_STLP_MEMBER_TEMPLATES */

public:
  size_type size() const { return _M_ht.size(); }
  size_type max_size() const { return _M_ht.max_size(); }
  bool empty() const { return _M_ht.empty(); }
  void swap(_Self& __hs) { _M_ht.swap(__hs._M_ht); }
  iterator begin() { return _M_ht.begin(); }
  iterator end() { return _M_ht.end(); }
  const_iterator begin() const { return _M_ht.begin(); }
  const_iterator end() const { return _M_ht.end(); }

public:
  pair<iterator,bool> insert(const value_type& __obj)
    { return _M_ht.insert_unique(__obj); }
#ifdef _STLP_MEMBER_TEMPLATES
  template <class _InputIterator>
  void insert(_InputIterator __f, _InputIterator __l)
    { _M_ht.insert_unique(__f,__l); }
#else
  void insert(const value_type* __f, const value_type* __l) {
    _M_ht.insert_unique(__f,__l);
  }
  void insert(const_iterator __f, const_iterator __l)
    { _M_ht.insert_unique(__f, __l); }
#endif /*_STLP_MEMBER_TEMPLATES */
  pair<iterator,bool> insert_noresize(const value_type& __obj)
    { return _M_ht.insert_unique_noresize(__obj); }    

  iterator find(const key_type& __key) { return _M_ht.find(__key); }
  const_iterator find(const key_type& __key) const { return _M_ht.find(__key); }

  _Tp& operator[](const key_type& __key) {
    iterator __it = _M_ht.find(__key);
    return (__it == _M_ht.end() ? 
	    _M_ht._M_insert(_value_type(__key, _Tp())).second : 
	    (*__it).second );
  }

  size_type count(const key_type& __key) const { return _M_ht.count(__key); }
  
  pair<iterator, iterator> equal_range(const key_type& __key)
    { return _M_ht.equal_range(__key); }
  pair<const_iterator, const_iterator>
  equal_range(const key_type& __key) const
    { return _M_ht.equal_range(__key); }

  size_type erase(const key_type& __key) {return _M_ht.erase(__key); }
  void erase(iterator __it) { _M_ht.erase(__it); }
  void erase(iterator __f, iterator __l) { _M_ht.erase(__f, __l); }
  void clear() { _M_ht.clear(); }

  void resize(size_type __hint) { _M_ht.resize(__hint); }
  size_type bucket_count() const { return _M_ht.bucket_count(); }
  size_type max_bucket_count() const { return _M_ht.max_bucket_count(); }
  size_type elems_in_bucket(size_type __n) const
    { return _M_ht.elems_in_bucket(__n); }
  static bool _STLP_CALL _M_equal (const _Self& __x, const _Self& __y) {
    return _Ht::_M_equal(__x._M_ht,__y._M_ht);
  }
};


template <class _Key, class _Tp, class _HashFcn, class _EqlKey, class _Alloc>
inline bool _STLP_CALL 
operator==(const hash_map<_Key,_Tp,_HashFcn,_EqlKey,_Alloc>& __hm1,
           const hash_map<_Key,_Tp,_HashFcn,_EqlKey,_Alloc>& __hm2)
{
  return hash_map<_Key,_Tp,_HashFcn,_EqlKey,_Alloc>::_M_equal(__hm1, __hm2);
}
#ifdef _STLP_USE_SEPARATE_RELOPS_NAMESPACE

template <class _Key, class _Tp, class _HashFcn, class _EqlKey, class _Alloc>
inline bool _STLP_CALL 
operator!=(const hash_map<_Key,_Tp,_HashFcn,_EqlKey,_Alloc>& __hm1,
           const hash_map<_Key,_Tp,_HashFcn,_EqlKey,_Alloc>& __hm2) {
  return !(__hm1 == __hm2);
}

template <class _Key, class _Tp, class _HashFcn, class _EqlKey, class _Alloc>
inline void _STLP_CALL 
swap(hash_map<_Key,_Tp,_HashFcn,_EqlKey,_Alloc>& __hm1,
     hash_map<_Key,_Tp,_HashFcn,_EqlKey,_Alloc>& __hm2)
{
  __hm1.swap(__hm2);
}

#endif /* _STLP_FUNCTION_TMPL_PARTIAL_ORDER */

template <class _Key, class _Tp, __DFL_TMPL_PARAM(_HashFcn,hash<_Key>),
          __DFL_TMPL_PARAM(_EqualKey,equal_to<_Key>),
          _STLP_DEFAULT_PAIR_ALLOCATOR_SELECT(const _Key, _Tp) >
class hash_multimap
{
private:
  typedef _STLP_HASHTABLE _Ht;
  typedef hash_multimap<_Key, _Tp, _HashFcn, _EqualKey, _Alloc> _Self;
public:
  typedef typename _Ht::key_type key_type;
  typedef _Tp data_type;
  typedef _Tp mapped_type;
  typedef typename _Ht::value_type _value_type;
  typedef _value_type value_type;
  typedef typename _Ht::hasher hasher;
  typedef typename _Ht::key_equal key_equal;

  typedef typename _Ht::size_type size_type;
  typedef typename _Ht::difference_type difference_type;
  typedef typename _Ht::pointer pointer;
  typedef typename _Ht::const_pointer const_pointer;
  typedef typename _Ht::reference reference;
  typedef typename _Ht::const_reference const_reference;

  typedef typename _Ht::iterator iterator;
  typedef typename _Ht::const_iterator const_iterator;

  typedef typename _Ht::allocator_type allocator_type;

  hasher hash_funct() const { return _M_ht.hash_funct(); }
  key_equal key_eq() const { return _M_ht.key_eq(); }
  allocator_type get_allocator() const { return _M_ht.get_allocator(); }

private:
  _Ht _M_ht;
public:
  hash_multimap() : _M_ht(100, hasher(), key_equal(), allocator_type()) {}
  explicit hash_multimap(size_type __n)
    : _M_ht(__n, hasher(), key_equal(), allocator_type()) {}
  hash_multimap(size_type __n, const hasher& __hf)
    : _M_ht(__n, __hf, key_equal(), allocator_type()) {}
  hash_multimap(size_type __n, const hasher& __hf, const key_equal& __eql,
                const allocator_type& __a = allocator_type())
    : _M_ht(__n, __hf, __eql, __a) {}

#ifdef _STLP_MEMBER_TEMPLATES
  template <class _InputIterator>
  hash_multimap(_InputIterator __f, _InputIterator __l)
    : _M_ht(100, hasher(), key_equal(), allocator_type())
    { _M_ht.insert_equal(__f, __l); }
  template <class _InputIterator>
  hash_multimap(_InputIterator __f, _InputIterator __l, size_type __n)
    : _M_ht(__n, hasher(), key_equal(), allocator_type())
    { _M_ht.insert_equal(__f, __l); }
  template <class _InputIterator>
  hash_multimap(_InputIterator __f, _InputIterator __l, size_type __n,
                const hasher& __hf)
    : _M_ht(__n, __hf, key_equal(), allocator_type())
    { _M_ht.insert_equal(__f, __l); }
# ifdef _STLP_NEEDS_EXTRA_TEMPLATE_CONSTRUCTORS
  template <class _InputIterator>
  hash_multimap(_InputIterator __f, _InputIterator __l, size_type __n,
                const hasher& __hf, const key_equal& __eql)
    : _M_ht(__n, __hf, __eql, allocator_type())
    { _M_ht.insert_equal(__f, __l); }
#  endif
  template <class _InputIterator>
  hash_multimap(_InputIterator __f, _InputIterator __l, size_type __n,
                const hasher& __hf, const key_equal& __eql,
                const allocator_type& __a _STLP_ALLOCATOR_TYPE_DFL)
    : _M_ht(__n, __hf, __eql, __a)
    { _M_ht.insert_equal(__f, __l); }

#else
  hash_multimap(const value_type* __f, const value_type* __l)
    : _M_ht(100, hasher(), key_equal(), allocator_type())
    { _M_ht.insert_equal(__f, __l); }
  hash_multimap(const value_type* __f, const value_type* __l, size_type __n)
    : _M_ht(__n, hasher(), key_equal(), allocator_type())
    { _M_ht.insert_equal(__f, __l); }
  hash_multimap(const value_type* __f, const value_type* __l, size_type __n,
                const hasher& __hf)
    : _M_ht(__n, __hf, key_equal(), allocator_type())
    { _M_ht.insert_equal(__f, __l); }
  hash_multimap(const value_type* __f, const value_type* __l, size_type __n,
                const hasher& __hf, const key_equal& __eql,
                const allocator_type& __a = allocator_type())
    : _M_ht(__n, __hf, __eql, __a)
    { _M_ht.insert_equal(__f, __l); }

  hash_multimap(const_iterator __f, const_iterator __l)
    : _M_ht(100, hasher(), key_equal(), allocator_type())
    { _M_ht.insert_equal(__f, __l); }
  hash_multimap(const_iterator __f, const_iterator __l, size_type __n)
    : _M_ht(__n, hasher(), key_equal(), allocator_type())
    { _M_ht.insert_equal(__f, __l); }
  hash_multimap(const_iterator __f, const_iterator __l, size_type __n,
                const hasher& __hf)
    : _M_ht(__n, __hf, key_equal(), allocator_type())
    { _M_ht.insert_equal(__f, __l); }
  hash_multimap(const_iterator __f, const_iterator __l, size_type __n,
                const hasher& __hf, const key_equal& __eql,
                const allocator_type& __a = allocator_type())
    : _M_ht(__n, __hf, __eql, __a)
    { _M_ht.insert_equal(__f, __l); }
#endif /*_STLP_MEMBER_TEMPLATES */

public:
  size_type size() const { return _M_ht.size(); }
  size_type max_size() const { return _M_ht.max_size(); }
  bool empty() const { return _M_ht.empty(); }
  void swap(_Self& __hs) { _M_ht.swap(__hs._M_ht); }

  iterator begin() { return _M_ht.begin(); }
  iterator end() { return _M_ht.end(); }
  const_iterator begin() const { return _M_ht.begin(); }
  const_iterator end() const { return _M_ht.end(); }

public:
  iterator insert(const value_type& __obj) 
    { return _M_ht.insert_equal(__obj); }
#ifdef _STLP_MEMBER_TEMPLATES
  template <class _InputIterator>
  void insert(_InputIterator __f, _InputIterator __l) 
    { _M_ht.insert_equal(__f,__l); }
#else
  void insert(const value_type* __f, const value_type* __l) {
    _M_ht.insert_equal(__f,__l);
  }
  void insert(const_iterator __f, const_iterator __l) 
    { _M_ht.insert_equal(__f, __l); }
#endif /*_STLP_MEMBER_TEMPLATES */
  iterator insert_noresize(const value_type& __obj)
    { return _M_ht.insert_equal_noresize(__obj); }    

  iterator find(const key_type& __key) { return _M_ht.find(__key); }
  const_iterator find(const key_type& __key) const 
    { return _M_ht.find(__key); }

  size_type count(const key_type& __key) const { return _M_ht.count(__key); }
  
  pair<iterator, iterator> equal_range(const key_type& __key)
    { return _M_ht.equal_range(__key); }
  pair<const_iterator, const_iterator>
  equal_range(const key_type& __key) const
    { return _M_ht.equal_range(__key); }

  size_type erase(const key_type& __key) {return _M_ht.erase(__key); }
  void erase(iterator __it) { _M_ht.erase(__it); }
  void erase(iterator __f, iterator __l) { _M_ht.erase(__f, __l); }
  void clear() { _M_ht.clear(); }

public:
  void resize(size_type __hint) { _M_ht.resize(__hint); }
  size_type bucket_count() const { return _M_ht.bucket_count(); }
  size_type max_bucket_count() const { return _M_ht.max_bucket_count(); }
  size_type elems_in_bucket(size_type __n) const
    { return _M_ht.elems_in_bucket(__n); }
  static bool _STLP_CALL _M_equal (const _Self& __x, const _Self& __y) {
    return _Ht::_M_equal(__x._M_ht,__y._M_ht);
  }
};


template <class _Key, class _Tp, class _HashFcn, class _EqlKey, class _Alloc>
inline bool _STLP_CALL 
operator==(const hash_multimap<_Key,_Tp,_HashFcn,_EqlKey,_Alloc>& __hm1,
           const hash_multimap<_Key,_Tp,_HashFcn,_EqlKey,_Alloc>& __hm2)
{
  return hash_multimap<_Key,_Tp,_HashFcn,_EqlKey,_Alloc>::_M_equal(__hm1, __hm2);
}

#ifdef _STLP_USE_SEPARATE_RELOPS_NAMESPACE

template <class _Key, class _Tp, class _HashFcn, class _EqlKey, class _Alloc>
inline bool _STLP_CALL 
operator!=(const hash_multimap<_Key,_Tp,_HashFcn,_EqlKey,_Alloc>& __hm1,
           const hash_multimap<_Key,_Tp,_HashFcn,_EqlKey,_Alloc>& __hm2) {
  return !(__hm1 == __hm2);
}

template <class _Key, class _Tp, class _HashFcn, class _EqlKey, class _Alloc>
inline void _STLP_CALL 
swap(hash_multimap<_Key,_Tp,_HashFcn,_EqlKey,_Alloc>& __hm1,
     hash_multimap<_Key,_Tp,_HashFcn,_EqlKey,_Alloc>& __hm2)
{
  __hm1.swap(__hm2);
}

#endif /* _STLP_USE_SEPARATE_RELOPS_NAMESPACE */


// Specialization of insert_iterator so that it will work for hash_map
// and hash_multimap.

#ifdef _STLP_CLASS_PARTIAL_SPECIALIZATION

template <class _Key, class _Tp, class _HashFn,  class _EqKey, class _Alloc>
class insert_iterator<hash_map<_Key, _Tp, _HashFn, _EqKey, _Alloc> > {
protected:
  typedef hash_map<_Key, _Tp, _HashFn, _EqKey, _Alloc> _Container;
  _Container* container;
public:
  typedef _Container          container_type;
  typedef output_iterator_tag iterator_category;
  typedef void                value_type;
  typedef void                difference_type;
  typedef void                pointer;
  typedef void                reference;

  insert_iterator(_Container& __x) : container(&__x) {}
  insert_iterator(_Container& __x, typename _Container::iterator)
    : container(&__x) {}
  insert_iterator<_Container>&
  operator=(const typename _Container::value_type& __val) { 
    container->insert(__val);
    return *this;
  }
  insert_iterator<_Container>& operator*() { return *this; }
  insert_iterator<_Container>& operator++() { return *this; }
  insert_iterator<_Container>& operator++(int) { return *this; }
};

template <class _Key, class _Tp, class _HashFn,  class _EqKey, class _Alloc>
class insert_iterator<hash_multimap<_Key, _Tp, _HashFn, _EqKey, _Alloc> > {
protected:
  typedef hash_multimap<_Key, _Tp, _HashFn, _EqKey, _Alloc> _Container;
  _Container* container;
  typename _Container::iterator iter;
public:
  typedef _Container          container_type;
  typedef output_iterator_tag iterator_category;
  typedef void                value_type;
  typedef void                difference_type;
  typedef void                pointer;
  typedef void                reference;

  insert_iterator(_Container& __x) : container(&__x) {}
  insert_iterator(_Container& __x, typename _Container::iterator)
    : container(&__x) {}
  insert_iterator<_Container>&
  operator=(const typename _Container::value_type& __val) { 
    container->insert(__val);
    return *this;
  }
  insert_iterator<_Container>& operator*() { return *this; }
  insert_iterator<_Container>& operator++() { return *this; }
  insert_iterator<_Container>& operator++(int) { return *this; }
};

#endif /* _STLP_CLASS_PARTIAL_SPECIALIZATION */

// do a cleanup
# undef hash_map
# undef hash_multimap

# define __hash_map__ __FULL_NAME(hash_map)
# define __hash_multimap__ __FULL_NAME(hash_multimap)


_STLP_END_NAMESPACE

# if defined (_STLP_USE_WRAPPER_FOR_ALLOC_PARAM) 
#  include <stl/wrappers/_hash_map.h>
# endif /*  WRAPPER */

#endif /* _STLP_INTERNAL_HASH_MAP_H */

// Local Variables:
// mode:C++
// End:

