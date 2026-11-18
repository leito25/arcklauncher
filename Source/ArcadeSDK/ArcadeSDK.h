#pragma once

#include <stdint.h>

#if defined(_WIN32)
#  if defined(ARCADE_SDK_BUILD)
#    define ARCADE_API __declspec(dllexport)
#  else
#    define ARCADE_API __declspec(dllimport)
#  endif
#else
#  define ARCADE_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ArcadeResult {
    ARCADE_OK = 0,
    ARCADE_INVALID_ARGUMENT = 1,
    ARCADE_INVALID_STATE = 2,
    ARCADE_IO_ERROR = 3,
    ARCADE_INVALID_PROFILE = 4,
    ARCADE_INTERNAL_ERROR = 5
} ArcadeResult;

/* Strings are UTF-8. A null profile_directory uses the per-user default. */
ARCADE_API ArcadeResult ArcadeInit(const char* game_id, const char* profile_directory);
ARCADE_API ArcadeResult ArcadeAddPlaytime(uint64_t seconds);
ARCADE_API ArcadeResult ArcadeUpdateHighScore(uint64_t score);
ARCADE_API ArcadeResult ArcadeUnlockAchievement(const char* achievement_id);
ARCADE_API ArcadeResult ArcadeShutdown(void);
/* Valid until the next SDK call on the same thread. */
ARCADE_API const char* ArcadeLastError(void);

#ifdef __cplusplus
}
#endif
