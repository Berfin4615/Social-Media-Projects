"""ZTE P802 icin Google Takvim -> MicroSIP -> sesli toplanti hatirlaticisi.

Mevcut credentials.json ve token.json dosyalari ayni klasorde kalmali.

Kullanim:
    py takvim_asistan.py --list
    py takvim_asistan.py --preview
    py takvim_asistan.py --test-next --ses neseli --delay 10
    py takvim_asistan.py --minutes 15 --ses neseli
"""

import argparse
import asyncio
from datetime import datetime, timedelta, timezone
import html
import json
from pathlib import Path
import re
import subprocess
import tempfile
import time

import miniaudio
import numpy as np
import sounddevice as sd
from google.auth.transport.requests import Request
from google.oauth2.credentials import Credentials
from google_auth_oauthlib.flow import InstalledAppFlow
from googleapiclient.discovery import build

from ayarlar import KULLANICI_ADI
from asistan_ara import (
    TELEFON, SES_PROFILLERI, VARSAYILAN_SES,
    microsip_bul, sanal_hoparlor_bul, ses_dosyasi_olustur,
)

KLASOR = Path(__file__).resolve().parent
KIMLIK = KLASOR / "credentials.json"
TOKEN = KLASOR / "token.json"
KAYIT = KLASOR / "aranan_toplantilar.json"
SCOPES = ["https://www.googleapis.com/auth/calendar.readonly"]
# Telefon Türkiye’de kullanıldığı için saatleri Türkiye saatiyle seslendir.
TURKIYE_SAATI = timezone(timedelta(hours=3))


def google_takvime_baglan():
    if not KIMLIK.is_file():
        raise FileNotFoundError(
            "credentials.json bulunamadi. Google Cloud Desktop app OAuth dosyasini bu klasore koy."
        )
    creds = None
    if TOKEN.is_file():
        creds = Credentials.from_authorized_user_file(str(TOKEN), SCOPES)
    if not creds or not creds.valid:
        if creds and creds.expired and creds.refresh_token:
            creds.refresh(Request())
        else:
            flow = InstalledAppFlow.from_client_secrets_file(str(KIMLIK), SCOPES)
            creds = flow.run_local_server(port=0)
        TOKEN.write_text(creds.to_json(), encoding="utf-8")
    return build("calendar", "v3", credentials=creds, cache_discovery=False)


def olay_baslangici(olay):
    bilgi = olay.get("start", {}).get("dateTime")
    if not bilgi:
        return None
    dt = datetime.fromisoformat(bilgi.replace("Z", "+00:00"))
    return dt if dt.tzinfo else None


def toplantimi(olay):
    if olay.get("hangoutLink") or olay.get("conferenceData"):
        return True
    metin = (olay.get("summary") or "").casefold()
    anahtarlar = (
        "toplant", "görüş", "gorus", "meeting", "meet", "call", "sync",
        "standup", "stand-up", "1:1", "interview", "mülakat", "mulakat",
        "randevu", "appointment", "demo", "sunum", "presentation",
    )
    return any(anahtar in metin for anahtar in anahtarlar)


def uygun_olay(olay, only_meetings):
    if olay.get("status") == "cancelled":
        return False
    if olay.get("eventType", "default") != "default":
        return False
    if olay_baslangici(olay) is None:
        return False
    if any(
        kisi.get("self") and kisi.get("responseStatus") == "declined"
        for kisi in olay.get("attendees", [])
    ):
        return False
    return not only_meetings or toplantimi(olay)


def etkinlikleri_getir(servis, calendar_id, simdi, bakis_dakika, only_meetings):
    params = dict(
        calendarId=calendar_id,
        timeMin=(simdi - timedelta(minutes=1)).isoformat(),
        timeMax=(simdi + timedelta(minutes=bakis_dakika)).isoformat(),
        singleEvents=True,
        orderBy="startTime",
        showDeleted=False,
        maxResults=250,
    )
    sonuclar = []
    while True:
        yanit = servis.events().list(**params).execute()
        sonuclar.extend(
            olay for olay in yanit.get("items", [])
            if uygun_olay(olay, only_meetings)
        )
        sayfa = yanit.get("nextPageToken")
        if not sayfa:
            break
        params["pageToken"] = sayfa
    return sonuclar


