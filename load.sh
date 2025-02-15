#!/bin/bash
# This script installs necessary GStreamer packages and python3-pip.
# It checks the execution of each command and prints status messages.

# Function to run a command and check its result.
run_command() {
    echo "Executing: $1"
    eval "$1"
    if [ $? -eq 0 ]; then
        echo "Success: Command completed successfully."
    else
        echo "Error: Command failed. Exiting."
        exit 1
    fi
}

# Update package lists before installing
run_command "sudo apt-get update"

# Install the first set of GStreamer packages
echo "Installing first set of GStreamer packages..."
run_command "sudo apt-get install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav libgstrtspserver-1.0-dev"

# Install the second set of GStreamer packages
echo "Installing second set of GStreamer packages..."
run_command "sudo apt-get install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio"

# Install python3-pip
echo "Installing python3-pip..."
run_command "sudo apt install -y python3-pip"

echo "All installations completed successfully!"
