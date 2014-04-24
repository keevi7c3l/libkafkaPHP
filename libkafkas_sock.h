/* $Id$ */
#ifndef LIBKAFKAS_SOCK_H
#define LIBKAFKAS_SOCK_H
#include "common.h"


PHPAPI KFKSock* kafka_sock_create(char *host, int host_len, unsigned short port, double timeout, int persistent,  char *persistent_id, zend_bool lazy_connect);
PHPAPI int kafka_sock_connect(KFKSock *kafka_sock TSRMLS_DC);
PHPAPI int kafka_sock_server_open(KFKSock *kafka_sock, int force_connect TSRMLS_DC);
PHPAPI int kafka_sock_disconnect(KFKSock *kafka_sock TSRMLS_DC);
PHPAPI int kafka_sock_write(KFKSock *kafka_sock,unsigned char* buffer, size_t sz TSRMLS_DC);
// PHPAPI char * kafka_sock_read(KFKSock *kafka_sock, int *buf_len TSRMLS_DC);
PHPAPI void kafka_stream_close(KFKSock *kafka_sock TSRMLS_DC);
PHPAPI int kafka_check_eof(KFKSock *kafka_sock TSRMLS_DC);
PHPAPI void kafka_free_socket(KFKSock *kafka_sock );
#endif /* LIBKAFKAS_SOCK_H */