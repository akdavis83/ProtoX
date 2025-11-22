# Copyright (c) 2023-present The Quantum Coin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://opensource.org/license/mit/.

include(AddWindowsResources)

configure_file(${PROJECT_SOURCE_DIR}/cmake/qtc-build-config.h.in qtc-build-config.h USE_SOURCE_PERMISSIONS @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

#=============================
# Subprojects
#=============================
# Subprojects include subdirectories that do or could have tests
# and/or benchmark binaries, such as all subtrees and univalue.
# These need to be included before CMAKE_*_OUTPUT_DIRECTORY variables
# are set, so output locations of subproject tests and libraries are
# not overridden.
include(../cmake/crc32c.cmake)
include(../cmake/leveldb.cmake)
include(../cmake/minisketch.cmake)
add_subdirectory(univalue)
if (ENABLE_IPC AND NOT WITH_EXTERNAL_LIBMULTIPROCESS)
  include(../cmake/libmultiprocess.cmake)
  add_libmultiprocess(ipc/libmultiprocess)
endif()
#=============================
# secp256k1 subtree
#=============================
message("")
message("Configuring secp256k1 subtree...")
set(SECP256K1_DISABLE_SHARED ON CACHE BOOL "" FORCE)
set(SECP256K1_ENABLE_MODULE_ECDH OFF CACHE BOOL "" FORCE)
set(SECP256K1_ENABLE_MODULE_RECOVERY ON CACHE BOOL "" FORCE)
set(SECP256K1_ENABLE_MODULE_MUSIG OFF CACHE BOOL "" FORCE)
set(SECP256K1_BUILD_BENCHMARK OFF CACHE BOOL "" FORCE)
set(SECP256K1_BUILD_TESTS ${BUILD_TESTS} CACHE BOOL "" FORCE)
set(SECP256K1_BUILD_EXHAUSTIVE_TESTS ${BUILD_TESTS} CACHE BOOL "" FORCE)
if(NOT BUILD_TESTS)
  # Always skip the ctime tests, if we are building no other tests.
  # Otherwise, they are built if Valgrind is available. See SECP256K1_VALGRIND.
  set(SECP256K1_BUILD_CTIME_TESTS ${BUILD_TESTS} CACHE BOOL "" FORCE)
endif()
set(SECP256K1_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
include(GetTargetInterface)
# -fsanitize and related flags apply to both C++ and C,
# so we can pass them down to libsecp256k1 as CFLAGS and LDFLAGS.
get_target_interface(SECP256K1_APPEND_CFLAGS "" sanitize_interface COMPILE_OPTIONS)
string(STRIP "${SECP256K1_APPEND_CFLAGS} ${APPEND_CPPFLAGS}" SECP256K1_APPEND_CFLAGS)
string(STRIP "${SECP256K1_APPEND_CFLAGS} ${APPEND_CFLAGS}" SECP256K1_APPEND_CFLAGS)
set(SECP256K1_APPEND_CFLAGS ${SECP256K1_APPEND_CFLAGS} CACHE STRING "" FORCE)
get_target_interface(SECP256K1_APPEND_LDFLAGS "" sanitize_interface LINK_OPTIONS)
string(STRIP "${SECP256K1_APPEND_LDFLAGS} ${APPEND_LDFLAGS}" SECP256K1_APPEND_LDFLAGS)
set(SECP256K1_APPEND_LDFLAGS ${SECP256K1_APPEND_LDFLAGS} CACHE STRING "" FORCE)
# We want to build libsecp256k1 with the most tested RelWithDebInfo configuration.
enable_language(C)
foreach(config IN LISTS CMAKE_BUILD_TYPE CMAKE_CONFIGURATION_TYPES)
  if(config STREQUAL "")
    continue()
  endif()
  string(TOUPPER "${config}" config)
  set(CMAKE_C_FLAGS_${config} "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
endforeach()
# If the CFLAGS environment variable is defined during building depends
# and configuring this build system, its content might be duplicated.
if(DEFINED ENV{CFLAGS})
  deduplicate_flags(CMAKE_C_FLAGS)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
add_subdirectory(secp256k1)
set_target_properties(secp256k1 PROPERTIES
  EXCLUDE_FROM_ALL TRUE
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set top-level target output locations.
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

add_custom_target(generate_build_info
  BYPRODUCTS ${PROJECT_BINARY_DIR}/src/qtc-build-info.h
  COMMAND ${CMAKE_COMMAND} -DBUILD_INFO_HEADER_PATH=${PROJECT_BINARY_DIR}/src/qtc-build-info.h -DSOURCE_DIR=${PROJECT_SOURCE_DIR} -P ${PROJECT_SOURCE_DIR}/cmake/script/GenerateBuildInfo.cmake
  COMMENT "Generating qtc-build-info.h"
  VERBATIM
)
add_library(qtc_clientversion STATIC EXCLUDE_FROM_ALL
  clientversion.cpp
)
target_link_libraries(qtc_clientversion
  PRIVATE
    core_interface
)
add_dependencies(qtc_clientversion generate_build_info)

add_subdirectory(crypto)
add_subdirectory(util)

# QTC Quantum-Safe Cryptography - TODO: Implement in Phase 4
# add_library(qtc_quantum STATIC EXCLUDE_FROM_ALL
#   crypto/kyber/kyber1024.cpp     # TODO: Port from JavaScript implementation
#   crypto/dilithium/dilithium3.cpp # TODO: Port from JavaScript implementation
# )
# target_link_libraries(qtc_quantum
#   PRIVATE
#     core_interface
# )
if(ENABLE_IPC)
  add_subdirectory(ipc)
endif()

add_library(qtc_consensus STATIC EXCLUDE_FROM_ALL
  arith_uint256.cpp
  consensus/merkle.cpp
  consensus/tx_check.cpp
  hash.cpp
  primitives/block.cpp
  primitives/transaction.cpp
  pubkey.cpp
  script/interpreter.cpp
  script/script.cpp
  script/script_error.cpp
  uint256.cpp
)
target_link_libraries(qtc_consensus
  PRIVATE
    core_interface
    qtc_crypto
    secp256k1
    # qtc_quantum  # TODO: Add back in Phase 4 when quantum crypto is implemented
)

if(WITH_ZMQ)
  add_subdirectory(zmq)
endif()

# Home for common functionality shared by different executables and libraries.
# Similar to `qtc_util` library, but higher-level.
add_library(qtc_common STATIC EXCLUDE_FROM_ALL
  addresstype.cpp
  base58.cpp
  bech32.cpp
  chain.cpp
  chainparams.cpp
  chainparamsbase.cpp
  coins.cpp
  common/args.cpp
  common/bloom.cpp
  common/config.cpp
  common/init.cpp
  common/interfaces.cpp
  common/messages.cpp
  common/netif.cpp
  common/pcp.cpp
  common/run_command.cpp
  common/settings.cpp
  common/signmessage.cpp
  common/system.cpp
  common/url.cpp
  compressor.cpp
  core_read.cpp
  core_write.cpp
  deploymentinfo.cpp
  external_signer.cpp
  init/common.cpp
  kernel/chainparams.cpp
  key.cpp
  key_io.cpp
  merkleblock.cpp
  net_permissions.cpp
  net_types.cpp
  netaddress.cpp
  netbase.cpp
  outputtype.cpp
  policy/feerate.cpp
  policy/policy.cpp
  pow.cpp
  protocol.cpp
  psbt.cpp
  rpc/rawtransaction_util.cpp
  rpc/request.cpp
  rpc/util.cpp
  scheduler.cpp
  script/descriptor.cpp
  script/miniscript.cpp
  script/parsing.cpp
  script/sign.cpp
  script/signingprovider.cpp
  script/solver.cpp
)
target_link_libraries(qtc_common
  PRIVATE
    core_interface
    qtc_consensus
    qtc_util
    univalue
    secp256k1
    Boost::headers
    $<TARGET_NAME_IF_EXISTS:USDT::headers>
    $<$<PLATFORM_ID:Windows>:ws2_32>
)

# PQ Noise-only transport module
if(ENABLE_PQ_NOISE)
  add_library(qtc_pq_noise STATIC EXCLUDE_FROM_ALL
    net/pq_noise.cpp
    net/pq_metrics.cpp
  )
  target_include_directories(qtc_pq_noise
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}
  )
  target_link_libraries(qtc_pq_noise
    PRIVATE
      core_interface
      qtc_crypto
      oqs::oqs
  )
  target_link_libraries(qtc_common PRIVATE qtc_pq_noise)
  target_compile_definitions(qtc_common PRIVATE ENABLE_PQ_NOISE=1)
endif()

include(InstallBinaryComponent)

if(ENABLE_WALLET)
  add_subdirectory(wallet)

  if(BUILD_WALLET_TOOL)
    add_executable(qtc-wallet
      qtc-wallet.cpp
      init/qtc-wallet.cpp
      wallet/wallettool.cpp
    )
    add_windows_resources(qtc-wallet qtc-wallet-res.rc)
    add_windows_application_manifest(qtc-wallet)
    target_link_libraries(qtc-wallet
      core_interface
      qtc_wallet
      qtc_common
      qtc_util
      Boost::headers
    )
    install_binary_component(qtc-wallet HAS_MANPAGE)
  endif()
endif()


# P2P and RPC server functionality used by `qtcd` and `qtc-qt` executables.
add_library(qtc_node STATIC EXCLUDE_FROM_ALL
  addrdb.cpp
  addrman.cpp
  banman.cpp
  bip324.cpp
  blockencodings.cpp
  blockfilter.cpp
  consensus/tx_verify.cpp
  dbwrapper.cpp
  deploymentstatus.cpp
  flatfile.cpp
  headerssync.cpp
  httprpc.cpp
  httpserver.cpp
  i2p.cpp
  index/base.cpp
  index/blockfilterindex.cpp
  index/coinstatsindex.cpp
  index/txindex.cpp
  init.cpp
  kernel/chain.cpp
  kernel/checks.cpp
  kernel/coinstats.cpp
  kernel/context.cpp
  kernel/cs_main.cpp
  kernel/disconnected_transactions.cpp
  kernel/mempool_removal_reason.cpp
  mapport.cpp
  net.cpp
  net/pqnoise/pqnoise_state.cpp
  net_processing.cpp
  netgroup.cpp
  node/abort.cpp
  node/blockmanager_args.cpp
  node/blockstorage.cpp
  node/caches.cpp
  node/chainstate.cpp
  node/chainstatemanager_args.cpp
  node/coin.cpp
  node/coins_view_args.cpp
  node/connection_types.cpp
  node/context.cpp
  node/database_args.cpp
  node/eviction.cpp
  node/interface_ui.cpp
  node/interfaces.cpp
  node/kernel_notifications.cpp
  node/mempool_args.cpp
  node/mempool_persist.cpp
  node/mempool_persist_args.cpp
  node/miner.cpp
  node/mini_miner.cpp
  node/minisketchwrapper.cpp
  node/peerman_args.cpp
  node/psbt.cpp
  node/timeoffsets.cpp
  node/transaction.cpp
  node/txdownloadman_impl.cpp
  node/txreconciliation.cpp
  node/utxo_snapshot.cpp
  node/warnings.cpp
  noui.cpp
  policy/ephemeral_policy.cpp
  policy/fees.cpp
  policy/fees_args.cpp
  policy/packages.cpp
  policy/rbf.cpp
  policy/settings.cpp
  policy/truc_policy.cpp
  rest.cpp
  rpc/blockchain.cpp
  rpc/external_signer.cpp
  rpc/fees.cpp
  rpc/mempool.cpp
  rpc/mining.cpp
  rpc/net.cpp
  rpc/node.cpp
  rpc/output_script.cpp
  rpc/rawtransaction.cpp
  rpc/server.cpp
  rpc/server_util.cpp
  rpc/signmessage.cpp
  rpc/txoutproof.cpp
  script/sigcache.cpp
  signet.cpp
  torcontrol.cpp
  txdb.cpp
  txgraph.cpp
  txmempool.cpp
  txorphanage.cpp
  txrequest.cpp
  validation.cpp
  validationinterface.cpp
  versionbits.cpp
  $<$<TARGET_EXISTS:qtc_wallet>:wallet/init.cpp>
  $<$<NOT:$<TARGET_EXISTS:qtc_wallet>>:dummywallet.cpp>
)
target_link_libraries(qtc_node
  PRIVATE
    core_interface
    qtc_common
    qtc_util
    $<TARGET_NAME_IF_EXISTS:qtc_zmq>
    leveldb
    minisketch
    univalue
    Boost::headers
    $<TARGET_NAME_IF_EXISTS:libevent::core>
    $<TARGET_NAME_IF_EXISTS:libevent::extra>
    $<TARGET_NAME_IF_EXISTS:libevent::pthreads>
    $<TARGET_NAME_IF_EXISTS:USDT::headers>
)

# Quantum Coin wrapper executable that can call other executables.
if(BUILD_BITCOIN_BIN)
  add_executable(qtc qtc.cpp)
  add_windows_resources(qtc qtc-res.rc)
  add_windows_application_manifest(qtc)
  target_link_libraries(qtc core_interface qtc_util)
  install_binary_component(qtc)
endif()

# Quantum Coin Core qtcd.
if(BUILD_DAEMON)
  add_executable(qtcd
    qtcd.cpp
    init/qtcd.cpp
  )
  add_windows_resources(qtcd qtcd-res.rc)
  add_windows_application_manifest(qtcd)
  target_link_libraries(qtcd
    core_interface
    qtc_node
    $<TARGET_NAME_IF_EXISTS:qtc_wallet>
  )
  install_binary_component(qtcd HAS_MANPAGE)
endif()
if(ENABLE_IPC AND BUILD_DAEMON)
  add_executable(qtc-node
    qtcd.cpp
    init/qtc-node.cpp
  )
  target_link_libraries(qtc-node
    core_interface
    qtc_node
    qtc_ipc
    $<TARGET_NAME_IF_EXISTS:qtc_wallet>
  )
  install_binary_component(qtc-node)
endif()

if(ENABLE_IPC AND BUILD_TESTS)
    # qtc_ipc_test library target is defined here in src/CMakeLists.txt
    # instead of src/test/CMakeLists.txt so capnp files in src/test/ are able to
    # reference capnp files in src/ipc/capnp/ by relative path. The Cap'n Proto
    # compiler only allows importing by relative path when the importing and
    # imported files are underneath the same compilation source prefix, so the
    # source prefix must be src/, not src/test/
    add_library(qtc_ipc_test STATIC EXCLUDE_FROM_ALL
      test/ipc_test.cpp
    )
    target_capnp_sources(qtc_ipc_test ${PROJECT_SOURCE_DIR}
      test/ipc_test.capnp
    )
    add_dependencies(qtc_ipc_test qtc_ipc_headers)
endif()


add_library(qtc_cli STATIC EXCLUDE_FROM_ALL
  compat/stdin.cpp
  rpc/client.cpp
)
target_link_libraries(qtc_cli
  PUBLIC
    core_interface
    univalue
)


# Quantum Coin Core RPC client
if(BUILD_CLI)
  add_executable(qtc-cli qtc-cli.cpp)
  add_windows_resources(qtc-cli qtc-cli-res.rc)
  add_windows_application_manifest(qtc-cli)
  target_link_libraries(qtc-cli
    core_interface
    qtc_cli
    qtc_common
    qtc_util
    libevent::core
    libevent::extra
  )
  install_binary_component(qtc-cli HAS_MANPAGE)
endif()


if(BUILD_TX)
  add_executable(qtc-tx qtc-tx.cpp)
  add_windows_resources(qtc-tx qtc-tx-res.rc)
  add_windows_application_manifest(qtc-tx)
  target_link_libraries(qtc-tx
    core_interface
    qtc_common
    qtc_util
    univalue
  )
  install_binary_component(qtc-tx HAS_MANPAGE)
endif()


if(BUILD_UTIL)
  add_executable(qtc-util qtc-util.cpp)
  add_windows_resources(qtc-util qtc-util-res.rc)
  add_windows_application_manifest(qtc-util)
  target_link_libraries(qtc-util
    core_interface
    qtc_common
    qtc_util
  )
  install_binary_component(qtc-util HAS_MANPAGE)
endif()


if(BUILD_GUI)
  add_subdirectory(qt)
endif()


if(BUILD_KERNEL_LIB)
  add_subdirectory(kernel)
endif()

if(BUILD_UTIL_CHAINSTATE)
  add_executable(qtc-chainstate
    qtc-chainstate.cpp
  )
  # TODO: The `SKIP_BUILD_RPATH` property setting can be deleted
  #       in the future after reordering Guix script commands to
  #       perform binary checks after the installation step.
  # Relevant discussions:
  # - https://github.com/hebasto/qtc/pull/236#issuecomment-2183120953
  # - https://github.com/qtc/qtc/pull/30312#issuecomment-2191235833
  set_target_properties(qtc-chainstate PROPERTIES
    SKIP_BUILD_RPATH OFF
  )
  target_link_libraries(qtc-chainstate
    PRIVATE
      core_interface
      qtckernel
  )
endif()


add_subdirectory(test/util)
if(BUILD_BENCH)
  add_subdirectory(bench)
endif()

if(BUILD_TESTS)
  add_subdirectory(test)
endif()

if(BUILD_FUZZ_BINARY)
  add_subdirectory(test/fuzz)
endif()

# QTC Quantum Wallet
add_library(qtc_quantum_wallet STATIC EXCLUDE_FROM_ALL
  qtc_wallet.cpp
)
target_link_libraries(qtc_quantum_wallet
  PRIVATE
    core_interface
    qtc_crypto
)

# QTC Quantum-Safe Mining Algorithm
# Phase 3: Argon2-based PoW removed; provide an interface target to carry
# required dependencies until new mining sources are added.
add_library(qtc_quantum_mining INTERFACE)
target_link_libraries(qtc_quantum_mining
  INTERFACE
    core_interface
    qtc_crypto
    qtc_quantum_wallet
)

# Add quantum mining to consensus library
target_link_libraries(qtc_consensus
  PRIVATE
    qtc_quantum_mining
)

# QTC Quantum Miner Application
add_executable(qtc-miner
    qtc-miner.cpp
)
target_link_libraries(qtc-miner
    PRIVATE
        qtc_consensus
        qtc_crypto
        qtc_quantum_mining
        qtc_quantum_wallet
        qtc_common
        util
        univalue
)

# Install miner
install(TARGETS qtc-miner DESTINATION bin)

# QTC Wallet CLI Tool
add_executable(qtc-wallet-cli
    qtc-wallet-cli.cpp
)
target_link_libraries(qtc-wallet-cli
    PRIVATE
        qtc_consensus
        qtc_crypto
        qtc_quantum_wallet
        qtc_common
        util
        univalue
)

# Install wallet CLI
install(TARGETS qtc-wallet-cli DESTINATION bin)

# QTC Enhanced Wallet CLI
add_executable(qtc-wallet-enhanced
    qtc-wallet-enhanced.cpp
)
target_link_libraries(qtc-wallet-enhanced
    PRIVATE
        qtc_consensus
        qtc_crypto
        qtc_quantum_wallet
        qtc_common
        util
        univalue
)

install(TARGETS qtc-wallet-enhanced DESTINATION bin)

# QTC Mining Demonstration
add_executable(qtc-mining-demo
    qtc-mining-demo.cpp
)
target_link_libraries(qtc-mining-demo
    PRIVATE
        qtc_consensus
        qtc_crypto
        qtc_quantum_mining
        qtc_quantum_wallet
        qtc_common
        util
        univalue
)

install(TARGETS qtc-mining-demo DESTINATION bin)
