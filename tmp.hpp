//=======================================================================
// Copyright (c) 2013-2014 Baptiste Wicht.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPP_UTILS_TMP_HPP
#define CPP_UTILS_TMP_HPP

#include <tuple>
#include <utility>

#define HAS_MEM_FUNC(func, name)                                              \
template<typename T, typename Sign>                                           \
struct name {                                                                 \
    typedef char yes[1];                                                      \
    typedef char no [2];                                                      \
    template <typename U, U> struct type_check;                               \
    template <typename _1> static yes &chk(type_check<Sign, &_1::func> *);    \
    template <typename   > static no  &chk(...);                              \
    static bool constexpr const value = sizeof(chk<T>(0)) == sizeof(yes);     \
}

#define HAS_STATIC_FIELD(field, name)                                                       \
template <class T>                                                                          \
class name {                                                                                \
    template<typename U, typename =                                                         \
        typename std::enable_if<!std::is_member_pointer<decltype(&U::field)>::value>::type> \
    static std::true_type check(int);                                                       \
    template <typename>                                                                     \
    static std::false_type check(...);                                                      \
public:                                                                                     \
    static constexpr const bool value = decltype(check<T>(0))::value;                       \
};

namespace cpp {

template<std::size_t I, typename Tuple, typename Functor>
struct for_each_tuple_t_impl {
    static void for_each(Functor&& func){
        std::forward<Functor>(func).template operator()<typename std::tuple_element<I, Tuple>::type>();
        for_each_tuple_t_impl<I - 1, Tuple, Functor>::for_each(std::forward<Functor>(func));
    }
};

template<typename Tuple, typename Functor>
struct for_each_tuple_t_impl<0, Tuple, Functor> {
    static void for_each(Functor&& func){
        std::forward<Functor>(func).template operator()<typename std::tuple_element<0, Tuple>::type>();
    }
};

template<typename Tuple, typename Functor>
void for_each_tuple_t(Functor&& func){
    for_each_tuple_t_impl<std::tuple_size<Tuple>::value - 1, Tuple, Functor>::for_each(std::forward<Functor>(func));
}

template<bool B>
struct not_u : std::integral_constant<bool, !B> {};

template<typename C>
struct not_c : std::integral_constant<bool, !C::value> {};

template<bool H, bool... T>
struct and_u : std::integral_constant<bool, H && and_u<T...>::value> {};

template<bool H>
struct and_u<H> : std::integral_constant<bool, H> {};

template<typename... C>
using and_c = and_u<C::value...>;

template<bool H, bool... T>
struct or_u : std::integral_constant<bool, H || or_u<T...>::value> {};

template<bool H>
struct or_u<H> : std::integral_constant<bool, H> {};

template<typename... C>
using or_c = or_u<C::value...>;

//enable_if utilities

template<bool B, typename T = void>
using disable_if = std::enable_if<!B, T>;

template<bool B, typename T = void>
using disable_if_t = typename std::enable_if<!B, T>::type;

namespace detail {

//Note: Unfortunately, CLang is bugged (Bug 11723), therefore, it is not
//possible to use universal enable_if/disable_if directly, it is necessary to
//use the dummy :( FU Clang!

enum class enabler_t { DUMMY };
constexpr const enabler_t dummy = enabler_t::DUMMY;

} //end of detail

//Note: For error reporting reasons, it is not a good idea to define these next
//utilities using each other (for instance enable_if_c using enable_if_u)

template<bool B>
using enable_if_u = std::enable_if_t<B, detail::enabler_t>;

template<bool B>
using disable_if_u = std::enable_if_t<not_u<B>::value, detail::enabler_t>;

template<typename C>
using enable_if_c = std::enable_if_t<C::value, detail::enabler_t>;

template<typename C>
using disable_if_c = std::enable_if_t<not_c<C>::value, detail::enabler_t>;

template<bool... B>
using enable_if_all_u = std::enable_if_t<and_u<B...>::value, detail::enabler_t>;

template<bool... B>
using disable_if_all_u = std::enable_if_t<not_c<and_u<B...>>::value, detail::enabler_t>;

template<typename... C>
using enable_if_all_c = std::enable_if_t<and_c<C...>::value, detail::enabler_t>;

template<typename... C>
using disable_if_all_c = std::enable_if_t<not_c<and_c<C...>>::value, detail::enabler_t>;

template<bool... B>
using enable_if_one_u = std::enable_if_t<or_u<B...>::value, detail::enabler_t>;

template<bool... B>
using disable_if_one_u = std::enable_if_t<not_c<or_u<B...>>::value, detail::enabler_t>;

template<typename... C>
using enable_if_one_c = std::enable_if_t<or_c<C...>::value, detail::enabler_t>;

template<typename... C>
using disable_if_one_c = std::enable_if_t<not_c<or_c<C...>>::value, detail::enabler_t>;

//Type traits simplifications

template<typename T>
using add_const_lvalue_t = std::add_lvalue_reference_t<std::add_const_t<T>>;

template<typename T>
using add_const_rvalue_t = std::add_rvalue_reference_t<std::add_const_t<T>>;

//Other TMP Utilities

template<template<typename...> class TT, typename T>
struct is_specialization_of : std::false_type {};

template<template<typename...> class TT, typename... Args>
struct is_specialization_of<TT, TT<Args...>> : std::true_type {};

//Variadic manipulations utilities

template<std::size_t N, typename... T>
struct nth_type {
    using type = typename std::tuple_element<N, std::tuple<T...>>::type;
};

template<typename... T>
struct first_type {
    using type = typename nth_type<0, T...>::type;
};

template<typename... T>
struct last_type {
    using type = typename nth_type<sizeof...(T)-1, T...>::type;
};

//Some helpers

template<std::size_t N, typename... T>
using nth_type_t = typename nth_type<N, T...>::type;

template<std::size_t N, typename... T>
using first_type_t = typename first_type<N, T...>::type;

template<std::size_t N, typename... T>
using last_type_t = typename last_type<N, T...>::type;

template<int I, typename T1, typename... T, enable_if_u<(I == 0)> = detail::dummy>
auto nth_value(T1&& t, T&&... /*args*/) -> decltype(std::forward<T1>(t)) {
    return std::forward<T1>(t);
}

template<int I, typename T1, typename... T, enable_if_u<(I > 0)> = detail::dummy>
auto nth_value(T1&& /*t*/, T&&... args)
        -> decltype(std::forward<nth_type_t<I, T1, T...>>(std::declval<nth_type_t<I, T1, T...>>())){
    return std::forward<nth_type_t<I, T1, T...>>(nth_value<I - 1>((std::forward<T>(args))...));
}

template<typename... T>
auto last_value(T&&... args){
    return std::forward<last_type_t<T...>>(nth_value<sizeof...(T) - 1>(args...));
}

template<typename... T>
auto first_value(T&&... args){
    return std::forward<first_type_t<T...>>(nth_value<0>(args...));
}

template<typename V, typename F, typename... S>
struct all_convertible_to : std::integral_constant<bool, and_c<all_convertible_to<V, F>, all_convertible_to<V, S...>>::value> {};

template<typename V, typename F>
struct all_convertible_to<V, F> : std::integral_constant<bool, std::is_convertible<F, V>::value> {};

template<std::size_t I, std::size_t S, typename F, typename... T>
struct is_homogeneous_helper {
    template<std::size_t I1, std::size_t S1, typename Enable = void>
    struct helper_int : std::integral_constant<bool, and_c<std::is_same<F, nth_type_t<I1, T...>>, is_homogeneous_helper<I1+1, S1, F, T...>>::value> {};

