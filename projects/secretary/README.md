# 📞 ZTE AI Sekreter — Eski IP telefondan toplantı hatırlatıcısı

![Sakız patlatan karakter GIF](assets/sakiz_patlatan_asistan.gif)

ZTE **ZXV10 P802** IP telefonunu Windows + Python ile özelleştirmek için hazırlanmış örnek proje. Telefonun LCD ekranına bir karakter yükleme denemesi, **MicroSIP ile bilgisayardan arama**, **VB-CABLE ile Türkçe sesli anons** ve **Google Takvim'den toplantı adını okuma** adımlarını içerir.

> **Gerçekte ne yapıyor?** Takvimdeki saatli bir etkinlik yaklaştığında masa telefonunu arayıp başlığını ve saatini sesli okur. Bu sürüm **canlı, çift yönlü yapay zekâ sohbeti** yapmaz. Telefonun açıldığını SIP üzerinden algılamaz: arama başlatıldıktan **yaklaşık 8 saniye sonra** ses oynatır. Yukarıdaki hareketli GIF, görsel tanıtım içindir; LCD'ye doğrudan GIF yüklenmez.

## İçindekiler

| Dosya | Görevi |
|---|---|
| `asistan_ara.py` | Elle, belirli saatte veya gecikmeli sesli arama |
| `takvim_asistan.py` | Google Takvim'i okur; toplantı hatırlatması ve video denemesi |
| `ayarlar_ornek.py` | Kişiye özel örnek telefon IP'si ve hitap |
| `requirements.txt` | Python bağımlılıkları |
| `screen.xml` | P802 özel bekleme ekranı için örnek XML (cihaza göre değişebilir) |
| `assets/karakter_onizleme.png` | Statik karakter önizlemesi |
| `assets/sakiz_patlatan_asistan.gif` | Sosyal medya/README için animasyon |
| `docs/KURULUM.md` | Baştan sona ayrıntılı kurulum |
| `docs/SORUN_GIDERME.md` | Karşılaşılabilen hatalar |
| `.gitignore` | Kişisel dosyaları Git'ten dışlar |

## Hızlı başlangıç — Windows

Önce `docs/KURULUM.md` içindeki ağ, MicroSIP ve VB-CABLE adımlarını tamamla.

```powershell
copy ayarlar_ornek.py ayarlar.py
notepad ayarlar.py
py -m pip install -r requirements.txt
```

`ayarlar.py` içinde `TELEFON_IP` ve `KULLANICI_ADI` değerlerini kendine göre değiştir.

**10 saniye sonra sesli deneme araması:**

```powershell
py asistan_ara.py --delay 10 --ses neseli
```

**Google Takvim yetkilendirmesinden sonra sıradaki toplantıyı listele:**

```powershell
py takvim_asistan.py --list
```

**Sıradaki gerçek etkinliğin adını telefonda söyle — video için 10 saniye gecikmeli:**

```powershell
py takvim_asistan.py --test-next --ses neseli --delay 10
```

**Toplantıdan 15 dakika önce otomatik ara:**

```powershell
py takvim_asistan.py --minutes 15 --ses neseli
```

Kullanılabilen ses profilleri: `neseli`, `tatli`, `dogal`, `erkek`.

**Önemli:** `--delay 10` yalnızca test aramasının gecikmesidir; gerçek hatırlatma aralığı varsayılan olarak **15 dakikadır**. Ses üretimi veya Google yetkilendirmesi uzarsa arama 10 saniyeden geç başlayabilir. Program açık kalmalı; Windows uyku moduna geçmemelidir.

## Gereken yazılımlar

- Windows bilgisayar ve yerel ağa bağlı ZTE P802 (başka SIP telefonlarında ekran uyumu farklı olabilir)
- Python 3 (Windows'ta `py` komutu)
- [MicroSIP](https://www.microsip.org/downloads) — yerel SIP araması
- [VB-CABLE](https://vb-audio.com/Cable/) — Python sesini MicroSIP mikrofonuna yönlendirme
- Google Takvim için [Google Calendar API](https://developers.google.com/workspace/calendar/api/quickstart/python) ve Desktop OAuth `credentials.json`
- Edge TTS için internet erişimi

## Güvenlik ve paylaşım

**GitHub'a asla yükleme:** `credentials.json`, `token.json`, gerçek `ayarlar.py`, kişisel takvim kayıtları, şifreler veya cihazın arka etiketinde görünen kimlik bilgileri. `.gitignore` eklenmiştir ama geçmişte commit ettiğin bir sırrı geçmişten silmez. Yalnızca güvendiğin yerel ağda çalıştır; XML için kullanılan HTTP sunucusunu internete açma.

Bu repo örnek/tanıtım projesidir. Kurulum kullanılan telefonun firmware sürümüne ve Windows ağ/ses ayarlarına göre değişebilir. Lisans eklemek istersen GitHub'da kendi tercihine göre ayrıca seçebilirsin.