def baslik_temizle(olay):
    baslik = html.unescape(olay.get("summary") or "isimsiz etkinlik")
    baslik = re.sub(r"https?://\S+", "", baslik)
    baslik = re.sub(r"<[^>]*>", "", baslik)
    baslik = re.sub(r"[\x00-\x1f\x7f]", " ", baslik)
    baslik = " ".join(baslik.split())[:120]
    return baslik or "isimsiz etkinlik"


def sayi_yazi(n):
    """Saatleri TTS'nin Turkce daha dogal okuyabilmesi icin kelimeye cevir."""
    birler = ("sıfır", "bir", "iki", "üç", "dört", "beş", "altı", "yedi", "sekiz", "dokuz")
    onlar = ("", "on", "yirmi", "otuz", "kırk", "elli")
    if n < 10:
        return birler[n]
    if n < 60:
        k, b = divmod(n, 10)
        return onlar[k] + (" " + birler[b] if b else "")
    return str(n)


def zaman_soyle(dt):
    """Orn. saat on dort otuzda, saat dokuzda, saat on dort sifir beste."""
    yerel = dt.astimezone(TURKIYE_SAATI)
    kelimeler = [sayi_yazi(yerel.hour)]
    if yerel.minute:
        if yerel.minute < 10:
            kelimeler.append("sıfır")
        kelimeler.append(sayi_yazi(yerel.minute))
    son = kelimeler[-1].split()[-1]
    son_sesli = next((c for c in reversed(son) if c in "aeıioöuü"), "a")
    ek_sesli = "a" if son_sesli in "aıou" else "e"
    ek_sessiz = "t" if son[-1] in "çfhkpsşt" else "d"
    return "saat " + " ".join(kelimeler) + ek_sessiz + ek_sesli


def mesaj_olustur(olaylar, dakika, deneme=False):
    if not olaylar:
        return "Takviminde yaklaşan bir etkinlik bulamadım."
    if len(olaylar) == 1:
        olay = olaylar[0]
        ad = baslik_temizle(olay)
        saat = zaman_soyle(olay_baslangici(olay))
        if deneme:
            return (
                f"{KULLANICI_ADI}! Minik bir prova yapıyoruz. "
                f"Takvimindeki sıradaki etkinliğin adı: {ad}. "
                f"Başlangıç zamanı: {saat}. "
                "Duyduysan tamamdır, takvim asistanın hazır!"
            )
        return (
            f"{KULLANICI_ADI}! Minik bir takvim alarmı! "
            f"Takviminde {ad} var. "
            f"Yaklaşık {dakika} dakika sonra, {saat} başlayacak. "
            "Hadi, hazırlık moduna geçiyoruz!"
        )
    isimler = "; ".join(baslik_temizle(olay) for olay in olaylar[:4])
    if deneme:
        return f"Takvim provası! Sıradaki etkinlikler: {isimler}."
    return (
        f"{KULLANICI_ADI}! Takviminde yaklaşık {dakika} dakika sonra başlayacak "
        f"{len(olaylar)} etkinlik var. "
        f"İsimleri: {isimler}. "
        "Geri sayım başladı, hadi hazırlanalım!"
    )


