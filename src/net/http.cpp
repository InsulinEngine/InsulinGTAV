#include "net/http.h"

#include <string.h>
#include <stdio.h>

namespace {

    bool ci_equal(const char* a, unsigned a_len, const char* b) {
        unsigned n = 0;
        while (b[n]) n++;
        if (a_len != n) return false;
        for (unsigned i = 0; i < n; i++) {
            char x = a[i], y = b[i];
            if (x >= 'A' && x <= 'Z') x = (char)(x - 'A' + 'a');
            if (y >= 'A' && y <= 'Z') y = (char)(y - 'A' + 'a');
            if (x != y) return false;
        }
        return true;
    }

    unsigned to_uint(const char* p, unsigned len) {
        unsigned v = 0;
        for (unsigned i = 0; i < len; i++) {
            if (p[i] < '0' || p[i] > '9') break;
            v = v * 10 + (unsigned)(p[i] - '0');
        }
        return v;
    }

    const char* reason_for(int status) {
        switch (status) {
            case 200: return "OK";
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 413: return "Payload Too Large";
            case 503: return "Service Unavailable";
            default:  return "Error";
        }
    }
}

namespace net {

    parse_result parse_request(const char* buf, unsigned len, request* out) {
        // Find the end of the header block first. Refusing oversized blocks
        // before any parsing keeps the work bounded no matter what arrives.
        unsigned end = 0;
        bool found = false;
        unsigned scan_to = len < header_cap ? len : header_cap;
        for (unsigned i = 0; i + 3 < scan_to; i++) {
            if (buf[i] == '\r' && buf[i + 1] == '\n' && buf[i + 2] == '\r' && buf[i + 3] == '\n') {
                end = i + 4;
                found = true;
                break;
            }
        }
        if (!found) return len >= header_cap ? parse_result::bad : parse_result::need_more;

        out->m              = method::other;
        out->path           = 0;
        out->path_len       = 0;
        out->query          = buf;   // empty span, never null
        out->query_len      = 0;
        out->content_length = 0;
        out->pin            = 0;
        out->pin_len        = 0;
        out->header_bytes   = end;

        // Request line: METHOD SP TARGET SP VERSION CRLF
        unsigned sp1 = 0;
        while (sp1 < end && buf[sp1] != ' ' && buf[sp1] != '\r') sp1++;
        if (sp1 >= end || buf[sp1] != ' ') return parse_result::bad;

        if      (ci_equal(buf, sp1, "GET"))  out->m = method::get;
        else if (ci_equal(buf, sp1, "PUT"))  out->m = method::put;
        else if (ci_equal(buf, sp1, "POST")) out->m = method::post;
        else                                 return parse_result::bad;

        unsigned target = sp1 + 1;
        unsigned sp2 = target;
        while (sp2 < end && buf[sp2] != ' ' && buf[sp2] != '\r') sp2++;
        if (sp2 >= end || buf[sp2] != ' ') return parse_result::bad;
        if (sp2 == target) return parse_result::bad;

        unsigned q = target;
        while (q < sp2 && buf[q] != '?') q++;
        out->path     = buf + target;
        out->path_len = q - target;
        if (q < sp2) {
            out->query     = buf + q + 1;
            out->query_len = sp2 - (q + 1);
        }

        // Headers. Each line is NAME ":" OWS VALUE CRLF.
        unsigned i = target;
        while (i + 1 < end && !(buf[i] == '\r' && buf[i + 1] == '\n')) i++;
        i += 2;

        while (i + 1 < end && !(buf[i] == '\r' && buf[i + 1] == '\n')) {
            unsigned line_end = i;
            while (line_end + 1 < end && !(buf[line_end] == '\r' && buf[line_end + 1] == '\n')) line_end++;

            unsigned colon = i;
            while (colon < line_end && buf[colon] != ':') colon++;
            if (colon < line_end) {
                unsigned name_len = colon - i;
                unsigned v = colon + 1;
                while (v < line_end && (buf[v] == ' ' || buf[v] == '\t')) v++;
                unsigned v_len = line_end - v;

                if (ci_equal(buf + i, name_len, "content-length")) {
                    out->content_length = to_uint(buf + v, v_len);
                } else if (ci_equal(buf + i, name_len, "x-insulin-pin")) {
                    out->pin     = buf + v;
                    out->pin_len = v_len;
                }
            }
            i = line_end + 2;
        }

        return parse_result::ok;
    }

    unsigned write_response(char* out, unsigned cap,
                            int status, const char* content_type,
                            const char* body, unsigned body_len) {
        if (!body) body_len = 0;

        // snprintf reports what it *would* have written, so an overlong header
        // is caught before any of the body is copied.
        int n = snprintf(out, cap,
                         "HTTP/1.1 %d %s\r\n"
                         "Content-Type: %s\r\n"
                         "Content-Length: %u\r\n"
                         "Connection: close\r\n"
                         "\r\n",
                         status, reason_for(status),
                         content_type ? content_type : "application/octet-stream",
                         body_len);
        if (n < 0 || (unsigned)n >= cap) return 0;
        if ((unsigned)n + body_len > cap) return 0;

        if (body_len) memcpy(out + n, body, body_len);
        return (unsigned)n + body_len;
    }
}
