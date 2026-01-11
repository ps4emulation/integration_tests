#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <orbis/Pad.h>
#include <orbis/UserService.h>
#include <orbis/libkernel.h>

static int32_t printError(const char* fname, int32_t err) {
  if (err == ORBIS_OK) return ORBIS_OK;
  printf("*** %s error: %08x\n", fname, err);
  return err;
}

int32_t main(void) {
  // No buffering
  setvbuf(stdout, NULL, _IONBF, 0);

  OrbisUserServiceInitializeParams param;
  param.priority = ORBIS_KERNEL_PRIO_FIFO_LOWEST;
  printError("sceUserServiceInitialize", sceUserServiceInitialize(&param));

  int32_t user;
  printError("sceUserServiceGetInitialUser", sceUserServiceGetInitialUser(&user));
  printf("Runner user: %d\n", user);

  printf("Live gamepad detection (Metro Redux)\n");
  bool testEnd = false;
  while (!testEnd) {
    OrbisPadData data   = {};
    int32_t      handle = -1;
    while (true) {
      handle = scePadOpen(user, ORBIS_PAD_PORT_TYPE_STANDARD, 0, nullptr);
      printError("scePadOpen", handle);
      if (handle < 0) abort();

      if (printError("scePadReadState", scePadReadState(handle, &data)) == ORBIS_OK && data.connected == true) {
        printf("  timestamp: %zu\nconn count: %u\n", data.timestamp, (uint32_t)data.count);
        break;
      }
      printError("scePadClose", scePadClose(handle));
      sceKernelUsleep(1000000);
    }
    OrbisPadInformation info;
    printError("scePadGetControllerInformation", scePadGetControllerInformation(handle, &info));
    printf("  Pad found! %d\n    touchDensity: %f\ntouchRes: %u,%u\ndeadZones: %u,%u\nconnType: %u (%u)\n", handle, info.touchpadDensity,
           (uint32_t)info.touchResolutionX, (uint32_t)info.touchResolutionY, (uint32_t)info.stickDeadzoneL, (uint32_t)info.stickDeadzoneR,
           (uint32_t)info.connectionType, (uint32_t)info.count);

    printf("  Press Square to restart test\n");
    while (scePadReadState(handle, &data) == ORBIS_OK) {
      if (data.buttons & ORBIS_PAD_BUTTON_OPTIONS) {
        testEnd = true;
        break;
      } else if (data.buttons & ORBIS_PAD_BUTTON_SQUARE) {
        printf("  Restarting test in 5 seconds...\n");
        sceKernelUsleep(5000000);
        break;
      }
    }
  }

  return 0;
}