    template<std::size_t I1, std::size_t S1>
    struct helper_int<I1, S1, std::enable_if_t<I1 == S1>> : std::integral_constant<bool, std::is_same<F, nth_type_t<I1, T...>>::value> {};

    static constexpr const auto value = helper_int<I, S>::value;
};

template<typename F, typename... T>
struct is_homogeneous : std::integral_constant<bool, is_homogeneous_helper<0, sizeof...(T)-1, F, T...>::value> {};

template<typename... T>
struct is_sub_homogeneous;

template<>
struct is_sub_homogeneous<> : std::false_type {};

template<typename T>
struct is_sub_homogeneous<T> : std::false_type {};

template<typename T1, typename T2>
struct is_sub_homogeneous<T1, T2> : std::integral_constant<bool, not_c<std::is_same<T1, T2>>::value> {};

template<typename T1, typename T2, typename T3, typename... T>
struct is_sub_homogeneous<T1, T2, T3, T...> : std::integral_constant<bool,
    and_c<
        std::is_same<T1, T2>,
        is_sub_homogeneous<T2, T3, T...>
    >::value> {};

template<typename F, std::size_t I1, std::size_t... I, typename... T, enable_if_u<(sizeof...(I) == 0)> = detail::dummy>
void for_each_in_subset(F&& f, const std::index_sequence<I1, I...>& /*i*/, T&&... args){
    f(std::forward<nth_type_t<I1, T...>>(nth_value<I1>(args...)));
}

template<typename F, std::size_t I1, std::size_t... I, typename... T, enable_if_u<(sizeof...(I) > 0)> = detail::dummy>
void for_each_in_subset(F&& f, const std::index_sequence<I1, I...>& /*i*/, T&&... args){
    f(std::forward<nth_type_t<I1, T...>>(nth_value<I1>(args...)));
    for_each_in_subset(f, std::index_sequence<I...>(), std::forward<T>(args)...);
}

template<typename F, typename... T>
void for_each_in(F&& f, T&&... args){
    for_each_in_subset(f, std::make_index_sequence<sizeof...(T)>(), std::forward<T>(args)...);
}

} //end of namespace cpp

#endif //CPP_UTILS_TMP_HPP
