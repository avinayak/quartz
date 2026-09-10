CC ?= cc
PKG_CONFIG ?= pkg-config
PREFIX ?= /usr/local
DESTDIR ?=

bindir := $(PREFIX)/bin
libexecdir := $(PREFIX)/libexec/quartz-settings
datadir := $(PREFIX)/share
builddir := build
themedir := $(datadir)/quartz-settings/theme/Quartz-System6
THEME_SOURCE_DIR ?= ../theme/Quartz-System6
EXTRAS_SOURCE_DIR ?= ../quartz-extras
extrasdir := $(datadir)/quartz-settings/extras

CPPFLAGS ?=
CPPFLAGS += -DQUARTZ_TYPOGRAPHY_PROGRAM='"$(themedir)/typography.py"'
CFLAGS ?= -O2 -g
WARNINGS := -Wall -Wextra -Wpedantic -Wformat=2 -Wshadow -Wpointer-arith \
	-Wcast-align -Wstrict-prototypes

PROGRAMS := \
	$(builddir)/quartz-settings \
	$(builddir)/quartz-gtk2-audit \
	$(builddir)/quartz-gtk3-audit \
	$(builddir)/quartz-gtk4-audit
GENERATED := \
	$(builddir)/org.quartz.Settings.desktop \
	$(builddir)/quartz-theme-apply \
	$(builddir)/quartz-panel-theme \
	$(builddir)/quartz-panel-theme.service

UI_TESTS := $(builddir)/test-extras $(builddir)/test-activation $(builddir)/test-css3 $(builddir)/test-css4

.PHONY: all check check-dither check-firefox check-cursors check-deps check-guards check-panel check-prefix check-presets check-preview check-renderer check-ui clean FORCE install uninstall

all: check-deps $(PROGRAMS) $(GENERATED)

check-deps:
	@missing=""; \
	for package in gtk+-2.0 gtk+-3.0 gtk4; do \
		if ! $(PKG_CONFIG) --exists "$$package"; then missing="$$missing $$package"; fi; \
	done; \
	if [ -n "$$missing" ]; then \
		printf 'Missing development packages:%s\n' "$$missing" >&2; \
		printf 'Ubuntu: sudo apt install build-essential pkg-config libgtk2.0-dev libgtk-3-dev libgtk-4-dev\n' >&2; \
		exit 1; \
	fi; \
	if ! $(PKG_CONFIG) --atleast-version=4.14 gtk4; then \
		printf 'GTK 4.14 or newer is required (Ubuntu MATE 24.04 provides it).\n' >&2; \
		exit 1; \
	fi

$(builddir):
	install -d -m 0755 "$@"

FORCE:

$(builddir)/quartz-settings: src/quartz-settings.c FORCE | $(builddir)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) \
		-DQUARTZ_AUDIT_DIR='"$(libexecdir)"' \
		-DQUARTZ_EXTRAS_PROGRAM='"$(extrasdir)/install.sh"' \
		-DQUARTZ_APPLY_PROGRAM='"$(libexecdir)/quartz-theme-apply"' \
		$$( $(PKG_CONFIG) --cflags gtk+-3.0 ) \
		-o "$@" "$<" $$( $(PKG_CONFIG) --libs gtk+-3.0 )

$(builddir)/quartz-gtk2-audit: src/quartz-gtk2-audit.c | $(builddir)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) \
		-Wno-deprecated-declarations -Wno-strict-prototypes \
		$$( $(PKG_CONFIG) --cflags gtk+-2.0 ) \
		-o "$@" "$<" $$( $(PKG_CONFIG) --libs gtk+-2.0 )

$(builddir)/quartz-gtk3-audit: src/quartz-gtk3-audit.c | $(builddir)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) \
		-Wno-deprecated-declarations \
		$$( $(PKG_CONFIG) --cflags gtk+-3.0 ) \
		-o "$@" "$<" $$( $(PKG_CONFIG) --libs gtk+-3.0 )

$(builddir)/quartz-gtk4-audit: src/quartz-gtk4-audit.c | $(builddir)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) \
		-Wno-deprecated-declarations -Wno-pedantic \
		$$( $(PKG_CONFIG) --cflags gtk4 ) \
		-o "$@" "$<" $$( $(PKG_CONFIG) --libs gtk4 )

$(builddir)/org.quartz.Settings.desktop: data/org.quartz.Settings.desktop.in FORCE | $(builddir)
	sed 's|@BINDIR@|$(bindir)|g' "$<" >"$@"

$(builddir)/quartz-theme-apply: data/quartz-theme-apply.in FORCE | $(builddir)
	sed 's|@THEME_SOURCE_DIR@|$(themedir)|g' "$<" >"$@"
	chmod 0755 "$@"

$(builddir)/quartz-panel-theme: data/quartz-panel-theme.in | $(builddir)
	cp "$<" "$@"
	chmod 0755 "$@"

$(builddir)/quartz-panel-theme.service: data/quartz-panel-theme.service.in FORCE | $(builddir)
	sed 's|@LIBEXECDIR@|$(libexecdir)|g' "$<" >"$@"

check-renderer: $(builddir)/quartz-theme-apply
	bash tests/test-theme-renderer.sh "$(builddir)/quartz-theme-apply"

check-presets:
	bash tests/test-presets.sh

check-preview:
	bash tests/test-preview.sh

check-guards:
	bash tests/test-shared-guards.sh

# Optional: requires ImageMagick and xcursorgen, not GTK development packages.
check-cursors:
	bash tests/test-cursors.sh

