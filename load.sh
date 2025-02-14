#!/bin/bash

set -e  # Hata durumunda scripti durdur
set -o pipefail  # Pipe hatalarını algıla
set -u  # Tanımsız değişken kullanılırsa hata ver

# Renk kodları
GREEN="\e[32m"
RED="\e[31m"
RESET="\e[0m"

echo -e "${GREEN}Python sürüm kontrol ediliyor...${RESET}"
python3 --version || { echo -e "${RED}Python3 bulunamadı! Lütfen yükleyin.${RESET}"; exit 1; }

echo -e "${GREEN}Ninja yükleniyor...${RESET}"
pip3 install ninja || { echo -e "${RED}Ninja yüklenemedi!${RESET}"; exit 1; }

echo -e "${GREEN}GLib deposu klonlanıyor...${RESET}"
git clone https://github.com/GNOME/glib.git || { echo -e "${RED}GLib klonlanamadı!${RESET}"; exit 1; }
cd glib || { echo -e "${RED}glib dizinine girilemedi!${RESET}"; exit 1; }

echo -e "${GREEN}GLib 2.76.6 sürümüne geçiliyor...${RESET}"
git checkout 2.76.6 || { echo -e "${RED}Checkout başarısız!${RESET}"; exit 1; }

echo -e "${GREEN}Meson yapılandırması başlatılıyor...${RESET}"
meson build --prefix=/usr || { echo -e "${RED}Meson yapılandırması başarısız!${RESET}"; exit 1; }

echo -e "${GREEN}Ninja ile derleme başlatılıyor...${RESET}"
ninja -C build/ || { echo -e "${RED}Derleme başarısız!${RESET}"; exit 1; }

echo -e "${GREEN}GLib yükleniyor...${RESET}"
cd build/ || { echo -e "${RED}Build dizinine girilemedi!${RESET}"; exit 1; }
ninja install || { echo -e "${RED}GLib yüklenemedi!${RESET}"; exit 1; }

echo -e "${GREEN}GLib sürümü kontrol ediliyor...${RESET}"
pkg-config --modversion glib-2.0 || { echo -e "${RED}GLib sürümü alınamadı!${RESET}"; exit 1; }

echo -e "${GREEN}GLib kurulumu başarıyla tamamlandı!${RESET}"

# ========================
# DeepStream Kurulumu
# ========================

echo -e "${GREEN}DeepStream kurulumu başlatılıyor...${RESET}"

# NVIDIA kaynaklarını ekle
sudo apt update && sudo apt install -y apt-transport-https ca-certificates curl gnupg

echo -e "${GREEN}NVIDIA paket anahtarı ekleniyor...${RESET}"
curl -fsSL https://nvidia.github.io/nvidia-container-runtime/gpgkey | sudo gpg --dearmor -o /usr/share/keyrings/nvidia-container-runtime-keyring.gpg

echo -e "${GREEN}NVIDIA DeepStream deposu ekleniyor...${RESET}"
echo "deb [signed-by=/usr/share/keyrings/nvidia-container-runtime-keyring.gpg] https://nvidia.github.io/deepstream/deb/$(lsb_release -cs)/ ./ " | sudo tee /etc/apt/sources.list.d/nvidia-container-runtime.list

sudo apt update

echo -e "${GREEN}DeepStream SDK yükleniyor...${RESET}"
sudo apt install -y deepstream-6.3 || { echo -e "${RED}DeepStream yüklenemedi!${RESET}"; exit 1; }

echo -e "${GREEN}DeepStream kurulumu tamamlandı!${RESET}"

# ========================
# DeepStream Doğrulama
# ========================

echo -e "${GREEN}DeepStream sürüm kontrol ediliyor...${RESET}"
dpkg -l | grep deepstream || { echo -e "${RED}DeepStream sürümü doğrulanamadı!${RESET}"; exit 1; }

echo -e "${GREEN}DeepStream test başlatılıyor...${RESET}"
deepstream-app --version || { echo -e "${RED}DeepStream test başarısız!${RESET}"; exit 1; }

echo -e "${GREEN}DeepStream başarıyla kuruldu!${RESET}"
