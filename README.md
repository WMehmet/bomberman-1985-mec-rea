# Bomberman 1985 MEC REA Edition

[![Language](https://img.shields.io/badge/Language-C99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-brightgreen.svg)]()
[![Graphics](https://img.shields.io/badge/Graphics-SDL2%20%2F%20CHR--ROM-orange.svg)]()
[![Audio](https://img.shields.io/badge/Audio-Procedural%20NES%20APU-purple.svg)]()
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

> **"Emine Haksever & Mehmet Ertuğrul Çakmak tarafından modernize edilmiştir."**

Orijinal 1985 Nintendo Famicom / NES Bomberman oyununun 6502 assembly kaynak kodundan tersine mühendislikle saf **C99** ve **SDL2** kullanılarak sıfırdan geliştirilmiş, kare-hassas (1:1 frame-perfect), modernleştirilmiş ve geniş ekran destekli yerel masaüstü sürümüdür.

---

## 🌟 Öne Çıkan Özellikler (Key Features)

* **Orijinal NES Fiziği & Mekanikleri (1:1 Frame-Perfect):**
  * Sabit 60.0988 Hz NTSC kare zamanlaması ve sıfır gecikmeli hibrit döngü (zero input lag).
  * Orijinal 6502 `ADJUST_BOMBMAN_VPOS` ve `ADJUST_BOMBMAN_HPOS` algoritmalarıyla pürüzsüz köşe kaydırma (Grid Corner Slide).
  * 160 kare (~2.66s) bomba fünye zamanlayıcısı ve 7 aşamalı tuğla parçalanma animasyonu.
  * Orijinal 8 aşamalı Bomberman erime ölüm animasyonu.
* **Tüm 50 Bölüm (All 50 NES Stages):**
  * 1'den 50'ye kadar tüm bölümlerde orijinal NES düşman kadrosu (*Ballom, O'Neal, Dahl, Minvo, Doria, Ovape, Pass, Pontan*).
  * Çıkış kapısı ve gizli bonus yükseltmeleri (uzaktan kumanda, hız, bomba artışı, alev yeleği, duvar geçme vb.).
* **Modernleştirilmiş Türkçe Arayüz & Menü:**
  * Retro jenerik, ana menü, yüksek skor tablosu ve oyun içi ayarlar.
  * Özel NES piksel Türkçe yazı tipi motoru (`Ç, Ğ, İ, Ö, Ş, Ü` desteği).
  * Tarih kayıtlı kalıcı yüksek skor takip sistemi (`bomberman_scores.dat`).
* **Ultra Hafif & Bağımsız (Zero Bloat Standalone):**
  * Ağır oyun motorları veya frameworkler içermez (Linux AppImage ~960 KB, Windows Exe ~2.6 MB).
  * **Algoritmik Ses Sentezleme:** Harici MP3/WAV dosyaları yerine kod içinden gerçek zamanlı üretilen NES 2A03 APU kare dalga, üçgen dalga ve gürültü sentezleyicisi.
  * **Gömülü Grafikler:** 8 KB orijinal CHR-ROM bitmap verisi.
* **Geniş Ekran & Tam Ekran Desteği:**
  * 16:9 formatında görüntüyü deforme etmeden haritayı genişleten dinamik kamera takip sistemi.

---

## 🎮 Kontroller (Controls)

| Tuş | İşlev |
| :--- | :--- |
| **Ok Tuşları / WASD** | Bomberman Hareketi & Menü Gezinme |
| **ENTER / BOŞLUK / Z** | Menü Seçimi & Bomba Bırakma |
| **X** | Uzaktan Patlatma (Detonator Bonusu varken) |
| **R** | Bölümü Anında Yeniden Başlatma |
| **ESC** | Türkçe Oyun Ayarları / Ana Menü |
| **F11** | Tam Ekran / Pencere Modu Geçişi |

---

## 🛠️ Derleme ve Çalıştırma (Building)

### Gereksinimler
- **Linux:** `gcc`, `make`, `SDL2` geliştirme kütüphanesi (`libsdl2-dev` veya `sdl2`).
- **Windows:** `MinGW-w64` / `w64devkit` ve `SDL2`.

### Linux'ta Derleme:
```bash
git clone https://github.com/KULLANICI_ADINIZ/bomberman-1985-mec-rea.git
cd bomberman-1985-mec-rea
make
./bomberman
```

### Windows'ta Derleme:
```cmd
build.bat
```
veya MinGW terminalinde:
```bash
make -f Makefile.win
```

---

## 📁 Proje Mimarisi

```text
├── src/
│   ├── main.c        # Ana döngü, girdi işleme ve ekran ölçekleme
│   ├── game.c        # 50 bölüm, bomba, düşman yapay zekası ve fizik motoru
│   ├── graphics.c    # CHR-ROM metatile render, HUD, jenerik ve menü çizimi
│   ├── audio.c       # NES APU yazılımsal ses ve müzik sentezleyicisi
│   ├── settings.c    # Oyun içi ayarlar, tuş atama ve tam ekran kontrolü
│   ├── scores.c      # Tarih kayıtlı kalıcı en yüksek skor sistemi
│   ├── font.c        # Türkçe karakter destekli NES bitmap font motoru
│   ├── chr_rom.h     # 8 KB ham NES grafik verisi
│   └── types.h       # Veri tipleri ve sabitler
├── bomberman.rc      # Windows simge, manifest ve sürüm meta verileri
├── bomberman.manifest# Windows 10/11 uyumluluk manifesti
├── icon.ico          # Çoklu çözünürlüklü yeşil Bomberman simgesi (16x16 - 256x256)
├── Makefile          # Linux derleme betiği
└── Makefile.win      # Windows derleme betiği
```

---

## 👥 Katkıda Bulunanlar (Credits)

* **Orijinal Oyun:** Hudson Soft / Nintendo (1985)
* **Modernizasyon & Porting:** Emine Haksever & Mehmet Ertuğrul Çakmak

## 📄 Lisans
Bu proje [MIT Lisansı](LICENSE) altında sunulmaktadır.
