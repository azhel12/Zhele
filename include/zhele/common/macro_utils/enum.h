/**
 * @file
 * @brief Macros for enum
 * 
 * @details
 * DECLARE_ENUM_OPERATIONS is used to declare enum bitwise operations
 * to use enum type as a type safe flags
 * 
 * @author Konstantin Chizhov
 * @date 2018
 * @license FreeBSD
 */

#ifndef ZHELE_ENUM_H
#define ZHELE_ENUM_H

#define DECLARE_ENUM_OPERATIONS(ENUM_NAME)                               \
inline ENUM_NAME                                                         \
operator|(ENUM_NAME left, ENUM_NAME right)                               \
{ return ENUM_NAME(static_cast<uint32_t>(left) | static_cast<uint32_t>(right)); }  \
                                                                         \
inline ENUM_NAME                                                         \
operator&(ENUM_NAME left, ENUM_NAME right)                               \
{ return ENUM_NAME(static_cast<uint32_t>(left) & static_cast<uint32_t>(right)); }  \
                                                                         \
inline ENUM_NAME                                                         \
operator^(ENUM_NAME left, ENUM_NAME right)                               \
{ return ENUM_NAME(static_cast<uint32_t>(left) ^ static_cast<uint32_t>(right)); }  \
                                                                         \
inline ENUM_NAME                                                         \
operator~(ENUM_NAME left)                                                \
{ return ENUM_NAME(~static_cast<uint32_t>(left)); }                           \
                                                                         \
inline const ENUM_NAME&                                                  \
operator|=(ENUM_NAME& left, ENUM_NAME right)                             \
{ return left = left | right; }                                          \
                                                                         \
inline const ENUM_NAME&                                                  \
operator&=(ENUM_NAME& left, ENUM_NAME right)                             \
{ return left = left & right; }                                          \
                                                                         \
inline const ENUM_NAME&                                                  \
operator^=(ENUM_NAME& left, ENUM_NAME right)                             \
{ return left = left ^ right; }                                          \

#define DECLARE_ENUM_OPERATIONS_IN_CLASS(ENUM_NAME)                               \
friend inline ENUM_NAME                                                         \
operator|(ENUM_NAME left, ENUM_NAME right)                               \
{ return ENUM_NAME(static_cast<uint32_t>(left) | static_cast<uint32_t>(right)); }  \
                                                                         \
friend inline ENUM_NAME                                                         \
operator&(ENUM_NAME left, ENUM_NAME right)                               \
{ return ENUM_NAME(static_cast<uint32_t>(left) & static_cast<uint32_t>(right)); }  \
                                                                         \
friend inline ENUM_NAME                                                         \
operator^(ENUM_NAME left, ENUM_NAME right)                               \
{ return ENUM_NAME(static_cast<uint32_t>(left) ^ static_cast<uint32_t>(right)); }  \
                                                                         \
friend inline ENUM_NAME                                                         \
operator~(ENUM_NAME left)                                                \
{ return ENUM_NAME(~static_cast<uint32_t>(left)); }                           \
                                                                         \
friend inline const ENUM_NAME&                                                  \
operator|=(ENUM_NAME& left, ENUM_NAME right)                             \
{ return left = left | right; }                                          \
                                                                         \
friend inline const ENUM_NAME&                                                  \
operator&=(ENUM_NAME& left, ENUM_NAME right)                             \
{ return left = left & right; }                                          \
                                                                         \
friend inline const ENUM_NAME&                                                  \
operator^=(ENUM_NAME& left, ENUM_NAME right)                             \
{ return left = left ^ right; }                                          \

#define DECLARE_ENUM_OPERATIONS_TEMPLATE(ENUM_NAME, TEMPLATE)            \
TEMPLATE                                                                 \
inline ENUM_NAME                                                         \
operator|(ENUM_NAME left, ENUM_NAME right)                               \
{ return ENUM_NAME(static_cast<uint32_t>(left) | static_cast<uint32_t>(right)); }  \
                                                                         \
TEMPLATE                                                                 \
inline ENUM_NAME                                                         \
operator&(ENUM_NAME left, ENUM_NAME right)                               \
{ return ENUM_NAME(static_cast<uint32_t>(left) & static_cast<uint32_t>(right)); }  \
                                                                         \
TEMPLATE                                                                 \
inline ENUM_NAME                                                         \
operator^(ENUM_NAME left, ENUM_NAME right)                               \
{ return ENUM_NAME(static_cast<uint32_t>(left) ^ static_cast<uint32_t>(right)); }  \
                                                                         \
TEMPLATE                                                                 \
inline ENUM_NAME                                                         \
operator~(ENUM_NAME left)                                                \
{ return ENUM_NAME(~static_cast<uint32_t>(left)); }                           \
                                                                         \
TEMPLATE                                                                 \
inline const ENUM_NAME&                                                  \
operator|=(ENUM_NAME& left, ENUM_NAME right)                             \
{ return left = left | right; }                                          \
                                                                         \
TEMPLATE                                                                 \
inline const ENUM_NAME&                                                  \
operator&=(ENUM_NAME& left, ENUM_NAME right)                             \
{ return left = left & right; }                                          \
                                                                         \
TEMPLATE                                                                 \
inline const ENUM_NAME&                                                  \
operator^=(ENUM_NAME& left, ENUM_NAME right)                             \
{ return left = left ^ right; }

namespace Zhele
{
	template<class Enum>
	bool HasAllFlags(Enum value, Enum flags)
	{
		return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flags)) == static_cast<uint32_t>(flags);
	}
	
	template<class Enum>
	bool HasAnyFlag(Enum value, Enum flags)
	{
		return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flags)) != 0;
	}
}

#endif //! ZHELE_ENUM_H