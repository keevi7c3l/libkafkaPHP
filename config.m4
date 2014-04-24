dnl $Id$

PHP_ARG_ENABLE(libkafkas, whether to enable libkafkas support,
dnl Make sure that the comment is aligned:
[  --enable-libkafkas           Enable libkafkas support])

if test "$PHP_LIBKAFKAS" != "no"; then
  PHP_SUBST(LIBKAFKAS_SHARED_LIBADD)
  PHP_REQUIRE_CXX()
  PHP_ADD_LIBRARY(stdc++, "", LIBKAFKAS_SHARED_LIBADD)
  EXTRA_LDFLAGS="$EXTRA_LDFLAGS -lz -lsnappy -lkafka"  
  PHP_NEW_EXTENSION(libkafkas, libkafkas.cpp libkafkas_sock.cpp, $ext_shared)
  dnl PHP_NEW_EXTENSION(libkafkas, libkafkas.cpp libkafkas_action.cpp libkafkas_sock.cpp, $ext_shared)
fi