def arama_ve_ses(metin, microsip_yolu=None, ses_profili=VARSAYILAN_SES, arama_zamani=None):
    exe = microsip_bul(microsip_yolu)
    cihaz, adi = sanal_hoparlor_bul()
    print(f"Ses aygiti: {adi} | Profil: {ses_profili}")
    print("Ses hazirlaniyor...")
    print("Okunacak metin: " + metin)

    with tempfile.TemporaryDirectory() as tmp:
        mp3 = Path(tmp) / "bildirim.mp3"
        asyncio.run(ses_dosyasi_olustur(metin, mp3, ses_profili))
        ses = miniaudio.decode_file(
            str(mp3),
            output_format=miniaudio.SampleFormat.FLOAT32,
            nchannels=1,
            sample_rate=48000,
        )
        pcm = np.frombuffer(ses.samples, dtype=np.float32)
        if arama_zamani is not None:
            kalan = arama_zamani - time.monotonic()
            if kalan > 0:
                print(f"Aramaya {kalan:.1f} saniye kaldi. Video kaydini baslatabilirsin.")
                time.sleep(kalan)
        print(f"Telefon araniyor: {TELEFON}")
        subprocess.Popen([str(exe), TELEFON])
        # MicroSIP cevap bilgisini iletmedigi icin sabit bekleme kullanilir.
        print("Telefonu acabilirsin. Ses 8 saniye sonra baslayacak...")
        time.sleep(8)
        sd.play(pcm, samplerate=48000, device=cihaz, blocking=True)
        print("Hatirlatma okundu. Telefon gorusmesini kapatabilirsin.")


def kayitlari_oku():
    if not KAYIT.exists():
        return {}
    try:
        data = json.loads(KAYIT.read_text(encoding="utf-8"))
        return data if isinstance(data, dict) else {}
    except (OSError, json.JSONDecodeError):
        return {}


def kayitlari_yaz(kayit):
    secilen = dict(sorted(kayit.items(), key=lambda kv: kv[1])[-500:])
    gecici = KAYIT.with_suffix(".tmp")
    gecici.write_text(json.dumps(secilen, ensure_ascii=False, indent=2), encoding="utf-8")
    gecici.replace(KAYIT)


def olay_anahtari(olay):
    return (olay.get("id") or "?") + "|" + olay_baslangici(olay).isoformat()


def gelecek_olaylari_bul(servis, args):
    simdi = datetime.now().astimezone()
    olaylar = etkinlikleri_getir(
        servis, args.calendar_id, simdi, 2 * 24 * 60, args.only_meetings
    )
    return [olay for olay in olaylar if olay_baslangici(olay) > simdi]


def listele(servis, args):
    gelecek = gelecek_olaylari_bul(servis, args)
    print(f"\nOnumuzdeki 48 saatte {len(gelecek)} uygun etkinlik:")
    for olay in gelecek:
        print(f"  {olay_baslangici(olay).astimezone(TURKIYE_SAATI):%d.%m.%Y %H:%M}  {baslik_temizle(olay)}")
    if not gelecek:
        print("  Uygun etkinlik yok. Google Takvim'de saatli test toplantisi ekle.")


def sonraki_etkinligi_dene(servis, args, sesli):
    gelecek = gelecek_olaylari_bul(servis, args)
    if not gelecek:
        print("Onumuzdeki 48 saatte okunacak etkinlik bulunamadi.")
        return
    olay = gelecek[0]
    metin = mesaj_olustur([olay], args.minutes, deneme=True)
    print("\nSiradaki etkinlik: " + baslik_temizle(olay))
    print("Okunacak metin: " + metin)
    if sesli:
        arama_ve_ses(metin, args.microsip, args.ses, args.arama_zamani)
    else:
        print("Telefonda dinlemek icin: py takvim_asistan.py --test-next --ses neseli")


