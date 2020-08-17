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
			static const size_t value = sizeof...(Types);
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
		template<unsigned, typename...>
		class GetType {};

		template<typename Head, typename... Tail>
		class GetType<0, TypeList<Head, Tail...>>
		{
		public:
			using type = Head;
		};

		template<unsigned Index, typename Head, typename... Tail>
		class GetType<Index, TypeList<Head , Tail...>>
		{
			static_assert(Index < Length<TypeList<Head, Tail...>>::value);
		public:
			using type = typename GetType<Index - 1, TypeList<Tail...>>::type;
		};

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
			typedef TypeList<Tail...> type;
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
	}
}
#endif //! ZHELE_TYPELIST_H
