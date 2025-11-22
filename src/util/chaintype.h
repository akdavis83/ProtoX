// Copyright (c) 2023 The QTC Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef QTC_UTIL_CHAINTYPE_H
#define QTC_UTIL_CHAINTYPE_H

#include <optional>
#include <string>

enum class ChainType {
    MAIN,
    TESTNET,
    SIGNET,
    REGTEST,
    TESTNET4,
};

std::string ChainTypeToString(ChainType chain);

std::optional<ChainType> ChainTypeFromString(std::string_view chain);

#endif // QTC_UTIL_CHAINTYPE_H
