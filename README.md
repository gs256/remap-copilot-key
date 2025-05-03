# Remap Copilot Key

Reassigns `Copilot` key back to `Ctrl` without PowerToys

<img src="assets/keyboard-image.jpg" width=400/>

## How does it work?

`Copilot` key triggers `Win+Shift+F23` shortcut. This app sets up a hook for Windows keyboard events and emulates `Ctrl` if it encounters `Win+Shift+F23` combination.

## Installation

1. Download [latest release](https://github.com/gs256/remap-copilot-key/releases/latest).

2. Press `Win+R`, type `shell:startup` and press `Enter` to open the Start-up folder.

3. Copy downloaded `remap-copilot-key.exe` file into the Start-up folder to have it run automatically when Windows starts.

4. Double-click the executable to launch it manually for the first time.

## Known issues

1. `Win`, `Shift` and `F23` keys are still being pressed when you hit the `Copilot` key but are immediately released afterwards. Keyboard Manager in PowerToys has the same behaviour.

2. If you press `Win` or `Shift` along with `Copilot` key and then release one of these three keys, the other two keys will also be released. Keyboard Manager in PowerToys has similar behaviour.

## License

MIT
