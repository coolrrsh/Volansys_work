#include <gst/gst.h>

int main(int argc, char *argv[]) {
  GstElement *pipeline;
  GstBus *bus;
  GstMessage *msg;
  GMainLoop *loop;

  /* Initialize GStreamer */
  gst_init(&argc, &argv);
  loop = g_main_loop_new(NULL, FALSE);

  /* Create the playbin element */
  /* playbin automatically handles the RTSP connection and decoding */
  pipeline = gst_element_factory_make("playbin", "client-pipeline");

  if (!pipeline) {
    g_printerr("Not all elements could be created.\n");
    return -1;
  }

  /* Set the RTSP URL provided by your server */
  g_object_set(pipeline, "uri", "rtsp://127.0.0.1:8554/stream", NULL);

  /* Start playing */
  gst_element_set_state(pipeline, GST_STATE_PLAYING);
  g_print("Connecting to stream...\n");

  /* Wait until error or EOS */
  bus = gst_element_get_bus(pipeline);
  msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                   GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

  /* Free resources */
  if (msg != NULL)
    gst_message_unref(msg);
  
  gst_object_unref(bus);
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(pipeline);
  return 0;
}
