#pragma once

#include "test.h"

#include <vector>

class VideoOut {
  private:
  s32                              handle {0};
  s32                              width {0};
  s32                              height {0};
  std::vector<std::pair<s64, u64>> phys_bufs {};
  OrbisKernelEqueue                flip_queue {nullptr};
  bool                             flip_event  = false;
  s32                              current_buf = -1;

  public:
  VideoOut(s32 width, s32 height) {
    this->width  = width;
    this->height = height;
    // Open VideoOut handle
    this->handle = sceVideoOutOpen(255, 0, 0, nullptr);
    // Set flip rate to 60fps
    s32 result = sceVideoOutSetFlipRate(this->handle, 0);
    UNSIGNED_INT_EQUALS(0, result);
    // Create flip equeue
    result = sceKernelCreateEqueue(&flip_queue, "VideoOutFlipEqueue");
    UNSIGNED_INT_EQUALS(0, result);
  };

  // Manually define a destructor to close everything.
  ~VideoOut() {
    if (flip_event) {
      s32 result = sceVideoOutDeleteFlipEvent(flip_queue, handle);
      UNSIGNED_INT_EQUALS(0, result);
      flip_event = false;
    }
    if (flip_queue != nullptr) {
      s32 result = sceKernelDeleteEqueue(flip_queue);
      UNSIGNED_INT_EQUALS(0, result);
    }
    if (handle > 0) {
      s32 result = sceVideoOutClose(handle);
      UNSIGNED_INT_EQUALS(0, result);
      handle = 0;
    }
    for (auto& [phys_off, size]: phys_bufs) {
      s32 result = sceKernelReleaseDirectMemory(phys_off, size);
      UNSIGNED_INT_EQUALS(0, result);
    }
    phys_bufs.~vector();
  };

  s32 getStatus(OrbisVideoOutFlipStatus* status) { return sceVideoOutGetFlipStatus(handle, status); };

  s32 flipFrame(s64 flip_arg) {
    if (phys_bufs.size() == 0) {
      // Force a blank frame if no buffers are registered
      current_buf = -1;
    }
    s32 result = sceVideoOutSubmitFlip(handle, current_buf, 1, flip_arg);
    if (++current_buf == phys_bufs.size()) {
      current_buf = 0;
    }
    return result;
  };

  s32 submitAndFlip(s64 flip_arg) {
    if (phys_bufs.size() == 0) {
      // SubmitAndFlip fails on buffer -1, save time by failing early.
      return -1;
    }
    s64 cmd_dmem_ptr = 0;
    s32 result       = sceKernelAllocateMainDirectMemory(0x4000, 0x4000, 0, &cmd_dmem_ptr);
    UNSIGNED_INT_EQUALS(0, result);

    void* cmd_ptr = nullptr;
    result        = sceKernelMapDirectMemory(&cmd_ptr, 0x4000, 0x33, 0, cmd_dmem_ptr, 0x4000);
    UNSIGNED_INT_EQUALS(0, result);

    // Write GPU init packet to the pointer.
    u32* cmds       = (u32*)cmd_ptr;
    cmds += sceGnmDrawInitDefaultHardwareState350(cmds, 0x100);

    // Write a flip packet to the pointer.
    cmds[0] = 0xc03e1000;
    cmds[1] = 0x68750777;
    cmds += 64;
    u32 stream_size = (u32)((u64)cmds - (u64)cmd_ptr);

    // GPU flips are not allowed on buffer -1.
    result = sceGnmSubmitAndFlipCommandBuffers(1, &cmd_ptr, &stream_size, nullptr, nullptr, handle, current_buf, 1, flip_arg);
    if (++current_buf == phys_bufs.size()) {
      current_buf = 0;
    }
    return result;
  };

  s32 addBuffer() {
    // Create a buffer attribute
    OrbisVideoOutBufferAttribute attr {};
    attr.pixel_format   = 0x80002200;
    attr.tiling_mode    = 0;
    attr.aspect_ratio   = 0;
    attr.width          = width;
    attr.height         = height;
    attr.pitch_in_pixel = width;
    attr.option         = 0;
    attr.reserved0      = 0;
    attr.reserved1      = 0;

    // calc some stuff, logic taken from red_prig's shader test homebrew.
    u64 pitch        = (attr.width + 127) / 128;
    u64 pad_width    = pitch * 128;
    u64 pad_height   = ((attr.height + 63) & (~63));
    u64 size         = pad_width * pad_height * 4;
    u64 aligned_size = (size + 16 * 1024 - 1) & ~(16 * 1024 - 1);

    // Map memory for buffer.
    s64 dmem_addr = 0;
    s32 result    = sceKernelAllocateMainDirectMemory(aligned_size, 64 * 1024, 3, &dmem_addr);
    UNSIGNED_INT_EQUALS(0, result);
    void* addr = nullptr;
    result     = sceKernelMapDirectMemory(&addr, aligned_size, 0x33, 0, dmem_addr, 64 * 1024);
    UNSIGNED_INT_EQUALS(0, result);
    memset(addr, 0, aligned_size);

    // Add buffer info to the buffers list
    phys_bufs.emplace_back(dmem_addr, aligned_size);

    // Register buffer
    return sceVideoOutRegisterBuffers(handle, phys_bufs.size() - 1, &addr, 1, &attr);
  };

  s32 removeBuffers() {
    current_buf = -1;
    return sceVideoOutUnregisterBuffers(handle, 0);
  }

  s32 addFlipEvent(void* user_data) {
    s32 result = sceVideoOutAddFlipEvent(flip_queue, handle, user_data);
    flip_event = true;
    return result;
  };

  s32 deleteFlipEvent() {
    s32 result = sceVideoOutDeleteFlipEvent(flip_queue, handle);
    flip_event = false;
    return result;
  };

  s32 waitFlipEvent(OrbisKernelEvent* ev, s32* out, u32* timeout) { return sceKernelWaitEqueue(flip_queue, ev, 1, out, timeout); };
};