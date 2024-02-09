/**
 * @file
 * Implement types list for C++20
 * 
 * @author Alexey Zhelonkin (based on VladislavS code)
 * @date 2023
 * @license FreeBSD
 */
#ifndef ZHELE_TYPELIST_H
#define ZHELE_TYPELIST_H

#include <concepts>
#include <type_traits>
#include <utility>

namespace Zhele::TemplateUtils
{
    struct DummyTypeBoxBase {};

    template <typename T, typename Enabler = void>
    constexpr bool is_complete_v = false;

    template <typename T>
    constexpr bool is_complete_v<T, std::void_t<decltype(sizeof(T) != 0)>> = true;

    template<typename T>
    struct TypeBox : public std::conditional_t<std::is_class_v<T> && is_complete_v<T>, T, DummyTypeBoxBase>
    {
        using type = T;
    };

    template<typename T>
    bool operator == (TypeBox<T>, TypeBox<T>) { return true; }
    template<typename T, typename U>
    bool operator == (TypeBox<T>, TypeBox<U>) { return false; }

    /**
     * @brief Type unbox
     * 
     * @tparam Value Type box
    */
    template<auto Value>
    using TypeUnbox = typename decltype(Value)::type;

    /**
     * @brief Value box
     * 
     * @tparam Value Value
    */
    template <auto Value>
    struct ValueBox
    {
        static constexpr auto value = Value;
    };

    /**
     * @brief Unbox value
     * 
     * @tparam T Value box
     * 
     * @returns Unboxed value
    */
    template<typename T>
    constexpr auto value_unbox() { return T::type::value; }

    /**
     * @brief Unbox value
     * 
     * @param t Value box
     * 
     * @returns Unboxed value
    */
    constexpr auto value_unbox(auto t) { return decltype(t)::type::value; }

    /**
     * @brief Implements type list
    */
    template<typename... Ts>
    class TypeList
    {
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
        static consteval auto head()
        {
            return [] <typename T, typename... Us> (TypeList<T, Us...>) {
                return TypeBox<T>{};
            } (TypeList<Ts...>{});
        }

