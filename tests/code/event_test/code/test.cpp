#include "test.h"

#include "CppUTest/TestHarness.h"
#include "video.h"

#include <bit>

TEST_GROUP (EventTest) {
  void setup() {}
  void teardown() {}
};

static void PrintEventData(OrbisKernelEvent* ev) {
  printf("ev->ident = 0x%08lx\n", ev->ident);
  printf("ev->filter = %hd\n", ev->filter);
  printf("ev->flags = %u\n", ev->flags);
  printf("ev->fflags = %u\n", ev->fflags);
  printf("ev->data = 0x%08lx\n", ev->data);
  if (ev->data != 0) {
    if (ev->filter == -13) {
      // Video out event, cast data appropriately.
      VideoOutEventData data = *reinterpret_cast<VideoOutEventData*>(&ev->data);
      printf("ev->data->time = %d\n", data.time);
      printf("ev->data->counter = %d\n", data.counter);
      printf("ev->data->flip_arg = 0x%08lx\n", data.flip_arg);
    } else {
      printf("*(ev->data) = 0x%08lx\n", *(u64*)ev->data);
    }
  }
  printf("ev->user_data = 0x%08lx\n", ev->user_data);
  if (ev->user_data != 0) {
    printf("*(ev->user_data) = 0x%08lx\n", *(u64*)ev->user_data);
  }
  printf("\n");
}

static void PrintFlipStatus(OrbisVideoOutFlipStatus* status) {
  printf("status->count = 0x%08lx\n", status->count);
  printf("status->process_time = %ld\n", status->process_time);
  printf("status->tsc_time = %ld\n", status->tsc_time);
  printf("status->flip_arg = 0x%08lx\n", status->flip_arg);
  printf("status->submit_tsc = %ld\n", status->submit_tsc);
  printf("status->num_gpu_flip_pending = %d\n", status->num_gpu_flip_pending);
  printf("status->num_flip_pending = %d\n", status->num_flip_pending);
  printf("status->current_buffer = %d\n\n", status->current_buffer);
}

