# BMO Pomodoro 🤖⏱️

🇬🇧 [English README](README.md)

**ESP32** ve **0.96 inç OLED ekran** kullanılarak hazırlanan, web üzerinden kontrol edilebilen el yapımı bir Pomodoro cihazı.

Bu, sosyal medya proje koleksiyonumda paylaştığım ilk proje! Elektroniği, gömülü yazılımı, küçük bir web arayüzünü, OLED animasyonlarını ve kendi tasarladığım 3D kasayı tek bir projede birleştiriyor. 💜

> Fikir aslında çok basitti: Benimle çalışan, benimle mola veren ve odaklanma süresini tamamladığımda benimle kutlama yapan minik bir masa arkadaşı oluşturmak.

![BMO Pomodoro projesi](assets/BMO.jpeg)

---

## Özellikler ✨

- 25 dakikalık odaklanma modu
- 5 dakikalık mola modu
- 1 ile 180 dakika arasında özel süre belirleme
- Başlatma, duraklatma, devam ettirme ve sıfırlama
- ESP32 üzerinde çalışan tarayıcı tabanlı kontrol paneli
- Canlı süre ve bağlantı durumu
- OLED ekranda hareketli BMO yüzleri
- Bekleme sırasında göz kırpma animasyonu
- Odaklanma, mola, duraklatma ve kutlama ifadeleri
- Tamamlanan Pomodoro sayacı
- Kayıtlı Wi-Fi ağına bağlanamazsa otomatik erişim noktası oluşturma
- SolidWorks ile tasarlanmış ön ve arka kasa parçaları

> Tamamlanan Pomodoro sayısı yalnızca bellekte tutulur ve ESP32 yeniden başlatıldığında sıfırlanır.

---

## Proje Dosyaları 📁

```text
bmo-pomodoro-project/
│
├── assets/
│   └── BMO.jpeg
│
├── circuit/
│   └── bmo-connection.png
│
├── code/
│   └── bmo-pomodoro.ino
│
├── models/
│   ├── Front.SLDPRT
│   └── Back.SLDPRT
│
├── README.md
└── README_TR.md
```

| Dosya | Açıklama |
|---|---|
| `code/bmo-pomodoro.ino` | ESP32 kodu, OLED animasyonları, sayaç mantığı, API ve web arayüzü |
| `circuit/bmo-connection.png` | ESP32 ve OLED bağlantı şeması |
| `models/Front.SLDPRT` | Kasanın ön bölümü için SolidWorks modeli |
| `models/Back.SLDPRT` | Kasanın arka bölümü için SolidWorks modeli |
| `assets/BMO.jpeg` | Projenin tamamlanmış hâlinin fotoğrafı |

---

## Kullanılan Malzemeler 🛠️

- ESP32 geliştirme kartı
- 0.96 inç 128×64 I²C OLED ekran
- SSD1306 uyumlu OLED kontrolcüsü
- Jumper kablolar
- USB kablosu
- Arduino IDE veya PlatformIO kurulu bir bilgisayar
- 3D baskı ya da el yapımı kasa
- İsteğe bağlı boya ve dekorasyon malzemeleri

Bu projedeki kasa, ön panel ve arka kapak olmak üzere iki ayrı SolidWorks parçası şeklinde tasarlandı.

---

## Devre Bağlantısı 🔌

![ESP32 OLED bağlantı şeması](circuit/bmo-connection.png)

OLED ekran, I²C haberleşme protokolünü kullanır.

| OLED Pini | Bağlantı |
|---|---|
| `VCC` | `3.3V` |
| `GND` | `GND` |
| `SDA` | ESP32 SDA pini |
| `SCL` | ESP32 SCL pini |

### Önemli Pin Uyarısı

Paylaşılan bağlantı şemasında şu pinler gösteriliyor:

```cpp
#define OLED_SDA 21
#define OLED_SCL 22
```

Mevcut Arduino kodunda ise şu pinler tanımlı:

```cpp
#define OLED_SDA 20
#define OLED_SCL 21
```

Bu değerler kullandığın ESP32 modeline ve fiziksel bağlantına uygun olmalı. Klasik ESP32 DevKit kartını paylaşılan şemadaki gibi bağlayacaksan kodu şu şekilde değiştirmelisin:

