#ifndef LIBKAFKAS_COMMON_H
#define LIBKAFKAS_COMMON_H

extern "C" {
#include "php.h"
#include "php_ini.h"
#include <ext/standard/php_smart_str.h>
}

#define le_name "Kafka Socket"
#define KAFKA_SOCK_STATUS_FAILED 0
#define KAFKA_SOCK_STATUS_DISCONNECTED 1
#define KAFKA_SOCK_STATUS_UNKNOWN 2
#define KAFKA_SOCK_STATUS_CONNECTED 3

// /* serializers */
#define KAFKA_SERIALIZER_NONE       0
#define KAFKA_SERIALIZER_PHP        1
#define KAFKA_SERIALIZER_IGBINARY   2

// /* SCAN options */
#define KAFKA_SCAN_NORETRY 0
#define KAFKA_SCAN_RETRY 1

typedef enum {ATOMIC, MULTI, PIPELINE} kafka_mode;

/* {{{ struct KafkaSock */
typedef struct {
    php_stream     *stream;//
    char           *host;//
    short          port;   //
    double         timeout;//
    zend_bool      lazy_connect;//
    int            status;//
    int            persistent;//
    char           *persistent_id; //
    double         read_timeout;//
    int            serializer;//
    kafka_mode     mode;//        
} KFKSock;
/* }}} */

#endif
