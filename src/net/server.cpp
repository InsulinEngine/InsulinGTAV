#include "net/server.h"
#include "platform/log.h"

// The C headers come first on purpose: orbis/Net.h uses size_t and the
// fixed-width types without including anything that defines them, so it only
// compiles behind stddef.h/stdint.h.
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <orbis/Net.h>
#include <orbis/libkernel.h>

// The toolchain declares BSD socket names in sys/socket.h, but libxnet.a is an
// empty 8-byte archive - the names are declared and never provided, so a plain
// socket()/bind() build links clean here and fails to resolve on console. The
// sceNet entry points in orbis/Net.h are the real ones.
//
// orbis/_types/net.h carries OrbisNetId, OrbisNetSockaddr and
// ORBIS_NET_AF_INET / ORBIS_NET_SOCK_STREAM, but stops there: there is no
// sockaddr_in equivalent and no protocol or option constants. Those are defined
// here, with the standard SCE values, and the struct mirrors the 16-byte layout
// of OrbisNetSockaddr (1 + 1 + 14).
#define ORBIS_NET_IPPROTO_TCP   6
#define ORBIS_NET_SOL_SOCKET    0xffff
#define ORBIS_NET_SO_REUSEADDR  0x00000004

typedef struct {
    uint8_t  sin_len;
    uint8_t  sin_family;
    uint16_t sin_port;
    uint32_t sin_addr;
    uint16_t sin_vport;
    char     sin_zero[6];
} orbis_sockaddr_in;

namespace {

    volatile bool  g_running  = false;
    volatile bool  g_stopping = false;
    OrbisNetId     g_listen   = -1;
    // The connection currently being served, or -1. server_stop() aborts this
    // as well as the listener: a client that opens a socket and then sends
    // nothing parks serve_one in recv forever, and without this abort the
    // join below would hang the game thread that toggled the server off.
    volatile OrbisNetId g_client = -1;
    OrbisPthread   g_thread   = 0;
    unsigned short g_port     = 0;

    char        g_root[256];
    char        g_pin[5];
    net::config g_cfg;

    int sock_read(void* ctx, char* buf, unsigned cap) {
        OrbisNetId s = (OrbisNetId)(long)ctx;
        return (int)sceNetRecv(s, buf, cap, 0);
    }

    int sock_write(void* ctx, const char* buf, unsigned len) {
        OrbisNetId s = (OrbisNetId)(long)ctx;
        unsigned sent = 0;
        while (sent < len) {
            int n = (int)sceNetSend(s, buf + sent, len - sent, 0);
            if (n <= 0) return -1;
            sent += (unsigned)n;
        }
        return (int)sent;
    }

    void* listener(void*) {
        platform::klogf("net: listener up on port %u", (unsigned)g_port);

        while (!g_stopping) {
            OrbisNetSockaddr  peer;
            OrbisNetSocklen_t peer_len = sizeof(peer);
            OrbisNetId c = sceNetAccept(g_listen, &peer, &peer_len);
            if (c < 0) {
                if (g_stopping) break;
                continue;
            }
            g_client = c;
            net::serve_one(g_cfg, (void*)(long)c, sock_read, sock_write);
            g_client = -1;
            sceNetSocketClose(c);
        }

        platform::klogf("net: listener down");
        g_running = false;
        return 0;
    }
}

namespace net {

    bool server_start(unsigned short port, const char* web_root,
                      const char* pin, state_fn state) {
        if (g_running) return true;

        g_stopping = false;
        g_client   = -1;
        g_port     = port;

        snprintf(g_root, sizeof(g_root), "%s", web_root);
        memcpy(g_pin, pin, 4);
        g_pin[4] = 0;

        g_cfg.web_root = g_root;
        memcpy(g_cfg.pin, g_pin, 5);
        g_cfg.state = state;

        sceNetInit();

        g_listen = sceNetSocket("insulin", ORBIS_NET_AF_INET,
                                ORBIS_NET_SOCK_STREAM, ORBIS_NET_IPPROTO_TCP);
        if (g_listen < 0) {
            platform::klogf("net: socket failed (%d)", (int)g_listen);
            return false;
        }

        int on = 1;
        sceNetSetsockopt(g_listen, ORBIS_NET_SOL_SOCKET, ORBIS_NET_SO_REUSEADDR,
                         &on, sizeof(on));

        orbis_sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_len    = sizeof(addr);
        addr.sin_family = ORBIS_NET_AF_INET;
        addr.sin_addr   = 0;                           // INADDR_ANY
        addr.sin_port   = sceNetHtons(port);

        if (sceNetBind(g_listen, (OrbisNetSockaddr*)&addr, sizeof(addr)) < 0) {
            platform::klogf("net: bind to %u failed", (unsigned)port);
            sceNetSocketClose(g_listen);
            g_listen = -1;
            return false;
        }
        if (sceNetListen(g_listen, 2) < 0) {
            platform::klogf("net: listen failed");
            sceNetSocketClose(g_listen);
            g_listen = -1;
            return false;
        }

        // An explicit stack size: scePthreadCreate with a NULL attr gives a tiny
        // stack on this platform, and a frame that overruns it faults on entry
        // with no useful log. 128 KB is far more than this thread's frames need
        // (serve_one's buffers are static, not local).
        OrbisPthreadAttr attr;
        scePthreadAttrInit(&attr);
        scePthreadAttrSetstacksize(&attr, 128 * 1024);

        g_running = true;
        if (scePthreadCreate(&g_thread, &attr, listener, 0, "insulin_http") != 0) {
            platform::klogf("net: thread create failed");
            g_running = false;
            sceNetSocketClose(g_listen);
            g_listen = -1;
            scePthreadAttrDestroy(&attr);
            return false;
        }
        scePthreadAttrDestroy(&attr);

        platform::logf("net", "server started on port %u", (unsigned)port);
        return true;
    }

    void server_stop() {
        if (!g_running) return;
        g_stopping = true;

        // Abort unblocks whichever call the listener is parked in - accept on
        // the listen socket, or recv on a connection that went quiet. Without
        // both, the join below waits for a client that may never speak again.
        OrbisNetId c = g_client;
        if (c >= 0) sceNetSocketAbort(c, 0);

        if (g_listen >= 0) {
            sceNetSocketAbort(g_listen, 0);
            sceNetSocketClose(g_listen);
            g_listen = -1;
        }
        scePthreadJoin(g_thread, 0);
        g_thread  = 0;
        g_client  = -1;
        g_running = false;
        platform::logf("net", "server stopped");
    }

    bool           server_running() { return g_running; }
    unsigned short server_port()    { return g_port; }
}
