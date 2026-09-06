#include "net/conn.h"
#include "net/jobs.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace {

    // Big enough for any reply this server produces: the largest is a static
    // file, and files above this are refused rather than streamed. Keeping it
    // static avoids an allocation on the request path, and the buffer lives in
    // .bss rather than on the server thread's stack - a large frame there
    // faults on entry with no useful log.
    const unsigned k_io_cap = 256 * 1024;
    static char g_in[net::header_cap + 4096];
    static char g_out[k_io_cap];

    void reply(void* ctx, net::write_fn wr, int status,
               const char* type, const char* body, unsigned body_len) {
        unsigned n = net::write_response(g_out, sizeof(g_out), status, type, body, body_len);
        if (n) wr(ctx, g_out, n);
    }

    bool span_eq(const char* p, unsigned n, const char* want) {
        return n == (unsigned)strlen(want) && memcmp(p, want, n) == 0;
    }

    // The PIN may arrive as a header or as ?pin=NNNN, because the very first
    // navigation cannot set a header. Both are checked against the same value.
    bool pin_ok(const net::config& cfg, const net::request& r) {
        if (r.pin && r.pin_len == 4 && memcmp(r.pin, cfg.pin, 4) == 0) return true;

        const char* q = r.query;
        unsigned    n = r.query_len;
        for (unsigned i = 0; i + 4 <= n; i++) {
            bool at_start = (i == 0) || q[i - 1] == '&';
            if (at_start && memcmp(q + i, "pin=", 4) == 0) {
                unsigned v = i + 4, len = 0;
                while (v + len < n && q[v + len] != '&') len++;
                return len == 4 && memcmp(q + v, cfg.pin, 4) == 0;
            }
        }
        return false;
    }

    const char* content_type_for(const char* path, unsigned len) {
        struct { const char* ext; const char* type; } table[] = {
            { ".html", "text/html; charset=utf-8" },
            { ".js",   "text/javascript" },
            { ".css",  "text/css" },
            { ".json", "application/json" },
            { ".png",  "image/png" },
            { ".jpg",  "image/jpeg" },
        };
        for (unsigned i = 0; i < sizeof(table) / sizeof(table[0]); i++) {
            unsigned e = (unsigned)strlen(table[i].ext);
            if (len >= e && memcmp(path + len - e, table[i].ext, e) == 0) return table[i].type;
        }
        return "application/octet-stream";
    }

    // Rejects "..", backslashes and absolute paths outright rather than trying
    // to normalise them. A companion server has no legitimate use for any of
    // them, and refusing is easier to get right than canonicalising.
    bool path_is_safe(const char* p, unsigned n) {
        if (n == 0 || p[0] != '/') return false;
        for (unsigned i = 0; i < n; i++) {
            if (p[i] == '\\') return false;
            if (p[i] == '.' && i + 1 < n && p[i + 1] == '.') return false;
        }
        return true;
    }

    // Reads a bare JSON number for "key". Good enough for the two flat objects
    // this server accepts, and it cannot run off the end of the body.
    bool json_number(const char* body, unsigned len, const char* key, float* out) {
        char pattern[32];
        int pn = snprintf(pattern, sizeof(pattern), "\"%s\"", key);
        if (pn <= 0) return false;

        for (unsigned i = 0; i + (unsigned)pn <= len; i++) {
            if (memcmp(body + i, pattern, (unsigned)pn) != 0) continue;
            unsigned j = i + (unsigned)pn;
            while (j < len && (body[j] == ' ' || body[j] == ':')) j++;

            char num[32];
            unsigned k = 0;
            while (j < len && k + 1 < sizeof(num) &&
                   ((body[j] >= '0' && body[j] <= '9') || body[j] == '-' ||
                    body[j] == '+' || body[j] == '.' || body[j] == 'e' || body[j] == 'E')) {
                num[k++] = body[j++];
            }
            if (k == 0) return false;
            num[k] = 0;
            *out = (float)atof(num);
            return true;
        }
        return false;
    }

    bool json_true(const char* body, unsigned len, const char* key) {
        char pattern[32];
        int pn = snprintf(pattern, sizeof(pattern), "\"%s\"", key);
        if (pn <= 0) return false;
        for (unsigned i = 0; i + (unsigned)pn <= len; i++) {
            if (memcmp(body + i, pattern, (unsigned)pn) != 0) continue;
            unsigned j = i + (unsigned)pn;
            while (j < len && (body[j] == ' ' || body[j] == ':')) j++;
            return j + 4 <= len && memcmp(body + j, "true", 4) == 0;
        }
        return false;
    }

    void serve_static(const net::config& cfg, void* ctx, net::write_fn wr,
                      const char* path, unsigned path_len) {
        if (!path_is_safe(path, path_len)) {
            reply(ctx, wr, 403, "text/plain", "forbidden", 9);
            return;
        }

        // "/" means the page itself.
        const char* rel = path;
        unsigned    rel_len = path_len;
        if (rel_len == 1) { rel = "/index.html"; rel_len = 11; }

        char full[512];
        int n = snprintf(full, sizeof(full), "%s%.*s", cfg.web_root, (int)rel_len, rel);
        if (n <= 0 || (unsigned)n >= sizeof(full)) {
            reply(ctx, wr, 404, "text/plain", "not found", 9);
            return;
        }

        FILE* fp = fopen(full, "rb");
        if (!fp) {
            reply(ctx, wr, 404, "text/plain", "not found", 9);
            return;
        }

        // Read straight into the response buffer, after the space the header
        // will need. write_response copies it into place; that one extra copy
        // buys a single code path for every reply.
        static char file_buf[k_io_cap - 1024];
        size_t got = fread(file_buf, 1, sizeof(file_buf), fp);
        int    more = fgetc(fp);
        fclose(fp);

        if (more != EOF) {
            reply(ctx, wr, 413, "text/plain", "file too large", 14);
            return;
        }

        reply(ctx, wr, 200, content_type_for(rel, rel_len), file_buf, (unsigned)got);
    }
}

