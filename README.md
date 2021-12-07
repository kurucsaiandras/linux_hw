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
# Megvalósított program
Az eredeti feladatkiírásunkoz képest csak a konfigurálható process kijelzést nem sikerült megvalósítani.

Szerver elindítása:
* a kernel modul make paranccsal való lefordítása
* "dtc spidev_disabler.dts -O dtb >spidev_disabler.dtbo" parancs segítéségvel a dts fájl fordítása mely az SPI bus 0 használatához szükséges
* a fordított dts betöltése a "sudo dtoverlay -d . spidev_disabler" paranccsal
* a kernel modul betöltése a "sudo insmod max7219x4.ko" paranccsal
* szerver lefordítása a gcc segítségével pl gcc server.c -o server
* a szerver elindítása a ./server {portszám} paranccsal

A kliensoldalon a csatlakozás ip cím alapján történik, a kapcsolat megfelelőségét a gui a bal alsó sarokban indikálja.

**Bemutató videó URL:**
https://youtu.be/Xmn5PWXf6pk

# Tapasztalatok

Nem érdemes belekezdeni a megvalósításba a dokumentációk megfelelő tanulmányozása nélkül. A QT saját típusrendszerének elsajátítása sok utánaolvasást igényelt.
A QT-ban a hardverrel kapcsolatos bájtok generálása az elképzelésnél könnyebben ment, viszont néhány funkció megvalósítása a dokumentáció alaposabb tanulmányozását igényelte (pl mikrofon lekérdezése).
A kernel modul első betöltésekor tapasztaltuk hogy az SPI bus 0 már le van foglalva a rendszeren más modul által így ehhez kellett megoldást találni ami időigényes volt.

Tapasztalatot gyűjtöttünk a Qt alkalmazások, a kernel modulok készítésében, valamint debuggolásában (pl dmesg), továbbá egy valós fizikai hardver interfészelésében, és a hozzá szükséges felhasználóbarát grafikus felület tervezésében.
