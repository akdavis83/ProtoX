// Copyright (c) 2022 The QTC Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

<<<<<<< Updated upstream:src/policy/fees/block_policy_estimator_args.h
#ifndef BITCOIN_POLICY_FEES_BLOCK_POLICY_ESTIMATOR_ARGS_H
#define BITCOIN_POLICY_FEES_BLOCK_POLICY_ESTIMATOR_ARGS_H
=======
#ifndef QTC_POLICY_FEES_ARGS_H
#define QTC_POLICY_FEES_ARGS_H
>>>>>>> Stashed changes:src/policy/fees_args.h

#include <util/fs.h>

class ArgsManager;

/** @return The fee estimates data file path. */
fs::path FeeestPath(const ArgsManager& argsman);

<<<<<<< Updated upstream:src/policy/fees/block_policy_estimator_args.h
#endif // BITCOIN_POLICY_FEES_BLOCK_POLICY_ESTIMATOR_ARGS_H
=======
#endif // QTC_POLICY_FEES_ARGS_H
>>>>>>> Stashed changes:src/policy/fees_args.h
