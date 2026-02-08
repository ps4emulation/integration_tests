#include <orbis/SystemService.h>
#include <orbis/UserService.h>

extern "C" int32_t sceKernelGetCompiledSdkVersion(uint32_t* sdk); // TODO correct declaration in OpenOrbis headers

int main(int ac, char** av) {
  // No buffering
  setvbuf(stdout, NULL, _IONBF, 0);

  sceUserServiceInitialize(nullptr);

  int32_t user = ORBIS_USER_SERVICE_USER_ID_INVALID;
  sceUserServiceGetInitialUser(&user);
  uint32_t sdk;
  sceKernelGetCompiledSdkVersion(&sdk);

  printf("My firmware: %x\nInit user: %d\n", sdk, user);

  sceSystemServiceLoadExec("EXIT", nullptr);
  return 0;
}
