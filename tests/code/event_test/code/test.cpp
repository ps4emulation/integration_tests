#include "test.h"

#include "CppUTest/TestHarness.h"
#include "video.h"

#include <bit>

TEST_GROUP (EventTest) {
  void setup() {}
  void teardown() {}
};

static void PrintEventData(OrbisKernelEvent* ev) {
  printf("ev->ident = 0x%lx\n", ev->ident);
  printf("ev->filter = %hd\n", ev->filter);
  printf("ev->flags = %u\n", ev->flags);
  printf("ev->fflags = %u\n", ev->fflags);
  printf("ev->data = 0x%lx\n", ev->data);
  if (ev->data != 0) {
    if (ev->filter == -13) {
      // Video out event, cast data appropriately.
      VideoOutEventData data = *reinterpret_cast<VideoOutEventData*>(&ev->data);
      printf("ev->data->time = %d\n", data.time);
      printf("ev->data->counter = %d\n", data.counter);
      printf("ev->data->flip_arg = 0x%lx\n", data.flip_arg);
    }
  }
  printf("ev->user_data = 0x%lx\n", ev->user_data);
  if (ev->user_data != 0) {
    printf("*(ev->user_data) = 0x%lx\n", *(u64*)ev->user_data);
  }
  printf("\n");
}