namespace net {

    void serve_one(const config& cfg, void* ctx, read_fn rd, write_fn wr) {
        // Read until the header block is complete or the cap is hit.
        unsigned len = 0;
        request  r;
        parse_result pr = parse_result::need_more;

        while (len < sizeof(g_in)) {
            int got = rd(ctx, g_in + len, (unsigned)sizeof(g_in) - len);
            if (got <= 0) return;              // closed or errored: no reply
            len += (unsigned)got;

            pr = parse_request(g_in, len, &r);
            if (pr != parse_result::need_more) break;
        }

        if (pr != parse_result::ok) {
            reply(ctx, wr, 400, "text/plain", "bad request", 11);
            return;
        }

        if (!pin_ok(cfg, r)) {
            reply(ctx, wr, 401, "text/plain", "pin required", 12);
            return;
        }

        // Pull in whatever body was promised, up to what the buffer holds.
        unsigned body_len = r.content_length;
        if (body_len > sizeof(g_in) - r.header_bytes) {
            reply(ctx, wr, 413, "text/plain", "body too large", 14);
            return;
        }
        while (len < r.header_bytes + body_len) {
            int got = rd(ctx, g_in + len, (unsigned)sizeof(g_in) - len);
            if (got <= 0) return;
            len += (unsigned)got;
        }
        const char* body = g_in + r.header_bytes;

        if (span_eq(r.path, r.path_len, "/api/state") && r.m == method::get) {
            static char json[512];
            unsigned n = cfg.state(json, sizeof(json));
            if (!n) {
                reply(ctx, wr, 503, "text/plain", "no state yet", 12);
                return;
            }
            reply(ctx, wr, 200, "application/json", json, n);
            return;
        }

        if (span_eq(r.path, r.path_len, "/api/teleport") && r.m == method::post) {
            job j;
            j.kind   = job_kind::teleport;
            j.z      = 0.0f;
            j.ground = json_true(body, body_len, "ground");
            if (!json_number(body, body_len, "x", &j.x) ||
                !json_number(body, body_len, "y", &j.y)) {
                reply(ctx, wr, 400, "text/plain", "x and y required", 16);
                return;
            }
            json_number(body, body_len, "z", &j.z);

            if (!jobs().push(j)) {
                reply(ctx, wr, 503, "text/plain", "busy", 4);
                return;
            }
            reply(ctx, wr, 200, "application/json", "{\"ok\":true}", 11);
            return;
        }

        // Anything else under /api/ is a mistake, not a file.
        if (r.path_len >= 5 && memcmp(r.path, "/api/", 5) == 0) {
            reply(ctx, wr, 404, "text/plain", "not found", 9);
            return;
        }

        serve_static(cfg, ctx, wr, r.path, r.path_len);
    }
}
