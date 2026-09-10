
# Quartz

> [!WARNING]
> **Experimental software — do not install on real hardware.**
> Use only in a disposable virtual machine, and take a snapshot before installing.
> Quartz changes desktop settings and may break your desktop or leave it unusable.
> Only proceed if you understand what the installer does and know how to recover
> your environment. **Use entirely at your own risk.** This project is provided
> as-is, without warranty. I accept no responsibility for damage, data loss, or
> any other consequences of installing or using it.

A classic Macintosh-inspired desktop theme for **Ubuntu MATE 24.04**. Quartz
includes shared GTK 2, GTK 3, GTK 4 and Marco themes, icons, bitmap fonts,
cursors, and Quartz Settings for adjusting the look of your desktop.

<img width="1440" height="900" alt="vm-20260909-222344" src="https://github.com/user-attachments/assets/67b75e6f-5148-4a3d-ad10-4eec140a84e0" />

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

## Compatibility

Quartz styles shared desktop and toolkit components. Applications with custom
interfaces may retain some of their own appearance. See the
[theme notes](theme/Quartz-System6/README.md) and
[Quartz Settings guide](quartz-settings/README.md) for details.

## Credits

Quartz builds on the work of these projects, artists, and maintainers:

- **[NineIcons / Platinum9](https://github.com/grassmunk/Platinum9)** by
  **grassmunk and contributors** — the original NineIcons icon set, available
  in the repository's [NineIcons folder](https://github.com/grassmunk/Platinum9/tree/master/NineIcons).
- **[Nine Icons 48X](https://store.kde.org/p/1749686)** by **drgordbord** — the
  enhanced icon pack bundled with Quartz. Its upstream credits include NineIcons,
  Platinum9, and [Chicago95](https://github.com/grassmunk/Chicago95) by grassmunk
  and contributors. See the [icon source notes](icons/NineIcons48x.SOURCE.md)
  for the exact release and packaging changes.
- **Apple and the original Macintosh designers** — the classic System 7 bitmap
  fonts and cursor artwork. The [font source notes](fonts/System7-BITMAP-SOURCE.md),
  [Geneva notes](fonts/Geneva-SOURCE.md), and
  [cursor source notes](cursors/Quartz-System7-cursors/SOURCE.md) document their provenance.
- **Community bitmap font authors and maintainers** — Spleen, Tamzen, Cozette,
  Scientifica, Creep, Cherry, Kirsch, Proggy, Gohufont, ProFont, and X11 Fixed.
  See the [community font credits](fonts/community/README.md) for upstream links
  and [original license notices](fonts/community/licenses/).
- **Unsplash photographers** — the optional wallpapers in the
  [System7 collection](https://unsplash.com/collections/ycaoGBS5pZ8/system7).
  The wallpaper installer preserves photographer names in the desktop catalog.

Bundled third-party assets retain their original terms and attribution; see
`fonts/*SOURCE.md`, `fonts/community/licenses/`,
[icon credits](icons/NineIcons48x.SOURCE.md), and
[cursor credits](cursors/Quartz-System7-cursors/SOURCE.md).
Original Apple bitmap artwork and fonts are not covered by a new license grant.