static void PrintFlipStatus(OrbisVideoOutFlipStatus* status) {
  printf("status->count = 0x%lx\n", status->count);
  printf("status->process_time = %ld\n", status->process_time);
  printf("status->tsc_time = %ld\n", status->tsc_time);
  printf("status->flip_arg = 0x%lx\n", status->flip_arg);
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
  // For these, trigger state resets every time it's returned.
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

  // Add a second user event edge, this time with id 64
  result = sceKernelAddUserEventEdge(eq, 64);
  UNSIGNED_INT_EQUALS(0, result);

  // Trigger both events
  result = sceKernelTriggerUserEvent(eq, 32, &data1);
  UNSIGNED_INT_EQUALS(0, result);
  result = sceKernelTriggerUserEvent(eq, 64, &data2);
  UNSIGNED_INT_EQUALS(0, result);

  // Now sceKernelWaitEqueue should return both events.
  OrbisKernelEvent evs[2];
  memset(evs, 0, sizeof(evs));
  count  = 0;
  result = sceKernelWaitEqueue(eq, evs, 2, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(2, count);

  // Check returned data for both events.
  PrintEventData(&evs[0]);
  if (evs[0].ident == 32) {
    CHECK_EQUAL(32, evs[0].ident);
    CHECK_EQUAL(-11, evs[0].filter);
    CHECK_EQUAL(32, evs[0].flags);
    CHECK_EQUAL(0, evs[0].fflags);
    CHECK(evs[0].data == 0);
    CHECK(evs[0].user_data != 0);
    CHECK_EQUAL(100, *(u64*)evs[0].user_data);
  } else {
    CHECK_EQUAL(64, evs[0].ident);
    CHECK_EQUAL(-11, evs[0].filter);
    CHECK_EQUAL(32, evs[0].flags);
    CHECK_EQUAL(0, evs[0].fflags);
    CHECK(evs[0].data == 0);
    CHECK(evs[0].user_data != 0);
    CHECK_EQUAL(200, *(u64*)evs[0].user_data);
  }
  PrintEventData(&evs[1]);
  if (evs[1].ident == 32) {
    CHECK_EQUAL(32, evs[1].ident);
    CHECK_EQUAL(-11, evs[1].filter);
    CHECK_EQUAL(32, evs[1].flags);
    CHECK_EQUAL(0, evs[1].fflags);
    CHECK(evs[1].data == 0);
    CHECK(evs[1].user_data != 0);
    CHECK_EQUAL(100, *(u64*)evs[1].user_data);
  } else {
    CHECK_EQUAL(64, evs[1].ident);
    CHECK_EQUAL(-11, evs[1].filter);
    CHECK_EQUAL(32, evs[1].flags);
    CHECK_EQUAL(0, evs[1].fflags);
    CHECK(evs[1].data == 0);
    CHECK(evs[1].user_data != 0);
    CHECK_EQUAL(200, *(u64*)evs[1].user_data);
  }

  // Now only trigger the second.
  result = sceKernelTriggerUserEvent(eq, 64, &data1);
  UNSIGNED_INT_EQUALS(0, result);

  // Only the second event should return.
  memset(evs, 0, sizeof(evs));
  count  = 0;
  result = sceKernelWaitEqueue(eq, evs, 2, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  PrintEventData(&evs[0]);
  CHECK_EQUAL(64, evs[0].ident);
  CHECK_EQUAL(-11, evs[0].filter);
  CHECK_EQUAL(32, evs[0].flags);
  CHECK_EQUAL(0, evs[0].fflags);
  CHECK(evs[0].data == 0);
  CHECK(evs[0].user_data != 0);
  CHECK_EQUAL(100, *(u64*)evs[0].user_data);

  // Delete the first user event.
  result = sceKernelDeleteUserEvent(eq, 32);
  UNSIGNED_INT_EQUALS(0, result);

  // The second user event should remain present and triggerable.
  result = sceKernelTriggerUserEvent(eq, 64, &data1);
  UNSIGNED_INT_EQUALS(0, result);

  // Only the second event should return.
  memset(evs, 0, sizeof(evs));
  count  = 0;
  result = sceKernelWaitEqueue(eq, evs, 2, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  PrintEventData(&evs[0]);
  CHECK_EQUAL(64, evs[0].ident);
  CHECK_EQUAL(-11, evs[0].filter);
  CHECK_EQUAL(32, evs[0].flags);
  CHECK_EQUAL(0, evs[0].fflags);
  CHECK(evs[0].data == 0);
  CHECK(evs[0].user_data != 0);
  CHECK_EQUAL(100, *(u64*)evs[0].user_data);

  // Delete the second user event.
  result = sceKernelDeleteUserEvent(eq, 64);
  UNSIGNED_INT_EQUALS(0, result);

  // You cannot have two events with the same ident and filter.
  result = sceKernelAddUserEventEdge(eq, 32);
  UNSIGNED_INT_EQUALS(0, result);
  // This call, despite succeeding, will do nothing.
  result = sceKernelAddUserEvent(eq, 32);
  UNSIGNED_INT_EQUALS(0, result);

  result = sceKernelTriggerUserEvent(eq, 32, &data1);
  UNSIGNED_INT_EQUALS(0, result);

  // Wait for event to trigger.
  memset(evs, 0, sizeof(evs));
  count  = 0;
  result = sceKernelWaitEqueue(eq, evs, 2, &count, nullptr);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  PrintEventData(&evs[0]);
  CHECK_EQUAL(32, evs[0].ident);
  CHECK_EQUAL(-11, evs[0].filter);
  CHECK_EQUAL(32, evs[0].flags);
  CHECK_EQUAL(0, evs[0].fflags);
  CHECK(evs[0].data == 0);
  CHECK(evs[0].user_data != 0);
  CHECK_EQUAL(100, *(u64*)evs[0].user_data);

  // Delete event
  result = sceKernelDeleteUserEvent(eq, 32);
  UNSIGNED_INT_EQUALS(0, result);
  // Further proof there's not a second event
  result = sceKernelDeleteUserEvent(eq, 32);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ENOENT, result);

  // Delete the equeue when tests complete.
  result = sceKernelDeleteEqueue(eq);
  UNSIGNED_INT_EQUALS(0, result);
}

TEST(EventTest, FlipEventTest) {
  // Test video out flip events.
  // First we need to properly open libSceVideoOut
  VideoOut* handle = new VideoOut(1920, 1080);

  // Register buffers
  s32 result = handle->addBuffer();
  UNSIGNED_INT_EQUALS(0, result);
  result = handle->addBuffer();
  UNSIGNED_INT_EQUALS(1, result);
  result = handle->addBuffer();
  UNSIGNED_INT_EQUALS(2, result);

  OrbisVideoOutFlipStatus status {};
  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);

  // Create a flip event
  u64 num = 1;
  result  = handle->addFlipEvent(&num);
  UNSIGNED_INT_EQUALS(0, result);

  // Not sure what we're dealing with, so from here, start logging info.
  PrintFlipStatus(&status);

  // Perform a flip
  result = handle->flipFrame(0x100);
  UNSIGNED_INT_EQUALS(0, result);

  // Now we can wait on the flip event equeue prepared earlier.
  OrbisKernelEvent ev {};
  s32              count = 0;
  result                 = handle->waitFlipEvent(&ev, 1, &count, -1);
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
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(1, *(u64*)ev.user_data);
  CHECK_EQUAL(1, ev_data.counter);
  CHECK_EQUAL(0x100, ev_data.flip_arg);

  // Flip events only trigger once.
  result = handle->waitFlipEvent(&ev, 1, &count, 1000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Check flip status
  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);

  PrintFlipStatus(&status);

  // Perform a flip
  result = handle->flipFrame(0x200);
  UNSIGNED_INT_EQUALS(0, result);

  // Now we can wait on the flip event equeue prepared earlier.
  count  = 0;
  result = handle->waitFlipEvent(&ev, 1, &count, -1);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Check returned data
  PrintEventData(&ev);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data != 0);
  ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(1, *(u64*)ev.user_data);
  CHECK_EQUAL(1, ev_data.counter);
  CHECK_EQUAL(0x200, ev_data.flip_arg);

  // Flip events only trigger once.
  result = handle->waitFlipEvent(&ev, 1, &count, 1000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Check flip status
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
  result = handle->waitFlipEvent(&ev, 1, &count, -1);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Check returned data
  PrintEventData(&ev);
  CHECK(ev.data != 0);
  ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  CHECK_EQUAL(1, ev_data.counter);
  CHECK_EQUAL(0x100, ev_data.flip_arg);

  // Check flip status
  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);

  PrintFlipStatus(&status);

  // We did two submits, so the video out event should fire again.
  result = handle->waitFlipEvent(&ev, 1, &count, -1);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Check returned data
  PrintEventData(&ev);
  CHECK(ev.data != 0);
  ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  CHECK_EQUAL(1, ev_data.counter);
  CHECK_EQUAL(0x300, ev_data.flip_arg);

  // Check flip status
  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);

  PrintFlipStatus(&status);

  // Fire two video out events in quick succession
  result = handle->flipFrame(0x400);
  UNSIGNED_INT_EQUALS(0, result);
  result = handle->flipFrame(0x500);
  UNSIGNED_INT_EQUALS(0, result);

  // Wait for all flips to occur
  result = handle->waitFlip();
  UNSIGNED_INT_EQUALS(0, result);

  // Both flips are done, check status and event
  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);
  PrintFlipStatus(&status);

  // Check returned data
  result = handle->waitFlipEvent(&ev, 1, &count, -1);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  PrintEventData(&ev);
  CHECK(ev.data != 0);
  ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  // Counter is how many times the event was triggered.
  CHECK_EQUAL(2, ev_data.counter);
  CHECK_EQUAL(0x500, ev_data.flip_arg);

  // Shouldn't trigger again.
  result = handle->waitFlipEvent(&ev, 1, &count, 1000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Now test EOP flips
  result = handle->submitAndFlip(0x1000);
  UNSIGNED_INT_EQUALS(0, result);
  result = handle->submitAndFlip(0x2000);
  UNSIGNED_INT_EQUALS(0, result);
  result = handle->submitAndFlip(0x3000);
  UNSIGNED_INT_EQUALS(0, result);

  // Wait for all flips to occur
  result = handle->waitFlip();
  UNSIGNED_INT_EQUALS(0, result);

  PrintFlipStatus(&status);

  // Wait for EOP flip to occur.
  result = handle->waitFlipEvent(&ev, 1, &count, -1);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  PrintEventData(&ev);
  CHECK_EQUAL(0x6000000000000, ev.ident);
  CHECK_EQUAL(-13, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK(ev.data != 0);
  ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(1, *(u64*)ev.user_data);
  CHECK_EQUAL(3, ev_data.counter);
  CHECK_EQUAL(0x3000, ev_data.flip_arg);

  // Print status again.
  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);
  PrintFlipStatus(&status);

  result = handle->submitAndFlip(0x30000000000);
  UNSIGNED_INT_EQUALS(0, result);

  // Wait for EOP flip to occur.
  result = handle->waitFlipEvent(&ev, 1, &count, -1);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  PrintEventData(&ev);
  CHECK(ev.data != 0);
  ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  CHECK_EQUAL(1, ev_data.counter);
  CHECK_EQUAL(0x30000000000, ev_data.flip_arg);

  // Print status again.
  result = handle->getStatus(&status);
  UNSIGNED_INT_EQUALS(0, result);
  PrintFlipStatus(&status);

  // Try adding a second flip event
  s64 num2 = 2;
  result   = handle->addFlipEvent(&num2);
  UNSIGNED_INT_EQUALS(0, result);

  // Trigger a flip, then wait for two events with no timeout.
  result = handle->flipFrame(0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Wait for all flips to occur
  result = handle->waitFlip();
  UNSIGNED_INT_EQUALS(0, result);

  OrbisKernelEvent events[2];
  result = handle->waitFlipEvent(events, 2, &count, 10000);
  UNSIGNED_INT_EQUALS(0, result);
  // Despite adding another flip event, we still only get the one event.
  CHECK_EQUAL(1, count);

  // Print the event data.
  PrintEventData(&events[0]);

  CHECK(events[0].data != 0);
  ev_data = *reinterpret_cast<VideoOutEventData*>(&events[0].data);
  // The user data matches the new event
  CHECK(events[0].user_data != 0);
  CHECK_EQUAL(2, *(u64*)events[0].user_data);
  CHECK_EQUAL(1, ev_data.counter);
  CHECK_EQUAL(0x10000, ev_data.flip_arg);

  // Trigger a flip, then wait for two events with no timeout.
  result = handle->flipFrame(0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Wait for all flips to occur
  result = handle->waitFlip();
  UNSIGNED_INT_EQUALS(0, result);

  result = handle->waitFlipEvent(events, 2, &count, 10000);
  UNSIGNED_INT_EQUALS(0, result);
  // Despite adding another flip event, we still only get the one event.
  CHECK_EQUAL(1, count);

  // Print the event data.
  PrintEventData(&events[0]);

  // The user data still matches the new event
  CHECK(events[0].user_data != 0);
  CHECK_EQUAL(2, *(u64*)events[0].user_data);

  // Delete the event
  result = handle->deleteFlipEvent();
  UNSIGNED_INT_EQUALS(0, result);

  // This should result in having no events.
  // Trigger a flip
  result = handle->flipFrame(0x10000);
  UNSIGNED_INT_EQUALS(0, result);

  // Wait for all flips to occur
  result = handle->waitFlip();
  UNSIGNED_INT_EQUALS(0, result);

  // Wait for event.
  result = handle->waitFlipEvent(events, 1, &count, 10000);
  // Since there's no event left, nothing will be triggered when the flip occurs.
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Clean up after test
  delete (handle);
}

TEST(EventTest, VblankEventTest) {
  // Another type of video out event, these trigger automatically when the PS4 draws blank frames.
  VideoOut* handle = new VideoOut(1920, 1080);

  // Add vblank event
  s64 val    = 1;
  s32 result = handle->addVblankEvent(&val);
  UNSIGNED_INT_EQUALS(0, result);

  // Wait with no timeout, this should return after vblank
  OrbisKernelEvent ev {};
  s32              count;
  result = handle->waitVblankEvent(&ev, 1, &count, -1);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Print event data
  PrintEventData(&ev);

  CHECK_EQUAL(0x7000000000000, ev.ident);
  CHECK_EQUAL(-13, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  // Data for these follows the same format as internal data for flip events.
  CHECK(ev.data != 0);
  VideoOutEventData ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  // It is highly unlikely that more than 1 vblank happens between adding and waiting.
  // Based on this assumption, we can check counter.
  CHECK_EQUAL(1, ev_data.counter);
  // ev_data flip arg seems to come from how many vblanks have occurred.
  // There's no way to compare this to anything, as there will always be a chance of a race condition.
  CHECK(ev_data.time > 0);
  // user_data should be passed down from adding the event.
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(val, *(s64*)ev.user_data);

  // Add a new event, this should replace the old one.
  s64 new_val = 2;
  result      = handle->addVblankEvent(&new_val);

  // Wait with no timeout, this should return after vblank
  result = handle->waitVblankEvent(&ev, 1, &count, -1);
  UNSIGNED_INT_EQUALS(0, result);
  CHECK_EQUAL(1, count);

  // Print event data
  PrintEventData(&ev);

  CHECK_EQUAL(0x7000000000000, ev.ident);
  CHECK_EQUAL(-13, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  // Data for these follows the same format as internal data for flip events.
  CHECK(ev.data != 0);
  ev_data = *reinterpret_cast<VideoOutEventData*>(&ev.data);
  CHECK_EQUAL(1, ev_data.counter);
  // Flip arg is the number of vblanks that have occurred.
  // First test can't check for these, since it's possible we fire an event on the first vblank.
  // Here we know the flip arg must be positive, since we know a vblank has occurred.
  CHECK(ev_data.flip_arg > 0);
  CHECK(ev_data.time > 0);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(new_val, *(s64*)ev.user_data);

  // Delete vblank event
  result = handle->deleteVblankEvent();
  UNSIGNED_INT_EQUALS(0, result);

  // Now vblank events won't fire.
  // Validate using sceVideoOutWaitVblank
  result = handle->waitVblank();
  UNSIGNED_INT_EQUALS(0, result);

  // Wait with short timeout, this will return after timeout with error timedout
  result = handle->waitVblankEvent(&ev, 1, &count, 1000);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Clean up after test
  delete (handle);
}

TEST(EventTest, TimerEventTest) {
  // Timer events are pretty self explanatory.
  // They have a timer, and when it expires, they trigger.
  // This test will be rather simple, as I want to keep this single threaded.
  OrbisKernelEqueue eq {};
  s32               result = sceKernelCreateEqueue(&eq, "TimerEventQueue");
  UNSIGNED_INT_EQUALS(0, result);

  // Start by adding a timer event
  s64 data = 0x100;
  result   = sceKernelAddTimerEvent(eq, 0x10, 100000, &data);
  UNSIGNED_INT_EQUALS(0, result);

  // This shouldn't fire immediately, confirm by waiting with a short timeout.
  OrbisKernelEvent ev {};
  s32              count   = 0;
  u32              timeout = 1000;
  result                   = sceKernelWaitEqueue(eq, &ev, 1, &count, &timeout);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Now perform a sceKernelUsleep, wait out the timer timeout.
  result = sceKernelUsleep(100000);
  UNSIGNED_INT_EQUALS(0, result);

  // The event should've triggered, and should be returned by this wait
  count   = 0;
  timeout = 1000;
  result  = sceKernelWaitEqueue(eq, &ev, 1, &count, &timeout);
  UNSIGNED_INT_EQUALS(0, result);
  UNSIGNED_INT_EQUALS(1, count);

  PrintEventData(&ev);

  // Check validity of returned data
  CHECK_EQUAL(0x10, ev.ident);
  CHECK_EQUAL(-7, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  // Trigger count perhaps?
  CHECK_EQUAL(1, ev.data);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(data, *(s64*)ev.user_data);

  // Since the event has the clear flag, sceKernelWaitEqueue should've reset the timer.
  // This wait should fail, as the timer was only recently reset.
  count   = 0;
  timeout = 1000;
  result  = sceKernelWaitEqueue(eq, &ev, 1, &count, &timeout);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Now perform a sceKernelUsleep, wait out the timer timeout.
  result = sceKernelUsleep(100000);
  UNSIGNED_INT_EQUALS(0, result);

  // The event should've triggered, and should be returned by this wait
  count   = 0;
  timeout = 1000;
  result  = sceKernelWaitEqueue(eq, &ev, 1, &count, &timeout);
  UNSIGNED_INT_EQUALS(0, result);
  UNSIGNED_INT_EQUALS(1, count);

  PrintEventData(&ev);

  // Check validity of returned data
  CHECK_EQUAL(0x10, ev.ident);
  CHECK_EQUAL(-7, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK_EQUAL(1, ev.data);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(data, *(s64*)ev.user_data);

  // Wait longer this time
  result = sceKernelUsleep(1000000);
  UNSIGNED_INT_EQUALS(0, result);

  // The event should've triggered, and should be returned by this wait
  count   = 0;
  timeout = 1000;
  result  = sceKernelWaitEqueue(eq, &ev, 1, &count, &timeout);
  UNSIGNED_INT_EQUALS(0, result);
  UNSIGNED_INT_EQUALS(1, count);

  PrintEventData(&ev);

  // Check validity of returned data
  CHECK_EQUAL(0x10, ev.ident);
  CHECK_EQUAL(-7, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  // This event should trigger 10 times, which sets data to 10.
  CHECK_EQUAL(10, ev.data);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(data, *(s64*)ev.user_data);

  // Replace timer event.
  s64 data2 = 0x200;
  result    = sceKernelAddTimerEvent(eq, 0x10, 2000000, &data2);
  UNSIGNED_INT_EQUALS(0, result);

  // If we wait out the new event timeout manually, we'll only see 1 trigger.
  result = sceKernelUsleep(2000000);
  UNSIGNED_INT_EQUALS(0, result);

  count   = 0;
  timeout = 1000;
  result  = sceKernelWaitEqueue(eq, &ev, 1, &count, &timeout);
  UNSIGNED_INT_EQUALS(0, result);
  UNSIGNED_INT_EQUALS(1, count);

  PrintEventData(&ev);

  // Check validity of returned data
  CHECK_EQUAL(0x10, ev.ident);
  CHECK_EQUAL(-7, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK_EQUAL(1, ev.data);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(data2, *(s64*)ev.user_data);

  // Now test for a weird edge case.
  // When replacing the timer, if the new timer is longer,
  // there is a period where sceKernelWaitEqueue will wait out the event timer.
  // This call will fail
  count   = 0;
  timeout = 1000;
  result  = sceKernelWaitEqueue(eq, &ev, 1, &count, &timeout);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Now perform a sceKernelUsleep, but for half of the old timer
  result = sceKernelUsleep(50000);
  UNSIGNED_INT_EQUALS(0, result);

  // This wait should fail, as we're still before the original timeout
  count   = 0;
  timeout = 1000;
  result  = sceKernelWaitEqueue(eq, &ev, 1, &count, &timeout);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  // Now run sceKernelWaitEqueue with a long enough timeout to fall in the difference.
  // Instead of failing with ETIMEDOUT, this call blocks until the event is triggered.
  count   = 0;
  timeout = 60000;
  result  = sceKernelWaitEqueue(eq, &ev, 1, &count, &timeout);
  UNSIGNED_INT_EQUALS(0, result);
  UNSIGNED_INT_EQUALS(1, count);

  PrintEventData(&ev);

  // Check validity of returned data
  CHECK_EQUAL(0x10, ev.ident);
  CHECK_EQUAL(-7, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK_EQUAL(1, ev.data);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(data2, *(s64*)ev.user_data);

  // Delete the timer event
  result = sceKernelDeleteTimerEvent(eq, 0x10);
  UNSIGNED_INT_EQUALS(0, result);

  // Add timer event back in.
  result = sceKernelAddTimerEvent(eq, 0x10, 100000, &data);
  UNSIGNED_INT_EQUALS(0, result);

  // Wait for the event to trigger
  result = sceKernelUsleep(100000);
  UNSIGNED_INT_EQUALS(0, result);

  // Now replace it
  result = sceKernelAddTimerEvent(eq, 0x10, 200000, &data2);
  UNSIGNED_INT_EQUALS(0, result);

  // The trigger state isn't cleared
  count   = 0;
  timeout = 1000;
  result  = sceKernelWaitEqueue(eq, &ev, 1, &count, &timeout);
  UNSIGNED_INT_EQUALS(0, result);
  UNSIGNED_INT_EQUALS(1, count);

  PrintEventData(&ev);

  // Check validity of returned data
  CHECK_EQUAL(0x10, ev.ident);
  CHECK_EQUAL(-7, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK_EQUAL(1, ev.data);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(data2, *(s64*)ev.user_data);

  // Check if the same edge case is present
  count   = 0;
  timeout = 1000;
  result  = sceKernelWaitEqueue(eq, &ev, 1, &count, &timeout);
  UNSIGNED_INT_EQUALS(ORBIS_KERNEL_ERROR_ETIMEDOUT, result);

  count   = 0;
  timeout = 100000;
  result  = sceKernelWaitEqueue(eq, &ev, 1, &count, &timeout);
  UNSIGNED_INT_EQUALS(0, result);
  UNSIGNED_INT_EQUALS(1, count);

  PrintEventData(&ev);

  // Check validity of returned data
  CHECK_EQUAL(0x10, ev.ident);
  CHECK_EQUAL(-7, ev.filter);
  CHECK_EQUAL(32, ev.flags);
  CHECK_EQUAL(0, ev.fflags);
  CHECK_EQUAL(1, ev.data);
  CHECK(ev.user_data != 0);
  CHECK_EQUAL(data2, *(s64*)ev.user_data);

  // Delete the equeue
  result = sceKernelDeleteEqueue(eq);
  UNSIGNED_INT_EQUALS(0, result);
}