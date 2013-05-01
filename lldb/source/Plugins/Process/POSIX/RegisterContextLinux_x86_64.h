//===-- RegisterContextLinux_x86_64.h ---------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_RegisterContextLinux_x86_64_H_
#define liblldb_RegisterContextLinux_x86_64_H_

#include "Plugins/Process/POSIX/RegisterContext_x86_64.h"

using namespace lldb_private;

class RegisterContextLinux_x86_64:
    public RegisterContext_x86_64
{
public:
    RegisterContextLinux_x86_64(Thread &thread, uint32_t concrete_frame_idx);
    virtual ~RegisterContextLinux_x86_64();

    size_t GetGPRSize();

protected:
    virtual const lldb_private::RegisterInfo *
    GetRegisterInfo();

private:
    static lldb_private::RegisterInfo *m_register_infos;
    void UpdateRegisterInfo();
};

#endif
