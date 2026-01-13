CC = gcc
CFLAGS = `pkg-config --cflags gstreamer-1.0 gstreamer-rtsp-server-1.0 gstreamer-sdp-1.0`
LIBS = `pkg-config --libs gstreamer-1.0 gstreamer-rtsp-server-1.0 gstreamer-sdp-1.0`

all:    rtsp_server rtsp_client
rtsp_server:
	$(CC) rtsp_server.c -o rtsp-server $(CFLAGS) $(LIBS)
rtsp_client:
	$(CC) rtsp_client.c -o rtsp-client $(CFLAGS) $(LIBS)


#all: rtsp_client.c
#	gcc rtsp_client.c -o rtsp_client `pkg-config --cflags --libs gstreamer-1.0`

clean: rtsp_client rtsp_server
	rm rtsp-client rtsp-server
