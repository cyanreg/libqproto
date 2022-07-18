/* 
 * Copyright © 2022 Lynne
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the “Software”), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LIBQPROTO_INPUT_HEADER
#define LIBQPROTO_INPUT_HEADER

#include <libqproto/common.h>

typedef struct QprotoInputSource {
    enum QprotoConnectionType type;
    union {
        int reverse;
        const char *url;
        int (*read_input)(void *opaque, QprotoBuffer *data, int64_t offset);
    };

    void *opaque;
} QprotoInputSource;

/* List of callbacks. All are optional. */
typedef struct QprotoInputCallbacks {
    void (*control_cb)(void *opaque, int cease, int resend_init, int error,
                       uint8_t redirect[16], uint16_t redirect_port,
                       int seek_requested, int64_t seek_offset, uint32_t seek_seq);

    void (*feedback_cb)(void *opaque, QprotoStream *st, uint64_t epoch_offset,
                        uint64_t bandwidth, uint32_t fec_corrections,
                        uint32_t corrupt_packets, uint32_t missing_packets);

    void (*stream_register_cb)(void *opaque, QprotoStream *st, QprotoStreamOptions *opts);
    void (*font_register_cb)(void *opaque, QprotoBuffer *data, const char *name);
    void (*epoch_cb)(void *opaque, uint64_t epoch);

    void (*metadata_cb)(void *opaque, QprotoStream *st, QprotoMetadata *meta);
    void (*stream_pkt_cb)(void *opaque, QprotoStream *st, QprotoPacket *pkt, int present);
    void (*user_pkt_cb)(void *opaque, QprotoBuffer *data, uint16_t descriptor);

    void (*stream_close_cb)(void *opaque, QprotoStream *st);
} QprotoInputCallbacks;

typedef struct QprotoInputOptions {
    /* Whether to always check and correct using Raptor codes in the headers. */
    int always_test_headers;
} QprotoInputOptions;

/* Open a Qproto stream or a file for reading. */
int qp_input_open(QprotoContext *qp, QprotoInputSource *in,
                  QprotoInputCallbacks *cb,
                  QprotoInputOptions *opts);

/* Adjusts input options on the fly. */
int qp_input_set_options(QprotoContext *qp, QprotoInputOptions *opts);

/* Seek into the stream, if possible. */
int qp_input_seek(QprotoContext *qp, QprotoStream *st, int64_t offset, int absolute);

/* Process a single packet and call its relevant callback. If no input is
 * available within the timeout duration (nanoseconds),
 * will return QP_ERROR(EAGAIN).
 * Can be called multiple times from different threads. */
int qp_input_process(QprotoContext *qp, int64_t timeout);

/* Start a thread that will call qp_input_process as data becomes available. */
int qp_input_start_thread(QprotoContext *qp);

#endif
