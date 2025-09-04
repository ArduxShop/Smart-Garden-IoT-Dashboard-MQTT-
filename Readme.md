# 🌱 Smart Garden IoT Dashboard (MQTT)
![Smart Garden IoT Dashboard](https://github.com/ArduxShop/Smart-Garden-IoT-Dashboard-MQTT-/blob/main/Smart%20Garden%20IoT%20Dashboard.png?raw=true)

Deskripsi
```
Deskripsi
---------
Dashboard berbasis web untuk monitoring sensor dan kontrol pompa pada sistem Smart Garden berbasis MQTT.

Fitur:
- Suhu (°C)
- Kelembaban udara (%)
- Kelembaban tanah (%)
- Kontrol manual/auto pompa
- Log pompa dengan sumber aksi
- Mode otomatis dengan timeout dan countdown

Dibangun menggunakan:
- HTML / CSS / JS
- Chart.js (visualisasi gauge)
- MQTT.js (komunikasi MQTT)
```

```
Fitur Versi Awal
----------------
Monitoring Sensor
- Menampilkan suhu, kelembaban udara, dan kelembaban tanah
- Visualisasi menggunakan gauge (Chart.js)

Kontrol Relay Pompa
- Tombol ON/OFF untuk kontrol manual
- Tombol AUTO/MANUAL untuk mode pompa
- Status relay tampil di dashboard

Log Pompa
- Menyimpan semua aksi (relay ON/OFF, mode AUTO/MANUAL)
- Disimpan di localStorage agar persist antar reload
- Sumber log: "Manual", "Dashboard", "ESP"
```

```
Penambahan / Update Sejak Awal
-------------------------------
1. Penyimpanan Status
- Status relay (ON/OFF) dan mode (AUTO/MANUAL) disimpan di localStorage
- Memastikan dashboard tetap sinkron saat reload halaman

2. Log dengan Sumber Lebih Detail
- Manual -> pengguna tekan tombol ON/OFF
- Dashboard -> pengguna tekan tombol AUTO/MANUAL
- ESP -> relay melaporkan status dari hardware
- Otomatis -> mode AUTO aktif karena timeout (tidak ada interaksi)

3. Timeout Manual → AUTO
- Jika mode MANUAL tidak ada interaksi selama 1 menit, otomatis pindah ke mode AUTO
- Log tercatat dengan sumber "Otomatis"

4. Countdown AUTO
- Setelah timeout memicu mode AUTO, muncul countdown 2 menit
  Mode: AUTO (Eksekusi dalam 120s)
- Hitungan berkurang tiap detik sampai 0, lalu tampil normal: Mode: AUTO
- Tombol AUTO manual tidak memicu countdown, log tetap "Dashboard"

5. Log Nyala/Mati Pompa
- Dilengkapi label "Otomatis" jika relay ON/OFF karena timeout
- Tetap mencatat "ESP" jika relay berubah status dari hardware (non-timeout)

6. Tampilan Dashboard
- Gauge visualisasi untuk sensor
- Status relay dan mode muncul di bawah tombol kontrol
- Semua aksi otomatis/manual tercatat di tabel log
```

```
Cara Pakai
----------
1. Koneksi ke Broker MQTT
- Default: HiveMQ public broker
  const client = mqtt.connect("wss://broker.hivemq.com:8884/mqtt", { clientId });
- Subscribe topik: smartgarden/sensor, smartgarden/relay/status

2. Kontrol Pompa
- ON/OFF -> hanya bisa di mode MANUAL
- AUTO/MANUAL -> tombol mengubah mode, log "Dashboard"
- Timeout 1 menit di mode MANUAL -> pindah AUTO otomatis

3. Logs
- Tabel menampilkan aksi terbaru di atas
- Disimpan di localStorage -> tetap ada saat reload

4. Countdown
- Hanya muncul jika mode AUTO via timeout
- Tidak berlaku jika tombol AUTO ditekan manual
```

```
Struktur File
-------------
| File       | Fungsi                     |
|------------|----------------------------|
| index.html | Dashboard utama            |
| Chart.js   | CDN untuk gauge            |
| MQTT.js    | CDN untuk MQTT             |
| localStorage | Penyimpanan log dan status |
```

```
Catatan Teknis
--------------
- Mode AUTO manual tidak trigger countdown -> pengguna tetap kontrol penuh
- Mode AUTO via timeout trigger countdown 120s -> memberi waktu sebelum eksekusi penuh
- Semua log aksi tersimpan di localStorage sehingga bisa persist antar reload
```

```
Pinout Wemos / Sensor / Relay
-----------------------------
| Perangkat    | Wemos Pin      | Keterangan |
|--------------|----------------|------------|
| DHT22        | 3V3            | VCC        |
| DHT22        | G              | GND        |
| DHT22        | D4 (GPIO2)     | Data       |
| Rain Sensor  | 3V3            | VCC        |
| Rain Sensor  | G              | GND        |
| Rain Sensor  | A0 (ADC0)      | Data       |
| Relay        | 5V             | VCC        |
| Relay        | G              | GND        |
| Relay        | D5 (GPIO14)    | IN         |
