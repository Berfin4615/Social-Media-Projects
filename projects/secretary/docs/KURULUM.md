# Kurulum — Adım adım (Windows + ZTE P802)

## 1. Fiziksel ve ağ bağlantısı

- P802'ye güç ver (etiket üzerindeki uygun adaptör ile) ve Ethernet üzerinden bilgisayara/yerel ağa bağla.
- Modeme uzun kablo çekmek istemiyorsan Windows'un Wi-Fi internet paylaşımını Ethernet adaptörüne açabilirsin: `Win + R` → `ncpa.cpl` → Wi-Fi Özellikler → Paylaşım → Ethernet.
- Windows Ethernet IP örneği: `192.168.137.1`; telefon IP örneği: `192.168.137.50`; subnet mask: `255.255.255.0`. **Bunlar örnektir; herkes kendi gerçek IP'sini kullanmalı.**
- Telefonun IP'sini Status/Network ekranından öğren. Tarayıcıda `http://<TELEFON_IP>` ile web paneline gir.

## 2. Telefon ekranı için XML (isteğe bağlı)

`screen.xml`, bu proje için hazırlanan *örnek* bir özel bekleme ekranı dosyasıdır. Firmware `1.0.1.107` üzerinde kullanılan XML sözdizimi herkeste aynı sonucu vermeyebilir. Hareketli GIF bu ekranda doğrudan oynatılamaz.

Telefonun web panelinde **Advanced Settings → Idle Screen XML Download**:

| Alan | Önerilen deneme değeri |
|---|---|
| Enable Idle Screen XML Download | `YES, HTTP` |
| Download Screen XML At Boot-up | `Yes` |
| Use custom filename | `Yes` |
| Idle Screen XML Server Path | `192.168.137.1/screen.xml` *(PC IP'ni kullan)* |

XML dosyasının bulunduğu klasörde PowerShell ile HTTP sunucusu başlat:

```powershell
py -m http.server 80 --bind 192.168.137.1
```

Bilgisayarda `http://192.168.137.1/screen.xml` adresinin açıldığını kontrol et. Gerekirse Windows güvenlik duvarında yalnızca kendi telefon IP'sinden TCP/80 erişimine izin ver. Ayarları kaydet; telefonu yeniden başlat. Alternatif olarak `Menu → Preference → Download SCR XML` kullanılabilir. `GET ... 200` dosyanın iletildiğini gösterir, LCD'de çizimin işlendiğini tek başına kanıtlamaz.

Port 80 kullanımda veya engelliyse `8000` portuyla deneme yapılabilir; XML sunucu yolu ve güvenlik duvarı kuralını da buna göre değiştir.

## 3. MicroSIP ile ilk arama

1. [MicroSIP](https://www.microsip.org/downloads) yükle.
2. **Settings → Enable Local Account**, ardından **Local Account** seç.
3. P802 web panelinden `Disable Direct IP Calls = No` olduğundan emin ol.
4. MicroSIP'de `sip:<TELEFON_IP>` (örnek: `sip:192.168.137.50`) adresini ara.
5. Telefon çalıyorsa bu aşama tamamdır. Bu arama **yerel SIP** aramasıdır; GSM hattı gerekmez.

## 4. Ses yolunu kur

1. [VB-CABLE](https://vb-audio.com/Cable/) kur (kurulum sonrası Windows'u yeniden başlatman gerekebilir).
2. MicroSIP'te mikrofon / Input Device = `CABLE Output` seç.
3. Python, `CABLE Input` aygıtına ses verir; MicroSIP bunu mikrofon sinyali olarak telefona aktarır.
4. Ses alamıyorsan Windows Sound aygıt adlarını ve MicroSIP mikrofon seçimini yeniden kontrol et.

## 5. Python kurulumu

```powershell
copy ayarlar_ornek.py ayarlar.py
notepad ayarlar.py
py -m pip install -r requirements.txt
py asistan_ara.py --delay 10 --ses neseli
```

`ayarlar.py` dosyasında `TELEFON_IP` telefonunun **gerçek IP'si** olacak. Ses internetten sentezlendiği için Edge TTS sırasında çevrimiçi olmalısın. **Aramayı açtıktan sonra Enter'a basılmaz; mesaj, çağrı başlatıldıktan yaklaşık 8 saniye sonra oynatılır.**

## 6. Google Takvim'i bağla

1. [Google Cloud Console](https://console.cloud.google.com/) üzerinden proje oluştur.
2. **Google Calendar API** etkinleştir.
3. Google Auth Platform → **Branding / Audience** kısmını yapılandır. Uygulama `Testing` modundaysa hesabını **Test users** listesine ekle (403 `access_denied` için).
4. OAuth client türü: **Desktop app**. İndirilen JSON dosyasını proje klasöründe `credentials.json` olarak kaydet.
5. `py takvim_asistan.py --list` komutunu çalıştır. Google yetkilendirmesini tamamla. Aynı klasörde otomatik `token.json` oluşur.
6. `token.json` ve `credentials.json` dosyalarını **GitHub'a gönderme**. `Testing` modunda Google'ın yetkilendirmesi belirli süre sonra yeniden istenebilir.

## 7. Test ve canlı çalışma

```powershell
# Herhangi bir gerçek takvim etkinliği kullanmadan örnek arama
py takvim_asistan.py --test --ses neseli --delay 10

# Gelecek etkinlikleri göster
py takvim_asistan.py --list

# Bir sonraki gerçek etkinliğin adını önizle, arama yapmaz
py takvim_asistan.py --preview

# Video için, sıradaki gerçek etkinlikle 10 saniye sonra ara
py takvim_asistan.py --test-next --ses neseli --delay 10

# Toplantıdan 15 dakika önce otomatik ara
py takvim_asistan.py --minutes 15 --ses neseli
```

Canlı komut çalışırken terminali açık bırak. Testi bitirmek için `Ctrl+C` kullan. İki ayrı takvim takipçisi aynı anda çalışmamalı; birden çok arama tetiklenebilir.

## 8. Bu sürümün sınırları

- Telefona **tek yönlü sesli anons** gider; telefonun mikrofonundan serbest yapay zekâ sohbeti yoktur.
- Telefonun açıldığı SIP durumunu doğrudan takip etmez. Ses yaklaşık 8 saniye sonra oynatılır; kullanıcı açmazsa da mesaj kayıttan çalınır.
- LCD'deki `screen.xml` **statik resim denemesi**; eklenen `GIF` yalnızca GitHub/sosyal medya tanıtımı içindir.
- Telefon IP'si değişirse `ayarlar.py` güncellenmelidir.
