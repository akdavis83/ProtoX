# Libraries

| Name                     | Description |
|--------------------------|-------------|
| *libqtc_cli*         | RPC client functionality used by *qtc-cli* executable |
| *libqtc_common*      | Home for common functionality shared by different executables and libraries. Similar to *libqtc_util*, but higher-level (see [Dependencies](#dependencies)). |
| *libqtc_consensus*   | Consensus functionality used by *libqtc_node* and *libqtc_wallet*. |
| *libqtc_crypto*      | Hardware-optimized functions for data encryption, hashing, message authentication, and key derivation. |
| *libqtc_kernel*      | Consensus engine and support library used for validation by *libqtc_node*. |
| *libqtcqt*           | GUI functionality used by *qtc-qt* and *qtc-gui* executables. |
| *libqtc_ipc*         | IPC functionality used by *qtc-node*, *qtc-wallet*, *qtc-gui* executables to communicate when [`-DENABLE_IPC=ON`](multiprocess.md) is used. |
| *libqtc_node*        | P2P and RPC server functionality used by *qtcd* and *qtc-qt* executables. |
| *libqtc_util*        | Home for common functionality shared by different executables and libraries. Similar to *libqtc_common*, but lower-level (see [Dependencies](#dependencies)). |
| *libqtc_wallet*      | Wallet functionality used by *qtcd* and *qtc-wallet* executables. |
| *libqtc_wallet_tool* | Lower-level wallet functionality used by *qtc-wallet* executable. |
| *libqtc_zmq*         | [ZeroMQ](../zmq.md) functionality used by *qtcd* and *qtc-qt* executables. |

## Conventions

- Most libraries are internal libraries and have APIs which are completely unstable! There are few or no restrictions on backwards compatibility or rules about external dependencies. An exception is *libqtc_kernel*, which, at some future point, will have a documented external interface.

- Generally each library should have a corresponding source directory and namespace. Source code organization is a work in progress, so it is true that some namespaces are applied inconsistently, and if you look at [`add_library(qtc_* ...)`](../../src/CMakeLists.txt) lists you can see that many libraries pull in files from outside their source directory. But when working with libraries, it is good to follow a consistent pattern like:

  - *libqtc_node* code lives in `src/node/` in the `node::` namespace
  - *libqtc_wallet* code lives in `src/wallet/` in the `wallet::` namespace
  - *libqtc_ipc* code lives in `src/ipc/` in the `ipc::` namespace
  - *libqtc_util* code lives in `src/util/` in the `util::` namespace
  - *libqtc_consensus* code lives in `src/consensus/` in the `Consensus::` namespace

## Dependencies

- Libraries should minimize what other libraries they depend on, and only reference symbols following the arrows shown in the dependency graph below:

<table><tr><td>

```mermaid

%%{ init : { "flowchart" : { "curve" : "basis" }}}%%

graph TD;

qtc-cli[qtc-cli]-->libqtc_cli;

qtcd[qtcd]-->libqtc_node;
qtcd[qtcd]-->libqtc_wallet;

qtc-qt[qtc-qt]-->libqtc_node;
qtc-qt[qtc-qt]-->libqtcqt;
qtc-qt[qtc-qt]-->libqtc_wallet;

qtc-wallet[qtc-wallet]-->libqtc_wallet;
qtc-wallet[qtc-wallet]-->libqtc_wallet_tool;

libqtc_cli-->libqtc_util;
libqtc_cli-->libqtc_common;

libqtc_consensus-->libqtc_crypto;

libqtc_common-->libqtc_consensus;
libqtc_common-->libqtc_crypto;
libqtc_common-->libqtc_util;

libqtc_kernel-->libqtc_consensus;
libqtc_kernel-->libqtc_crypto;
libqtc_kernel-->libqtc_util;

libqtc_node-->libqtc_consensus;
libqtc_node-->libqtc_crypto;
libqtc_node-->libqtc_kernel;
libqtc_node-->libqtc_common;
libqtc_node-->libqtc_util;

libqtcqt-->libqtc_common;
libqtcqt-->libqtc_util;

libqtc_util-->libqtc_crypto;

libqtc_wallet-->libqtc_common;
libqtc_wallet-->libqtc_crypto;
libqtc_wallet-->libqtc_util;

libqtc_wallet_tool-->libqtc_wallet;
libqtc_wallet_tool-->libqtc_util;

classDef bold stroke-width:2px, font-weight:bold, font-size: smaller;
class qtc-qt,qtcd,qtc-cli,qtc-wallet bold
```
</td></tr><tr><td>

**Dependency graph**. Arrows show linker symbol dependencies. *Crypto* lib depends on nothing. *Util* lib is depended on by everything. *Kernel* lib depends only on consensus, crypto, and util.

</td></tr></table>

- The graph shows what _linker symbols_ (functions and variables) from each library other libraries can call and reference directly, but it is not a call graph. For example, there is no arrow connecting *libqtc_wallet* and *libqtc_node* libraries, because these libraries are intended to be modular and not depend on each other's internal implementation details. But wallet code is still able to call node code indirectly through the `interfaces::Chain` abstract class in [`interfaces/chain.h`](../../src/interfaces/chain.h) and node code calls wallet code through the `interfaces::ChainClient` and `interfaces::Chain::Notifications` abstract classes in the same file. In general, defining abstract classes in [`src/interfaces/`](../../src/interfaces/) can be a convenient way of avoiding unwanted direct dependencies or circular dependencies between libraries.

- *libqtc_crypto* should be a standalone dependency that any library can depend on, and it should not depend on any other libraries itself.

- *libqtc_consensus* should only depend on *libqtc_crypto*, and all other libraries besides *libqtc_crypto* should be allowed to depend on it.

- *libqtc_util* should be a standalone dependency that any library can depend on, and it should not depend on other libraries except *libqtc_crypto*. It provides basic utilities that fill in gaps in the C++ standard library and provide lightweight abstractions over platform-specific features. Since the util library is distributed with the kernel and is usable by kernel applications, it shouldn't contain functions that external code shouldn't call, like higher level code targeted at the node or wallet. (*libqtc_common* is a better place for higher level code, or code that is meant to be used by internal applications only.)

- *libqtc_common* is a home for miscellaneous shared code used by different Quantum Coin Core applications. It should not depend on anything other than *libqtc_util*, *libqtc_consensus*, and *libqtc_crypto*.

- *libqtc_kernel* should only depend on *libqtc_util*, *libqtc_consensus*, and *libqtc_crypto*.

- The only thing that should depend on *libqtc_kernel* internally should be *libqtc_node*. GUI and wallet libraries *libqtcqt* and *libqtc_wallet* in particular should not depend on *libqtc_kernel* and the unneeded functionality it would pull in, like block validation. To the extent that GUI and wallet code need scripting and signing functionality, they should be able to get it from *libqtc_consensus*, *libqtc_common*, *libqtc_crypto*, and *libqtc_util*, instead of *libqtc_kernel*.

- GUI, node, and wallet code internal implementations should all be independent of each other, and the *libqtcqt*, *libqtc_node*, *libqtc_wallet* libraries should never reference each other's symbols. They should only call each other through [`src/interfaces/`](../../src/interfaces/) abstract interfaces.

## Work in progress

- Validation code is moving from *libqtc_node* to *libqtc_kernel* as part of [The libqtckernel Project #27587](https://github.com/qtc/qtc/issues/27587)
