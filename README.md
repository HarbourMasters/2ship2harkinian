[comment]: <> (Todo: Make Light Mode Image)
[comment]: <> (Todo: Make Dark Mode Image)

# 2 Ship 2 Harkinian

> [!NOTE]
> ### 📱 This fork adds an unofficial **iPhone & iPad** port
> Majora's Mask on your phone with touch controls — no jailbreak, no controller needed.
> **[Jump to the iOS guide ↓](#2-ship-2-harkinian-for-iphone--ipad-unofficial)**
>
> Everything else in this repo matches upstream. This port is a community effort and is
> **not** supported by the official HarbourMasters Discord — please open an issue here.

## Discord

Official Discord: https://discord.com/invite/shipofharkinian

If you're having any trouble after reading through this `README`, feel free ask for help in the 2 Ship 2 Harkinian Support text channels. Please keep in mind that we do not condone piracy.

# Quick Start

2Ship does not include any copyrighted assets.  You are required to provide a supported copy of the game.

### 1. Verify your ROM dump
You can verify you have dumped a supported copy of the game by using the compatibility checker at https://2ship.equipment/. If you'd prefer to manually validate your ROM dump, you can cross-reference its `sha1` hash with the hashes [here](docs/supportedHashes.json).

### 2. Download 2 Ship 2 Harkinian from [Releases](https://github.com/HarbourMasters/2Ship2Harkinian/releases)

### 3. Launch the Game!
#### Windows
* Extract the zip
* Launch `2ship.exe`

#### Linux
* Place your supported copy of the game in the same folder as the appimage.
* Execute `2ship.appimage`. You may have to `chmod +x` the appimage via terminal.

#### macOS
* Run `2ship.app`.
* When prompted, select your supported copy of the game.

#### iOS (iPhone / iPad)
* Installation works differently — see the
  [iOS guide](#2-ship-2-harkinian-for-iphone--ipad-unofficial) below.

### 4. Play!

Congratulations, you are now sailing with 2 Ship 2 Harkinian! Have fun!

# 2 Ship 2 Harkinian for iPhone & iPad (unofficial)

A native iOS build of 2Ship — Majora's Mask running on your phone, with on-screen touch
controls, Metal rendering at your display's full resolution, and up to 120fps on ProMotion
devices. No jailbreak. No controller required.

This is a community fork, not an official HarbourMasters release — **please don't ask their
Discord for support with it.** Open an issue here instead.

> Like every Ship of Harkinian project, this ships **no game content whatsoever**. You supply
> your own legally dumped Majora's Mask ROM, and the file you make from it never leaves your
> devices.

## What you need

| | |
| --- | --- |
| **A device** | iPhone or iPad running **iOS 16 or newer** |
| **A computer** | Windows, macOS or Linux — used once to install the app and copy your game data |
| **Your own ROM** | A Majora's Mask **US** ROM you dumped yourself, converted on desktop (step 3) |
| **An Apple ID** | A free one is fine (see the 7-day note below) |
| **A controller** | *Optional.* Touch controls cover the whole game, including ocarina songs |

## Installing

**1 — Get the app.** Download the `2ship-ios-ipa` artifact from the
[Actions tab](../../actions/workflows/ios.yml) (open the newest successful *ios* run, then
the artifact at the bottom) and unzip it to get `2ship-ios.ipa`. You can also
[build it yourself](#building-it-yourself).

**2 — Put it on your device.** Sideloading installs apps outside the App Store using your own
Apple ID. Two tools work; pick either:

* **[Sideloadly](https://sideloadly.io/)** *(recommended — simplest, works on Windows/macOS)*
  Plug the device in, drag the `.ipa` onto the window, enter your Apple ID, press **Start**.
* **[AltStore](https://altstore.io/)** — install AltServer on the computer, install AltStore
  to the device, then add the `.ipa` from within AltStore.

On the device, enable **Settings → Privacy & Security → Developer Mode** first (it reboots).
If the app icon appears but won't open, go to **Settings → General → VPN & Device
Management** and trust your Apple ID.

**3 — Give it your game data.** On your computer, run the desktop version of 2Ship once with
your ROM. It produces a file called **`mm.o2r`** — that's your game, converted. Copy it into
the app:

* **Windows:** *Apple Devices* app → your device → **Files** → **2Ship** → drag `mm.o2r` in
* **macOS:** Finder → your device → **Files** → **2Ship**
* **On the device itself:** Files app → *On My iPhone* → **2Ship**

**4 — Play.** Launch 2Ship. Saves, settings, mods and logs all live in that same visible
folder, so they're easy to back up — and they survive app updates.

## Playing with touch

The on-screen controls are designed for the way Majora's Mask actually plays:

* **Left thumb** — a floating analog stick that appears wherever you touch
* **Right thumb** — A and B where you'd expect them, with the four C-buttons above
* **Shoulders** — Z for targeting, R for your shield, L in the corner
* **Anywhere else** — drag to move the camera
* **♪ button** — swaps the right side to a row of piano keys for ocarina songs
* **Gear** — opens the full settings menu · **eye** — hides the controls for screenshots

Pair a Bluetooth controller (Xbox, PlayStation, Backbone) in iOS Settings and the overlay
hides itself automatically; unplug it and the controls come back.

Everything is adjustable in **Settings → Touch Controls**: opacity, button size, stick style,
camera sensitivity, an edge-hugging layout, and optional **gyro aiming** for first-person.

## Performance

Defaults target a sharp, smooth picture on a modern device. If you want to trade one for the
other, everything is live in the settings menu — no rebuild:

* **Internal Resolution** — the main dial. Lower it for more frames.
* **Native Resolution** — off renders at a smaller size and upscales (much faster on older devices)
* **Current FPS / Match Refresh Rate** — 120fps is available on ProMotion screens
* **Crisp Fonts** — sharper menu text, slightly more memory (restart to apply)

## Troubleshooting

| Symptom | Fix |
| --- | --- |
| **App expired after a week** | Free Apple IDs sign apps for 7 days. Re-run Sideloadly, or let AltStore refresh it automatically over WiFi. A paid Apple Developer account lasts a year. |
| **"2Ship needs game data"** | `mm.o2r` isn't in the app's folder yet — see step 3. |
| **Black screen but music plays** | You're on an old build; grab the newest artifact. |
| **Everything crashes at launch** | Delete `2ship2harkinian.json` from the app's folder via the Files app. That resets settings only — your saves are untouched. |
| **No sound** | Check the ringer/silent switch isn't the issue — recent builds play regardless — then Settings → Audio. |
| **Game runs in slow motion** | Update; newer builds keep correct speed by dropping frames instead. |
| **Controls feel too big/small** | Settings → Touch Controls → Button Scale and UI Scale. |

Still stuck? [Open an issue](../../issues) and attach `logs/2s2h.log` from the app's folder.

## Building it yourself

CI builds on every push to the `ios` branch (see
[.github/workflows/ios.yml](.github/workflows/ios.yml)) — you don't need a Mac to use it, only
a GitHub account. To build locally you need macOS with Xcode 16+, CMake ≥3.26 and Ninja:

```bash
# one-time: cross-compile the audio/image libraries for iphoneos
bash .github/scripts/build-ios-deps.sh "$PWD/ios-deps-prefix"

cmake -H. -Bbuild-cmake -GNinja   -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphoneos   -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0   -DPLATFORM=OS64 -DDEPLOYMENT_TARGET=16.0   -DCMAKE_BUILD_TYPE=Release   -DCMAKE_PREFIX_PATH="$PWD/ios-deps-prefix" -DCMAKE_FIND_ROOT_PATH="$PWD/ios-deps-prefix"   -DBUNDLE_ID=com.example.twoship
cmake --build build-cmake --target 2ship -j
# then zip build-cmake/mm/2s2h.app into a Payload/ folder as an unsigned .ipa
```

The engine changes live in a companion fork:
[libultraship `ios` branch](https://github.com/PatFromSplatt/libultraship/tree/ios).

## Honest limitations

* **No in-app ROM extraction.** Convert on desktop, copy the file once (step 3).
* **Sideloading only.** This can never be on the App Store, so there's no auto-update — you
  re-install to get new builds (your saves stay put).
* **Free Apple IDs re-sign every 7 days.** Not this project's doing; that's Apple's limit.
* Tested most on recent iPhones. iPad works and is landscape-only, but has had less testing —
  reports welcome.

## Credits

[2 Ship 2 Harkinian](https://github.com/HarbourMasters/2ship2harkinian) by HarbourMasters,
built on [libultraship](https://github.com/Kenix3/libultraship) by Kenix3 and contributors.
Touch-layout ideas come from Waterdish's
[Android ports](https://github.com/Waterdish/2ship2harkinian-Android). This iOS port is an
independent community effort — all bugs in it are ours, not theirs.

# Configuration

### Default keyboard configuration
| N64 | A | B | Z | Start | Analog stick | C buttons | D-Pad |
| - | - | - | - | - | - | - | - |
| Keyboard | X | C | Z | Space | WASD | Arrow keys | TFGH |

### Other shortcuts
| Keys | Action |
| - | - |
| F1 | Toggle menubar |
| F11 | Fullscreen |
| Tab | Toggle Alternate assets |
| Ctrl+R | Reset |

### Graphics Backends
Currently, there are three rendering APIs supported: DirectX 11 (Windows), OpenGL (all platforms), and Metal (macOS). You can change which API to use in the `Settings` menu of the menubar, which requires a restart.

If you're having an issue with crashing, you can also change the API manually in the `2ship2harkinian.json` file by finding the `"Backend": {` section and updating the backend ID and name. Be sure to use one of the valid values:

- `0` = DirectX 11 (default on Windows)
- `1` = OpenGL
- `2` = Metal (default on macOS)

# Custom Assets

Custom assets are packed in `.o2r` or `.otr` files. To use custom assets, place them in the `mods` folder.

If you're interested in creating and/or packing your own custom asset `.o2r`/`.otr` files, check out the following tools:
* [**retro - OTR and O2R generator**](https://github.com/HarbourMasters64/retro)
* [**fast64 - Blender plugin (Note that MM is not fully supported at this time)**](https://github.com/HarbourMasters/fast64)

# Development

If you want to manually compile 2S2H, please consult the [building instructions](docs/BUILDING.md).

# Nightly Builds
If you want to playtest a continuous integration build, you can find them at the links below. Keep in mind that these are for playtesting only, and you will likely encounter bugs and possibly crashes. 

* [Windows](https://nightly.link/HarbourMasters/2ship2harkinian/workflows/main/develop/2ship-windows.zip)
* [Linux](https://nightly.link/HarbourMasters/2ship2harkinian/workflows/main/develop/2ship-linux.zip)
* [Mac](https://nightly.link/HarbourMasters/2ship2harkinian/workflows/main/develop/2ship-mac.zip)

<a href="https://github.com/Kenix3/libultraship/">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="./docs/poweredbylus.darkmode.png">
    <img alt="Powered by libultraship" src="./docs/poweredbylus.lightmode.png">
  </picture>
</a>

