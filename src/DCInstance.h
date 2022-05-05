#pragma once

#define WIN32_LEAN_AND_MEAN
#include <discord_register.h>
#include <discord_rpc.h>
#include <windows.h>

#include <chrono>

class DCInstance {
   public:
    static inline DiscordRichPresence Presence;

    static void InitializeRPC(const char*);

    static void UpdateDetails(const char*);
    static void UpdateState(const char*);
    static void UpdateLImage(const char*, const char*);
    static void UpdateSImage(const char*, const char*);
};
