/*
   Copyright (C) 2009-2015 Red Hat, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, see <http://www.gnu.org/licenses/>.
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "display-channel.h"

void display_channel_compress_stats_reset(DisplayChannel *display)
{
    spice_return_if_fail(display);

#ifdef COMPRESS_STAT
    stat_reset(&display->quic_stat);
    stat_reset(&display->lz_stat);
    stat_reset(&display->glz_stat);
    stat_reset(&display->jpeg_stat);
    stat_reset(&display->zlib_glz_stat);
    stat_reset(&display->jpeg_alpha_stat);
    stat_reset(&display->lz4_stat);
#endif
}

void display_channel_compress_stats_print(const DisplayChannel *display_channel)
{
    spice_return_if_fail(display_channel);

#ifdef COMPRESS_STAT
    uint64_t glz_enc_size;

    glz_enc_size = display_channel->enable_zlib_glz_wrap ?
                       display_channel->zlib_glz_stat.comp_size :
                       display_channel->glz_stat.comp_size;

    spice_info("==> Compression stats for display %u", display_channel->common.base.id);
    spice_info("Method   \t  count  \torig_size(MB)\tenc_size(MB)\tenc_time(s)");
    spice_info("QUIC     \t%8d\t%13.2f\t%12.2f\t%12.2f",
               display_channel->quic_stat.count,
               stat_byte_to_mega(display_channel->quic_stat.orig_size),
               stat_byte_to_mega(display_channel->quic_stat.comp_size),
               stat_cpu_time_to_sec(display_channel->quic_stat.total)
               );
    spice_info("GLZ      \t%8d\t%13.2f\t%12.2f\t%12.2f",
               display_channel->glz_stat.count,
               stat_byte_to_mega(display_channel->glz_stat.orig_size),
               stat_byte_to_mega(display_channel->glz_stat.comp_size),
               stat_cpu_time_to_sec(display_channel->glz_stat.total)
               );
    spice_info("ZLIB GLZ \t%8d\t%13.2f\t%12.2f\t%12.2f",
               display_channel->zlib_glz_stat.count,
               stat_byte_to_mega(display_channel->zlib_glz_stat.orig_size),
               stat_byte_to_mega(display_channel->zlib_glz_stat.comp_size),
               stat_cpu_time_to_sec(display_channel->zlib_glz_stat.total)
               );
    spice_info("LZ       \t%8d\t%13.2f\t%12.2f\t%12.2f",
               display_channel->lz_stat.count,
               stat_byte_to_mega(display_channel->lz_stat.orig_size),
               stat_byte_to_mega(display_channel->lz_stat.comp_size),
               stat_cpu_time_to_sec(display_channel->lz_stat.total)
               );
    spice_info("JPEG     \t%8d\t%13.2f\t%12.2f\t%12.2f",
               display_channel->jpeg_stat.count,
               stat_byte_to_mega(display_channel->jpeg_stat.orig_size),
               stat_byte_to_mega(display_channel->jpeg_stat.comp_size),
               stat_cpu_time_to_sec(display_channel->jpeg_stat.total)
               );
    spice_info("JPEG-RGBA\t%8d\t%13.2f\t%12.2f\t%12.2f",
               display_channel->jpeg_alpha_stat.count,
               stat_byte_to_mega(display_channel->jpeg_alpha_stat.orig_size),
               stat_byte_to_mega(display_channel->jpeg_alpha_stat.comp_size),
               stat_cpu_time_to_sec(display_channel->jpeg_alpha_stat.total)
               );
    spice_info("LZ4      \t%8d\t%13.2f\t%12.2f\t%12.2f",
               display_channel->lz4_stat.count,
               stat_byte_to_mega(display_channel->lz4_stat.orig_size),
               stat_byte_to_mega(display_channel->lz4_stat.comp_size),
               stat_cpu_time_to_sec(display_channel->lz4_stat.total)
               );
    spice_info("-------------------------------------------------------------------");
    spice_info("Total    \t%8d\t%13.2f\t%12.2f\t%12.2f",
               display_channel->lz_stat.count + display_channel->glz_stat.count +
                                                display_channel->quic_stat.count +
                                                display_channel->jpeg_stat.count +
                                                display_channel->lz4_stat.count +
                                                display_channel->jpeg_alpha_stat.count,
               stat_byte_to_mega(display_channel->lz_stat.orig_size +
                                 display_channel->glz_stat.orig_size +
                                 display_channel->quic_stat.orig_size +
                                 display_channel->jpeg_stat.orig_size +
                                 display_channel->lz4_stat.orig_size +
                                 display_channel->jpeg_alpha_stat.orig_size),
               stat_byte_to_mega(display_channel->lz_stat.comp_size +
                                 glz_enc_size +
                                 display_channel->quic_stat.comp_size +
                                 display_channel->jpeg_stat.comp_size +
                                 display_channel->lz4_stat.comp_size +
                                 display_channel->jpeg_alpha_stat.comp_size),
               stat_cpu_time_to_sec(display_channel->lz_stat.total +
                                    display_channel->glz_stat.total +
                                    display_channel->zlib_glz_stat.total +
                                    display_channel->quic_stat.total +
                                    display_channel->jpeg_stat.total +
                                    display_channel->lz4_stat.total +
                                    display_channel->jpeg_alpha_stat.total)
               );
#endif
}

DisplayChannelClient *dcc_new(DisplayChannel *display,
                              RedClient *client, RedsStream *stream,
                              int mig_target,
                              uint32_t *common_caps, int num_common_caps,
                              uint32_t *caps, int num_caps,
                              SpiceImageCompression image_compression,
                              spice_wan_compression_t jpeg_state,
                              spice_wan_compression_t zlib_glz_state)

{
    DisplayChannelClient *dcc;

    dcc = (DisplayChannelClient*)common_channel_new_client(
        (CommonChannel *)display, sizeof(DisplayChannelClient),
        client, stream, mig_target, TRUE,
        common_caps, num_common_caps,
        caps, num_caps);
    spice_return_val_if_fail(dcc, NULL);

    ring_init(&dcc->palette_cache_lru);
    dcc->palette_cache_available = CLIENT_PALETTE_CACHE_SIZE;
    dcc->image_compression = image_compression;
    dcc->jpeg_state = jpeg_state;
    dcc->zlib_glz_state = zlib_glz_state;

    return dcc;
}

void dcc_add_stream_agent_clip(DisplayChannelClient* dcc, StreamAgent *agent)
{
    StreamClipItem *item = stream_clip_item_new(dcc, agent);
    int n_rects;

    item->clip_type = SPICE_CLIP_TYPE_RECTS;

    n_rects = pixman_region32_n_rects(&agent->clip);
    item->rects = spice_malloc_n_m(n_rects, sizeof(SpiceRect), sizeof(SpiceClipRects));
    item->rects->num_rects = n_rects;
    region_ret_rects(&agent->clip, item->rects->rects, n_rects);

    red_channel_client_pipe_add(RED_CHANNEL_CLIENT(dcc), (PipeItem *)item);
}

MonitorsConfig* monitors_config_ref(MonitorsConfig *monitors_config)
{
    monitors_config->refs++;

    return monitors_config;
}

void monitors_config_unref(MonitorsConfig *monitors_config)
{
    if (!monitors_config) {
        return;
    }
    if (--monitors_config->refs != 0) {
        return;
    }

    spice_debug("freeing monitors config");
    free(monitors_config);
}

static void monitors_config_debug(MonitorsConfig *mc)
{
    int i;

    spice_debug("monitors config count:%d max:%d", mc->count, mc->max_allowed);
    for (i = 0; i < mc->count; i++)
        spice_debug("+%d+%d %dx%d",
                    mc->heads[i].x, mc->heads[i].y,
                    mc->heads[i].width, mc->heads[i].height);
}

MonitorsConfig* monitors_config_new(QXLHead *heads, ssize_t nheads, ssize_t max)
{
    MonitorsConfig *mc;

    mc = spice_malloc(sizeof(MonitorsConfig) + nheads * sizeof(QXLHead));
    mc->refs = 1;
    mc->count = nheads;
    mc->max_allowed = max;
    memcpy(mc->heads, heads, nheads * sizeof(QXLHead));
    monitors_config_debug(mc);

    return mc;
}

static MonitorsConfigItem *monitors_config_item_new(RedChannel* channel,
                                                    MonitorsConfig *monitors_config)
{
    MonitorsConfigItem *mci;

    mci = (MonitorsConfigItem *)spice_malloc(sizeof(*mci));
    mci->monitors_config = monitors_config;

    red_channel_pipe_item_init(channel,
                               &mci->pipe_item, PIPE_ITEM_TYPE_MONITORS_CONFIG);
    return mci;
}

static inline void red_monitors_config_item_add(DisplayChannelClient *dcc)
{
    DisplayChannel *dc = DCC_TO_DC(dcc);
    MonitorsConfigItem *mci;

    mci = monitors_config_item_new(dcc->common.base.channel,
                                   monitors_config_ref(dc->monitors_config));
    red_channel_client_pipe_add(&dcc->common.base, &mci->pipe_item);
}

void dcc_push_monitors_config(DisplayChannelClient *dcc)
{
    MonitorsConfig *monitors_config = DCC_TO_DC(dcc)->monitors_config;

    if (monitors_config == NULL) {
        spice_warning("monitors_config is NULL");
        return;
    }

    if (!red_channel_client_test_remote_cap(&dcc->common.base,
                                            SPICE_DISPLAY_CAP_MONITORS_CONFIG)) {
        return;
    }
    red_monitors_config_item_add(dcc);
    red_channel_client_push(&dcc->common.base);
}

static SurfaceDestroyItem *surface_destroy_item_new(RedChannel *channel,
                                                    uint32_t surface_id)
{
    SurfaceDestroyItem *destroy;

    destroy = spice_malloc(sizeof(SurfaceDestroyItem));
    destroy->surface_destroy.surface_id = surface_id;
    red_channel_pipe_item_init(channel, &destroy->pipe_item,
                               PIPE_ITEM_TYPE_DESTROY_SURFACE);

    return destroy;
}

void dcc_push_destroy_surface(DisplayChannelClient *dcc, uint32_t surface_id)
{
    DisplayChannel *display;
    RedChannel *channel;
    SurfaceDestroyItem *destroy;

    if (!dcc) {
        return;
    }

    display = DCC_TO_DC(dcc);
    channel = RED_CHANNEL(display);

    if (COMMON_CHANNEL(display)->during_target_migrate ||
        !dcc->surface_client_created[surface_id]) {
        return;
    }

    dcc->surface_client_created[surface_id] = FALSE;
    destroy = surface_destroy_item_new(channel, surface_id);
    red_channel_client_pipe_add(RED_CHANNEL_CLIENT(dcc), &destroy->pipe_item);
}

int display_channel_get_streams_timeout(DisplayChannel *display)
{
    int timeout = INT_MAX;
    Ring *ring = &display->streams;
    RingItem *item = ring;

    red_time_t now = red_get_monotonic_time();
    while ((item = ring_next(ring, item))) {
        Stream *stream;

        stream = SPICE_CONTAINEROF(item, Stream, link);
        red_time_t delta = (stream->last_time + RED_STREAM_TIMEOUT) - now;

        if (delta < 1000 * 1000) {
            return 0;
        }
        timeout = MIN(timeout, (unsigned int)(delta / (1000 * 1000)));
    }
    return timeout;
}

void display_channel_set_stream_video(DisplayChannel *display, int stream_video)
{
    spice_return_if_fail(display);
    spice_return_if_fail(stream_video != SPICE_STREAM_VIDEO_INVALID);

    switch (stream_video) {
    case SPICE_STREAM_VIDEO_ALL:
        spice_info("sv all");
        break;
    case SPICE_STREAM_VIDEO_FILTER:
        spice_info("sv filter");
        break;
    case SPICE_STREAM_VIDEO_OFF:
        spice_info("sv off");
        break;
    default:
        spice_warn_if_reached();
        return;
    }

    display->stream_video = stream_video;
}

static void stop_streams(DisplayChannel *display)
{
    Ring *ring = &display->streams;
    RingItem *item = ring_get_head(ring);

    while (item) {
        Stream *stream = SPICE_CONTAINEROF(item, Stream, link);
        item = ring_next(ring, item);
        if (!stream->current) {
            stream_stop(display, stream);
        } else {
            spice_info("attached stream");
        }
    }

    display->next_item_trace = 0;
    memset(display->items_trace, 0, sizeof(display->items_trace));
}

void display_channel_surface_unref(DisplayChannel *display, uint32_t surface_id)
{
    RedSurface *surface = &display->surfaces[surface_id];
    RedWorker *worker = COMMON_CHANNEL(display)->worker;
    QXLInstance *qxl = red_worker_get_qxl(worker);
    DisplayChannelClient *dcc;
    RingItem *link, *next;

    if (--surface->refs != 0) {
        return;
    }

    // only primary surface streams are supported
    if (is_primary_surface(display, surface_id)) {
        stop_streams(display);
    }
    spice_assert(surface->context.canvas);

    surface->context.canvas->ops->destroy(surface->context.canvas);
    if (surface->create.info) {
        qxl->st->qif->release_resource(qxl, surface->create);
    }
    if (surface->destroy.info) {
        qxl->st->qif->release_resource(qxl, surface->destroy);
    }

    region_destroy(&surface->draw_dirty_region);
    surface->context.canvas = NULL;
    FOREACH_DCC(display, link, next, dcc) {
        dcc_push_destroy_surface(dcc, surface_id);
    }

    spice_warn_if(!ring_is_empty(&surface->depend_on_me));
}

/* TODO: perhaps rename to "ready" or "realized" ? */
bool display_channel_surface_has_canvas(DisplayChannel *display,
                                        uint32_t surface_id)
{
    return display->surfaces[surface_id].context.canvas != NULL;
}
