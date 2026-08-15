# third_party/stb/stb_image.h — provenance

This is a **host-test-only copy**. The PS4 build does not use it:
`CMakeLists.txt` adds only `src` via `target_include_directories`, and the
orbis compile invocation resolves `<stb/stb_image.h>` through
`-isystem /home/bbc/OpenOrbis-PS4-Toolchain/include`, i.e. the toolchain's own
copy. This file exists only because Windows clang, used for the host unit
test (`tests/image_decode_test.cpp`), cannot reach the toolchain's copy over
its WSL UNC path (`\\wsl.localhost\Ubuntu\...`) without also pulling in the
toolchain's own libc headers ahead of the MSVC CRT ones, which conflict
outright (`size_t` redefinition, missing `_aligned_malloc`/`_aligned_free`,
missing `fopen_s`). `-I third_party` exposes only `stb/stb_image.h` and
sidesteps that.

## Source

`/home/bbc/OpenOrbis-PS4-Toolchain/include/stb/stb_image.h` (inside WSL).

## Version

`stb_image - v2.25` (from the header's own version comment, line 1).

## Verified identical on 2026-08-15

```
$ wsl.exe bash -lc "md5sum /home/bbc/OpenOrbis-PS4-Toolchain/include/stb/stb_image.h"
a1170ba8b5f36154a8b9859f17ee8470  /home/bbc/OpenOrbis-PS4-Toolchain/include/stb/stb_image.h

$ md5sum third_party/stb/stb_image.h
a1170ba8b5f36154a8b9859f17ee8470  third_party/stb/stb_image.h
```

Both md5s match. Nothing enforces this automatically — there is no CI in this
project to hang a check on — so re-run the command below by hand after any
toolchain update, and re-copy the header if the sums diverge. A stale copy
here would not fail loudly: the host test would keep passing against old
behaviour while the PS4 build picked up new behaviour from the toolchain
copy, which is the worst shape this bug could take.

## Re-verify

```
wsl.exe bash -lc "md5sum /home/bbc/OpenOrbis-PS4-Toolchain/include/stb/stb_image.h" && md5sum third_party/stb/stb_image.h
```

## License

stb_image.h is dual-licensed, take your pick (both blocks are present verbatim
in the header, starting at its `LICENSE` section):

- **MIT License** (Copyright (c) 2017 Sean Barrett), or
- **Public domain** (www.unlicense.org)
