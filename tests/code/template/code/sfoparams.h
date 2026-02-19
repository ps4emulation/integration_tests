#pragma once

#include <cstdint>

// Credits: https://www.psdevwiki.com/ps4/Param.sfo#ATTRIBUTE + a little reverse job by @igor725
union SfoAttributes {
  struct {
    // ATTRIBUTE
    bool isInitUserLogoutSupported         : 1;
    bool dialogEnterButtonAssignment       : 1;
    bool menuWarningForPsMove              : 1;
    bool supportsStereoscopic3D            : 1;
    bool suspendsOnPsButtonPress           : 1;
    bool systemDialogEnterButtonAssignment : 1;
    bool isOverwritesDefaultShareMenu      : 1;
    bool _unknownAttribute_1x7             : 1;
    bool suspendsOnSpecialOutputResolution : 1;
    bool isHdcpEnabled                     : 1;
    bool isHdcpDisabledForNonGames         : 1;
    bool _unknownAttribute_1x11            : 1;
    bool _unknownAttribute_1x12            : 1;
    bool _unknownAttribute_1x13            : 1;
    bool isVrSupported                     : 1;
    bool isSixCpuMode                      : 1;
    bool isSevenCpuMode                    : 1;
    bool _unknownAttribute_1x17            : 1;
    bool _unknownAttribute_1x18            : 1;
    bool _unknownAttribute_1x19            : 1;
    bool _unknownAttribute_1x20            : 1;
    bool _unknownAttribute_1x21            : 1;
    bool _unknownAttribute_1x22            : 1;
    bool isNeoModeSupported                : 1;
    bool _unknownAttribute_1x24            : 1;
    bool _unknownAttribute_1x25            : 1;
    bool isVrRequired                      : 1;
    bool _unknownAttribute_1x27            : 1;
    bool _unknownAttribute_1x28            : 1;
    bool isHdrSupported                    : 1;
    bool _unknownAttribute_1x29            : 1;
    bool displayLocation                   : 1;

    // ATTRIBUTE2
    bool _unknownAttribute_2x0                   : 1;
    bool isVideoRecordingSupported               : 1;
    bool isContentSearchSupported                : 1;
    bool _unknownAttribute_2x3                   : 1;
    bool isPsVrEyeToEyeDistanceDisabled          : 1;
    bool isPsVrEyeToEyeDistanceChangeable        : 1;
    bool _unknownAttribute_2x6                   : 1;
    bool _unknownAttribute_2x7                   : 1;
    bool isBroadcastSeparateModeSupported        : 1;
    bool doNotApplyDummyLoadForTrackingMove      : 1;
    bool _unknownAttribute_2x10                  : 1;
    bool isOneOnOneMatchEventSupported           : 1;
    bool isTeamOnTeamTournamentSupported         : 1;
    bool _unknownAttribute_2x13                  : 1;
    bool _unknownAttribute_2x14                  : 1;
    bool noTwoMegabytePages                      : 1;
    bool reserveTwoMegabytePagesForRoDataAndText : 1;
    bool _unknownAttribute_2x17                  : 1;
    bool _unknownAttribute_2x18                  : 1;
    bool _unknownAttribute_2x19                  : 1;
    bool useImprovedThreadScheduler              : 1;
    bool _unknownAttribute_2x21                  : 1;
    bool _unknownAttribute_2x22                  : 1;
    bool appRunsOnPlayStation5AndComplyTRC4211   : 1;
    bool _unknownAttribute_2x24                  : 1;
    bool _unknownAttribute_2x25                  : 1;
    bool forceGpu800MHzClockCounter              : 1;
    bool _unknownAttribute_2x27                  : 1;
    bool _unknownAttribute_2x28                  : 1;
    bool _unknownAttribute_2x29                  : 1;
    bool _unknownAttribute_2x30                  : 1;
    bool _unknownAttribute_2x31                  : 1;
  };

  struct {
    uint32_t attribute1;
    uint32_t attribute2;
  };

  uint64_t attributes;
};
