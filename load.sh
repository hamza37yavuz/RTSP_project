#!/bin/bash
# This script installs the necessary GStreamer packages, python3-pip, and related Python packages.
# It also clones the yolov7 repository and converts the model to ONNX format.
# Each command execution is checked, and status messages are displayed.

# Function to execute a command and check its result
run_command() {
    echo "Executing command: $1"
    eval "$1"
    if [ $? -eq 0 ]; then
        echo "Success: Command completed successfully."
    else
        echo "Error: Command failed. Exiting."
        exit 1
    fi
}

# Update package lists
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

# Install other required packages
echo "Installing additional required packages..."
run_command "sudo apt-get install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstrtspserver-1.0-dev libglib2.0-dev libcairo2-dev libopencv-dev pkg-config"

# Install gdown for downloading Google Drive folders
echo "Installing gdown..."
run_command "pip install gdown"

# Download dataset from Google Drive
echo "Downloading dataset from Google Drive..."
run_command "gdown --folder https://drive.google.com/drive/folders/1JqsFmPwo3tIQys_B7KUs5t3lC7IOQxgw"

# Rename and move dataset files
echo "Renaming and moving dataset files..."
run_command "cd TUYZ_2021_Ornek_Veri_Seti && mv '2021 Örnek Video.mp4' test.mp4 && mv test.mp4 .. && cd .."

# Remove the downloaded folder
echo "Cleaning up dataset folder..."
run_command "rm -rf TUYZ_2021_Ornek_Veri_Seti"

echo "All installations and operations have been successfully completed!"
