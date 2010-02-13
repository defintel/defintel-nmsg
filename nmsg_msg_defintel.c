/* nmsg_msg_defintel.c - Defintel nmsg_msg modules */

/*
 * Copyright (c) 2010 by Network Defence Intelligence, Inc. ("Defintel")
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND DEFINTEL DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL DEFINTEL BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* Import. */

#include <nmsg.h>
#include <nmsg/msgmod_plugin.h>

#define nmsg_msgmod_ctx nmsg_msgmod_ctx_sink
#include "sink.c"
#undef nmsg_msgmod_ctx

/* Export. */

struct nmsg_msgmod_plugin *nmsg_msgmod_ctx_array[] = {
	&nmsg_msgmod_ctx_sink,
	NULL
};
