/**
 * @file
 * Implement types list for C++20
 * 
 * @author Alexey Zhelonkin (based on VladislavS code)
 * @date 2023
 * @license MIT
 */

#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace Zhele::template_utils {
  /**
   * @brief Dummy type box base for incomplete types
   */
  struct dummy_type_box_base {};

  /**
   * @brief Check if type is complete
   * 
   * @tparam T Type
   * 
   * @retval true Type is complete
   * @retval false Type is not complete
   */
  template<typename T, typename = void>
  struct is_complete : std::false_type {};

  template<typename T>
  struct is_complete<T, std::void_t<decltype(sizeof(T))>> : std::true_type {};

  template<typename T>
  inline constexpr bool is_complete_v = is_complete<T>::value;


  template<typename T>
  struct type_box : public std::conditional_t<std::is_class_v<T> && is_complete_v<T>, T, dummy_type_box_base> {
    using type = T;
  };
  template<typename T>
  consteval bool operator == (type_box<T>, type_box<T>) { return true; }
  template<typename T, typename U>
  consteval bool operator == (type_box<T>, type_box<U>) { return false; }

  /**
   * @brief Type unbox
   * 
   * @tparam Boxed type
  */
  template<auto Value>
  using type_unbox = typename decltype(Value)::type;

  /**
   * @brief Implements type list
  */
  template<typename... Ts>
  class type_list {
  public:
    /**
     * @brief Returns typelist size
     * 
     * @returns Typelist size (length)
    */
    static consteval auto size() { return sizeof...(Ts); }

    /**
     * @brief Check typelist is empty
     * 
     * @retval true Typelist is empty
     * @retval false Typelist is not empty
    */
    static consteval bool is_empty() { return (sizeof...(Ts) == 0); }

    /**
     * @brief Returns head of typelist
     * 
     * @returns Boxed head type
    */
    static consteval auto head() {
#if defined(__cpp_pack_indexing) && __cpp_pack_indexing >= 202311L
      return type_box<Ts...[0]>{};
#else
      return []<typename T, typename... Us>(type_list<T, Us...>) { return type_box<T>{}; }(type_list<Ts...>{});
#endif
    }

    /**
     * @brief Returns tail of typelist
     *
     * @returns Typelist with tail
    */
    static consteval auto tail() {
      return []<typename T, typename... Us>(type_list<T, Us...>) { return type_list<Us...>{}; }(type_list<Ts...>{});
    }

    /**
     * @brief Returns type by index
     * 
     * @todo Replace with static operator[] in c++23
     * 
     * @par index Type index
     * 
     * @returns Boxed required type
    */
    template<int index>
    static consteval auto get() {
      static_assert(index < static_cast<int>(size()));

      if constexpr (index < 0)
        return type_box<void>{};
#if defined(__cpp_pack_indexing) && __cpp_pack_indexing >= 202311L
      else
        return type_box<Ts...[static_cast<std::size_t>(index)]>{};
#else
      else if constexpr (index == 0)
        return head();
      else
        return tail().template get<index - 1>();
#endif
    }

    /**
     * @brief Search type in typelist
     * 
     * @tparam T Required type
     * 
     * @returns Type index or -1 if type does not present in typelist
    */
    template<typename T>
    static consteval int search() {
      if constexpr (is_empty()) {
        return -1;
      } else if constexpr (std::is_same_v<type_unbox<head()>, T>) {
        return 0;
      } else {
        return tail().template search<T>() == -1
          ? -1
          : tail().template search<T>() + 1;
      }
    }

    /**
     * @brief Method \ref search with boxed parameter
    */
    template<typename T>
    static consteval int search(type_box<T>) {
      return search<T>();   
    }

    /**
     * @brief Returns last type in typelist
     *
     * @returns Boxed last type
    */
    static consteval auto back() { return (type_box<Ts>{}, ...); }

    /**
     * @brief Insert type to front of typelist
     * 
     * @tparam T Type for insert
     * 
     * @returns Typebox with new typelist
    */
    template<typename T>
    static consteval auto push_front() { return type_list<T, Ts...>{}; }

    /**
     * @brief Method \ref push_front with boxed parameter
    */
    template<typename T>
    static consteval auto push_front(type_box<T>) { return push_front<T>(); }

    /**
     * @brief Insert type to back of typelist
     * 
     * @tparam T Type for insert
     * 
     * @returns Typebox with new typelist
    */
    template<typename T>
    static consteval auto push_back()  { return type_list<Ts..., T>{}; }

    /**
     * @brief Method \ref push_back with boxed parameter
    */
    template<typename T>
    static consteval auto push_back(type_box<T>) { return push_back<T>(); }

    /**
     * @brief Check that typelis contains given type
     * 
     * @tparam T Required type
     * 
     * @retval true Typelist contains given type
     * @retval false Typelist not contains given type
    */
    template<typename T>
    static consteval bool contains() { return (... || std::is_same_v<T, Ts>); }

    /**
     * @brief Method \ref contains with boxed parameter
    */
    template<typename T>
    static consteval bool contains(type_box<T>) { return contains<T>(); }

    /**
     * @brief Check that typelist is unique by given comparator
     * 
     * @retval true Typelist is unique
     * @retval false Typelist does not unique
    */
    static consteval bool is_unique(auto func) { return is_unique_<Ts...>(func); }

    /**
     * @brief Check that typelist does not have duplicates
     * 
     * @retval true Typelist does not have duplicates
     * @retval false Typelist has duplicates
    */
    static consteval bool is_unique() {
      return is_unique([](auto v1, auto v2) { return std::is_same_v<type_unbox<v1>, type_unbox<v2>>; });
    }

    /**
     * @brief Returns typelist without duplicates
     * 
     * @returns Typelist without duplicates
    */
    static consteval auto remove_duplicates() {
      if constexpr (is_empty())
        return type_list<>{};
      else if constexpr (size() == 1)
        return type_list<Ts...>{};
      else if constexpr (tail().contains(head()))
        return type_list<>{} + tail().remove_duplicates();
      else
        return type_list<type_unbox<head()>>{} + tail().remove_duplicates();
    }

    /**
     * @brief Returns typelist without duplicates
     * 
     * @returns Typelist without duplicates
    */
    static consteval auto unique() {
      return remove_duplicates();
    }

    /**
     * @brief Sort typelist by given predicate
     * 
     * @returns Sorted typelist
    */
    static consteval auto sort(auto pred) {
      return insertion_sort<0, 1>(pred);
    }

    /**
     * @brief Accumulate types in typelist by given function
     * 
     * @param func Function-processor for types
     * 
     * @returns New typelist
    */
    static consteval auto accumulate(auto func) {
      return (type_list<>{} + ... + func(type_box<Ts>{}));
    }

    /**
     * @brief Apply a callback for each type in the list (empty list — no call)
     */
    static constexpr void foreach(auto&&)
    requires (sizeof...(Ts) == 0)
    {}

    /**
     * @brief Apply a callback for each type in the list
     * 
     * @param fn Callback with template operator()<T>
     * 
     * @returns Nothing
     */
    static constexpr void foreach(auto&& fn)
    requires (sizeof...(Ts) > 0) && (... && requires { std::declval<std::remove_cvref_t<decltype(fn)>&>().template operator()<Ts>(); }) {
      (std::forward<decltype(fn)>(fn).template operator()<Ts>(), ...);
    }

    /**
     * @brief Apply a callback for each type in the list
     * 
     * @param fn Callback with type_box<T> parameter
     * 
     * @returns Nothing
     */
    static constexpr void foreach(auto&& fn)
    requires (sizeof...(Ts) > 0) && (... && std::invocable<decltype(fn), type_box<Ts>>) {
      (std::forward<decltype(fn)>(fn)(type_box<Ts>{}), ...);
    }

    /**
     * @brief Apply function for each type in typelist while function not returns true
     * 
     * @param func Function
     * 
     * @retval true Func returns true on one type
     * @retval false Func not returns true
    */
    static constexpr bool any(auto func) {
      return (std::forward<decltype(func)>(func)(type_box<Ts> {}) || ...);
    }

    /**
     * @brief Transform each type in typelist with given function
     * 
     * @param func Type transformer
     * 
     * @returns New typelist
    */
    static consteval auto transform(auto func) { return type_list<type_unbox<func(type_box<Ts>{})>...>{}; }

    /**
     * @brief Filter types by given binary predicate
     * 
     * @tparam First type argument for predicate
     * 
     * @param pred Predicate
     * 
     * @returns New typelist
    */
    template<typename T>
    static consteval auto filter(auto pred) {
      return (type_list<>{}+ ... +std::conditional_t<pred(type_box<T>{}, type_box<Ts>{}),type_list<Ts>,type_list<>>{});
    }

    /**
     * @brief Method \ref filter with boxed parameter
    */
    template<typename T>
    static consteval auto filter(type_box<T> box, auto pred) { return filter<T>(pred); }

    /**
     * @brief Filter types by given unary predicate
     * 
     * @param pred Unary predicate
     * 
     * @returns New typelist
    */
    static consteval auto filter(auto pred) {
      return (type_list<>{} + ... + std::conditional_t<pred(type_box<Ts>{}), type_list<Ts>, type_list<>>{});
    }

    template<typename... Us>
    friend class type_list;
  private:
    /**
     * @brief Dummy is_unique_ for empty list
     * 
     * @par unused
     * 
     * @returns Always true
    */
    static consteval bool is_unique_(auto) { return true; }

    /**
     * @brief Check typelist is unique by given predicate
     * 
     * @tparam T Head
     * @tparam Us Tail
     * 
     * @param func Binary predicate
     * 
     * @retval true Typelist is unique by predicate
     * @retval false Typelist does not unique by predicate
    */
    template <typename T, typename... Us>
    static consteval bool is_unique_(auto func) {
      return !(func(type_box<T>{}, type_box<Us>{}) || ...) && is_unique_<Us...>(func);
    }

    template<auto i, auto j>
    static consteval auto insertion_sort(auto pred) {
      if constexpr (i == size()) {
        return type_list<Ts...>{};
      } else if constexpr (j == size()) {
        return insertion_sort<i + 1, i + 2>(pred);
      } else if constexpr (!pred(get<i>(), get<j>())) {
        return swap<i, j>().template insertion_sort<i, j + 1>(pred);
      } else {
        return insertion_sort<i, j + 1>(pred);
      }
    }

    template<auto i, auto j>
    static consteval auto swap() {
      return []<typename... Us, unsigned... Indexes>(type_list<Ts...>, std::index_sequence<Indexes...>){
        return type_list<type_unbox<get<Indexes != i && Indexes != j ? Indexes : Indexes == i ? j : i>()> ...>{};
      }(type_list<Ts...>{}, std::make_index_sequence<size()>());
    }
  };

  template<typename... Ts, typename... Us>
  consteval bool operator == (type_list<Ts...>, type_list<Us...>) { return false; }

  template<typename... Ts>
  consteval bool operator == (type_list<Ts...>, type_list<Ts...>) { return true; }

  template<typename... Ts, typename... Us>
  consteval bool operator != (type_list<Ts...>, type_list<Us...>) { return true; }

  template<typename... Ts>
  consteval bool operator != (type_list<Ts...>, type_list<Ts...>) { return false; }

  template<typename... Ts, typename... Us>
  consteval auto operator + (type_list<Ts...>, type_list<Us...>) { return type_list<Ts..., Us...> {}; }

  template<typename... Ts, typename... Us>
  consteval auto operator - (type_list<Ts...> first, type_list<Us...> second) {
    return first.filter([second](auto type) { return !second.contains(type); });
  }
}

