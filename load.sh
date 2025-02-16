#!/bin/bash
# Bu betik, gerekli GStreamer paketlerini, python3-pip'i ve ilgili Python paketlerini kurar.
# Ayrıca, yolov7 deposunu klonlar ve modeli ONNX formatına dönüştürür.
# Her komutun yürütülmesini kontrol eder ve durum mesajları yazdırır.

# Bir komutu çalıştırmak ve sonucunu kontrol etmek için fonksiyon
run_command() {
    echo "Komut çalıştırılıyor: $1"
    eval "$1"
    if [ $? -eq 0 ]; then
        echo "Başarılı: Komut başarıyla tamamlandı."
    else
        echo "Hata: Komut başarısız oldu. Çıkılıyor."
        exit 1
    fi
}

# Paket listelerini güncelle
run_command "sudo apt-get update"

# GStreamer paketlerinin ilk setini kur
echo "İlk set GStreamer paketleri kuruluyor..."
run_command "sudo apt-get install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav libgstrtspserver-1.0-dev"

# GStreamer paketlerinin ikinci setini kur
echo "İkinci set GStreamer paketleri kuruluyor..."
run_command "sudo apt-get install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio"

# python3-pip'i kur
echo "python3-pip kuruluyor..."
run_command "sudo apt install -y python3-pip"

# Diğer gerekli paketleri kur
echo "Diğer paketler kuruluyor..."
run_command "sudo apt-get install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstrtspserver-1.0-dev libglib2.0-dev libcairo2-dev libopencv-dev pkg-config"

# Python paketlerini kur
echo "Python paketleri kuruluyor: torch, onnx, onnxruntime..."
run_command "pip install torch onnx onnxruntime"

# yolov7 deposunu klonla
echo "yolov7 deposu klonlanıyor..."
run_command "git clone https://github.com/WongKinYiu/yolov7.git"

# yolov7 dizinine geç
echo "yolov7 dizinine geçiliyor..."
run_command "cd yolov7"

echo "Libraries..."
run_command "pip install -r requirements.txt"

# Modeli ONNX formatına dönüştür
echo "Model ONNX formatına dönüştürülüyor..."
run_command "python export.py --weights yolov7.pt --img-size 640 --batch-size 1 --dynamic --simplify"

echo "Tüm kurulumlar ve işlemler başarıyla tamamlandı!"
