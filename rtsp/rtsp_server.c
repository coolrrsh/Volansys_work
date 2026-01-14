#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <gst/rtsp-server/rtsp-media.h>
#include <gst/sdp/gstsdpmessage.h>
/*
static void on_media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data) {
    // This is called when a client connects and the SDP is being prepared
    GstRTSPSessionPool *pool;
    GstSDPMessage *sdp;
    
    // Get the SDP representation of the media
    sdp = gst_rtsp_media_get_sdp_message(media);
    
    gchar *sdp_text = gst_sdp_message_as_text(sdp);
    g_print("--- Generated SDP for Client ---\n%s\n-------------------------------\n", sdp_text);
    
    g_free(sdp_text);
    gst_sdp_message_free(sdp);
}
*/
static void on_media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data) {
    GstSDPMessage *sdp;

    // Create a new empty SDP message
    gst_sdp_message_new(&sdp);

    // Ask the media to fill the SDP message with its configuration
    // This is the modern replacement for the missing function
    gst_rtsp_media_setup_sdp(media, sdp, NULL);

    gchar *sdp_text = gst_sdp_message_as_text(sdp);
    g_print("--- Generated SDP for RTSP Receiver ---\n%s\n---------------------------------------\n", sdp_text);

    g_free(sdp_text);
    gst_sdp_message_free(sdp);
}


int main (int argc, char *argv[]) {
/* initialize the components */  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactory *factory;gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);
  server = gst_rtsp_server_new ();
  mounts = gst_rtsp_server_get_mount_points (server);
  factory = gst_rtsp_media_factory_new ();/* feed the default src file */
  gst_rtsp_media_factory_set_launch (factory, "( " "videotestsrc is-live=1 ! x264enc tune=zerolatency !  h264parse ! rtph264pay name=pay0 pt=96 " "audiotestsrc is-live=1 ! audioconvert ! opusenc ! rtpopuspay name=pay1 pt=97 " ")");
  gst_rtsp_media_factory_set_shared (factory, TRUE);
  gst_rtsp_mount_points_add_factory (mounts, "/stream", factory);
  g_object_unref (mounts);
  g_signal_connect(factory, "media-configure", G_CALLBACK(on_media_configure), NULL);
  gst_rtsp_server_attach (server, NULL);/* show current stream URL to user */
  g_print ("stream ready at rtsp://127.0.0.1:8554/stream\n");
  g_main_loop_run (loop);
  return 0;
}


