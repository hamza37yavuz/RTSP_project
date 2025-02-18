#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <gst/video/video.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <glib.h>
#include <cairo.h>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <math.h>

// Global variable to hold the current filter mode (0: none, 1: Contrast, 2: Gamma Correction, 3: Brightness, 
// 4: Saturation, 5: Hue Shift, 6: Threshold, 7: Invert)
static int filterMode = 0; 
// Mutex to ensure thread-safe access to filterMode
GMutex global_mutex;

// Thread function for the command server to receive filter commands via TCP
void *command_server_thread(void *arg) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024];

    // Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        return NULL;
    }

    int opt = 1;
    // Set socket options to reuse address
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        return NULL;
    }

    // Configure socket address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9000);

    // Bind the socket to port 9000
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return NULL;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        return NULL;
    }
    printf("Command server listening on port 9000\n");

    // Main loop: accept connections and process commands
    while (1) {
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }
        int valread = read(client_fd, buffer, sizeof(buffer)-1);
        if (valread > 0) {
            buffer[valread] = '\0';
            // Lock mutex before updating filterMode
            g_mutex_lock(&global_mutex);
            // Check the received command and set the corresponding filter mode
            if (strcmp(buffer, "a") == 0)
                filterMode = 1; // Contrast
            else if (strcmp(buffer, "c") == 0)
                filterMode = 2; // Gamma Correction
            else if (strcmp(buffer, "b") == 0)
                filterMode = 3; // Brightness
            else if (strcmp(buffer, "d") == 0)
                filterMode = 4; // Saturation
            else if (strcmp(buffer, "e") == 0)
                filterMode = 5; // Hue Shift
            else if (strcmp(buffer, "m") == 0)
                filterMode = 6; // Threshold
            else if (strcmp(buffer, "r") == 0)
                filterMode = 7; // Invert
            else if (strcmp(buffer, "n") == 0)
                filterMode = 0; // None
            else
                printf("Unknown command received: %s\n", buffer);
            // Unlock mutex after updating filterMode
            g_mutex_unlock(&global_mutex);
        }
        close(client_fd); // Close client connection
    }
    close(server_fd); // Close server socket (unreachable in this loop)
    return NULL;
}

