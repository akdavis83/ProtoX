// Copyright (c) 2024 The QTC Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef QTC_INTERFACES_TYPES_H
#define QTC_INTERFACES_TYPES_H

#include <uint256.h>

namespace interfaces {

//! Hash/height pair to help track and identify blocks.
struct BlockRef {
    uint256 hash;
    int height = -1;
};

} // namespace interfaces

#endif // QTC_INTERFACES_TYPES_H