```cpp
#define OLED_SDA 21
#define OLED_SCL 22
```

OLED ekranı bir bağlantı düzenine göre bağlayıp kodu diğer pin düzeniyle yüklememelisin.

---

## Gerekli Kütüphaneler 📚

Arduino IDE içerisindeki Library Manager üzerinden şu kütüphaneleri kur:

- `Adafruit GFX Library`
- `Adafruit SSD1306`

Şu kütüphaneler ESP32 Arduino çekirdeğiyle birlikte gelir:

- `Wire`
- `WiFi`
- `WebServer`

---

## Kurulum ve Kodu Yükleme 🚀

### 1. Arduino kodunu aç

Şu dosyayı Arduino IDE veya PlatformIO ile aç:

```text
code/bmo-pomodoro.ino
```

### 2. Wi-Fi bilgilerini düzenle

Kodda şu satırları bulup kendi ağ bilgilerini yaz:

```cpp
const char* WIFI_SSID = "WIFI_ADIN";
const char* WIFI_PASSWORD = "WIFI_SIFREN";
```

ESP32 yaklaşık 15 saniye boyunca bu ağa bağlanmayı dener.

### 3. BMO erişim noktası bilgilerini kontrol et

Kayıtlı Wi-Fi ağına bağlanamazsa BMO kendi ağını oluşturur:

```cpp
const char* AP_SSID = "BMO-Control";
const char* AP_PASSWORD = "bmo12345";
```

Projeyi herkese açık şekilde kullanmadan önce varsayılan erişim noktası şifresini değiştirmen önerilir.

### 4. OLED pinlerini kontrol et

`OLED_SDA` ve `OLED_SCL` değerlerinin hem ESP32 kartına hem de yaptığın fiziksel bağlantıya uyduğundan emin ol.

### 5. Kart ve port seçimini yap

Arduino IDE içerisinden kullandığın ESP32 modeline uygun kartı ve doğru seri portu seç.

### 6. Kodu yükle

Kodu derleyip ESP32'ye yükle. Ardından Serial Monitor'ü şu hızda aç:

```text
115200 baud
```

---

## BMO'ya Bağlanma 📱

### BMO ev Wi-Fi ağına bağlandıysa

OLED ekranda ve Serial Monitor'de yerel IP adresi gösterilir.

Aynı ağa bağlı telefon, tablet veya bilgisayarın tarayıcısında bu adresi aç:

```text
http://BMO_IP_ADRESI
```

Örnek:

```text
http://192.168.1.42
```

### BMO ev Wi-Fi ağına bağlanamadıysa

1. Telefon, tablet veya bilgisayarının Wi-Fi ayarlarını aç.
2. `BMO-Control` ağına bağlan.
3. Erişim noktası şifresini gir.
4. Tarayıcıdan şu adresi aç:

```text
http://192.168.4.1
```

Kontrol paneli doğrudan ESP32 tarafından yayınlandığı için ayrı bir mobil uygulamaya ya da bulut sunucusuna ihtiyaç yoktur.

---

## Web Kontrolleri 🌐

Tarayıcı arayüzünde şu kontroller bulunur:

- `25 DK ODAK` — 25 dakikalık odaklanma süresini seçer
- `5 DK MOLA` — 5 dakikalık mola süresini seçer
- `BAŞLAT` — sayacı başlatır
- `DURAKLAT` — sayacı duraklatır
- `DEVAM ET` — duraklatılan sayacı devam ettirir
- `SIFIRLA` — mevcut sayacı sıfırlar
- Özel süre alanı — 1 ile 180 dakika arasında süre kabul eder

Web sayfası her saniye ESP32'den güncel durumu alır. Yüz ifadesi, mod, kalan süre, bağlantı bilgisi ve tamamlanan Pomodoro sayısı otomatik olarak güncellenir.

---

## HTTP API 🔗

Projede küçük bir HTTP API de bulunuyor.

