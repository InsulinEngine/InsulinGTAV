#pragma once

// Ozark-compatible LOG* macros. On PS4 there is no console window; lines go to
// a file (pullable over FTP) and, for the loud ones, an on-screen notification.
namespace platform {
    void log_line(const char* tag, const char* msg);
    void logf(const char* tag, const char* fmt, ...);
    void notify(const char* msg);   // on-screen + logged

    // Kernel-log output (sceKernelDebugOutText, channel 0). GoldHEN broadcasts
    // the kernel log over TCP 3232, so these lines appear live in `nc <ip> 3232`
    // INTERLEAVED with the game's own klog and any crash dump - unlike the
    // /data file, which needs FTP after the fact. Every line is prefixed "IGV "
    // so it greps cleanly out of the firehose. Use for crash-window tracing.
    //
    // NOT named klog(): GoldHEN's Utilities.h does `#define klog printf`, which
    // would rewrite this identifier wherever that header is in scope.
    void klogf(const char* fmt, ...);
}

#define LOG(fmt, ...)                   platform::logf("Log",  fmt, ##__VA_ARGS__)
#define LOG_DEV(fmt, ...)               platform::logf("Dev",  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)             platform::logf("Err",  fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)              platform::logf("Warn", fmt, ##__VA_ARGS__)
#define LOG_CUSTOM(tag, fmt, ...)       platform::logf(tag,    fmt, ##__VA_ARGS__)
#define LOG_CUSTOM_ERROR(tag, fmt, ...) platform::logf(tag,    fmt, ##__VA_ARGS__)
