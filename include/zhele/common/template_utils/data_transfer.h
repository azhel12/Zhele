/**
 * @file
 * Contains declaration of callbacks
 * 
 * @author Konstantin Chizhov
 * @date 2014
 * @license FreeBSD
 */

#ifndef ZHELE_DATATRANSFER_H
#define ZHELE_DATATRANSFER_H

#include <type_traits>
#include <functional>
namespace Zhele
{
    // Using std::function insted pointer takes about 300 bytes flash and 60 bytes RAM.
    // Therefore, it's big question: std::function or pointer.
    /// Transfer callback pointer
    //using TransferCallback = std::add_pointer_t<void(void* data, unsigned size, bool success)>;
    using TransferCallback = std::function<void(void* data, unsigned size, bool success)>;
    /// Tagged transfer callback pointer
    //using TaggedTransferCallback = std::add_pointer_t<void(void* tag, void* data, unsigned size, bool success)>;
    using TaggedTransferCallback = std::function<void(void* tag, void* data, unsigned size, bool success)>;
}

#endif //!ZHELE_DATATRANSFER_H