# Quartz

A classic Macintosh-inspired desktop theme for **Ubuntu MATE 24.04**. Quartz
includes shared GTK 2, GTK 3, GTK 4 and Marco themes, icons, bitmap fonts,
cursors, and Quartz Settings for adjusting the look of your desktop.

## Install

Open a terminal in your Ubuntu MATE desktop session.

1. Install the dependencies:

   ```bash
   sudo apt update
   sudo apt install git build-essential pkg-config libgtk2.0-dev libgtk-3-dev libgtk-4-dev gtk2-engines-pixbuf fonts-terminus-otb fontconfig python3 curl jq file dconf-cli
   ```

2. Download Quartz and install it **as your normal desktop user, without sudo**:

   ```bash
   git clone https://github.com/avinayak/quartz.git
   cd quartz
   ./install.sh
   ```

3. Log out and back in, then open **Quartz Settings** from the applications
   menu to choose colors, fonts, corner radius, and window controls.

You can also choose **Code → Download ZIP** on this page, extract the archive,
open a terminal in the extracted folder, and run `./install.sh` after installing
the dependencies above.

Installation changes your user's desktop theme, fonts, icons, cursors, and
shared GTK settings. It builds Quartz Settings locally and installs it under
`~/.local`. Run it inside your logged-in MATE session; other desktops and macOS
are not supported. Internet access is required for the Extras wallpaper download.

The full Quartz Extras installer runs automatically. Wallpapers become available
in **Control Center → Appearance → Background**; your selected background stays
unchanged. Firefox theming is a separate, reversible opt-in action under
**Quartz Settings → Extras → Add quartz theme to firefox**.

## Update

From the folder you cloned:

```bash
git pull --ff-only
./install.sh
```

Log out and back in if the installer requests it. ZIP users can download and
extract a fresh copy and run its installer.

## Compatibility and credits

Quartz styles shared desktop and toolkit components. Applications with custom
interfaces may retain some of their own appearance. See the
[theme notes](theme/Quartz-System6/README.md) and
[Quartz Settings guide](quartz-settings/README.md) for details.

Bundled third-party assets retain their original terms and attribution; see
`fonts/*SOURCE.md`, `fonts/community/licenses/`,
[icon credits](icons/NineIcons48x.SOURCE.md), and
[cursor credits](cursors/Quartz-System7-cursors/SOURCE.md).
Original Apple bitmap artwork and fonts are not covered by a new license grant.
