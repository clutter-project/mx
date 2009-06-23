#! /bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

olddir=`pwd`
cd $srcdir

GTKDOCIZE=`which gtkdocize`
if test -z $GTKDOCIZE; then
        echo "*** No gtk-doc support ***"
        echo "EXTRA_DIST =" > gtk-doc.make
else
        gtkdocize || exit $?
        sed -e 's#) --mode=compile#) --tag=CC --mode=compile#' gtk-doc.make \
          > gtk-doc.temp \
                && mv gtk-doc.temp gtk-doc.make
        sed -e 's#) --mode=link#) --tag=CC --mode=link#' gtk-doc.make \
          > gtk-doc.temp \
                && mv gtk-doc.temp gtk-doc.make
fi

GLIB_GETTEXTIZE=`which glib-gettextize`
if test -z $GLIB_GETTEXTIZE; then
        echo "*** No glib-gettextize ***"
        exit 1
else
        glib-gettextize -f || exit $?
fi

AUTORECONF=`which autoreconf`
if test -z $AUTORECONF; then
        echo "*** No autoreconf found ***"
        exit 1
else
        autoreconf -v --install || exit $?
fi

cd $olddir

$srcdir/configure --disable-static "$@" && \
  echo "Now type 'make' to compile $PROJECT."
