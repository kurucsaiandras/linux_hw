# Házi feladat

Név/Nevek NEPTUN kóddal:
- Kurucsai András (WWEI3B)
- Kardos Martin (FRAV6D)

# Feladat kiírás
Feladatunkban egy olyan szoftvert valósítunk meg Qt, linux kernel modul, illetve socket felhasználásával amely képes egy Maxim Integrated MAX7219 chipeket használó LED mátrix áramkör vezérlésére.
Az áramkör 4 db 8x8-as mátrixot tartalmaz melyeket 4db kaszkádosított integrált áramkör vezérel, velük SPI interfészen keresztül lehet kommunikálni.

A szoftver az alábbi funkciókkal fog rendelkezni:
* Grafikus felület Qt-ban megvalósítva, bekapcsolható, személyre szabható funkciókkal.
* Socket kommunikáció két linux rendszer között.
* A grafikus felületen keresztül animáció készítése (frame by frame).
* Rajzok készítése a grafikus felületen, rajzok invertálása.
* Rajzok elmentése fájlba.
* Bekapcsolt mikrofon kijelzése.
* Konfigurálható process kijelzés. (Pl. fut-e a microsoft teams, fut-e a discord? stb.)
*
# Megvalósított program
Milyen funkciókat sikerült megvalósítani? Milyen eltérések vannak a kiíráshoz képest? Hogyan kell bekonfigurálni, elindítani?

Szerver elindítása:
1,) a kernel modul make paranccsal való lefordítása
2,) "dtc spidev_disabler.dts -O dtb >spidev_disabler.dtbo" parancs segítéségvel a dts fájl fordítása mely az SPI bus 0 használatához szükséges
3,) a fordított dts betöltése a "sudo dtoverlay -d . spidev_disabler" paranccsal
4,) a kernel modul betöltése a "sudo insmod max7219x4.ko" paranccsal
5,) szerver lefordítása a gcc segítségével pl gcc server.c -o server
6,) a szerver elindítása a ./server {portszám} paranccsal

**Bemutató videó URL:**
https://youtu.be/Xmn5PWXf6pk

# Tapasztalatok
Milyen tapasztalatokat gyűjtött a feladat elkészítése során? Mi volt egyszerűbb / nehezebb a tervezetnél? Visszatekintve mit csinálna másként? (pár mondatban)
Nem érdemes belekezdeni a megvalósításba a dokumentációk megfelelő tanulmányozása nélkül.
