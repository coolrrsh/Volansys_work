#include <gst/gst.h>
#include <string.h>

typedef struct {
    GstElement *video_depay;
    GstElement *audio_depay;
} CustomData;

static void on_rtspsrc_pad_removed(GstElement *src, GstPad *old_pad, CustomData *data) {
    GstPad *sink_pad = NULL;

    /* Check if this was the pad linked to video or audio */
    if (gst_pad_is_linked(old_pad)) {
        sink_pad = gst_pad_get_peer(old_pad);

        if (sink_pad) {
            gst_pad_unlink(old_pad, sink_pad);

            /* Get the name of the parent element to print a nice message */
            GstElement *parent = gst_pad_get_parent_element(sink_pad);
            g_print("Unlinked stream from %s due to pad removal.\n",
                    gst_element_get_name(parent));

            gst_object_unref(parent);
            gst_object_unref(sink_pad);
        }
    }
}

/* Callback: Connects rtspsrc to our manual branches */
static void on_rtspsrc_pad_added(GstElement *src, GstPad *new_pad, CustomData *data) {
    GstCaps *caps = gst_pad_get_current_caps(new_pad);
    GstStructure *str = gst_caps_get_structure(caps, 0);
    const gchar *media = gst_structure_get_string(str, "media");
    GstElement *target_depay = NULL;

    if (g_strcmp0(media, "video") == 0) {
        target_depay = data->video_depay;
    } else if (g_strcmp0(media, "audio") == 0) {
        target_depay = data->audio_depay;
    }

    if (target_depay) {
        GstPad *sink_pad = gst_element_get_static_pad(target_depay, "sink");
        if (!gst_pad_is_linked(sink_pad)) {
            if (gst_pad_link(new_pad, sink_pad) == GST_PAD_LINK_OK) {
                g_print("Linked %s stream successfully.\n", media);
            }
        }
        gst_object_unref(sink_pad);
    }
    if (caps) gst_caps_unref(caps);
}

int main(int argc, char *argv[]) {
    GstElement *pipeline, *rtspsrc;
    GstElement *v_depay, *v_parse, *v_decode, *v_conv, *v_sink, *v_q;
    GstElement *a_depay, *a_parse, *a_decode, *a_conv, *a_sink, *a_q;
    CustomData data;
    GMainLoop *loop;

    if (argc < 2) {
        g_print("Usage: %s rtsp://127.0.0.1:8554/stream\n", argv[0]);
        return -1;
    }

    gst_init(&argc, &argv);
    loop = g_main_loop_new(NULL, FALSE);

    pipeline = gst_pipeline_new("manual-rtsp-client");
    rtspsrc  = gst_element_factory_make("rtspsrc", "src");

    /* --- VIDEO BRANCH --- */
    v_depay  = gst_element_factory_make("rtph264depay", "v_depay");
    v_parse  = gst_element_factory_make("h264parse", "v_parse");
    v_decode = gst_element_factory_make("avdec_h264", "v_decode");
    v_q      = gst_element_factory_make("queue", "v_q");
    v_conv   = gst_element_factory_make("videoconvert", "v_conv");
    v_sink   = gst_element_factory_make("autovideosink", "v_sink");

    /* --- AUDIO BRANCH --- */
    a_depay  = gst_element_factory_make("rtpopusdepay", "a_depay");
    a_parse  = gst_element_factory_make("opusparse", "a_parse");
    a_decode = gst_element_factory_make("opusdec", "a_decode");
    a_q      = gst_element_factory_make("queue", "a_q");
    a_conv   = gst_element_factory_make("audioconvert", "a_conv");
    a_sink   = gst_element_factory_make("autoaudiosink", "a_sink");

    if (!pipeline || !v_decode || !a_decode) {
        g_printerr("Check that gst-libav and gst-plugins-good are installed.\n");
        return -1;
    }

    /* Set properties: 0 latency is better for testing */
    g_object_set(rtspsrc, "location", argv[1], "latency", 0, NULL);
    g_object_set(v_sink, "sync", FALSE, NULL);
    g_object_set(a_sink, "sync", FALSE, NULL);

    /* Add to Bin */
    gst_bin_add_many(GST_BIN(pipeline), rtspsrc, 
                     v_depay, v_parse, v_decode, v_q, v_conv, v_sink,
                     a_depay, a_parse, a_decode, a_q, a_conv, a_sink, NULL);

    /* --- STATIC LINKING (Immediate) --- */
    gst_element_link_many(v_depay, v_parse, v_decode, v_q, v_conv, v_sink, NULL);
    gst_element_link_many(a_depay, a_parse, a_decode, a_q, a_conv, a_sink, NULL);

    /* --- DYNAMIC LINKING (Connect to Source) --- */
    data.video_depay = v_depay;
    data.audio_depay = a_depay;
    g_signal_connect(rtspsrc, "pad-added", G_CALLBACK(on_rtspsrc_pad_added), &data);
    g_signal_connect(rtspsrc, "pad-removed", G_CALLBACK(on_rtspsrc_pad_removed), &data);
    
    /* Start Playing */
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_print("Connecting to RTSP Server...\n");
    g_main_loop_run(loop);

    /* Cleanup */
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    return 0;
}
