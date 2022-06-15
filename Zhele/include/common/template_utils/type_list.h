/**
 * @file
 * Implement types list for C++17
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#ifndef ZHELE_TYPELIST_H
#define ZHELE_TYPELIST_H

#include <type_traits>
#include <utility>

namespace Zhele
{
    namespace TemplateUtils
    {
        /**
         * @brief Container for types
         */
        template<typename... Types>
        class TypeList
        {
        };

        /**
         * @brief Type type length
         */
        template<typename...>
        class Length {};

        template<typename... Types>
        class Length<TypeList<Types...>>
        {
        public:
            static const unsigned value = sizeof...(Types);
        };

        /**
         * @brief Search type in typelist
         */
        template<typename, typename...>
        class TypeIndex {};

        template<typename Search>
        class TypeIndex<Search, TypeList<>>
        {
        public:
            static const int value = -1;
        };

        template<typename Search, typename... Tail>
        class TypeIndex<Search, TypeList<Search, Tail...>>
        {
        public:
            static const int value = 0;
        };

        template<typename Search, typename Head, typename... Tail>
        class TypeIndex<Search, TypeList<Head, Tail...>>
        {
        public:
            static const int value = TypeIndex<Search, TypeList<Tail...>>::value >= 0
                ? TypeIndex<Search, TypeList<Tail...>>::value + 1
                : TypeIndex<Search, TypeList<Tail...>>::value;
        };

        /**
         * @brief Select type from type list by index
         */
        template<int, typename...>
        class GetType {};

        template<typename Head, typename... Tail>
        class GetType<-1, TypeList<Head, Tail...>>
        {
        public:
            using type = void;
        };

        template<typename Head, typename... Tail>
        class GetType<0, TypeList<Head, Tail...>>
        {
        public:
            using type = Head;
        };

        template<int Index, typename Head, typename... Tail>
        class GetType<Index, TypeList<Head , Tail...>>
        {
            static_assert(Index < Length<TypeList<Head, Tail...>>::value);
        public:
            using type = typename GetType<Index - 1, TypeList<Tail...>>::type;
        };

        template<int Index, typename Types>
        using GetType_t = typename GetType<Index, Types>::type;

        /**
         * @brief Insert type to front of type
         */
        template <typename...>
        class InsertFront{};

        template <typename New, typename ...Types>
        class InsertFront<New, TypeList<Types...>>
        {
        public:
            using type = TypeList<New, Types...>;
        };

        /**
         * @brief Insert type to back of type
         */
        template <typename ...Types>
        class InsertBack;

        template <typename New, typename ...Types>
        class InsertBack<TypeList<Types...>, New>
        {
        public:
            using type = TypeList<Types..., New>;
        };

        /**
         * @brief Delete first type occurence from list
         */
        template <typename TypeToDelete, typename TypeList>
        class DeleteFirst
        {
        public:
            using type = TypeList;
        };

        template <typename TypeToDelete>
        class DeleteFirst<TypeToDelete, TypeList<>>
        {
        public:
            using type = TypeList<>;
        };

        template <typename TypeToDelete, typename... Tail>
        class DeleteFirst<TypeToDelete, TypeList<TypeToDelete, Tail...>>
        {
        public:
            using type = TypeList<Tail...>;
        };

        template <typename TypeToDelete, typename Head, typename... Tail>
        class DeleteFirst<TypeToDelete, TypeList<Head, Tail...>>
        {
        public:
            using type = typename InsertFront<Head, typename DeleteFirst<TypeToDelete, TypeList<Tail...>>::type>::type;
        };

        /**
         * @brief Delete all type occurence from list
         */
        template <typename TypeToDelete, typename TypeList>
        class DeleteAll : public DeleteFirst<TypeToDelete, TypeList> {};

        template <typename TypeToDelete, typename ...Tail>
        class DeleteAll<TypeToDelete, TypeList<TypeToDelete, Tail...>>
        {
        public:
            using type = typename DeleteAll<TypeToDelete, TypeList<Tail...>>::type;
        };

        template <typename TypeToDelete, typename Head, typename ...Tail>
        class DeleteAll<TypeToDelete, TypeList<Head, Tail...>>
        {
        public:
            using type = typename InsertFront<Head, typename DeleteAll<TypeToDelete, TypeList<Tail...>>::type>::type;
        };
        /**
         * @brief Remove duplicates
         */
        template <typename>
        class Unique;

        template <>
        class Unique<TypeList<>>
        {
        public:
            using type = TypeList<>;
        };

        template <typename Head, typename... Tail>
        class Unique<TypeList<Head, Tail...>>
        {
            using TailWithoutDuplicates = typename Unique<TypeList<Tail...>>::type;
            using TailWithoutHead = typename DeleteFirst<Head, TailWithoutDuplicates>::type;
        public:
            using type = typename InsertFront<Head, TailWithoutHead>::type;
        };

        /**
         * @brief Appends typelists
         */
        template<typename...>
        class Append{};

        template<>
        class Append<>
        {
        public:
            using type = TypeList<>;
        };

        template<typename T, typename... Types>
        class Append<T, TypeList<Types...>>
        {
        public:
            using type = TypeList<T, Types...>;
        };

        /// 1 types list
        template<typename... FirstListTypes>
        class Append<TypeList<FirstListTypes...>>
        {
        public:
            using type = TypeList<FirstListTypes...>;
        };

        /// 2 types lists
        template<typename... FirstListTypes, typename... SecondListTypes>
        class Append<TypeList<FirstListTypes...>, TypeList<SecondListTypes...>>
        {
        public:
            using type = TypeList<FirstListTypes..., SecondListTypes...>;
        };

        /// 3 types lists
        template<typename... FirstListTypes, typename... SecondListTypes, typename... ThirdListTypes>
        class Append<TypeList<FirstListTypes...>, TypeList<SecondListTypes...>, TypeList<ThirdListTypes...>>
        {
        public:
            using type = TypeList<FirstListTypes..., SecondListTypes..., ThirdListTypes...>;
        };

        /// 4 types lists
        template<typename... FirstListTypes, typename... SecondListTypes, typename... ThirdListTypes, typename... FourthListTypes>
        class Append<TypeList<FirstListTypes...>, TypeList<SecondListTypes...>, TypeList<ThirdListTypes...>, TypeList<FourthListTypes...>>
        {
        public:
            using type = TypeList<FirstListTypes..., SecondListTypes..., ThirdListTypes..., FourthListTypes...>;
        };

        /// 5 types lists
        template<typename... FirstListTypes, typename... SecondListTypes, typename... ThirdListTypes, typename... FourthListTypes, typename... FifthListTypes>
        class Append<TypeList<FirstListTypes...>, TypeList<SecondListTypes...>, TypeList<ThirdListTypes...>, TypeList<FourthListTypes...>, TypeList<FifthListTypes...>>
        {
        public:
            using type = TypeList<FirstListTypes..., SecondListTypes..., ThirdListTypes..., FourthListTypes..., FifthListTypes...>;
        };

        /// 6 types lists
        template<typename... FirstListTypes, typename... SecondListTypes, typename... ThirdListTypes, typename... FourthListTypes, typename... FifthListTypes, typename... SixthListTypes>
        class Append<TypeList<FirstListTypes...>, TypeList<SecondListTypes...>, TypeList<ThirdListTypes...>, TypeList<FourthListTypes...>, TypeList<FifthListTypes...>, TypeList<SixthListTypes...>>
        {
        public:
            using type = TypeList<FirstListTypes..., SecondListTypes..., ThirdListTypes..., FourthListTypes..., FifthListTypes..., SixthListTypes...>;
        };

        template<typename... T>
        using Append_t = typename Append<T...>::type;

        /**
         * @brief Select list slice
         */
        template<int, int, typename...>
        class Slice {};

        template<int Offset, typename List>
        class Slice<Offset, 0, List>
        {
        public:
            using type = TypeList<>;
        };

        template<int Offset, typename List>
        class Slice<Offset, 1, List>
        {
        public:
            using type = TypeList<typename GetType<Offset, List>::type>;
        };

        template<int Offset, int Length, typename List>
        class Slice<Offset, Length, List>
        {
        public:
            using type = typename InsertBack<typename Slice<Offset, Length - 1, List>::type, typename GetType<Offset + Length - 1, List>::type>::type;
        };

        /**
         * @brief Select by bool template argument
         */
        template <bool, typename IfTrue, typename IfFalse>
        class Select
        {
        public:
            using value = IfTrue;
        };

        template<typename IfTrue, typename IfFalse>
        class Select<false, IfTrue, IfFalse>
        {
        public:
            using value = IfFalse;
        };

        /**
         * @brief Select elements by predicate
         */
        template<template <typename> class, typename...>
        class Sample;

        template<template <typename> class Predicate, typename... Types>
        class Sample<Predicate, TypeList<Types...>>
        {
        public:
            using type = typename DeleteAll<void, TypeList<typename Select<Predicate<Types>::value, Types, void>::value ...>>::type;
        };

        template<template <typename> class Predicate, typename... Types>
        using Sample_t = typename Sample<Predicate, Types...>::type;

        /**
         * @brief Search type by predicate
         */
        template<template <typename> class, typename...>
        class Search;

        template<template <typename> class Predicate>
        class Search<Predicate, TypeList<>>
        {
        public:
            static const int value = -1;
        };

        template<template <typename> class Predicate, typename Head, typename... Tail>
        class Search<Predicate, TypeList<Head, Tail...>>
        {
        public:
            static const int value = Predicate<Head>::value
                ? 0
                : (Search<Predicate, TypeList<Tail...>>::value >= 0
                    ? Search<Predicate, TypeList<Tail...>>::value + 1
                    : Search<Predicate, TypeList<Tail...>>::value);
        };

        // Thanks https://codereview.stackexchange.com/questions/131194/selection-sorting-a-type-list-compile-time
        template <auto i, auto j, typename List>
        class SwapElementsInTypeList
        {
            template <typename IndexSequence>
            struct SwapElementsInTypeListImpl;
            template <unsigned... Indexes>
            struct SwapElementsInTypeListImpl<std::index_sequence<Indexes...>>
            {
                using type = TypeList<GetType_t<Indexes != i && Indexes != j ? Indexes : Indexes == i ? j : i, List> ...>;
            };

        public:
            using type = typename SwapElementsInTypeListImpl<std::make_index_sequence<Length<List>::value>>::type;
        };

        template <template <typename, typename> class Comparator, typename List>
        class TypeListSort
        {
            template <unsigned i, unsigned j, unsigned Size, typename LoopList>
            struct TypeListSortImpl
            {
                using PermutationList = std::conditional_t<
                    Comparator<GetType_t<i, LoopList>, GetType_t<j, LoopList>>::value,
                    typename SwapElementsInTypeList<i, j, LoopList>::type,
                    LoopList
                >;

                using type = typename TypeListSortImpl<i, j + 1, Size, PermutationList>::type;
            };

            template <unsigned i, unsigned Size, typename LoopList>
            struct TypeListSortImpl<i, Size, Size, LoopList>
            {
                using type = typename TypeListSortImpl<i + 1, i + 2, Size, LoopList>::type;
            };

            template <unsigned j, unsigned Size, class LoopList>
            struct TypeListSortImpl<Size, j, Size, LoopList>
            {
                using type = LoopList;
            };

        public:
            using type = typename TypeListSortImpl<0, 1, Length<List>::value, List>::type;
        };
    }
}
#endif //! ZHELE_TYPELIST_H
