/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details:
 *
 * Copyright (C) 2015 Riccardo Vangelisti <riccardo.vangelisti@sadel.it>
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <ModemManager.h>
#define _LIBMM_INSIDE_MM
#include <libmm-glib.h>

#include "mm-log.h"
#include "mm-base-modem-at.h"
#include "mm-call-huawei.h"

G_DEFINE_TYPE (MMCallHuawei, mm_call_huawei, MM_TYPE_BASE_CALL)

/*****************************************************************************/
/* Start the CALL */

typedef struct {
    MMBaseCall *self;
    MMBaseModem *modem;
    GSimpleAsyncResult *result;
} CallStartContext;

static void
call_start_context_complete_and_free (CallStartContext *ctx)
{
    g_simple_async_result_complete_in_idle (ctx->result);
    g_object_unref (ctx->result);
    g_object_unref (ctx->modem);
    g_object_unref (ctx->self);
    g_free (ctx);
}

static void
call_start_audio_ready (MMBaseModem *modem,
                        GAsyncResult *res,
                        CallStartContext *ctx)
{
    GError *error = NULL;
    const gchar *response = NULL;

    response = mm_base_modem_at_command_finish (modem, res, &error);
    if (error) {
        mm_dbg ("Couldn't start call audio: '%s'", error->message);
        g_simple_async_result_take_error (ctx->result, error);
        call_start_context_complete_and_free (ctx);
        return;
    }

    g_simple_async_result_set_op_res_gboolean (ctx->result, TRUE);
    call_start_context_complete_and_free (ctx);
}

static void
parent_call_start_ready (MMBaseCall *self,
                         GAsyncResult *res,
                         CallStartContext *ctx)
{
    GError *error = NULL;

    if (!MM_BASE_CALL_CLASS (mm_call_huawei_parent_class)->start_finish (self, res, &error)) {
        g_simple_async_result_take_error (ctx->result, error);
        call_start_context_complete_and_free (ctx);
        return;
    }

    /* Enable audio streaming on the audio port */
    mm_base_modem_at_command (ctx->modem,
                              "AT^DDSETEX=2",
                              5,
                              FALSE,
                              (GAsyncReadyCallback)call_start_audio_ready,
                              ctx);
}

static void
call_start (MMBaseCall *self,
            GAsyncReadyCallback callback,
            gpointer user_data)
{
    CallStartContext *ctx;

    /* Setup the context */
    ctx = g_new0 (CallStartContext, 1);
    ctx->result = g_simple_async_result_new (G_OBJECT (self),
                                             callback,
                                             user_data,
                                             call_start);
    ctx->self = g_object_ref (self);
    g_object_get (ctx->self,
                  MM_BASE_CALL_MODEM, &ctx->modem,
                  NULL);

    /* Chain up parent's dial sequence */
    MM_BASE_CALL_CLASS (mm_call_huawei_parent_class)->start (
        MM_BASE_CALL (self),
        (GAsyncReadyCallback)parent_call_start_ready,
        ctx);
}

/*****************************************************************************/

MMBaseCall *
mm_call_huawei_new (MMBaseModem *modem)
{
    return MM_BASE_CALL (g_object_new (MM_TYPE_CALL_HUAWEI,
                                       MM_BASE_CALL_MODEM, modem,
                                       NULL));
}

static void
mm_call_huawei_init (MMCallHuawei *self)
{
}

static void
mm_call_huawei_class_init (MMCallHuaweiClass *klass)
{
    MMBaseCallClass *base_call_class = MM_BASE_CALL_CLASS (klass);

    base_call_class->start = call_start;
}
