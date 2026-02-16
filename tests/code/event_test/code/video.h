#pragma once

#include "test.h"

#include <vector>

class VideoOut {
  private:
  // Video out data
  s32 handle      = 0;
  s32 width       = 0;
  s32 height      = 0;
  s32 current_buf = -1;

  // Buffer allocations
  void* buf_addr  = nullptr;
  s64   buf_paddr = 0;
  u64   buf_size  = 0;
  u64   buf_count = 0;

  // Equeue
  OrbisKernelEqueue flip_queue   = nullptr;
  OrbisKernelEqueue vblank_queue = nullptr;

  // Command buffer for EOP tests
  void* cmd_buf          = nullptr;
  s64   phys_cmd_buf     = 0;
  u64   cmd_buf_size     = 0x4000;
  u32   stream_size      = 0;
  u32   cmd_start_offset = 0;

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

    // Create vblank equeue
    result = sceKernelCreateEqueue(&vblank_queue, "VideoOutVblankEqueue");
    UNSIGNED_INT_EQUALS(0, result);

    // Initialize command buffer for EOP flip tests
    result = sceKernelAllocateMainDirectMemory(cmd_buf_size, 0x4000, 0, &phys_cmd_buf);
    UNSIGNED_INT_EQUALS(0, result);

    result = sceKernelMapDirectMemory(&cmd_buf, cmd_buf_size, 0x33, 0, phys_cmd_buf, 0x4000);
    UNSIGNED_INT_EQUALS(0, result);

    // Map a memory area for video out buffers
    // Calculate necessary buffer size, logic is taken from red_prig's shader test homebrew
    u64 pitch      = (width + 127) / 128;
    u64 pad_width  = pitch * 128;
    u64 pad_height = ((height + 63) & (~63));
    u64 size       = pad_width * pad_height * 4;
    buf_size       = (size + 16 * 1024 - 1) & ~(16 * 1024 - 1);

    // Perform the actual memory mapping
    result = sceKernelAllocateMainDirectMemory(buf_size, 0x10000, 3, &buf_paddr);
    UNSIGNED_INT_EQUALS(0, result);
    result = sceKernelMapDirectMemory(&buf_addr, buf_size, 0x33, 0, buf_paddr, 0x10000);
    UNSIGNED_INT_EQUALS(0, result);
    memset(buf_addr, 0, buf_size);
  };

  // Manually define a destructor to close everything.
  ~VideoOut() {
    // Delete flip event queue
    if (flip_queue != nullptr) {
      s32 result = sceKernelDeleteEqueue(flip_queue);
      UNSIGNED_INT_EQUALS(0, result);
    }

    // Delete vblank event queue
    if (vblank_queue != nullptr) {
      s32 result = sceKernelDeleteEqueue(vblank_queue);
      UNSIGNED_INT_EQUALS(0, result);
    }

    // Close video out handle
    if (handle > 0) {
      s32 result = sceVideoOutClose(handle);
      UNSIGNED_INT_EQUALS(0, result);
      handle = 0;
    }

    // Free command buffer allocation
    if (cmd_buf != nullptr) {
      s32 result = sceKernelReleaseDirectMemory(phys_cmd_buf, cmd_buf_size);
      UNSIGNED_INT_EQUALS(0, result);
    }

    // Free video out buffer allocation
    if (buf_addr != nullptr) {
      s32 result = sceKernelReleaseDirectMemory(buf_paddr, buf_size);
      UNSIGNED_INT_EQUALS(0, result);
    }
  };

  s32 getStatus(OrbisVideoOutFlipStatus* status) {
    memset(status, 0, sizeof(OrbisVideoOutFlipStatus));
    return sceVideoOutGetFlipStatus(handle, status);
  };

  s32 flipFrame(s64 flip_arg) {
    if (buf_count == 0) {
      // Force a blank frame if no buffers are registered
      current_buf = -1;
    }
    s32 result = sceVideoOutSubmitFlip(handle, current_buf, 1, flip_arg);
    if (++current_buf == buf_count) {
      current_buf = 0;
    }
    return result;
  };

  s32 submitAndFlip(s64 flip_arg) {
    if (buf_count == 0) {
      // SubmitAndFlip fails on buffer -1, save time by failing early.
      return -1;
    }

    // Write GPU init packet to the pointer.
    void* cmd_buf_start = (void*)((u64)cmd_buf + cmd_start_offset);
    u32*  cmds          = (u32*)cmd_buf_start;
    cmds += sceGnmDrawInitDefaultHardwareState350(cmds, 0x100);

    // Write a flip packet to the pointer.
    cmds[0] = 0xc03e1000;
    cmds[1] = 0x68750777;
    cmds += 64;
    stream_size = (u32)((u64)cmds - (u64)cmd_buf_start);

    cmd_start_offset += stream_size;
    if (cmd_start_offset + stream_size > cmd_buf_size) {
      cmd_start_offset = 0;
    }

    // Perform GPU submit
    s32 result = sceGnmSubmitAndFlipCommandBuffers(1, &cmd_buf_start, &stream_size, nullptr, nullptr, handle, current_buf, 1, flip_arg);
    if (++current_buf == buf_count) {
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

    // Register buffer
    s32 result = sceVideoOutRegisterBuffers(handle, buf_count, &buf_addr, 1, &attr);

    if (result >= 0) {
      // Buffer registered successfully, increment buffers count.
      buf_count++;
    }

    return result;
  };

  s32 removeBuffers() {
    current_buf = -1;
    return sceVideoOutUnregisterBuffers(handle, 0);
  }

  s32 addFlipEvent(void* user_data) { return sceVideoOutAddFlipEvent(flip_queue, handle, user_data); };

  s32 deleteFlipEvent() { return sceVideoOutDeleteFlipEvent(flip_queue, handle); };

  s32 waitFlipEvent(OrbisKernelEvent* ev, s32 num, s32* out, u32 timeout) {
    memset(ev, 0, sizeof(OrbisKernelEvent) * num);
    *out = 0;
    if (timeout == -1) {
      return sceKernelWaitEqueue(flip_queue, ev, num, out, nullptr);
    } else {
      return sceKernelWaitEqueue(flip_queue, ev, num, out, &timeout);
    }
  };

  s32 addVblankEvent(void* user_data) { return sceVideoOutAddVblankEvent(vblank_queue, handle, user_data); }

  s32 deleteVblankEvent() { return sceVideoOutDeleteVblankEvent(vblank_queue, handle); }

  s32 waitVblankEvent(OrbisKernelEvent* ev, s32 num, s32* out, u32 timeout) {
    memset(ev, 0, sizeof(OrbisKernelEvent) * num);
    *out = 0;
    if (timeout == -1) {
      return sceKernelWaitEqueue(vblank_queue, ev, num, out, nullptr);
    } else {
      return sceKernelWaitEqueue(vblank_queue, ev, num, out, &timeout);
    }
  };

  s32 waitFlip() {
    // Wait for flip
    s32 result = sceVideoOutIsFlipPending(handle);
    while (result > 0) {
      sceKernelUsleep(10000);
      result = sceVideoOutIsFlipPending(handle);
    };
    return result;
  };

  s32 waitVblank() { return sceVideoOutWaitVblank(handle); };
};