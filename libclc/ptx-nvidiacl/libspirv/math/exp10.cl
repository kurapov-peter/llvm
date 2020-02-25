//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <spirv/spirv.h>
#include "../../include/libdevice.h"
#include "../../../generic/lib/clcmacro.h"

#define __CLC_FUNCTION __spirv_ocl_exp10
#define __CLC_BUILTIN __nv_exp10
#include "unary_builtin.inc"
