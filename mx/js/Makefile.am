mainjsdir = $(datadir)/mx-2.0/js/
dist_mainjs_DATA = \
	cssEditor.js \
	cssEditor.css \
	$(NULL)

BUILT_SOURCES = cssEditorPaths.js

do_subst = sed -e 's|@abs_top_srcdir[@]|$(abs_top_srcdir)|g' \
               -e 's|@abs_top_builddir[@]|$(abs_top_builddir)|g' \
               -e 's|@localedir[@]|$(localedir)|g' \
               -e 's|@bindir[@]|$(bindir)|g' \
               -e 's|@libdir[@]|$(libdir)|g' \
               -e 's|@datadir[@]|$(datadir)|g' \
               -e 's|@libexecdir[@]|$(libexecdir)|g' \
               -e 's|@pkglibdir[@]|$(pkglibdir)|g' \
               -e 's|@pkgdatadir[@]|$(pkgdatadir)|g'

%: %.in
	$(AM_V_GEN) $(do_subst) $(srcdir)/$< > $@

mainjs_DATA = \
	$(BUILT_SOURCES) \
	$(NULL)

CLEANFILES = \
	$(BUILT_SOURCES)

EXTRA_DIST = \
	cssEditorPaths.js.in
