#ifndef LAUNCH_PSKIT_PAYLOAD_H
#define LAUNCH_PSKIT_PAYLOAD_H

#include <stdint.h>

/*
 * payload_args — layout passed in rdi by the ROP chain (GoldHEN / Mira / sleirsgoevy
 * 6.72 webkit JB lineage). Only sceKernelDlsym is load-bearing for us.
 */
struct payload_args {
    int (*sceKernelDlsym)(int handle, const char *sym, void **out);
    int *rwpipe;
    int *rwpair;
    int kpipe_handle;
    uint64_t kdata_base_addr;
    int *payloadout;
};

/* Sysmodule IDs (official SDK constants). */
#define SCE_SYSMODULE_SYSTEM_SERVICE 0x0080
#define SCE_SYSMODULE_USER_SERVICE   0x0096
#define SCE_SYSMODULE_LNC_UTIL       0x0019

/*
 * LncAppParam — app launch parameter block.
 * Layout matches libjbc / ItemzFlow reference used by PSKit daemon.
 */
typedef struct {
    uint64_t size;           /* sizeof(LncAppParam) */
    int32_t  user_id;        /* from sceUserServiceGetInitialUser */
    uint32_t app_opt;        /* unused, 0 */
    uint64_t crash_report;   /* unused, 0 */
    uint32_t check_flag;     /* 1 = skip system update */
} LncAppParam;

#define LAUNCHAPP_SKIP_SYSTEM_UPDATE 1

/*
 * OpenOrbis SDK module handle constants used as dlsym "handle" value.
 * -1 = search all loaded modules (works once the module is loaded).
 */
#define DLSYM_ANY ((int)-1)

#endif
