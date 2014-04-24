/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_LIBKAFKAS_H
#define PHP_LIBKAFKAS_H

extern zend_module_entry libkafkas_module_entry;
#define phpext_libkafkas_ptr &libkafkas_module_entry

#ifdef PHP_WIN32
#	define PHP_LIBKAFKAS_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_LIBKAFKAS_API __attribute__ ((visibility("default")))
#else
#	define PHP_LIBKAFKAS_API
#endif
extern "C" {
#ifdef ZTS
#include "TSRM.h"
#endif
}
// #include <libkafka/Message.h>
// #include <libkafka/produce/ProduceRequest.h>

PHP_MINIT_FUNCTION(libkafkas);
PHP_MSHUTDOWN_FUNCTION(libkafkas);
PHP_RINIT_FUNCTION(libkafkas);
PHP_RSHUTDOWN_FUNCTION(libkafkas);
PHP_MINFO_FUNCTION(libkafkas);

PHPAPI int connectKFK(INTERNAL_FUNCTION_PARAMETERS, int persistent);

PHP_METHOD(KafkaPHP, __construct);
PHP_METHOD(KafkaPHP, connect);
PHP_METHOD(KafkaPHP, pconnect);
PHP_METHOD(KafkaPHP, close);
PHP_METHOD(KafkaPHP, writeMsg);


#ifndef _MSC_VER
ZEND_BEGIN_MODULE_GLOBALS(libkafkas)
ZEND_END_MODULE_GLOBALS(libkafkas)
#endif

#ifdef ZTS
#define LIBKAFKAS_G(v) TSRMG(libkafkas_globals_id, zend_libkafkas_globals *, v)
#else
#define LIBKAFKAS_G(v) (libkafkas_globals.v)
#endif

#endif	/* PHP_LIBKAFKAS_H */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
