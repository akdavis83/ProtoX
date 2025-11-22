// Copyright (c) 2022 The QTC Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef QTC_KERNEL_CONTEXT_H
#define QTC_KERNEL_CONTEXT_H

namespace kernel {
//! Context struct holding the kernel library's logically global state, and
//! passed to external libqtc_kernel functions which need access to this
//! state. The kernel library API is a work in progress, so state organization
//! and member list will evolve over time.
//!
//! State stored directly in this struct should be simple. More complex state
//! should be stored to std::unique_ptr members pointing to opaque types.
struct Context {
    Context();
};
} // namespace kernel

#endif // QTC_KERNEL_CONTEXT_H
