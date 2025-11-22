// Copyright (c) 2015-2022 The QTC Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef QTC_COMMON_URL_H
#define QTC_COMMON_URL_H

#include <string>
#include <string_view>

/* Decode a URL.
 *
 * Notably this implementation does not decode a '+' to a ' '.
 */
std::string UrlDecode(std::string_view url_encoded);

#endif // QTC_COMMON_URL_H
