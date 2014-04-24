#!/bin/bash
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin
export PATH
phpize --clean
phpize
./configure --with-php-config=/usr/local/php/bin/php-config LDFLAGS=-L/usr/local/lib
make
make install
#cp .libs/libkafka.so /usr/local/php/lib/php/extensions/no-debug-non-zts-20090626/
/etc/init.d/php-fpm restart

phpize --clean