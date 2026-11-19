"""ZTE P802'yi MicroSIP ile arar ve VB-CABLE uzerinden Turkce ses verir.

Kurulum: py -m pip install -r requirements.txt
Deneme:  py asistan_ara.py --delay 10 --ses neseli
Profiller: neseli, tatli, dogal, erkek
"""

import argparse
import asyncio
from datetime import datetime, timedelta
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time

import edge_tts
import miniaudio
import numpy as np
import sounddevice as sd

from ayarlar import TELEFON_IP, KULLANICI_ADI
TELEFON = f"sip:{TELEFON_IP}"

# Turkce icin Edge TTS'de kullanilabilen iki ses, farkli hiz ve ton ayarlariyla.
# Profil, sesin akustik ozelliklerini degistirir; gercek duygu/oyunculuk garantisi vermez.
SES_PROFILLERI = {
    "neseli": {"voice": "tr-TR-EmelNeural", "rate": "+13%", "pitch": "+9Hz"},
    "tatli": {"voice": "tr-TR-EmelNeural", "rate": "+6%", "pitch": "+16Hz"},
    "dogal": {"voice": "tr-TR-EmelNeural", "rate": "+0%", "pitch": "+0Hz"},
    "erkek": {"voice": "tr-TR-AhmetNeural", "rate": "+7%", "pitch": "+4Hz"},
}
VARSAYILAN_SES = "neseli"
VARSAYILAN_MESAJ = (
    f"{KULLANICI_ADI}! Takvim alarmı değil, ben geldim! "
    "Küçük bir hatırlatma: bugün de harika şeyler yapacağız. "
    "Asistanın artık çok daha neşeli konuşuyor!"
)


def microsip_bul(elle_verilen):
    if elle_verilen:
        adaylar = [Path(elle_verilen)]
    else:
        adaylar = []
        komut = shutil.which("microsip.exe")
        if komut:
            adaylar.append(Path(komut))
        for kok in (os.getenv("ProgramFiles"), os.getenv("ProgramFiles(x86)"), os.getenv("LOCALAPPDATA")):
            if kok:
                adaylar.extend([
                    Path(kok) / "MicroSIP" / "microsip.exe",
                    Path(kok) / "Programs" / "MicroSIP" / "microsip.exe",
                ])
        adaylar.append(Path.cwd() / "microsip.exe")
    for aday in adaylar:
        if aday.is_file():
            return aday
    raise FileNotFoundError(
        "MicroSIP bulunamadi. --microsip ile microsip.exe dosyasinin tam yolunu belirt."
    )


def sanal_hoparlor_bul():
    for i, aygit in enumerate(sd.query_devices()):
        if "cable input" in aygit["name"].lower() and aygit["max_output_channels"] >= 1:
            return i, aygit["name"]
    raise RuntimeError(
        "VB-CABLE 'CABLE Input' bulunamadi. VB-CABLE kurulu mu? "
        "MicroSIP mikrofonu 'CABLE Output' olarak ayarli mi?"
    )


async def ses_dosyasi_olustur(metin, dosya, profil=VARSAYILAN_SES):
    """Mevcut iki parametreli cagrilari bozmadan ses profili ekler."""
    if profil not in SES_PROFILLERI:
        raise ValueError(f"Bilinmeyen ses profili: {profil}")
    ayar = SES_PROFILLERI[profil]
    await edge_tts.Communicate(
        metin, ayar["voice"], rate=ayar["rate"], pitch=ayar["pitch"]
    ).save(str(dosya))


def bekle(gecikme, saat):
    if saat:
        try:
            hedef_saat = datetime.strptime(saat, "%H:%M").time()
        except ValueError as hata:
            raise ValueError("--at HH:MM biciminde olmali (ornek: 21:30)") from hata
        simdi = datetime.now()
        hedef = datetime.combine(simdi.date(), hedef_saat)
        if hedef <= simdi:
            hedef += timedelta(days=1)
        saniye = (hedef - simdi).total_seconds()
        print(f"Arama zamani: {hedef:%d.%m.%Y %H:%M}")
    else:
        saniye = gecikme
    if saniye > 0:
        print(f"Arama icin {round(saniye)} saniye bekleniyor. Pencereyi kapatma.")
        time.sleep(saniye)


def main():
    parser = argparse.ArgumentParser(description="ZTE AI Asistan - Turkce sesli arama")
    zaman = parser.add_mutually_exclusive_group()
    zaman.add_argument("--delay", type=int, default=0, help="Kac saniye sonra aransin?")
    zaman.add_argument("--at", type=str, help="Hangi saatte aransin? HH:MM")
    parser.add_argument("--text", default=VARSAYILAN_MESAJ, help="Telefonda okunacak metin")
    parser.add_argument("--ses", choices=SES_PROFILLERI.keys(), default=VARSAYILAN_SES)
    parser.add_argument("--microsip", help="microsip.exe tam dosya yolu")
    args = parser.parse_args()
    if args.delay < 0:
        parser.error("--delay negatif olamaz")

    microsip = microsip_bul(args.microsip)
    ses_cihazi, cihaz_adi = sanal_hoparlor_bul()
    print(f"MicroSIP: {microsip}")
    print(f"Ses yolu: {cihaz_adi}")
    print(f"Ses profili: {args.ses}")
    print("Turkce ses olusturuluyor (internet gerekiyor)...")

    with tempfile.TemporaryDirectory() as klasor:
        mp3 = Path(klasor) / "asistan.mp3"
        asyncio.run(ses_dosyasi_olustur(args.text, mp3, args.ses))
        decoded = miniaudio.decode_file(
            str(mp3),
            output_format=miniaudio.SampleFormat.FLOAT32,
            nchannels=1,
            sample_rate=48000,
        )
        pcm = np.frombuffer(decoded.samples, dtype=np.float32)
        bekle(args.delay, args.at)
        print(f"Araniyor: {TELEFON}")
        subprocess.Popen([str(microsip), TELEFON])
        print("Telefonu acabilirsin. Ses 8 saniye sonra baslayacak...")
        time.sleep(8)
        print("Ses telefona gonderiliyor...")
        sd.play(pcm, samplerate=48000, device=ses_cihazi, blocking=True)
        print("Mesaj tamamlandi. Gorusmeyi telefondan kapatabilirsin.")


if __name__ == "__main__":
    try:
        main()
    except (FileNotFoundError, RuntimeError, ValueError) as hata:
        print(f"HATA: {hata}")
    except KeyboardInterrupt:
        print("Iptal edildi.")
