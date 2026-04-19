/*
 * launch_pskit — tiny PS4 payload that launches the PSKit Developer Daemon
 * (title_id PSKT00001) after GoldHEN is running.
 *
 * ABI: entered at offset 0 as `int _main(struct payload_args *args)`.
 * The ROP chain in exp_loader.js mmaps RWX, memcpys the blob, and calls _main
 * with args->sceKernelDlsym populated (GoldHEN/Mira convention).
 *
 * Build: see ../Makefile. Produces a flat binary with no relocations, so every
 * string literal is constructed on the stack (char a[] = "...").
 */

#include "payload.h"

/* ---- Minimal freestanding helpers ------------------------------------- */

static void *memset_local(void *dst, int c, unsigned long n) {
    unsigned char *d = (unsigned char *)dst;
    while (n--) *d++ = (unsigned char)c;
    return dst;
}

/* Resolve a symbol from "any loaded module". Returns 0 on success. */
static int resolve(struct payload_args *a, const char *name, void **out) {
    return a->sceKernelDlsym(DLSYM_ANY, name, out);
}

/* ---- Payload entry ---------------------------------------------------- */

int _main(struct payload_args *args) {
    if (!args || !args->sceKernelDlsym) return -1;

    /* Function pointers we need. */
    int (*sceSysmoduleLoadModule)(uint16_t id) = 0;
    int (*sceUserServiceInitialize)(const void *params) = 0;
    int (*sceUserServiceGetInitialUser)(int32_t *user_id) = 0;
    int (*sceLncUtilInitialize)(void) = 0;
    int (*sceLncUtilLaunchApp)(const char *title_id, const char *argv[],
                               LncAppParam *param) = 0;
    int (*sceSysUtilSendSystemNotificationWithText)(int req_id,
                                                    const char *text) = 0;

    /* Sysmodule + UserService are usually already loaded into the WebKit
     * browser process, so these resolve immediately.            */
    if (resolve(args, "sceSysmoduleLoadModule",
                (void **)&sceSysmoduleLoadModule)) return -2;
    if (resolve(args, "sceUserServiceInitialize",
                (void **)&sceUserServiceInitialize)) return -3;
    if (resolve(args, "sceUserServiceGetInitialUser",
                (void **)&sceUserServiceGetInitialUser)) return -4;

    /* LncUtil & SysUtil may not be loaded yet in a browser context — force. */
    sceSysmoduleLoadModule(SCE_SYSMODULE_LNC_UTIL);
    sceSysmoduleLoadModule(SCE_SYSMODULE_SYSTEM_SERVICE);

    if (resolve(args, "sceLncUtilInitialize",
                (void **)&sceLncUtilInitialize)) return -5;
    if (resolve(args, "sceLncUtilLaunchApp",
                (void **)&sceLncUtilLaunchApp)) return -6;
    /* Notification is best-effort — keep going if it's missing. */
    resolve(args, "sceSysUtilSendSystemNotificationWithText",
            (void **)&sceSysUtilSendSystemNotificationWithText);

    /* Init user service (priority 700 per daemon-learnings KB). */
    struct { unsigned int priority; } us_params;
    us_params.priority = 700;
    sceUserServiceInitialize(&us_params);

    int32_t user_id = 0;
    sceUserServiceGetInitialUser(&user_id);

    sceLncUtilInitialize();

    LncAppParam p;
    memset_local(&p, 0, sizeof(p));
    p.size       = sizeof(p);
    p.user_id    = user_id;
    p.check_flag = LAUNCHAPP_SKIP_SYSTEM_UPDATE;

    char title[] = { 'P','S','K','T','0','0','0','0','1', 0 };
    int launch_ret = sceLncUtilLaunchApp(title, 0, &p);

    if (sceSysUtilSendSystemNotificationWithText) {
        if (launch_ret > 0) {
            char ok_msg[] = { 'P','S','K','i','t',' ','D','a','e','m','o','n',
                              ' ','b','a','s','l','a','t','i','l','d','i', 0 };
            sceSysUtilSendSystemNotificationWithText(222, ok_msg);
        } else {
            char fail_msg[] = { 'P','S','K','i','t',' ','l','a','u','n','c','h',
                                ' ','f','a','i','l','e','d', 0 };
            sceSysUtilSendSystemNotificationWithText(222, fail_msg);
        }
    }

    return launch_ret > 0 ? 0 : launch_ret;
}
