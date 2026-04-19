# launch_pskit

Minimal PS4 payload. Single purpose: launch the PSKit Developer Daemon
(`PSKT00001`) after GoldHEN is already running.

Consumed by `../exp_loader.js` the same way `../goldhen.bin` is — raw flat
binary, mmap'd RWX, entered at offset 0 with `args->sceKernelDlsym` passed in
`rdi` (GoldHEN / Mira / sleirsgoevy 6.72 lineage loader convention).

## Build

Requires the OpenOrbis PS4 toolchain. Either point `OO_PS4_TOOLCHAIN` at a
local install, or reuse the Docker image PSKitDeveloperDaemon uses:

```bash
# Local (if OpenOrbis is installed on host)
export OO_PS4_TOOLCHAIN=/opt/openorbis
make

# Docker (ps4build-focal image — same as PSKit daemon build)
docker run --rm -v "$PWD":/work -w /work ps4build-focal bash -lc '
  export OO_PS4_TOOLCHAIN=/opt/openorbis
  export PATH=$PATH:$OO_PS4_TOOLCHAIN/bin/linux
  sed -i "s/clang$/clang-10/" Makefile
  sed -i "s/ld.lld$/ld.lld-10/" Makefile
  sed -i "s/llvm-objcopy$/llvm-objcopy-10/" Makefile
  make
'
```

Output: `../launch_pskit.bin` (sibling of `goldhen.bin`). Commit it.

## Runtime

1. WebKit JB chain → `goldhen.bin` → `msgs2.innerHTML = "GoldHEN Loaded"`
2. `index.html` sets `PLfile = "launch_pskit.bin"` and re-injects `exp_loader.js`
3. Loader mmaps RWX, memcpys, calls `_main(args)`
4. `_main`:
   - Resolves `sceSysmoduleLoadModule`, `sceUserServiceInitialize`,
     `sceUserServiceGetInitialUser`, `sceLncUtilInitialize`,
     `sceLncUtilLaunchApp`, `sceSysUtilSendSystemNotificationWithText`
     via `args->sceKernelDlsym`
   - Loads `LNC_UTIL` and `SYSTEM_SERVICE` sysmodules
   - Initializes user service (priority 700)
   - Calls `sceLncUtilLaunchApp("PSKT00001", NULL, &LncAppParam)`
   - Shows an on-screen notification with the result

No network, no file I/O, no background state.

## Loader ABI note

If deployment shows the payload runs but nothing launches, the loader may be
passing a `struct thread *` in `rdi` instead of `struct payload_args *`
(ps4-payload-sdk convention vs. Mira convention). In that case the fix is to
skip `args` entirely and resolve symbols via a fixed `sceKernelDlsym` offset
from the libkernel base that the ROP chain leaves in a known location — see
GoldHEN's `Source/init.c` for a reference implementation. Adjust `_main`'s
signature and symbol resolution path, rebuild.

## Constants

Change the title_id in `src/main.c` (`title[]`). Content ID isn't needed —
`sceLncUtilLaunchApp` takes the 9-char title_id only.
