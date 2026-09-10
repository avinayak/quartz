# Nine Icons 48X source

- Upstream: https://store.kde.org/p/1749686
- Author: `drgordbord`
- Original NineIcons: [grassmunk/Platinum9](https://github.com/grassmunk/Platinum9),
  in the [NineIcons directory](https://github.com/grassmunk/Platinum9/tree/master/NineIcons)
- Additional upstream credit: [grassmunk/Chicago95](https://github.com/grassmunk/Chicago95)
- Release asset: `NineIcons48x.tar.gz`, version 1.3, published 2023-05-23
- SHA-256: `ff40560cc92d633d25ba4bf824da0ed2261b1f6c00be3bf8f27e63cb3f941897`
- License: the upstream listing identifies the release as GPLv3

The upstream author describes Nine Icons 48X as an enhancement of grassmunk's
NineIcons, combining material from Platinum9 and Chicago95 with newly created
48-pixel folder and application icons. The archive in this directory is the
unchanged upstream release asset.

The upstream `index.theme` lists directories absent from the release, marks
raster directories as scalable, ends its directory list with an empty entry,
and names an unavailable lowercase `adwaita` fallback. The installer replaces
that manifest with `NineIcons48x.index.theme`, which declares only packaged
fixed-size directories and uses the installed MATE, Adwaita, and hicolor
fallback chain. Declared icon artwork is not modified.

The install staging step omits the archive's non-theme `Iconfactory` catalog
and `apps/16/Internet Explorer_2_16x16x8.png`. The latter's space-containing
icon name makes GTK 3.24 reject the entire generated cache; it is an obsolete,
application-specific alias and is not referenced by the normalized manifest.
All declared shared-theme artwork remains unchanged.