        /**
         * @brief Returns tail of typelist
         * 
         * @returns Typelist with tail
        */
        static consteval auto tail()
        {
            return []<typename T, typename... Us>(TypeList<T, Us...>) { return TypeList<Us...>{}; }(TypeList<Ts...>{});
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
        static consteval auto get()
        {
            static_assert(index < static_cast<int>(size()));

            if constexpr (index < 0)
                return TypeBox<void>();
            else if constexpr (index == 0)
                return head();
            else
                return tail().template get<index - 1>();
        }

        /**
         * @brief Search type in typelist
         * 
         * @tparam T Required type
         * 
         * @returns Type index or -1 if type does not present in typelist
        */
        template<typename T>
        static consteval int search()
        {
            if constexpr (is_empty())
                return -1;
            else if constexpr (std::is_same_v<TypeUnbox<head()>, T>)
                return 0;
            else
                return tail().template search<T>() == -1
                    ? -1
                    : tail().template search<T>() + 1;
        }

        /**
         * @brief Method \ref search with boxed parameter
        */
        template<typename T>
        static consteval int search(TypeBox<T>)
        {
            return search<T>();   
        }

        /**
         * @brief Returns last type in typelist
         * 
         * @returns Boxed last type
        */
        static consteval auto back() { return (TypeBox<Ts>{}, ...); }

        /**
         * @brief Insert type to front of typelist
         * 
         * @tparam T Type for insert
         * 
         * @returns Typebox with new typelist
        */
        template<typename T>
        static consteval auto push_front() { return TypeList<T, Ts...>{}; }

        /**
         * @brief Method \ref push_front with boxed parameter
        */
        template<typename T>
        static consteval auto push_front(TypeBox<T>) { return push_front<T>(); }

        /**
         * @brief Insert type to back of typelist
         * 
         * @tparam T Type for insert
         * 
         * @returns Typebox with new typelist
        */
        template<typename T>
        static consteval auto push_back()  { return TypeList<Ts..., T>{}; }

        /**
         * @brief Method \ref push_back with boxed parameter
        */
        template<typename T>
        static consteval auto push_back(TypeBox<T>) { return push_back<T>(); }

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
        static consteval bool contains(TypeBox<T>) { return contains<T>(); }

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
        static consteval bool is_unique()
        {
            return is_unique([](auto v1, auto v2) { return std::is_same_v<TypeUnbox<v1>, TypeUnbox<v2>>; });
        }

        /**
         * @brief Returns typelist without duplicates
         * 
         * @returns Typelist without duplicates
        */
        static consteval auto remove_duplicates()
        {
            if constexpr (is_empty())
                return TypeList<>{};
            else if constexpr (size() == 1)
                return TypeList<Ts...>{};
            else if constexpr (tail().contains(head()))
                return TypeList<>{} + tail().remove_duplicates();
            else
                return TypeList<TypeUnbox<head()>>{} + tail().remove_duplicates();
        }

        /**
         * @brief Sort typelist by given predicate
         * 
         * @returns Sorted typelist
        */
        static consteval auto sort(auto pred)
        {
            return insertion_sort<0, 1>(pred);
        }

        /**
         * @brief Accumulate types in typelist by given function
         * 
         * @param func Function-processor for types
         * 
         * @returns New typelist
        */
        static consteval auto accumulate(auto func)
        {
            return (TypeList<>{} + ... + func(TypeBox<Ts>{}));
        }

        /**
         * @brief Apply function for each type in typelist
         * 
         * @param func Function
         * 
         * @par Returns
         *  Nothing
        */
        static constexpr void foreach(auto func)
        {
            (func(TypeBox<Ts> {}), ...);
        }

        /**
         * @brief Transform each type in typelist with given function
         * 
         * @param func Type transformer
         * 
         * @returns New typelist
        */
        static consteval auto transform(auto func) { return TypeList<TypeUnbox<func(TypeBox<Ts>{})>...>{}; }

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
        static consteval auto filter(auto pred)
        {
            return (TypeList<>{}+ ... +std::conditional_t<pred(TypeBox<T>{}, TypeBox<Ts>{}),TypeList<Ts>,TypeList<>>{});
        }

        /**
         * @brief Method \ref filter with boxed parameter
        */
        template<typename T>
        static consteval auto filter(TypeBox<T> box, auto pred) { return filter<T>(pred); }

        /**
         * @brief Filter types by given unary predicate
         * 
         * @param pred Unary predicate
         * 
         * @returns New typelist
        */
        static consteval auto filter(auto pred)
        {
            return (TypeList<>{} + ... + std::conditional_t<pred(TypeBox<Ts>{}), TypeList<Ts>, TypeList<>>{});
        }

        template<typename... Us>
        friend class TypeList;
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
        static consteval bool is_unique_(auto func)
        {
            return !(func(TypeBox<T>{}, TypeBox<Us>{}) || ...) && is_unique_<Us...>(func);
        }

        template<auto i, auto j>
        static consteval auto insertion_sort(auto pred)
        {
            if constexpr (i == size()) {
                return TypeList<Ts...>{};
            } else if constexpr (j == size()) {
                return insertion_sort<i + 1, i + 2>(pred);
            } else if constexpr (!pred(get<i>(), get<j>())) {
                return swap<i, j>().template insertion_sort<i, j + 1>(pred);
            } else {
                return insertion_sort<i, j + 1>(pred);
            }
        }

        template<auto i, auto j>
        static consteval auto swap()
        {
            return []<typename... Us, unsigned... Indexes>(TypeList<Ts...>, std::index_sequence<Indexes...>){
                return TypeList<TypeUnbox<get<Indexes != i && Indexes != j ? Indexes : Indexes == i ? j : i>()> ...>{};
            }(TypeList<Ts...>{}, std::make_index_sequence<size()>());
        }
    };

    template<typename... Ts, typename... Us>
    consteval bool operator == (TypeList<Ts...>, TypeList<Us...>) { return false; }

    template<typename... Ts>
    consteval bool operator == (TypeList<Ts...>, TypeList<Ts...>) { return true; }

    template<typename... Ts, typename... Us>
    consteval bool operator != (TypeList<Ts...>, TypeList<Us...>) { return true; }

    template<typename... Ts>
    consteval bool operator != (TypeList<Ts...>, TypeList<Ts...>) { return false; }

    template<typename... Ts, typename... Us>
    consteval auto operator + (TypeList<Ts...>, TypeList<Us...>) { return TypeList<Ts..., Us...> {}; }

    template<typename... Ts, typename... Us>
    consteval auto operator - (TypeList<Ts...> first, TypeList<Us...> second)
    {
        return first.filter([second](auto type) { return !second.contains(type); });
    }
}

#endif //! ZHELE_TYPELIST_H