# Quartz Extras

Quartz Settings has **Appearance** and **Extras** tabs. Extras offers
**Download Recommended Wallpapers**, which runs the installed Quartz Extras
installer asynchronously and registers the System7 collection in MATE's wallpaper
chooser. It shows download activity, completion, and errors; the current desktop
background stays selected. The full Extras folder is installed with Settings so
this action works without the source checkout.

Extras also offers **Add quartz theme to firefox**. Close Firefox, click the
button, and reopen Firefox. It adds opaque Quartz-colored toolbars, outlined
compact tabs and address fields, and Quartz fonts. It reads your saved Quartz
palette; click again to refresh it after changing colors. It applies to existing
native, Snap, and Flatpak Firefox profiles for the current user. Start Firefox
once first if you do not yet have a profile. Python 3 and `pgrep` are required.

This is the explicitly authorized, opt-in exception to the project's shared-theme
scope. The full Extras installer ships with this helper through Quartz Settings
installation, but never activates the Firefox theme automatically. It styles
browser chrome, not web pages or Firefox's welcome-page content.

The helper preserves existing `userChrome.css` rules and `user.js` preferences,
backs up the original files in each profile's `quartz-firefox-backup` directory,
and enables Firefox's legacy stylesheet preference. Mozilla does
[not officially support browser CSS customization](https://support.mozilla.org/en-US/kb/firefox-advanced-customization-and-configuration);
Firefox updates may require adjustments to the stylesheet.

To remove it, close Firefox and run:

```bash
~/.local/share/quartz-settings/extras/firefox/install.py --remove
```

For a custom install prefix, use that prefix instead of `~/.local`. Removal
preserves unrelated edits and restores the original stylesheet preference.
Backups are retained with a `-removed-` suffix for recovery.

**Use Quartz logo instead of MATE** replaces the shared desktop menu logos
with a simple tall black diamond on a transparent background. The asset is
`logo/quartz-logo.svg`; symbolic menu icons use the same shape and follow the
desktop's foreground color. The `Quartz-Desktop-Logo` icon theme inherits the
current icon theme, preserving all other icons. MATE menus update immediately.

This option is activated only by its button. Subsequent full Extras installs
preserve the choice, recorded in
`${XDG_CONFIG_HOME:-$HOME/.config}/quartz-settings/desktop-logo-enabled`.
The helper needs Python 3, PyGObject (`python3-gi`), and `gtk-update-icon-cache`.
To return to the previous icons, remove that marker and choose the inherited
icon theme (normally NineIcons48x) in MATE Appearance → Customize → Icons.

Quartz Extras installs personal, user-scoped resources for the shared Quartz
desktop theme. Its current contents are wallpapers from the live
[System7 Unsplash collection](https://unsplash.com/collections/ycaoGBS5pZ8/system7).

## Install or refresh

The top-level `apply-ubuntu-mate-theme.sh` runs the entire installer whenever
the `quartz-extras` folder exists, including any extras added in the future,
not just wallpapers. It skips this step only when that folder is missing;
installation failures are reported as errors. Include this folder when
copying the project to a restored guest.

Run the installer as the logged-in MATE desktop user, without `sudo`:

```bash
./quartz-extras/install.sh
```

Each run reads the current collection, downloads every photo at up to 3840
pixels wide, and builds the MATE wallpaper catalog dynamically. This means
adding or removing a photo on Unsplash is reflected the next time the script
runs. The repository does not contain copies of the photos.

Wallpapers are stored in
`${XDG_DATA_HOME:-$HOME/.local/share}/backgrounds/quartz-extras/system7` and
registered through the user's shared `mate-background-properties` catalog.
The installer does not change the currently selected desktop background.

Wallpaper registration currently targets MATE on Linux. It requires `curl`,
`jq`, and the standard `file` utility, and it requires an internet connection
while the installer runs.

After installation, open **Control Center › Appearance › Background** to
choose one of the entries. Each entry begins with `System7` and includes the
photographer's name. If Appearance was already open, close and reopen it so
MATE reloads the catalog.

The photos retain their original aspect ratios and are not cropped. Their use
is governed by the [Unsplash License](https://unsplash.com/license); copyright
remains with each photographer.
