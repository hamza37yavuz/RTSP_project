# Real-Time RTSP Streaming: An Integrated Client-Server Architecture

## 1. Project Description

This project is aimed at providing a real-time streaming solution based on the RTSP protocol by integrating a high-performance C++ server with a versatile Python client to manage and deliver multimedia content. The server efficiently handles RTSP sessions by processing control commands over TCP while transmitting media data over UDP, ensuring low latency and reliable streaming performance. Additionally, the C++ server integrates several real-time image processing methods using OpenCV, including contrast adjustment, gamma correction, brightness enhancement, saturation boosting, hue shifting, thresholding, and image inversion. These processing techniques offer dynamic visual modifications to enhance the quality and versatility of the streamed video content. An automated installation script further simplifies the setup process, making system deployment and maintenance easier, and overall, the project is designed for adaptable multimedia streaming.

## 2. Technologies Used

• GStreamer: Used for multimedia processing and creating the streaming pipeline.
• GStreamer RTSP Server: Provides media streaming via the RTSP protocol.
• C/C++ Programming Language: Powers the RTSP server and image processing functions.
• Python Programming Language: Implements the client for managing streaming sessions and dynamic visual feedback.
• OpenCV: Applies real-time image processing techniques like contrast, brightness, and hue adjustments.
• Cairo: Draws overlays on video streams for visual mode indication.
• POSIX Threads (pthread): Enables multithreaded operations for concurrent command handling.
• Linux Socket Programming: Handles efficient TCP/UDP communication between server and client.
• Vast.ai: Provides the cloud infrastructure required to deploy and test the high-performance streaming solution.

## 3. Code Description

### 3.1. Server Code

#### a. Library Inclusions & Global Variables
- The server code includes GStreamer libraries for multimedia streaming and RTSP server functionality.
- OpenCV is integrated for image processing, Cairo is used for drawing overlays, and pthread handles multithreading.
- A global variable “filterMode” (with modes 0–7) and a GMutex are defined for thread-safe operations.

#### b. Command Server Thread
- A TCP socket is created to listen on port 9000 for incoming command connections.
- It reads short command messages ("a", "b", "c", "d", "e", "m", "r", "n") and updates the global filterMode accordingly to switch between various image processing filters.
- Mutex locks ensure thread safety when modifying filterMode.

#### c. GStreamer Pad Probe Callback
- A probe is attached to the overlay element’s sink pad to access video frame buffers in real time.
- Each buffer is mapped into a cv::Mat using the frame’s width, height, and stride.
- Image processing is applied based on the current filter mode:
  - Contrast Adjustment: Multiplies pixel values by 1.5.
  - Gamma Correction: Uses a lookup table with a gamma of 2.2.
  - Brightness Increase: Adds a constant value (50) to each pixel.
  - Saturation Boost: Converts to HSV, increases saturation, then converts back.
  - Hue Shift: Adjusts hue by 20 degrees in the HSV color space.
  - Thresholding: Converts to grayscale, applies a binary threshold, and converts back to color.
  - Inversion: Inverts the image colors using bitwise operations.

#### d. Overlay Draw Callback
- Cairo is used to draw overlay text indicating the current filter mode on the video stream.

#### e. Media Configure Callback
- The RTSP media element is retrieved and the overlay element is located by name.
- The overlay’s "draw" signal is connected to the overlay draw callback, and the pad probe callback is added to the overlay’s sink pad to ensure real-time processing.

#### f. Main Function & RTSP Server Setup
- GStreamer and the global mutex are initialized.
- The command server thread is started for dynamic filter control.
- An RTSP server is created and configured, including setting the listening address and mounting points.
- The GStreamer pipeline is set up with a file source, demuxing, decoding, video conversion, and encoding, along with a Cairo overlay for drawing.
- The RTSP server is attached and the main loop is started, making the stream available via RTSP.

### 3.2. Python Code Explanation (Client Side)

#### a. Library Imports
- The Python client imports cv2 for video capture and display, and socket for TCP communication.

#### b. Command Sending Function (send_command)
- A TCP connection is established to the command server.
- A command string is encoded and sent to control the server’s image processing mode.
- Exception handling is used to catch and report errors during transmission.

#### c. Main Function (main)
- RTSP Stream Capture:
  - The RTSP URL and command server details (IP and port) are defined.
  - cv2.VideoCapture is used to open the RTSP stream.
  - The code checks if the stream opens successfully; if not, an error is reported.
- Frame Processing Loop:
  - Frames are continuously read from the RTSP stream.
  - Each frame is displayed using cv2.imshow.
  - Keyboard input is monitored:
    - Pressing 'x' exits the loop and closes the stream.
    - Valid command keys (from the set "nrmedbca") trigger the send_command function to send the corresponding command to the server.
- Cleanup:
  - The video capture is released and any OpenCV windows are closed upon termination.

## 4. Instructions To Run The Code

A machine running Ubuntu 22.04 was rented from Vast.ai to install and test the system. The provided load.sh script automates the entire installation process, making it easy to set up the required environment on any machine with Ubuntu 22.04. The script begins by updating the package lists:
```
sudo apt-get update
```
It then installs all necessary GStreamer packages, Python3-pip, and additional libraries using commands such as:
```
sudo apt-get install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio libgstrtspserver-1.0-dev
```
The script also downloads an MP4 file from Google Drive, renames it, and organizes the file needed for the project. Once the environment is set up, the script compiles the C++ code by calling g++ with the appropriate flags:
```
g++ -o my_rtsp_server rtsp_server.cpp $(pkg-config --cflags --libs opencv4) $(pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0 glib-2.0 cairo) -pthread
```
The installation and compilation steps for the server are simplified with the help of load.sh. To run the server, use the following commands:
```
git clone https://github.com/hamza37yavuz/RTSP_project.git  
cd RTSP_project  
chmod +x load.sh  
./load.sh  
./my_rtsp_server
```
To run the client, use these commands:
```
git clone https://github.com/hamza37yavuz/RTSP_project.git  
cd RTSP_project  
pip install opencv-python  
python mainClient.py
```
This process ensures that all dependencies are correctly configured and the RTSP server is successfully created. In terms of performance, with no filter applied the stream reaches about 25 fps, which remains constant with the inversion filter. However, applying the contrast filter causes the frame rate to drop to about 20 fps, and using the saturation filter reduces the frame rate further to about 15 fps. On average, other filters provide frame rates between 15 and 25 fps. These fluctuations are partly due to the virtual machine environment and regional factors. Thanks to the automated installation and compilation process performed by load.sh, deploying a high-performance RTSP streaming solution becomes easy and efficient.

## 5. References

https://gstreamer.freedesktop.org/documentation/installing/on-linux.html?gi-language=c
