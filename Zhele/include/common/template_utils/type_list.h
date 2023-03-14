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

#include <type_traits>
#include <utility>

namespace Zhele::TemplateUtils
{
    /**
     * @brief Type box
     * 
     * @tparam T Type
    */
    template<typename T>
    using TypeBox = std::type_identity<T>;

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
            return []<typename T, typename... Us>(TypeList<T, Us...>){
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
        template<unsigned index>
        static consteval auto get()
        {
            static_assert(index < size());

            if constexpr (index == 0)
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
        static consteval int search();

        /**
         * @brief Method \ref search with boxed parameter
        */
        template<typename T>
        static consteval int search(TypeBox<T>)
        {
            return search<T>();   
        }

        /**
         * ??
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
        
        static consteval auto remove_duplicates()
        {
            //return std::conditional_t<tail().contains(head()), TypeList<>, TypeList<TypeUnbox<head()>>>{} + tail().remove_duplicates();
            return TypeList<Ts...>{};
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
        static inline void foreach(auto func)
        {
            (func(TypeBox<Ts> {}), ...);
        }

        /**
         * ??
        */
        template<auto I, typename T>
        static consteval auto generate()
        {
            return []<auto... Is>(std::index_sequence<Is...>) {
                return TypeList <TypeUnbox<(Is, TypeBox<T>{})>...> {};
            } (std::make_index_sequence<I>());
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
    };

    template<typename... Ts>
    template<typename T>
    consteval int TypeList<Ts...>::search()
    {        
        return std::is_same_v<TypeUnbox<head()>, T>
            ? 0
            : (tail().template search<T>() == -1
                ? -1
                : tail().template search<T>() + 1
            );
    }

    template<>
    template<typename T>
    consteval int TypeList<>::search()
    {        
        return -1;
    }

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
}

#endif //! ZHELE_TYPELIST_H