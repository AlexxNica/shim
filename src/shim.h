// Copyright (c) 2016 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>

#define PROXY "cc-proxy"

/* The shim would be handling fixed number of predefined fds.
 * This would be signal fd, stdin fd and a proxy socket connection fd.
 */
#define MAX_POLL_FDS 3

struct cc_shim {
	char       *container_id;
	int         proxy_sock_fd;
	char       *token;
	int        timeout;		/* reconnection timeout to proxy */
	char       *proxy_address;
	int         proxy_port;
};

/* 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
 * ┌────────────────────────────┬───────────────┬───────────────┐
 * │          Version           │ Header Length │   Reserved    │
 * ├────────────────────────────┼─────┬─┬───────┼───────────────┤
 * │          Reserved          │ Res.│E│ Type  │    Opcode     │
 * ├────────────────────────────┴─────┴─┴───────┴───────────────┤
 * │                      Payload Length                        │
 * ├────────────────────────────────────────────────────────────┤
 * │                                                            │
 * │                         Payload                            │
 * │                                                            │
 * │      (variable length, optional and opcode-specific)       │
 * │                                                            │
 * └────────────────────────────────────────────────────────────┘
 */

// Header size is length of header in 32 bit words.
#define  MIN_HEADER_WORD_SIZE    3

// Minimum supported proxy version.
#define  PROXY_VERSION        2

// Sizes in bytes
#define  VERSION_SIZE         2
#define  HEADER_LEN_SIZE      1

// Offsets expressed as byte offsets within the header
#define  HEADER_LEN_OFFSET    2
#define  RES_OFFSET           6
#define  OPCODE_OFFSET        7
#define  PAYLOAD_LEN_OFFSET   8
#define  PAYLOAD_OFFSET       12

struct frame_header {
	uint16_t    version;
	uint8_t     header_len;
	uint8_t     err;
	uint8_t     type;
	uint8_t     opcode;
	uint32_t    payload_len; 
};

struct frame {
	struct   frame_header header;
	uint8_t *payload;
};

enum frametype {
	frametype_command = 0,
	frametype_response,
	frametype_stream,
	frametype_notification
};

enum command {
	cmd_registervm = 0,
	cmd_unregistervm,
	cmd_attachvm,
	cmd_hyper,
	cmd_connectshim,
	cmd_disconnectshim,
	cmd_signal,
};

enum stream {
	stream_stdin,
	stream_stdout,
	stream_stderr,
};

enum notificationtype {
	notification_exitcode = 0,
};
