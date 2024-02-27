[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/Z8Z5NNXWL)
# QMamehook
###### "We have MAMEHOOKER at home." 

A bare-bones implementation of a MAME network output client, made primarily for compatible lightgun systems, using [MAMEHOOKER](https://dragonking.arcadecontrols.com/static.php?page=aboutmamehooker)-compatible *`gamename.ini`* files.

### Why WOULD you use this over MAMEHOOKER?
 - Cross-platform: i.e, works natively on Linux for native emulators, and the same code runs just as well on Windows.
 - Modern: Built on C++ & QT5/6, and made to interface with the MAME network output *standard*, meaning implicit support e.g. for RetroArch cores that use TCP localhost:8000 for feeding force feedback events.
 - Small & Simple: runs in the background with a single command, no admin privileges necessary.
 - Designed for light guns: Made for and exclusively compatible with the serial port interface used by PC light gun systems (which currently is [IR-GUN4ALL](https://github.com/SeongGino/ir-light-gun-plus) and [GUN4IR](https://forum.arcadecontrols.com/index.php/topic,161189.0.html)).
 - Compatible with MAMEHOOKER configs: Uses the same *.ini* files verbatim, no changes needed!

### Why would you NOT use this over MAMEHOOKER?
 - It's barebones: Strictly only supports light gun peripherals over serial (COM devices).
 - It's primarily for Linux: which has no options for this specific niche (Linux home light gun users wanting native force feedback) - for more elaborate setups, either [LEDSpicer](https://github.com/meduzapat/LEDSpicer) for Linux, or MAMEHOOKER itself for Windows might be better if you need more devices support.
 - Only supports MAME network output standard: for [DemulShooter](https://github.com/argonlefou/DemulShooter) users, that means MAMEHOOKER is still absolutely required for e.g. TeknoParrot/JConfig games or native Windows games et al (at least, until they too adopt the MAME network standard).
 - It's made by an idiot: Seriously, this is the first of two things I ever made for desktop PCs and my first pair of things using QT. *The entire effective codebase is like two pages of a single C++ class.* Contributions are welcome though to improve this!

### But y tho?
Because I wasn't happy with the other (or lacking thereof) solutions available, none of which supported simple serial devices for my lightguns, and I got very impatient and whipped this up in a day while working on GUN4ALL-GUI.

## Running:
> [!NOTE]
> Serial devices **must** be plugged in at runtime in order to work! QMamehook will emit a warning message if no compatible devices are detected. Currently, only devices bearing the GUN4ALL or GUN4IR vendor IDs will be detected.
>
> Also keep in mind that QMamehook will only correctly work with COM port writes (`cmw`) to ports **correlating to the intended player/slot number** (usually 1-4) - this does not need to match the COM port number in Windows, as the index is based on the count of *verified COM devices detected* (meaning always starting from "1"), not their ports.

> [!TIP]
> Running `QMamehook -v` will enable verbose output, which prints the exact output stream being received from the server app! Use this to e.g. see what MAME is sending out.
### For Linux:
##### Requirements: Anything with QT5 support.
 - AUR package coming soon!
 - Make sure your user is part of the `dialout` group (`# usermod -a -G dialout insertusernamehere`)

Just run the `QMamehook` executable in a terminal; send an interrupt signal (or `pkill QMamehook`) to stop it.

Game config files are searched in `~/.config/QMamehook/ini`, and the program output will indicate whether a correct file matching the `mame_start` message is found or not.
### For Windows:
##### Requirements: Windows 7 and up.
 - Download the latest release zip.
 - Extract the `QMamehook` folder to somewhere you can easily access - `QMamehook.exe` should be right beside `QT5Core.dll`, `QT5Network.dll`, and `QT5SerialPort.dll`.
 - Launch `QMamehook.exe`

Game config files are searched in `%LOCALAPPDATA%/QMamehook/QMamehook/ini` (yes, that's TWO QMamehook folders, oops), and the program output will indicate whether a correct file matching the `mame_start` message is found or not.

## Building:
#### Requires `qt-base` & `qt-serialport` (QT5/QT6)
 - Clone the repo:
  ```
  git clone https://github.com/SeongGino/QMamehook
  ```
 - Setup build directory:
  ```
  cd QMamehook
  mkdir build && mkdir build
  cmake .. -DCMAKE_BUILD_TYPE=Release
  ```
 - Make:
  ```
  make
  ```
And run:
  ```
  ./QMamehook
  ```

## TODO:
 - Fix code quality; getting a QT CLI app to use signals is kind of a PITA and the quit/finished signals aren't working rn for some reason.
 - Maybe other devices support soon?
 - Currently, configs are generated with whatever outputs are detected in the game session; is there a better way of grabbing all possible outputs rn? Or nah?

## Thanks
 - ArcadeForums users, whose collective support on the GUN4ALL project are what inspired this side hustle.
 - MAME, for the output signal standard that which this is made to communicate with.
 - Sleep deprivation
 - Stubbornness
 - And Autism
