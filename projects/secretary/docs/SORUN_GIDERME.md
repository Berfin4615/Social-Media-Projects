# Sorun giderme

| Sorun | Kontrol |
|---|---|
| Telefon web paneli açılmıyor | PC ve telefon aynı IP alt ağında mı? `ping <TELEFON_IP>` ile test et. |
| XML indirmesinde `GET` yok | HTTP sunucusu doğru portta mı? `Idle Screen XML Download` etkin mi? `Download Screen XML At Boot-up = Yes` kaydedildi mi? Windows güvenlik duvarı telefonun IP'sine izin veriyor mu? |
| XML için `GET ... 404` | `screen.xml` yanlış klasörde veya URL yanlış. |
| `GET ... 200` ama görüntü yok | Dosya geliyor; XML şeması/bitmap biçimi firmware ile uyumsuz olabilir. |
| MicroSIP aramıyor | `Local Account` seçili mi? Telefon doğrudan IP aramalarına izin veriyor mu? SIP için yerel ağ/güvenlik duvarı ayarlarını kontrol et. |
| Arama var ama ses yok | MicroSIP mikrofonu `CABLE Output`, Python çıkış aygıtı `CABLE Input` olmalı. |
| `credentials.json bulunamadi` | Google Cloud'dan **Desktop app** OAuth JSON indir, bu adla proje köküne koy. |
| Google `403 access_denied` | Google Auth Platform → Audience → Test users içinde kendi hesabını ekle. |
| Toplantı listede yok | Tüm gün etkinlikleri, iptal edilenler, reddedilenler ve bazı diğer etkinlik türleri filtrelenir; `--only-meetings` filtresini kapatıp dene. |
| Telefon beklenenden geç çalıyor | Ses sentezi ve Google OAuth isteği gecikebilir; `--delay` kesin gerçek zamanlı zamanlayıcı değildir. |
| Bir toplantı tekrar aranmıyor | Aynı etkinlik `aranan_toplantilar.json` içinde işaretlenmiş olabilir. Silmeden önce yedekle; silmek tekrar aramaya neden olabilir. |

## Güvenlik

Kimlik dosyalarını, OAuth token'larını, SIP şifrelerini veya gerçek kullanıcının takvim ekran görüntülerini public repoya ekleme. XML sunucusunu sadece yerel ağda çalıştır.
