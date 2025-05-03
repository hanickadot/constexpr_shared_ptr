// Smart pointer adaptors -*- C++ -*-

// Copyright The GNU Toolchain Authors.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file include/bits/out_ptr.h
 *  This is an internal header file, included by other library headers.
 *  Do not attempt to use it directly. @headername{memory}
 */

#ifndef _GLIBCXX_OUT_PTR_H
#define _GLIBCXX_OUT_PTR_H 1

#ifdef _GLIBCXX_SYSHDR
#pragma GCC system_header
#endif

#include <bits/version.h>

#ifdef __glibcxx_out_ptr // C++ >= 23

#include <tuple>
#include <bits/ptr_traits.h>

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  namespace __detail {
    template <typename Pointer, bool AllowReference> struct void_ptr_storage {
      union storage_type {
        Pointer * type_reference;
        Pointer type_value;
        void * void_value;
      };
      
      mutable storage_type storage;
      
      constexpr void_ptr_storage() noexcept: storage{.type_value{nullptr}} {
        
      }
      
      constexpr void_ptr_storage(std::nullptr_t) noexcept: storage{.type_value{nullptr}} { }
      
      explicit constexpr void_ptr_storage(Pointer & target) noexcept requires (AllowReference): storage{.type_reference{__builtin_addressof(target)}} {
        if (target != nullptr) {
          assert((static_cast<bool>(*this)));
        }
      }
      
      explicit constexpr void_ptr_storage(Pointer target) noexcept: storage{.type_value{target}} {
        if (target != nullptr) {
          assert((static_cast<bool>(*this)));
        }
        
      }
      
      explicit constexpr void_ptr_storage(void * target) noexcept: storage{.void_value{target}} {
        if (target != nullptr) {
          assert((static_cast<bool>(*this)));
        }
      }
      
      void_ptr_storage(const void_ptr_storage & other) = default;
      void_ptr_storage(void_ptr_storage && other) noexcept = default;
      ~void_ptr_storage() = default;
      
      consteval bool is_reference() const noexcept {
        return __builtin_is_within_lifetime(&storage.type_reference);
      }
      
      consteval bool is_pointer() const noexcept {
        return __builtin_is_within_lifetime(&storage.type_value);
      }
      
      consteval bool is_void() const noexcept {
        return __builtin_is_within_lifetime(&storage.void_value);
      }
      
      constexpr void_ptr_storage & operator=(std::convertible_to<Pointer> auto ptr) noexcept {
        storage.type_value = ptr;
        return *this;
      }
      
      constexpr Pointer * convert_to_pointer(void * ptr) const noexcept {
        Pointer * out =  __builtin_addressof(const_cast<Pointer &>(storage.type_value = static_cast<Pointer>(ptr)));
        if (ptr != nullptr) {
          assert(storage.type_value != nullptr);
        }
        return out;
      } 
      
      explicit constexpr operator bool() const noexcept {
        if consteval {
          if (is_reference()) {
            return *storage.type_reference != nullptr;
          } else if (is_pointer()) {
            return storage.type_value != nullptr;
          } else {
            assert(is_void());
            return storage.void_value != nullptr;
          }
        } else {
          if constexpr (AllowReference) {
            return *storage.type_reference != nullptr;
          } else {
            return storage.type_value != nullptr;
          }
        }
      }
      
      constexpr Pointer * get_type_pointer() const {
        if consteval {
          if constexpr (AllowReference) {
            if (is_reference()) {
              return storage.type_reference;
            }
          }
          
          if (is_pointer()) {
            return __builtin_addressof(storage.type_value);
          }
          
          assert(is_void());
          return convert_to_pointer(storage.void_value);
        } else {
          if constexpr (AllowReference) {
            return storage.type_reference;
          } else {
            return __builtin_addressof(storage.type_value);
          }
        }
      }
      
      constexpr void ** convert_to_void(Pointer ptr) const noexcept {
        void ** out = &const_cast<void *&>(storage.void_value = static_cast<void*>(ptr));
        if (ptr != nullptr) {
          assert(storage.void_value != nullptr);
        }
        return out;
      } 
      
      constexpr void ** get_void_pointer() const {
        if consteval {
          // if we don't have void * active, we need to convert it
          if constexpr (AllowReference) {
            if (is_reference()) {
              return convert_to_void(storage.type_value);
            }
          }
          
          if (is_pointer()) {
            return convert_to_void(storage.type_value);
          }
          
          assert(is_void());
          return __builtin_addressof(storage.void_value);
        } else {
          // TODO fix runtime
          if constexpr (AllowReference) {
            return static_cast<void **>(static_cast<void *>(storage.type_reference));
          } else {
            return static_cast<void **>(static_cast<void *>(__builtin_addressof(storage.type_value)));
          }
        }
      }
    };
    
    struct out_tag_t { };
    struct inout_tag_t { };
  }

  /// Smart pointer adaptor for functions taking an output pointer parameter.
  /**
   * @tparam _Smart The type of pointer to adapt.
   * @tparam _Pointer The type of pointer to convert to.
   * @tparam _Args... Argument types used when resetting the smart pointer.
   * @since C++23
   * @headerfile <memory>
   */
  template<typename _Smart, typename _Pointer, typename... _Args>
    class out_ptr_t
    {
#if _GLIBCXX_HOSTED
      static_assert(!__is_shared_ptr<_Smart> || sizeof...(_Args) != 0,
		    "a deleter must be used when adapting std::shared_ptr "
		    "with std::out_ptr");
#endif

    public:
      _GLIBCXX26_CONSTEXPR
      explicit
      out_ptr_t(_Smart& __smart, _Args... __args)
      : _M_impl{__detail::out_tag_t{}, __smart, std::forward<_Args>(__args)...}
      {
	if constexpr (requires { _M_impl._M_out_init(); })
	  _M_impl._M_out_init();
      }

      out_ptr_t(const out_ptr_t&) = delete;

      ~out_ptr_t() = default;

      _GLIBCXX26_CONSTEXPR
      operator _Pointer*() const noexcept
      { return _M_impl._M_get(); }

      _GLIBCXX26_CONSTEXPR
      operator void**() const noexcept requires (!same_as<_Pointer, void*>)
      {
        //static_assert(is_pointer_v<_Pointer>);
        return _M_impl._M_void_get();
      }

    private:
      // TODO: Move this to namespace scope? e.g. __detail::_Ptr_adapt_impl
      template<typename, typename, typename...>
	struct _Impl
	{
	  // This constructor must not modify __s because out_ptr_t and
	  // inout_ptr_t want to do different things. After construction
	  // they call _M_out_init() or _M_inout_init() respectively.
    _GLIBCXX26_CONSTEXPR
	  _Impl(__detail::out_tag_t, _Smart& __s, _Args&&... __args) noexcept
	  : _M_smart(__s), _M_ptr{nullptr}, _M_args(std::forward<_Args>(__args)...)
	  {
	    // _GLIBCXX_RESOLVE_LIB_DEFECTS
	    // 3734. Inconsistency in inout_ptr and out_ptr for empty case
	    if constexpr (requires { _M_smart.reset(); })
	      _M_smart.reset();
	    else
	      _M_smart = _Smart();
	  }
    
    _GLIBCXX26_CONSTEXPR
	  _Impl(__detail::inout_tag_t, _Smart& __s, _Args&&... __args) noexcept
	  : _M_smart(__s), _M_ptr{__s.release()}, _M_args(std::forward<_Args>(__args)...)
    { }

	  // The pointer value returned by operator Pointer*().
    _GLIBCXX26_CONSTEXPR
	  _Pointer*
	  _M_get() const noexcept
	  { return _M_ptr.get_type_pointer(); }
    
    _GLIBCXX26_CONSTEXPR
	  void **
	  _M_void_get() const noexcept
	  { return _M_ptr.get_void_pointer(); }

	  // Finalize the effects on the smart pointer.
    _GLIBCXX26_CONSTEXPR
	  ~_Impl() noexcept(false);

	  _Smart& _M_smart;
	  __detail::void_ptr_storage<_Pointer, false> _M_ptr;
	  [[no_unique_address]] tuple<_Args...> _M_args;
	};

      // Partial specialization for raw pointers, with conversion.
      template<typename _Tp, typename _Ptr> //requires (!is_same_v<_Ptr, _Tp*>)
	struct _Impl<_Tp*, _Ptr>
	{
    _GLIBCXX26_CONSTEXPR
    _Impl(__detail::out_tag_t, _Tp*& __p) noexcept
	  : _M_p(__p), _M_ptr{nullptr}
	  { }
    
    _GLIBCXX26_CONSTEXPR
    _Impl(__detail::inout_tag_t, _Tp*& __p) noexcept
	  : _M_p(__p), _M_ptr{_M_p}
	  { }

    _GLIBCXX26_CONSTEXPR
	  _Pointer*
	  _M_get() const noexcept
	  { return _M_ptr.get_type_pointer(); }
    
    _GLIBCXX26_CONSTEXPR
	  void **
	  _M_void_get() const noexcept
	  { return _M_ptr.get_void_pointer(); }

    _GLIBCXX26_CONSTEXPR
	  ~_Impl() noexcept { 
      // this will overwrite the pointer
      _M_p = *_M_ptr.get_type_pointer();
    }

	  _Tp*& _M_p;
    __detail::void_ptr_storage<_Ptr, false> _M_ptr;
	};
      using _Impl_t = _Impl<_Smart, _Pointer, _Args...>;

      _Impl_t _M_impl;

      template<typename, typename, typename...> friend class inout_ptr_t;
    };

  /// Smart pointer adaptor for functions taking an inout pointer parameter.
  /**
   * @tparam _Smart The type of pointer to adapt.
   * @tparam _Pointer The type of pointer to convert to.
   * @tparam _Args... Argument types used when resetting the smart pointer.
   * @since C++23
   * @headerfile <memory>
   */
  template<typename _Smart, typename _Pointer, typename... _Args>
    class inout_ptr_t
    {
#if _GLIBCXX_HOSTED
      static_assert(!__is_shared_ptr<_Smart>,
		    "std::inout_ptr can not be used to wrap std::shared_ptr");
#endif

    public:
      _GLIBCXX26_CONSTEXPR
      explicit
      inout_ptr_t(_Smart& __smart, _Args... __args)
      : _M_impl{__detail::inout_tag_t{}, __smart, std::forward<_Args>(__args)...} { }

      inout_ptr_t(const inout_ptr_t&) = delete;

      ~inout_ptr_t() = default;

      _GLIBCXX26_CONSTEXPR
      operator _Pointer*() const noexcept
      { return _M_impl._M_get(); }

      _GLIBCXX26_CONSTEXPR
      operator void**() const noexcept requires (!same_as<_Pointer, void*>)
      {
        static_assert(is_pointer_v<_Pointer>);
        return _M_impl._M_void_get();
      }

    private:
#if _GLIBCXX_HOSTED
      // Avoid an invalid instantiation of out_ptr_t<shared_ptr<T>, ...>
      using _Out_ptr_t
	= __conditional_t<__is_shared_ptr<_Smart>,
			  out_ptr_t<void*, void*>,
			  out_ptr_t<_Smart, _Pointer, _Args...>>;
#else
      using _Out_ptr_t = out_ptr_t<_Smart, _Pointer, _Args...>;
#endif
      using _Impl_t = typename _Out_ptr_t::_Impl_t;
      _Impl_t _M_impl;
    };