// GStreamer pad probe callback for processing video frame buffers in real time
static GstPadProbeReturn filter_pad_probe_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    // Only process buffer type probes
    if (!(info->type & GST_PAD_PROBE_TYPE_BUFFER))
        return GST_PAD_PROBE_OK;
    GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buffer)
        return GST_PAD_PROBE_OK;
    GstMapInfo map;
    // Map buffer for read/write access
    if (gst_buffer_map(buffer, &map, GST_MAP_READWRITE)) {
        // Get the peer pad and current caps to retrieve frame details
        GstPad *srcpad = gst_pad_get_peer(pad);
        GstCaps *caps = gst_pad_get_current_caps(srcpad);
        if (!caps) {
            gst_buffer_unmap(buffer, &map);
            gst_object_unref(srcpad);
            return GST_PAD_PROBE_OK;
        }
        // Get frame width and height from caps
        GstStructure *s = gst_caps_get_structure(caps, 0);
        int width, height;
        if (!gst_structure_get_int(s, "width", &width) || !gst_structure_get_int(s, "height", &height)) {
            gst_caps_unref(caps);
            gst_object_unref(srcpad);
            gst_buffer_unmap(buffer, &map);
            return GST_PAD_PROBE_OK;
        }
        
        // Try to obtain the stride information; if not available, assume 3 channels per pixel
        int stride = 0;
        if (!gst_structure_get_int(s, "stride", &stride)) {
            stride = width * 3; // Default: 3 channels
        }
        
        gst_caps_unref(caps);
        gst_object_unref(srcpad);

        int expectedSize = stride * height;
        // Check if the mapped buffer has enough data for a complete frame
        if ((int)map.size >= expectedSize) {
            // Create an OpenCV matrix from the buffer data using the provided stride
            cv::Mat frame(height, width, CV_8UC3, (void*)map.data, stride);
            int currentMode = 0;
            // Lock mutex to safely retrieve current filter mode
            g_mutex_lock(&global_mutex);
            currentMode = filterMode;
            g_mutex_unlock(&global_mutex);

            // Apply image processing based on the current filter mode
            if (currentMode == 1) {
                // Contrast Adjustment: Multiply pixel values by 1.5 to enhance contrast
                cv::Mat temp = frame.clone();
                double alpha = 1.5;
                temp.convertTo(frame, -1, alpha, 0);
            } else if (currentMode == 2) {
                // Gamma Correction: Use a lookup table to apply gamma correction with gamma=2.2
                cv::Mat temp = frame.clone();
                cv::Mat lut(1, 256, CV_8UC1);
                double gamma = 2.2;
                for (int i = 0; i < 256; i++) {
                    lut.at<uchar>(i) = cv::saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);
                }
                cv::LUT(temp, lut, frame);
            } else if (currentMode == 3) {
                // Brightness Enhancement: Increase brightness by adding 50 to each pixel channel
                frame += cv::Scalar(50, 50, 50);
            } else if (currentMode == 4) {
                // Saturation Boost: Convert frame to HSV, boost saturation, then convert back to BGR
                cv::Mat temp = frame.clone();
                cv::Mat hsv;
                cv::cvtColor(temp, hsv, cv::COLOR_BGR2HSV);
                for (int y = 0; y < hsv.rows; y++) {
                    for (int x = 0; x < hsv.cols; x++) {
                        cv::Vec3b &pixel = hsv.at<cv::Vec3b>(y, x);
                        int s = pixel[1];
                        s = cv::saturate_cast<uchar>(s * 1.5); // Increase saturation by 50%
                        pixel[1] = s;
                    }
                }
                cv::cvtColor(hsv, frame, cv::COLOR_HSV2BGR);
            } else if (currentMode == 5) {
                // Hue Shift: Adjust the hue by adding 20 degrees in the HSV color space (modulo 180)
                cv::Mat temp = frame.clone();
                cv::Mat hsv;
                cv::cvtColor(temp, hsv, cv::COLOR_BGR2HSV);
                for (int y = 0; y < hsv.rows; y++) {
                    for (int x = 0; x < hsv.cols; x++) {
                        cv::Vec3b &pixel = hsv.at<cv::Vec3b>(y, x);
                        int h = pixel[0];
                        h = (h + 20) % 180;
                        pixel[0] = h;
                    }
                }
                cv::cvtColor(hsv, frame, cv::COLOR_HSV2BGR);
            } else if (currentMode == 6) {
                // Thresholding: Convert to grayscale, apply binary threshold, then convert back to BGR
                cv::Mat temp = frame.clone();
                cv::Mat gray;
                cv::cvtColor(temp, gray, cv::COLOR_BGR2GRAY);
                cv::Mat thresh;
                cv::threshold(gray, thresh, 128, 255, cv::THRESH_BINARY);
                cv::cvtColor(thresh, frame, cv::COLOR_GRAY2BGR);
            } else if (currentMode == 7) {
                // Inversion: Invert the image colors using bitwise NOT operation
                cv::Mat temp = frame.clone();
                cv::bitwise_not(temp, frame);
            }
        }
        // Unmap the buffer after processing
        gst_buffer_unmap(buffer, &map);
    }
    return GST_PAD_PROBE_OK;
}

