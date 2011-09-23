#! /bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

olddir=`pwd`
cd $srcdir



INTLTOOLIZE=`which intltoolize`
if test -z $INTLTOOLIZE; then
        echo "*** No intltoolize found ***"
        exit 1
else
        intltoolize --force --copy --automake || exit $?
fi

GTKDOCIZE=`which gtkdocize`
if test -z $GTKDOCIZE; then
        echo "*** No gtk-doc support ***"
        echo "EXTRA_DIST =" > gtk-doc.make
        echo "CLEANFILES =" >> gtk-doc.make
else
        gtkdocize || exit $?
        sed -e 's#) --mode=compile#) --tag=CC --mode=compile#' gtk-doc.make \
          > gtk-doc.temp \
                && mv gtk-doc.temp gtk-doc.make
        sed -e 's#) --mode=link#) --tag=CC --mode=link#' gtk-doc.make \
          > gtk-doc.temp \
                && mv gtk-doc.temp gtk-doc.make
fi

AUTORECONF=`which autoreconf`
if test -z $AUTORECONF; then
        echo "*** No autoreconf found ***"
        exit 1
else
        ACLOCAL="${ACLOCAL-aclocal} $ACLOCAL_FLAGS" autoreconf -v --install || exit $?
fi

cd $olddir

$srcdir/configure --disable-static --enable-maintainer-flags "$@" && \
  echo "Now type 'make' to compile $PROJECT."
