# HoloCure Random Button Mod
A HoloCure mod that adds back the random character button!

![Example Screenshot](https://github.com/mashirochan/HoloCure-RandomButton/blob/de5a312a468a71780ec9020bb0197ac9f1d856d1/Random%20Button%20Example.gif)
![Blacklist Example](https://github.com/mashirochan/HoloCure-RandomButton/blob/c53c972cd36bb615c1ae283a5f6b42e25a8d2269/Blacklist%20Example.gif)

# Installation

Regarding the YYToolkit Launcher:

It's a launcher used to inject .dlls, so most anti-virus will be quick to flag it with Trojan-like behavior because of a similar use-case. The launcher is entirely open source (as is YYToolkit itself, the backbone of the project), so you're more than welcome to build everything yourself from the source: https://github.com/Archie-osu/YYToolkit/

- Download the .dll file from the latest release of my [Random Button mod](https://github.com/mashirochan/HoloCure-RandomButton/releases/latest)
- Download `Launcher.exe` from the [latest release of YYToolkit](https://github.com/Archie-osu/YYToolkit/releases/latest)
  - Place the `Launcher.exe` file anywhere you want for convenient access
- Open the folder your `HoloCure.exe` is in
  - Back up your game and save data while you're here!
  - Delete, rename, or move `steam_api64.dll` and `Steamworks_x64.dll` if you're on Steam
- Run the `Launcher.exe`
  - Click "Select" next to the Runner field
    - Select your `HoloCure.exe` (wherever it is)
  - Click "Open plugin folder" near the bottom right
    - This should create and open the `autoexec` folder wherever your `HoloCure.exe` is
    - Move or copy your `random-button-vX.X.X.dll` file into this folder
  - Click "Start process"
    - Hope with all your might that it works!

Not much testing at all has gone into this, so I'm really sorry if this doesn't work. Use at your own risk!

Feel free to join the [HoloCure Discord server](https://discord.gg/holocure) and look for the HoloCure Code Discussion thread in #holocure-general!

# Blacklist Format
The blacklist works off of internal character IDs, so you'll need to use a specific name for each.

The config variable format is a string array, like `"blacklist": ["sana", "roboco", "aki"]`.

| Character Name   | ID     | Character Name    | ID      | Character Name   | ID    |
|------------------|--------|-------------------|---------|------------------|-------|
| Amelia Watson    | ame    | Nekomata Okayu    | okayu   | Murasaki Shion   | shion |
| Gawr Gura        | gura   | Inugami Korone    | korone  | Nakiri Ayame     | ayame |
| Ninomae Ina'nis  | ina    | Tokino Sora       | sora    | Minato Aqua      | aqua  |
| Takanashi Kiara  | kiara  | AZKi              | azki    | Moona Hoshinova  | moona |
| Calliope Mori    | calli  | Roboco-san        | roboco  | Ayunda Risu      | risu  |
| Hakos Baelz      | bae    | Hoshimachi Suisei | suisei  | Airani Iofifteen | iofi  |
| Ouro Kronii      | kronii | Sakura Miko       | miko    | Kureiji Ollie    | ollie |
| Ceres Fauna      | fauna  | Akai Haato        | haato   | Pavolia Reine    | reine |
| Nanashi Mumei    | mumei  | Yozora Mel        | mel     | Anya Melfissa    | anya  |
| Tsukumo Sana     | sana   | Natsuiro Matsuri  | matsuri | Kobo Kanaeru     | kobo  |
| IRyS             | irys   | Aki Rosenthal     | aki     | Kaela Kovalskia  | kaela |
| Shirakami Fubuki | fubuki | Oozora Subaru     | subaru  | Vestia Zeta      | zeta  |
| Ookami Mio       | mio    | Yuzuki Choco      | choco   |                  |       |

# Troubleshooting

Here are some common problems you could have that are preventing your mod from not working correctly:

### YYToolkit Launcher Hangs on "Waiting for game..."
![Waiting for game...](https://i.imgur.com/DxDjOGz.png)

The most likely scenario for this is that you did not delete, rename, or move the `steam_api64.dll` and `Steamworks_x64.dll` files in whatever directory the `HoloCure.exe` that you want to mod is in.

### Failed to install plugin: random-button.dll
![Failed to install plugin](https://i.imgur.com/fcg1WWe.png)

The most likely scenario for this is that you tried to click "Add plugin" before "Open plugin folder", so the YYToolkit launcher has not created an `autoexec` folder yet. To solve this, either click "Open plugin folder" to create an `autoexec` folder automatically, or create one manually in the same directory as your `HoloCure.exe` file.
