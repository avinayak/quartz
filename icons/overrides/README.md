# Shared desktop icon sizes

The 22px and 24px `user-desktop` icons center the archive's original 16px
NineIcons artwork on transparent canvases. This prevents small panel and
toolbar slots from selecting and shrinking the 32px drawing. The native
16px and 32px icons are unchanged. SVG embeds the original PNG without
resampling it. The theme installer copies these overrides before rebuilding
the icon cache; include this directory when copying the project to a guest.
