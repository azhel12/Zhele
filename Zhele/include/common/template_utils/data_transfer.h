/**
 * @file
 * Contains declaration of
 * 
 * @author Konstantin Chizhov
 * @date 2014
 * @license FreeBSD
 */

#ifndef ZHELE_DATATRANSFER_H
#define ZHELE_DATATRANSFER_H

#include <type_traits>

namespace Zhele
{
	/// Transfer callback pointer
	using TransferCallback = std::add_pointer_t<void(void* data, unsigned size, bool success)>;
	/// Tagged transfer callback pointer
	using TaggedTransferCallback = std::add_pointer_t<void(void* tag, void* data, unsigned size, bool success)>;
}

#endif //!ZHELE_DATATRANSFER_H