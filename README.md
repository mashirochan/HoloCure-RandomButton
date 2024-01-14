# HoloCure Random Button Mod
A HoloCure mod that adds back the random character button!

![Example Screenshot](https://github.com/mashirochan/HoloCure-RandomButton/blob/de5a312a468a71780ec9020bb0197ac9f1d856d1/Random%20Button%20Example.gif)
![Blacklist Example](https://github.com/mashirochan/HoloCure-RandomButton/blob/c53c972cd36bb615c1ae283a5f6b42e25a8d2269/Blacklist%20Example.gif)

# Installation

- Download the `RandomButton.dll` file from the latest release of my [Random Button mod](https://github.com/mashirochan/HoloCure-RandomButton/releases/latest)
- Download the `InfiCore.dll` file from the latest release of my [InfiCore mod](https://github.com/mashirochan/HoloCure-InfiCore/releases/latest)
- Download the `CallbackManagerMod.dll` file from the latest release of PippleCultist's [Callback Manager mod](https://github.com/PippleCultist/CallbackManagerMod/releases/latest)
- Download `AurieManager.exe` from the [latest release of Aurie](https://github.com/AurieFramework/Aurie/releases/latest)
- (OPTIONAL) Open up `%localappdata%\HoloCure` in your File Explorer
  - Back up your save data!
- Run the `AurieManager.exe`
  - Click "Add Game" next to the game drop-down
    - Select your `HoloCure.exe` file
  - Click "Install Aurie" in the bottom right
  - Click "Add Mods" on the right
    - Select your `RandomButton.dll`, `InfiCore.dll`, and `CallbackManagerMod.dll` files
  - Click "Play Game"
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
