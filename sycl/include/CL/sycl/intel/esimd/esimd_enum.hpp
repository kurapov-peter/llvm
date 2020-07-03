//==---------------- esimd_enum.hpp - DPC++ Explicit SIMD API   ------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// definitions used in Explicit SIMD APIs.
//===----------------------------------------------------------------------===//

#pragma once

#include <CL/sycl/detail/defines.hpp>
#include <cstdint> // for uint* types

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {
namespace intel {
namespace gpu {

using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;

#ifdef __SYCL_DEVICE_ONLY__
// Mark a function being nodebug.
#define ESIMD_NODEBUG __attribute__((nodebug))
// Mark a "ESIMD global": accessible from all functions in current translation
// unit, separate copy per subgroup (work-item), mapped to SPIRV private storage
// class.
#define ESIMD_PRIVATE __attribute__((opencl_private))
// Bind a ESIMD global variable to a specific register.
#define ESIMD_REGISTER(n) __attribute__((register(n)))
#else
// TODO ESIMD define what this means on Windows host
#define ESIMD_NODEBUG
// On host device ESIMD global is a thread local static var. This assumes that
// each work-item is mapped to a separate OS thread on host device.
#define ESIMD_PRIVATE thread_local
#define ESIMD_REGISTER(n)
#endif

// Mark a function being noinline
#define ESIMD_NOINLINE __attribute__((noinline))
// Mark a function to be inlined
#define ESIMD_INLINE __attribute__((always_inline))

// Enums
enum { GENX_NOSAT = 0, GENX_SAT };

enum ChannelMaskType {
  ESIMD_R_ENABLE = 1,
  ESIMD_G_ENABLE = 2,
  ESIMD_GR_ENABLE = 3,
  ESIMD_B_ENABLE = 4,
  ESIMD_BR_ENABLE = 5,
  ESIMD_BG_ENABLE = 6,
  ESIMD_BGR_ENABLE = 7,
  ESIMD_A_ENABLE = 8,
  ESIMD_AR_ENABLE = 9,
  ESIMD_AG_ENABLE = 10,
  ESIMD_AGR_ENABLE = 11,
  ESIMD_AB_ENABLE = 12,
  ESIMD_ABR_ENABLE = 13,
  ESIMD_ABG_ENABLE = 14,
  ESIMD_ABGR_ENABLE = 15
};

#define NumChannels(Mask)                                                      \
  ((Mask & 1) + ((Mask & 2) >> 1) + ((Mask & 4) >> 2) + ((Mask & 8) >> 3))

#define HasR(Mask) ((Mask & 1) == 1)
#define HasG(Mask) ((Mask & 2) >> 1 == 1)
#define HasB(Mask) ((Mask & 4) >> 2 == 1)
#define HasA(Mask) ((Mask & 8) >> 3 == 1)

enum class EsimdAtomicOpType : uint16_t {
  ATOMIC_ADD = 0x0,
  ATOMIC_SUB = 0x1,
  ATOMIC_INC = 0x2,
  ATOMIC_DEC = 0x3,
  ATOMIC_MIN = 0x4,
  ATOMIC_MAX = 0x5,
  ATOMIC_XCHG = 0x6,
  ATOMIC_CMPXCHG = 0x7,
  ATOMIC_AND = 0x8,
  ATOMIC_OR = 0x9,
  ATOMIC_XOR = 0xa,
  ATOMIC_MINSINT = 0xb,
  ATOMIC_MAXSINT = 0xc,
  ATOMIC_FMAX = 0x10,
  ATOMIC_FMIN = 0x11,
  ATOMIC_FCMPWR = 0x12,
  ATOMIC_PREDEC = 0xff
};

// L1 or L3 cache hint kinds.
enum class CacheHint : uint8_t {
  None = 0,
  Uncached = 1,
  WriteBack = 2,
  WriteThrough = 3,
  Streaming = 4,
  ReadInvalidate = 5
};

} // namespace gpu

} // namespace intel
} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)