| Endpoint | Görevi |
|---|---|
| `GET /api/status` | Sayaç ve bağlantı durumunu döndürür |
| `GET /api/start` | Sayacı başlatır |
| `GET /api/pause` | Sayacı duraklatır |
| `GET /api/resume` | Sayacı devam ettirir |
| `GET /api/reset` | Sayacı sıfırlar |
| `GET /api/preset?mode=focus` | 25 dakikalık odaklanma modunu seçer |
| `GET /api/preset?mode=break` | 5 dakikalık mola modunu seçer |
| `GET /api/custom?minutes=30` | Özel süre belirler |

Örnek durum cevabı:

```json
{
  "state": "running",
  "mode": "focus",
  "modeLabel": "FOCUS MODE",
  "time": "24:35",
  "completed": 0,
  "network": "Wi-Fi: 192.168.1.42"
}
```

---

## OLED Yüz İfadeleri 😊

BMO, sayacın durumuna göre farklı bir yüz ifadesi gösterir:

| Durum | OLED Davranışı |
|---|---|
| Hazır | Mutlu yüz ve göz kırpma animasyonu |
| Odaklanma | Kararlı odaklanma ifadesi |
| Mola | Rahat ve uykulu ifade |
| Duraklatıldı | Şaşkın yüz ifadesi |
| Tamamlandı | Hareketli kutlama animasyonu |

Tamamlanan odaklanma süresi Pomodoro sayacını artırır. Mola süresinin tamamlanması sayacı artırmaz.

---

## Sorun Giderme 🧰

### OLED ekranda hiçbir şey görünmüyor

- `VCC` ve `GND` bağlantılarını kontrol et.
- SDA ve SCL pin tanımlarını doğrula.
- OLED adresinin `0x3C` olduğundan emin ol.
- Adafruit kütüphanelerinin kurulu olduğunu kontrol et.
- Bir I²C scanner kodu çalıştırmayı dene.
- Ekranın 128×64 ve SSD1306 uyumlu olduğunu doğrula.

### Kod derlenmiyor

İlk satırın `#include` ile başladığından emin ol:

```cpp
#include <Wire.h>
```

ESP32 kart paketinin ve gerekli Adafruit kütüphanelerinin kurulu olduğunu da kontrol et.

### Web sayfası açılmıyor

- BMO ile kullandığın cihazın aynı Wi-Fi ağına bağlı olduğundan emin ol.
- OLED veya Serial Monitor üzerinde görünen IP adresini kontrol et.
- BMO erişim noktası modundaysa önce `BMO-Control` ağına bağlan.
- Adresi `https://` yerine `http://` ile aç.

### Tamamlanan Pomodoro sayısı sıfırlanıyor

Bu, mevcut sürümde beklenen bir davranıştır. Sayaç RAM üzerinde tutulduğu için ESP32'nin gücü kesildiğinde veya cihaz yeniden başlatıldığında sıfırlanır.

---

## Geliştirme Fikirleri 🌱

- Fiziksel başlatma ve sıfırlama butonları
- Buzzer veya hoparlör bildirimi
- SD kart üzerinden ses efektleri
- Pille veya bataryayla çalışma
- Pil seviyesi göstergesi
- Tamamlanan Pomodoro sayısını Preferences veya EEPROM ile kaydetme
- Birden fazla Pomodoro profili
- Mobil uyumlu ayar ekranı
- Kablosuz yazılım güncelleme
- Daha gelişmiş kasa ve buton mekanizmaları
- Yeni OLED yüzleri ve animasyonları

---

## Güvenlik Notu ⚠️

Bağlantıları yapmadan önce ESP32 ve OLED ekranın voltaj gereksinimlerini kontrol et. Kabloları değiştirirken güç bağlantısını kes.

Yanlış bağlantılar veya uyumsuz güç kaynakları elektronik parçalara zarar verebilir.

---

## Proje Hakkında 💜

Bu proje, öğrenmek ve denemek amacıyla hazırlanmış hayran yapımı bir maker projesidir. İlham aldığı karakterin hak sahipleriyle bağlantılı değildir ve onlar tarafından desteklenmemektedir.

Kendi versiyonunu yaparsan görmeyi çoook isterim! Bir Issue açabilir, geliştirme önerilerini paylaşabilir veya Pull Request gönderebilirsin.

⭐ Projeyi beğendiysen repoya yıldız bırakarak destek olabilirsin.

**Üretmeye, denemeye ve minik masa arkadaşları yapmaya devam!** 🤖✨