// Callback for drawing an overlay on the video stream using Cairo
static void overlay_draw_callback(GstElement *overlay, cairo_t *cr, guint64 timestamp, GstVideoFrame *video_frame, gpointer user_data) {
    int currentMode = 0;
    // Lock mutex to get the current filter mode
    g_mutex_lock(&global_mutex);
    currentMode = filterMode;
    g_mutex_unlock(&global_mutex);
    // Determine the mode name based on the current filter mode
    const char* modeName = "";
    switch(currentMode) {
        case 0: modeName = "None"; break;
        case 1: modeName = "Contrast"; break;
        case 2: modeName = "Gamma Correction"; break;
        case 3: modeName = "Brightness"; break;
        case 4: modeName = "Saturation"; break;
        case 5: modeName = "Hue Shift"; break;
        case 6: modeName = "Threshold"; break;
        case 7: modeName = "Invert"; break;
        default: modeName = "Unknown"; break;
    }
    // Draw a semi-transparent rectangle as the background for the overlay text
    cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
    cairo_rectangle(cr, 5, 5, 250, 45);
    cairo_fill(cr);
    // Set text properties and draw the current filter mode on the overlay
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_font_size(cr, 32);
    cairo_move_to(cr, 10, 40);
    cairo_show_text(cr, modeName);
}

// Callback function for configuring media, setting up the overlay and pad probe for image processing
static void media_configure_callback(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data) {
    // Retrieve the RTSP media element
    GstElement *element = gst_rtsp_media_get_element(media);
    // Get the overlay element by name from the media element's bin
    GstElement *overlay = gst_bin_get_by_name(GST_BIN(element), "overlay");
    if (overlay) {
        // Connect the overlay's draw signal to our callback for rendering the filter mode text
        g_signal_connect(overlay, "draw", G_CALLBACK(overlay_draw_callback), NULL);
        // Get the sink pad of the overlay element
        GstPad *overlaySinkPad = gst_element_get_static_pad(overlay, "sink");
        if (overlaySinkPad) {
            // Add a pad probe to process each video buffer through our image processing callback
            gst_pad_add_probe(overlaySinkPad, GST_PAD_PROBE_TYPE_BUFFER,
                                (GstPadProbeCallback)filter_pad_probe_cb, NULL, NULL);
            gst_object_unref(overlaySinkPad);
        }
        gst_object_unref(overlay);
    }
    gst_object_unref(element);
}

// Main function to initialize GStreamer, create the RTSP server, and start the main loop
int main(int argc, char *argv[]) {
    // Initialize GStreamer with command-line arguments
    gst_init(&argc, &argv);
    // Initialize the global mutex for filter mode synchronization
    g_mutex_init(&global_mutex);
    pthread_t cmd_thread;
    // Create a new thread for the command server to receive filter commands
    if (pthread_create(&cmd_thread, NULL, command_server_thread, NULL) != 0) {
        perror("Failed to create command server thread");
        return -1;
    }
    // Create a new RTSP server instance
    GstRTSPServer *rtsp_server = gst_rtsp_server_new();
    // Set the RTSP server's listening address
    gst_rtsp_server_set_address(rtsp_server, "0.0.0.0");
    // Retrieve the mount points for the RTSP server
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(rtsp_server);
    // Create a new media factory for the RTSP server
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
    // Set the launch pipeline for the RTSP media factory, which defines the streaming pipeline
    gst_rtsp_media_factory_set_launch(factory,
        "( filesrc location=test.mp4 ! qtdemux name=demux "
        "demux. ! queue ! h264parse ! avdec_h264 ! videoconvert ! cairooverlay name=overlay ! "
        "videoconvert ! x264enc tune=zerolatency profile=main ! h264parse ! rtph264pay name=pay0 pt=96 )"
    );
    // Connect the media-configure signal to our callback for overlay and image processing setup
    g_signal_connect(factory, "media-configure", G_CALLBACK(media_configure_callback), NULL);
    // Add the media factory to the RTSP server's mount points with the specified stream path
    gst_rtsp_mount_points_add_factory(mounts, "/stream", factory);
    g_object_unref(mounts);
    // Attach the RTSP server to the default main context
    gst_rtsp_server_attach(rtsp_server, NULL);
    // Print the RTSP stream URL to the console
    g_print("RTSP Stream available at: rtsp://<PUBLIC_IP>:8554/stream\n");
    // Create and run the main loop to process GStreamer events
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);
    return 0;
}