/// @cond undocumented
namespace __detail
{
  // POINTER_OF metafunction
  template<typename _Tp>
    consteval auto
    __pointer_of()
    {
      if constexpr (requires { typename _Tp::pointer; })
	return type_identity<typename _Tp::pointer>{};
      else if constexpr (requires { typename _Tp::element_type; })
	return type_identity<typename _Tp::element_type*>{};
      else
	{
	  using _Traits = pointer_traits<_Tp>;
	  if constexpr (requires { typename _Traits::element_type; })
	    return type_identity<typename _Traits::element_type*>{};
	}
      // else POINTER_OF(S) is not a valid type, return void.
    }

  // POINTER_OF_OR metafunction
  template<typename _Smart, typename _Ptr>
    consteval auto
    __pointer_of_or()
    {
      using _TypeId = decltype(__detail::__pointer_of<_Smart>());
      if constexpr (is_void_v<_TypeId>)
	return type_identity<_Ptr>{};
      else
	return _TypeId{};
    }

  // Returns Pointer if !is_void_v<Pointer>, otherwise POINTER_OF(Smart).
  template<typename _Ptr, typename _Smart>
    consteval auto
    __choose_ptr()
    {
      if constexpr (!is_void_v<_Ptr>)
	return type_identity<_Ptr>{};
      else
	return __detail::__pointer_of<_Smart>();
    }