TEST(EventTest, UserEventTest) {
  // Need to test some equeue behavior.
  // Start by creating an equeue.
  OrbisKernelEqueue eq {};
  s32               result = sceKernelCreateEqueue(&eq, "TestEqueue");
  UNSIGNED_INT_EQUALS(0, result);

  // Add a user event to this equeue, use id 32.
  result = sceKernelAddUserEvent(eq, 32);
  UNSIGNED_INT_EQUALS(0, result);

  // Trigger the event
  u64 data1 = 100;
  result    = sceKernelTriggerUserEvent(eq, 32, &data1);
  UNSIGNED_INT_EQUALS(0, result);

  // Run sceKernelWaitEqueue to detect the returned event.
  OrbisKernelEvent ev {};
  s32              count {};
  result = sceKernelWaitEqueue(eq, &ev, 1, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Check returned data
  PrintEventData(&ev);
  CHECK_EQUAL(32, ev.ident);
  CHECK_EQUAL(-11, ev.filter);
  CHECK_EQUAL(0, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data == 0);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(100, *(u64*)ev.user_data);

  // Run sceKernelWaitEqueue to detect the returned event.
  // Since user events don't clear, this will return the data from the first trigger.
  memset(&ev, 0, sizeof(ev));
  count  = 0;
  result = sceKernelWaitEqueue(eq, &ev, 1, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Check returned data
  PrintEventData(&ev);
  CHECK_EQUAL(32, ev.ident);
  CHECK_EQUAL(-11, ev.filter);
  CHECK_EQUAL(0, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data == 0);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(100, *(u64*)ev.user_data);

  // Re-trigger the event, this should update userdata
  u64 data2 = 200;
  result    = sceKernelTriggerUserEvent(eq, 32, &data2);
  UNSIGNED_INT_EQUALS(0, result);

  // Run sceKernelWaitEqueue to detect the returned event.
  memset(&ev, 0, sizeof(ev));
  count  = 0;
  result = sceKernelWaitEqueue(eq, &ev, 1, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Check returned data
  PrintEventData(&ev);
  CHECK_EQUAL(32, ev.ident);
  CHECK_EQUAL(-11, ev.filter);
  CHECK_EQUAL(0, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data == 0);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(200, *(u64*)ev.user_data);

  // Run sceKernelWaitEqueue to detect the returned event.
  memset(&ev, 0, sizeof(ev));
  count = 0;
  // Succeeds, but only returns 1 event, since there's only one triggered event to return.
  result = sceKernelWaitEqueue(eq, &ev, 2, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Check returned data
  PrintEventData(&ev);
  CHECK_EQUAL(32, ev.ident);
  CHECK_EQUAL(-11, ev.filter);
  CHECK_EQUAL(0, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data == 0);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(200, *(u64*)ev.user_data);

  // Delete the user event.
  result = sceKernelDeleteUserEvent(eq, 32);
  UNSIGNED_INT_EQUALS(0, result);

  // Add a "user event edge", these are user events with the clear flag.
  // Presumably, these will not trigger multiple times.
  // Add a user event to this equeue, use id 32.
  result = sceKernelAddUserEventEdge(eq, 32);
  UNSIGNED_INT_EQUALS(0, result);

  // Trigger the event
  result = sceKernelTriggerUserEvent(eq, 32, &data1);
  UNSIGNED_INT_EQUALS(0, result);

  // Run sceKernelWaitEqueue to detect the returned event.
  memset(&ev, 0, sizeof(ev));
  count  = 0;
  result = sceKernelWaitEqueue(eq, &ev, 1, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Check returned data
  PrintEventData(&ev);
  CHECK_EQUAL(32, ev.ident);
  CHECK_EQUAL(-11, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data == 0);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(100, *(u64*)ev.user_data);

  // Run sceKernelWaitEqueue to detect the returned event.
  // Due to the clear flag, this should time out.
  memset(&ev, 0, sizeof(ev));
  count       = 0;
  u32 timeout = 100;
  result      = sceKernelWaitEqueue(eq, &ev, 1, &count, &timeout);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Re-trigger the event, this should update userdata
  result = sceKernelTriggerUserEvent(eq, 32, &data2);
  UNSIGNED_INT_EQUALS(0, result);

  // Run sceKernelWaitEqueue to detect the returned event.
  memset(&ev, 0, sizeof(ev));
  count  = 0;
  result = sceKernelWaitEqueue(eq, &ev, 1, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Check returned data
  PrintEventData(&ev);
  CHECK_EQUAL(32, ev.ident);
  CHECK_EQUAL(-11, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data == 0);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(200, *(u64*)ev.user_data);

  // Run sceKernelWaitEqueue to detect the returned event.
  memset(&ev, 0, sizeof(ev));
  count   = 0;
  timeout = 100;
  // Fails, since there isn't any event to return.
  result = sceKernelWaitEqueue(eq, &ev, 2, &count, &timeout);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Trigger the event twice, check behavior.
  result = sceKernelTriggerUserEvent(eq, 32, &data1);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelTriggerUserEvent(eq, 32, &data2);
  UNSIGNED_INT_EQUALS(0, result);

  // Run sceKernelWaitEqueue to detect the returned event.
  memset(&ev, 0, sizeof(ev));
  count  = 0;
  result = sceKernelWaitEqueue(eq, &ev, 1, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Check returned data
  PrintEventData(&ev);
  CHECK_EQUAL(32, ev.ident);
  CHECK_EQUAL(-11, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data == 0);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(200, *(u64*)ev.user_data);

  // Even though we trigger twice, this call will fail.
  memset(&ev, 0, sizeof(ev));
  count   = 0;
  timeout = 100;
  result  = sceKernelWaitEqueue(eq, &ev, 1, &count, &timeout);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Delete the user event.
  result = sceKernelDeleteUserEvent(eq, 32);
  UNSIGNED_INT_EQUALS(0, result);

  // Delete the equeue when tests complete.
  result = sceKernelDeleteEqueue(eq);
  UNSIGNED_INT_EQUALS(0, result);
}

TEST(EventTest, FlipEventTest) {
  // Test video out flip events.
  // First we need to properly open libSceVideoOut
  VideoOut* handle = new VideoOut(1920, 1080);

  // Register buffers
  handle->addBuffer();
  handle->addBuffer();
  handle->addBuffer();

  OrbisVideoOutFlipStatus status {};
  s32 result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);

  // Create a flip event
  result = handle->addFlipEvent(nullptr);
  UNSIGNED_INT_EQUALS(0, result);

  // Not sure what we're dealing with, so from here, start logging info.
  PrintFlipStatus(&status);

  // Perform a flip
  result = handle->flipFrame(0x100);
  UNSIGNED_INT_EQUALS(0, result);

  // Now we can wait on the flip event equeue prepared earlier.
  OrbisKernelEvent ev{};
  memset(&ev, 0, sizeof(ev));
  s32 count  = 0;
  result = handle->waitFlipEvent(&ev, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Check returned data
  PrintEventData(&ev);
  CHECK_EQUAL(0x6000000000000, ev.ident);
  CHECK_EQUAL(-13, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data != 0);
  VideoOutEventData ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  CHECK_EQUAL(1, ev_data.counter);
  CHECK_EQUAL(0x100, ev_data.flip_arg);

  // Flip events only trigger once.
  memset(&ev, 0, sizeof(ev));
  count   = 0;
  u32 timeout = 1000;
  result  = handle->waitFlipEvent(&ev, &count, &timeout);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Check flip status
  memset(&status, 0, sizeof(status));
  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);

  PrintFlipStatus(&status);

  // Perform a flip
  result = handle->flipFrame(0x200);
  UNSIGNED_INT_EQUALS(0, result);

  // Now we can wait on the flip event equeue prepared earlier.
  memset(&ev, 0, sizeof(ev));
  count  = 0;
  result = handle->waitFlipEvent(&ev, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Check returned data
  PrintEventData(&ev);
  CHECK_EQUAL(0x6000000000000, ev.ident);
  CHECK_EQUAL(-13, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data != 0);
  ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  CHECK_EQUAL(1, ev_data.counter);
  CHECK_EQUAL(0x200, ev_data.flip_arg);

  // Flip events only trigger once.
  memset(&ev, 0, sizeof(ev));
  count   = 0;
  timeout = 1000;
  result  = handle->waitFlipEvent(&ev, &count, &timeout);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Check flip status
  memset(&status, 0, sizeof(status));
  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);

  PrintFlipStatus(&status);

  // Fire two video out events in quick succession
  result = handle->flipFrame(0x100);
  UNSIGNED_INT_EQUALS(0, result);

  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);
  PrintFlipStatus(&status);

  result = handle->flipFrame(0x300);
  UNSIGNED_INT_EQUALS(0, result);

  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);
  PrintFlipStatus(&status);

  // Now we can wait on the flip event equeue prepared earlier.
  memset(&ev, 0, sizeof(ev));
  count  = 0;
  result = handle->waitFlipEvent(&ev, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Check returned data
  PrintEventData(&ev);
  CHECK_EQUAL(0x6000000000000, ev.ident);
  CHECK_EQUAL(-13, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data != 0);
  ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  CHECK_EQUAL(1, ev_data.counter);
  CHECK_EQUAL(0x100, ev_data.flip_arg);

  // Check flip status
  memset(&status, 0, sizeof(status));
  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);

  PrintFlipStatus(&status);

  // We did two submits, so the video out event should fire again.
  memset(&ev, 0, sizeof(ev));
  count  = 0;
  result = handle->waitFlipEvent(&ev, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Check returned data
  PrintEventData(&ev);
  CHECK_EQUAL(0x6000000000000, ev.ident);
  CHECK_EQUAL(-13, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data != 0);
  ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  CHECK_EQUAL(1, ev_data.counter);
  CHECK_EQUAL(0x300, ev_data.flip_arg);

  // Check flip status
  memset(&status, 0, sizeof(status));
  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);

  PrintFlipStatus(&status);

  // Fire two video out events in quick succession
  result = handle->flipFrame(0x400);
  UNSIGNED_INT_EQUALS(0, result);
  result = handle->flipFrame(0x500);
  UNSIGNED_INT_EQUALS(0, result);

  // Use sceVideoOutGetFlipStatus to wait for both flips to complete
  memset(&status, 0, sizeof(status));
  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);
  while (status.num_flip_pending != 0) {
    sceKernelUsleep(1000);

    memset(&status, 0, sizeof(status));
    result = handle->getStatus(&status);
    UNSIGNED_INT_EQUALS(0, result);
  }

  // Both flips are done, check status and event
  PrintFlipStatus(&status);

  // Check returned data
  memset(&ev, 0, sizeof(ev));
  count  = 0;
  result = handle->waitFlipEvent(&ev, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  PrintEventData(&ev);
  CHECK_EQUAL(0x6000000000000, ev.ident);
  CHECK_EQUAL(-13, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data != 0);
  ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  // Counter is how many times the event was triggered.
  CHECK_EQUAL(2, ev_data.counter);
  CHECK_EQUAL(0x500, ev_data.flip_arg);

  // Shouldn't trigger again.
  memset(&ev, 0, sizeof(ev));
  count  = 0;
  result = handle->waitFlipEvent(&ev, &count, &timeout);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Now test EOP flips
  result = handle->submitAndFlip(0x1000);
  UNSIGNED_INT_EQUALS(0, result);
  result = handle->submitAndFlip(0x2000);
  UNSIGNED_INT_EQUALS(0, result);
  result = handle->submitAndFlip(0x3000);
  UNSIGNED_INT_EQUALS(0, result);

  // Print status
  do {
    memset(&status, 0, sizeof(status));
    result = handle->getStatus(&status);
    PrintFlipStatus(&status);
    UNSIGNED_INT_EQUALS(0, result);

    sceKernelUsleep(1000);
  } while (status.num_flip_pending != 0);

  PrintFlipStatus(&status);

  // Wait for EOP flip to occur.
  memset(&ev, 0, sizeof(ev));
  count  = 0;
  result = handle->waitFlipEvent(&ev, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  PrintEventData(&ev);
  CHECK_EQUAL(0x6000000000000, ev.ident);
  CHECK_EQUAL(-13, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data != 0);
  ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  // Counter is how many times the event was triggered.
  CHECK_EQUAL(3, ev_data.counter);
  CHECK_EQUAL(0x3000, ev_data.flip_arg);

  // Print status again.
  memset(&status, 0, sizeof(status));
  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);
  PrintFlipStatus(&status);

  result = handle->submitAndFlip(0x30000000000);
  UNSIGNED_INT_EQUALS(0, result);

  // Wait for EOP flip to occur.
  memset(&ev, 0, sizeof(ev));
  count  = 0;
  result = handle->waitFlipEvent(&ev, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  PrintEventData(&ev);
  CHECK_EQUAL(0x6000000000000, ev.ident);
  CHECK_EQUAL(-13, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data != 0);
  ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  // Counter is how many times the event was triggered.
  CHECK_EQUAL(3, ev_data.counter);
  CHECK_EQUAL(0x30000000000, ev_data.flip_arg);

  // Print status again.
  memset(&status, 0, sizeof(status));
  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);
  PrintFlipStatus(&status);

  // Clean up after test
  delete (handle);
}
