// Copyright (c) 2023 Quantum Coin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "nontrivial-threadlocal.h"

#include <clang-tidy/ClangTidyModule.h>
#include <clang-tidy/ClangTidyModuleRegistry.h>

class Quantum CoinModule final : public clang::tidy::ClangTidyModule
{
public:
    void addCheckFactories(clang::tidy::ClangTidyCheckFactories& CheckFactories) override
    {
        CheckFactories.registerCheck<qtc::NonTrivialThreadLocal>("qtc-nontrivial-threadlocal");
    }
};

static clang::tidy::ClangTidyModuleRegistry::Add<Quantum CoinModule>
    X("qtc-module", "Adds qtc checks.");

volatile int Quantum CoinModuleAnchorSource = 0;