$(builddir)/test-extras: tests/test-extras.c src/quartz-settings.c | $(builddir)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) \
		$$( $(PKG_CONFIG) --cflags gtk+-3.0 ) \
		-o "$@" "$<" $$( $(PKG_CONFIG) --libs gtk+-3.0 )

$(builddir)/test-activation: tests/test-activation.c src/quartz-settings.c | $(builddir)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) \
		$$( $(PKG_CONFIG) --cflags gtk+-3.0 ) \
		-o "$@" "$<" $$( $(PKG_CONFIG) --libs gtk+-3.0 )

$(builddir)/test-css3: tests/test-css.c | $(builddir)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) \
		$$( $(PKG_CONFIG) --cflags gtk+-3.0 ) \
		-o "$@" "$<" $$( $(PKG_CONFIG) --libs gtk+-3.0 )

$(builddir)/test-css4: tests/test-css.c | $(builddir)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) -Wno-pedantic \
		$$( $(PKG_CONFIG) --cflags gtk4 ) \
		-o "$@" "$<" $$( $(PKG_CONFIG) --libs gtk4 )

# Optional: run under a desktop display or xvfb-run.
check-ui: $(UI_TESTS)
	bash tests/test-ui.sh

check-panel: $(builddir)/quartz-panel-theme
	bash tests/test-panel-theme.sh "$(builddir)/quartz-panel-theme"

check-prefix:
	bash tests/test-prefix-rebuild.sh

check-dither:
	python3 -m unittest discover -s "$(EXTRAS_SOURCE_DIR)/dither" -p 'test_*.py'

check-firefox:
	python3 -m unittest discover -s "$(EXTRAS_SOURCE_DIR)/firefox" -p 'test_*.py'

check: all check-typography check-renderer check-panel check-prefix check-presets check-preview check-guards
	@if command -v desktop-file-validate >/dev/null 2>&1; then \
		desktop-file-validate $(builddir)/org.quartz.Settings.desktop; \
	fi
	@for program in $(PROGRAMS); do test -x "$$program"; done
	@test -x "$(builddir)/quartz-theme-apply"

install: all
	install -d -m 0755 \
		"$(DESTDIR)$(bindir)" \
		"$(DESTDIR)$(libexecdir)" \
		"$(DESTDIR)$(datadir)/applications" \
		"$(DESTDIR)$(datadir)/systemd/user" \
		"$(DESTDIR)$(themedir)"
	install -m 0755 $(builddir)/quartz-settings \
		"$(DESTDIR)$(bindir)/quartz-settings"
	install -m 0755 \
		$(builddir)/quartz-gtk2-audit \
		$(builddir)/quartz-gtk3-audit \
		$(builddir)/quartz-gtk4-audit \
		"$(DESTDIR)$(libexecdir)/"
	install -m 0755 \
		$(builddir)/quartz-theme-apply \
		$(builddir)/quartz-panel-theme \
		"$(DESTDIR)$(libexecdir)/"
	install -m 0644 $(builddir)/org.quartz.Settings.desktop \
		"$(DESTDIR)$(datadir)/applications/org.quartz.Settings.desktop"
	install -m 0644 $(builddir)/quartz-panel-theme.service \
		"$(DESTDIR)$(datadir)/systemd/user/quartz-panel-theme.service"
	cp -R -- "$(THEME_SOURCE_DIR)/." "$(DESTDIR)$(themedir)/"
	@if test -d "$(EXTRAS_SOURCE_DIR)"; then \
		test -x "$(EXTRAS_SOURCE_DIR)/install.sh" || exit 1; \
		install -d -m 0755 "$(DESTDIR)$(extrasdir)"; \
		cp -R -- "$(EXTRAS_SOURCE_DIR)/." "$(DESTDIR)$(extrasdir)/"; \
	fi

uninstall:
	unlink "$(DESTDIR)$(bindir)/quartz-settings"
	unlink "$(DESTDIR)$(libexecdir)/quartz-gtk2-audit"
	unlink "$(DESTDIR)$(libexecdir)/quartz-gtk3-audit"
	unlink "$(DESTDIR)$(libexecdir)/quartz-gtk4-audit"
	unlink "$(DESTDIR)$(libexecdir)/quartz-theme-apply"
	unlink "$(DESTDIR)$(libexecdir)/quartz-panel-theme"
	rmdir "$(DESTDIR)$(libexecdir)"
	unlink "$(DESTDIR)$(datadir)/applications/org.quartz.Settings.desktop"
	unlink "$(DESTDIR)$(datadir)/systemd/user/quartz-panel-theme.service"
	@if test -d "$(DESTDIR)$(themedir)" && test ! -L "$(DESTDIR)$(themedir)"; then \
		find "$(DESTDIR)$(themedir)" -depth -delete; \
	fi
	@if test -d "$(DESTDIR)$(extrasdir)" && test ! -L "$(DESTDIR)$(extrasdir)"; then \
		find "$(DESTDIR)$(extrasdir)" -depth -delete; \
	fi
	@rmdir "$(DESTDIR)$(datadir)/quartz-settings/theme" 2>/dev/null || true
	@rmdir "$(DESTDIR)$(datadir)/quartz-settings" 2>/dev/null || true

clean:
	rm -f $(PROGRAMS) $(GENERATED) $(UI_TESTS)
	@rmdir "$(builddir)" 2>/dev/null || true

.PHONY: check-typography
check-typography:
	python3 -m unittest discover -s tests -p test_typography.py

# Optional: run after installing the bundled fonts and Typography policy.
.PHONY: check-font-rendering
check-font-rendering:
	python3 tests/check-community-rendering.py
	python3 tests/check-typography-sizes.py