def izle(servis, args):
    print(f"Takvim izleniyor. Hatirlatma: {args.minutes} dk once; kontrol: {args.poll} sn.")
    print(f"Ses profili: {args.ses}")
    print("Bilgisayar acik ve bu pencere calisiyor olmali. Cikmak icin Ctrl+C.")
    while True:
        try:
            simdi = datetime.now().astimezone()
            olaylar = etkinlikleri_getir(
                servis, args.calendar_id, simdi, args.minutes + 2, args.only_meetings
            )
            kayitlar = kayitlari_oku()
            bekleyen = []
            for olay in olaylar:
                baslangic = olay_baslangici(olay)
                hatirlatma = baslangic - timedelta(minutes=args.minutes)
                if (
                    hatirlatma <= simdi < baslangic
                    and simdi <= hatirlatma + timedelta(minutes=args.grace)
                    and olay_anahtari(olay) not in kayitlar
                ):
                    bekleyen.append(olay)
            if bekleyen:
                print("\nHatirlatma zamani: " + ", ".join(baslik_temizle(o) for o in bekleyen))
                arama_ve_ses(mesaj_olustur(bekleyen, args.minutes), args.microsip, args.ses)
                for olay in bekleyen:
                    kayitlar[olay_anahtari(olay)] = datetime.now(timezone.utc).isoformat()
                kayitlari_yaz(kayitlar)
            time.sleep(args.poll)
        except KeyboardInterrupt:
            print("\nTakvim izleme durduruldu.")
            return
        except Exception as hata:
            print(f"Gecici hata: {type(hata).__name__}: {hata}")
            time.sleep(max(15, args.poll))


def main():
    parser = argparse.ArgumentParser(description="ZTE P802 - neseli Google Takvim asistani")
    parser.add_argument("--test", action="store_true", help="Ornek toplantı adı ile arama testi")
    parser.add_argument("--list", action="store_true", help="Gelecek 48 saatteki etkinlikleri listele")
    parser.add_argument("--preview", action="store_true", help="Siradaki etkinligin okunacak metnini goster, arama yapma")
    parser.add_argument("--test-next", action="store_true", help="Siradaki GERCEK etkinligi telefondan hemen oku")
    parser.add_argument("--delay", type=int, default=0, help="Test aramasini baslattiktan kac saniye sonra yap? Ornek: 10")
    parser.add_argument("--minutes", type=int, default=15, help="Etkinlikten kac dakika once ara (varsayilan 15)")
    parser.add_argument("--poll", type=int, default=30, help="Takvimi kac saniyede bir kontrol et")
    parser.add_argument("--grace", type=int, default=4, help="Hatirlatma sonrasi tolerans, dakika")
    parser.add_argument("--only-meetings", action="store_true", help="Toplanti kelimeli veya video baglantili etkinlikler")
    parser.add_argument("--calendar-id", default="primary", help="Takvim kimligi (varsayilan primary)")
    parser.add_argument("--ses", choices=SES_PROFILLERI.keys(), default=VARSAYILAN_SES, help="Ses profili")
    parser.add_argument("--microsip", help="microsip.exe tam dosya yolu")
    args = parser.parse_args()
    if args.delay < 0:
        parser.error("--delay negatif olamaz")
    if args.delay and not (args.test or args.test_next):
        parser.error("--delay sadece --test veya --test-next ile kullanilabilir")
    args.arama_zamani = time.monotonic() + args.delay if args.delay else None
    if args.minutes < 1 or args.minutes > 1440:
        parser.error("--minutes 1 ile 1440 arasinda olmali")
    if args.poll < 10 or args.grace < 1:
        parser.error("--poll en az 10 saniye, --grace en az 1 dakika olmali")

    if args.test:
        baslangic = datetime.now().astimezone() + timedelta(minutes=args.minutes)
        ornek = {"summary": "Demo Proje Toplantısı", "start": {"dateTime": baslangic.isoformat()}}
        arama_ve_ses(mesaj_olustur([ornek], args.minutes), args.microsip, args.ses, args.arama_zamani)
        return

    servis = google_takvime_baglan()
    if args.list:
        listele(servis, args)
    elif args.preview:
        sonraki_etkinligi_dene(servis, args, sesli=False)
    elif args.test_next:
        sonraki_etkinligi_dene(servis, args, sesli=True)
    else:
        izle(servis, args)


if __name__ == "__main__":
    try:
        main()
    except (FileNotFoundError, ValueError, RuntimeError) as hata:
        print(f"HATA: {hata}")
    except KeyboardInterrupt:
        print("\nIptal edildi.")
