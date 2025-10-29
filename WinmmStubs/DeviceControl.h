#pragma once
#include "GlobalDefinitions.h"
#include "DeviceInfo.h"

// Public CDA Controller Entries
namespace Device {
    MMRESULT Open   (DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam);
    MMRESULT Play   (DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam);
    MMRESULT Stop   (DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam);
    MMRESULT Pause  (DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam);
    MMRESULT Resume (DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam);
    MMRESULT Seek   (DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam);
    MMRESULT Set    (DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam);
    MMRESULT Status (DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam);
    MMRESULT Info   (DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam);
    MMRESULT SysInfo(DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam);
    MMRESULT DevCaps(DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam);
    MMRESULT Close  (DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam);
}