  template<typename _Smart, typename _Sp, typename... _Args>
    concept __resettable = requires (_Smart& __s) {
      __s.reset(std::declval<_Sp>(), std::declval<_Args>()...);
    };
}
/// @endcond

  /// Adapt a smart pointer for functions taking an output pointer parameter.
  /**
   * @tparam _Pointer The type of pointer to convert to.
   * @param __s The pointer that should take ownership of the result.
   * @param __args... Arguments to use when resetting the smart pointer.
   * @return A std::inout_ptr_t referring to `__s`.
   * @since C++23
   * @headerfile <memory>
   */
  template<typename _Pointer = void, typename _Smart, typename... _Args>
    _GLIBCXX26_CONSTEXPR
    inline auto
    out_ptr(_Smart& __s, _Args&&... __args)
    {
      using _TypeId = decltype(__detail::__choose_ptr<_Pointer, _Smart>());
      static_assert(!is_void_v<_TypeId>, "first argument to std::out_ptr "
		    "must be a pointer-like type");

      using _Ret = out_ptr_t<_Smart, typename _TypeId::type, _Args&&...>;
      return _Ret(__s, std::forward<_Args>(__args)...);
    }

  /// Adapt a smart pointer for functions taking an inout pointer parameter.
  /**
   * @tparam _Pointer The type of pointer to convert to.
   * @param __s The pointer that should take ownership of the result.
   * @param __args... Arguments to use when resetting the smart pointer.
   * @return A std::inout_ptr_t referring to `__s`.
   * @since C++23
   * @headerfile <memory>
   */
  template<typename _Pointer = void, typename _Smart, typename... _Args>
    _GLIBCXX26_CONSTEXPR
    inline auto
    inout_ptr(_Smart& __s, _Args&&... __args)
    {
      using _TypeId = decltype(__detail::__choose_ptr<_Pointer, _Smart>());
      static_assert(!is_void_v<_TypeId>, "first argument to std::inout_ptr "
		    "must be a pointer-like type");

      using _Ret = inout_ptr_t<_Smart, typename _TypeId::type, _Args&&...>;
      return _Ret(__s, std::forward<_Args>(__args)...);
    }

  /// @cond undocumented
  template<typename _Smart, typename _Pointer, typename... _Args>
  template<typename _Smart2, typename _Pointer2, typename... _Args2>
    _GLIBCXX26_CONSTEXPR
    inline
    out_ptr_t<_Smart, _Pointer, _Args...>::
    _Impl<_Smart2, _Pointer2, _Args2...>::~_Impl() noexcept(false)
    {
      using _TypeId = decltype(__detail::__pointer_of_or<_Smart, _Pointer>());
      using _Sp = typename _TypeId::type;

      if (!_M_ptr)
	return;

      _Smart& __s = _M_smart;
      _Pointer& __p = *_M_ptr.get_type_pointer();

      auto __reset = [&](auto&&... __args) {
	if constexpr (__detail::__resettable<_Smart, _Sp, _Args...>)
	  __s.reset(static_cast<_Sp>(__p), std::forward<_Args>(__args)...);
	else if constexpr (is_constructible_v<_Smart, _Sp, _Args...>)
	  __s = _Smart(static_cast<_Sp>(__p), std::forward<_Args>(__args)...);
	else
	  static_assert(is_constructible_v<_Smart, _Sp, _Args...>);
      };

      if constexpr (sizeof...(_Args) >= 2)
	std::apply(__reset, std::move(_M_args));
      else if constexpr (sizeof...(_Args) == 1)
	__reset(std::get<0>(std::move(_M_args)));
      else
	__reset();
    }
  /// @endcond

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace

#endif // __glibcxx_out_ptr
#endif /* _GLIBCXX_OUT_PTR_H */