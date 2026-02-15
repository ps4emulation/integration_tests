#pragma once

#include "test.h"

#include <vector>

class VideoOut {
  private:
  // Video out data
  s32 handle;
  s32 width;
  s32 height;
  s32 current_buf = -1;

  // Buffer allocations
  std::vector<std::pair<s64, u64>> phys_bufs;

  // Equeue
  OrbisKernelEqueue flip_queue = nullptr;
  bool              flip_event = false;

  // Command buffer for EOP tests
  void* cmd_buf = nullptr;
  s64   phys_cmd_buf;
  u32   stream_size;

  public:
  VideoOut(s32 width, s32 height) {
    // Store requested frame width and height
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

    // Initialize command buffer for EOP flip tests
    result = sceKernelAllocateMainDirectMemory(0x4000, 0x4000, 0, &phys_cmd_buf);
    UNSIGNED_INT_EQUALS(0, result);

    result = sceKernelMapDirectMemory(&cmd_buf, 0x4000, 0x33, 0, phys_cmd_buf, 0x4000);
    UNSIGNED_INT_EQUALS(0, result);
  };

  // Manually define a destructor to close everything.
  ~VideoOut() {
    // Delete event queue
    if (flip_queue != nullptr) {
      s32 result = sceKernelDeleteEqueue(flip_queue);
      UNSIGNED_INT_EQUALS(0, result);
    }

    // Close video out handle
    if (handle > 0) {
      s32 result = sceVideoOutClose(handle);
      UNSIGNED_INT_EQUALS(0, result);
      handle = 0;
    }

    // Free allocations for video out buffers
    for (auto& [phys_off, size]: phys_bufs) {
      s32 result = sceKernelReleaseDirectMemory(phys_off, size);
      UNSIGNED_INT_EQUALS(0, result);
    }

    // Manually destruct vector to clean memory footprint
    phys_bufs.~vector();

    // Free command buffer allocation
    if (cmd_buf != nullptr) {
      s32 result = sceKernelReleaseDirectMemory(phys_cmd_buf, 0x4000);
      UNSIGNED_INT_EQUALS(0, result);
    }
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

    // Clear memory for command buffer
    memset(cmd_buf, 0, 0x4000);

    // Write GPU init packet to the pointer.
    u32* cmds = (u32*)cmd_buf;
    cmds += sceGnmDrawInitDefaultHardwareState350(cmds, 0x100);

    // Write a flip packet to the pointer.
    cmds[0] = 0xc03e1000;
    cmds[1] = 0x68750777;
    cmds += 64;
    stream_size = (u32)((u64)cmds - (u64)cmd_buf);

    // Perform GPU submit
    s32 result = sceGnmSubmitAndFlipCommandBuffers(1, &cmd_buf, &stream_size, nullptr, nullptr, handle, current_buf, 1, flip_arg);
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