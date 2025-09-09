# ESP32S3-CAM  [![en](https://img.shields.io/badge/lang-en-red.svg)](README.md) [![zh-CN](https://img.shields.io/badge/lang-zh--CN-green.svg)](README.zh-CN.md) [![fr](https://img.shields.io/badge/lang-fr-yellow.svg)](README.fr.md)


[![Release](https://img.shields.io/github/v/release/3SamuelW/ESP32S3-camera?display_name=tag)](https://github.com/3SamuelW/ESP32S3-camera/releases) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) [![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32--S3-orange)](https://platformio.org/) [![Last commit](https://img.shields.io/github/last-commit/3SamuelW/ESP32S3-camera)](https://github.com/3SamuelW/ESP32S3-camera/commits/main) [![GitHub stars](https://img.shields.io/github/stars/3SamuelW/ESP32S3-camera?style=social)](https://github.com/3SamuelW/ESP32S3-camera/stargazers)

---

***ESP32S3-CAM*** — un projet matériel + logiciel qui fonctionne sur une carte de développement ESP32-S3 avec caméra et écran. Ce dépôt contient les fichiers de conception PCB (Altium + Gerber/BOM) et le firmware (`PlatformIO` / `Arduino`) pour le projet.

------

## Résumé rapide

- **Matériel** : Schéma Altium et PCB (`hardware/Altium_Designer`), BOM et Gerber (`hardware/LCEDA`).
- **Logiciel** : Firmware basé sur PlatformIO dans `ESP32S3-CAM/`. L'environnement principal du projet est `esp32-s3-devkitc-1`.
- **Système de build** : PlatformIO (utilise la plateforme `espressif32`, framework `arduino`).

------

## Table des matières

- [Prérequis](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#prérequis)
- [Structure du projet](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#structure-du-projet)
- [Démarrage rapide (build & upload)](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#démarrage-rapide-build--upload)
- [Moniteur série](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#moniteur-série)
- [Fichiers matériel & Releases](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#fichiers-matériel--releases)
- [Notes de développement](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#notes-de-développement)
- [Contribuer](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#contribuer)
- [Licence](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#licence)

------

## Prérequis

- Git
- VS Code (recommandé) + extension PlatformIO IDE ou PlatformIO Core (CLI) installé (`pip install -U platformio` ou utilisez les installateurs officiels)
- Carte de développement ESP32-S3 (compatible avec `esp32-s3-devkitc-1`)
- Câble USB et pilotes série appropriés

------

## Structure du projet

```
ESP32S3-CAM/
├── platformio.ini 
├── convert.py
├── .gitignore
├── .pio/ 
└── src/
│ ├── main.cpp
│ ├── cameraTask.h/cpp
│ ├── webTask.h/cpp
│ ├── tfCard.h/cpp
│ ├── displayTask.h/cpp
│ ├── keyTask.h/cpp
│ ├── image.h
│ ├── config.h
└── README.md
```

------

## Démarrage rapide (build & upload)

> **Important** : vérifiez `upload_port` dans `platformio.ini` (par défaut dans le dépôt : `COM9`). Changez-le pour votre port série réel (ex. `COM3` sur Windows ou `/dev/ttyUSB0` sur Linux), ou surchargez-le au moment de l'upload.

### Utiliser PlatformIO dans VS Code (recommandé)

1. Ouvrez le dossier du dépôt dans VS Code.
2. Installez l'extension **PlatformIO IDE** si ce n'est pas déjà fait.
3. Dans la barre d'outils PlatformIO, choisissez l'environnement `esp32-s3-devkitc-1`.
4. Cliquez sur **Build** pour compiler.
5. Cliquez sur **Upload** pour flasher la carte.
6. Cliquez sur **Monitor** pour ouvrir le moniteur série.

### Utiliser PlatformIO CLI

Depuis le dossier `ESP32S3-CAM/` :

```bash
# build
cd ESP32S3-CAM
pio run -e esp32-s3-devkitc-1

# upload (utilisera upload_port de platformio.ini sauf si vous le surchargez)
pio run -e esp32-s3-devkitc-1 -t upload

# surcharger le port d'upload si nécessaire :
pio run -e esp32-s3-devkitc-1 -t upload --upload-port COM3
```

### Dépannage

- **L'upload échoue** : vérifiez `upload_port` et les pilotes, essayez un autre câble USB, utilisez `pio run -e esp32-s3-devkitc-1 -t upload --upload-port <votre-port>`.
- **Bibliothèques manquantes** : PlatformIO téléchargera `lib_deps`. Si le réseau est bloqué, installez les bibliothèques manuellement via PlatformIO ou placez-les dans `lib/`.

---

## Moniteur série

Débit par défaut dans `platformio.ini` : `monitor_speed = 115200`

Ouvrir le moniteur :

```bash
# PlatformIO CLI :
pio device monitor -p COM9 -b 115200
# ou
pio device monitor --port COM3 --baud 115200
```

---

## Fichiers matériel & Releases

- **Conception PCB (Altium)** : hardware/Altium_Designer/
- **BOM / Gerber / Pick-and-place** : hardware/LCEDA/

> [Télécharger les matériaux matériel](https://github.com/3SamuelW/ESP32S3-camera/releases/download/v1.0/hardware.zip)

---

## Notes de développement

- Environnement PlatformIO : `esp32-s3-devkitc-1` (voir `platformio.ini`)
- La PSRAM est activée dans la config (`board_build.psram = true` et `-DBOARD_HAS_PSRAM`).
- Les bibliothèques définies dans `lib_deps` seront automatiquement installées par PlatformIO pendant le build (ex. `TFT_eSPI`, `TJpg_Decoder`).

---

## Contribuer

- Forkez le dépôt et créez une branche de fonctionnalité.
- Gardez les commits petits et ciblés.
- Ouvrez une pull request décrivant vos changements.

> Si vous changez le câblage de la carte ou le modèle de caméra, mettez à jour `README.md` et `ESP32S3-CAM/config.h` en conséquence.

---

## Licence

Ajoutez un fichier LICENSE au dépôt (ex. MIT) si vous souhaitez open-sourcer le projet! En cas de doute, choisissez une licence permissive approuvée OSI comme MIT.

---

## Contact / Mainteneur

Mainteneur : 3SamuelW / [3samuelw@gmail.com](mailto:3samuelw@gmail.com)

Dépôt : https://github.com/3SamuelW/ESP32S3-camera