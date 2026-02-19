#pragma once

#include "CppUTest/TestHarness.h"

#define UNSIGNED_INT_EQUALS(expected, actual) UNSIGNED_LONGS_EQUAL_LOCATION((u32)expected, (u32)actual, NULLPTR, __FILE__, __LINE__)

using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

extern "C" {

typedef void* OrbisKernelEqueue;

struct VideoOutEventData {
  u64 time     : 12;
  u64 counter  : 4;
  u64 flip_arg : 48;
};

struct OrbisKernelEvent {
  u64 ident;
  s16 filter;
  u16 flags;
  u32 fflags;
  u64 data;
  u64 user_data;
};

constexpr s32 ORBIS_KERNEL_ERROR_ENOENT    = 0x80020002;
constexpr s32 ORBIS_KERNEL_ERROR_ETIMEDOUT = 0x8002003c;

s32 sceKernelUsleep(u32);
s32 sceKernelCreateEqueue(OrbisKernelEqueue* eq, const char* name);
s32 sceKernelDeleteEqueue(OrbisKernelEqueue eq);
s32 sceKernelWaitEqueue(OrbisKernelEqueue eq, OrbisKernelEvent* ev, s32 num, s32* out, u32* timeout);
s32 sceKernelAddUserEvent(OrbisKernelEqueue eq, s32 id);
s32 sceKernelAddUserEventEdge(OrbisKernelEqueue eq, s32 id);
s32 sceKernelTriggerUserEvent(OrbisKernelEqueue eq, s32 id, void* user_data);
s32 sceKernelDeleteUserEvent(OrbisKernelEqueue eq, s32 id);
s32 sceKernelAddTimerEvent(OrbisKernelEqueue eq, s32 id, u32 time, void* user_data);
s32 sceKernelDeleteTimerEvent(OrbisKernelEqueue eq, s32 id);

s32 sceKernelAllocateMainDirectMemory(u64 size, u64 align, s32 mtype, s64* phys_out);
s32 sceKernelMapDirectMemory(void** addr, u64 size, s32 prot, s32 flags, s64 offset, u64 align);
s32 sceKernelReleaseDirectMemory(s64 phys_addr, u64 size);

struct OrbisVideoOutFlipStatus {
  u64 count;
  u64 process_time;
  u64 tsc_time;
  s64 flip_arg;
  u64 submit_tsc;
  u64 reserved0;
  s32 num_gpu_flip_pending;
  s32 num_flip_pending;
  s32 current_buffer;
  u32 reserved1;
};

struct OrbisVideoOutBufferAttribute {
  u32 pixel_format;
  s32 tiling_mode;
  s32 aspect_ratio;
  u32 width;
  u32 height;
  u32 pitch_in_pixel;
  u32 option;
  u32 reserved0;
  u64 reserved1;
};

s32 sceVideoOutOpen(s32 user_id, s32 bus_type, s32 index, const void* param);
s32 sceVideoOutSetFlipRate(s32 handle, s32 flip_rate);
s32 sceVideoOutAddFlipEvent(OrbisKernelEqueue eq, s32 handle, void* user_data);
s32 sceVideoOutDeleteFlipEvent(OrbisKernelEqueue eq, s32 handle);
s32 sceVideoOutAddVblankEvent(OrbisKernelEqueue eq, s32 handle, void* user_data);
s32 sceVideoOutDeleteVblankEvent(OrbisKernelEqueue eq, s32 handle);
s32 sceVideoOutRegisterBuffers(s32 handle, s32 index, void* const* addrs, s32 buf_num, OrbisVideoOutBufferAttribute* attrs);
s32 sceVideoOutUnregisterBuffers(s32 handle, s32 attribute_index);
s32 sceVideoOutSubmitFlip(s32 handle, s32 buf_index, s32 flip_mode, s64 flip_arg);
s32 sceVideoOutGetFlipStatus(s32 handle, OrbisVideoOutFlipStatus* status);
s32 sceVideoOutIsFlipPending(s32 handle);
s32 sceVideoOutWaitVblank(s32 handle);
s32 sceVideoOutClose(s32 handle);

u32 sceGnmDrawInitDefaultHardwareState350(u32* cmd_buf, u32 num_dwords);
s32 sceGnmSubmitCommandBuffers(u32 count, void** dcb_gpu_addrs, u32* dcb_sizes_in_bytes, void** ccb_gpu_addrs, u32* ccb_sizes_in_bytes);
s32 sceGnmSubmitAndFlipCommandBuffers(u32 count, void** dcb_gpu_addrs, u32* dcb_sizes_in_bytes, void** ccb_gpu_addrs, u32* ccb_sizes_in_bytes, s32 video_handle,
                                      s32 buffer_index, s32 flip_mode, s64 flip_arg);
s32 sceGnmAddEqEvent(OrbisKernelEqueue eq, s32 event_id, void* user_data);
s32 sceGnmDeleteEqEvent(OrbisKernelEqueue eq, s32 event_id);
s32 sceGnmSubmitDone();
}