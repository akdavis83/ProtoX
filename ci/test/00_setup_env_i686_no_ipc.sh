#!/usr/bin/env bash
#
# Copyright (c) 2020-present The Quantum Coin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

export LC_ALL=C.UTF-8

export HOST=i686-pc-linux-gnu
export CONTAINER_NAME=ci_i686_no_multiprocess
export CI_IMAGE_NAME_TAG="mirror.gcr.io/ubuntu:24.04"
export CI_IMAGE_PLATFORM="linux/amd64"
export PACKAGES="llvm clang g++-multilib"
export DEP_OPTS="DEBUG=1 NO_IPC=1"
export GOAL="install"
export CI_LIMIT_STACK_SIZE=1
export TEST_RUNNER_EXTRA="--v2transport --usecli"
export BITCOIN_CONFIG="\
 --preset=dev-mode \
 -DENABLE_IPC=OFF \
 -DCMAKE_BUILD_TYPE=Debug \
 -DCMAKE_C_COMPILER='clang;-m32' \
 -DCMAKE_CXX_COMPILER='clang++;-m32' \
 -DAPPEND_CPPFLAGS='-DBOOST_MULTI_INDEX_ENABLE_SAFE_MODE' \
"
<<<<<<< Updated upstream:ci/test/00_setup_env_i686_no_ipc.sh
=======
export BITCOIN_CMD="qtc -m" # Used in functional tests
>>>>>>> Stashed changes:ci/test/00_setup_env_i686_multiprocess.sh
