#include <orbis/SystemService.h>
#include <orbis/UserService.h>

extern "C" int32_t sceKernelGetCompiledSdkVersion(void); // TODO correct declaration in OpenOrbis headers

int main(int ac, char** av) {
  // No buffering
  setvbuf(stdout, NULL, _IONBF, 0);

  sceUserServiceInitialize(nullptr);

  int32_t user = ORBIS_USER_SERVICE_USER_ID_INVALID;
  sceUserServiceGetInitialUser(&user);

  printf("My firmware: %x\nInit user: %d\n", sceKernelGetCompiledSdkVersion(), user);

  sceSystemServiceLoadExec("EXIT", nullptr);
  return 0;
}
