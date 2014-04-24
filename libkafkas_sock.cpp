/* $Id$ */
extern "C" {
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_network.h"
#include <ext/standard/php_smart_str.h>
#include <ext/standard/php_var.h>
#include <zend_exceptions.h>
#include <ext/standard/php_math.h>
}
#include <sys/types.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include "common.h"
#include "php_libkafkas.h"
#include "libkafkas_sock.h"

#include <iostream>
#include <string>
#include <cstring>
using namespace std;
using std::string;
#define UNSERIALIZE_ONLY_VALUES 0
#define UNSERIALIZE_ALL 1
extern zend_class_entry *kafka_ce;
extern zend_class_entry *kafka_exception_ce;
extern zend_class_entry *spl_ce_RuntimeException;


PHPAPI void kafka_stream_close(KFKSock *kafka_sock TSRMLS_DC) {
	// std::cout <<"kafka_stream_close::1::" <<std::endl;
	if (!kafka_sock->persistent) {
		php_stream_close(kafka_sock->stream);
	} else {
		php_stream_pclose(kafka_sock->stream);
	}
}

PHPAPI int kafka_check_eof(KFKSock *kafka_sock TSRMLS_DC)
{
 // std::cout <<"kafka_check_eof::1::" <<std::endl;
  int eof;
  int count = 0;

  if (!kafka_sock->stream) {
    return -1;
  }

  eof = php_stream_eof(kafka_sock->stream);
  for (; eof; count++) {
    if((MULTI == kafka_sock->mode) ||count == 10) { /* too many failures */
      if(kafka_sock->stream) { /* close stream if still here */
        kafka_stream_close(kafka_sock TSRMLS_CC);
        kafka_sock->stream = NULL;
        kafka_sock->mode   = ATOMIC;
        kafka_sock->status = KAFKA_SOCK_STATUS_FAILED;
      }
      zend_throw_exception(kafka_exception_ce, "Connection lost", 0 TSRMLS_CC);
     return -1;
    }

    if(kafka_sock->stream) { /* close existing stream before reconnecting */
      kafka_stream_close(kafka_sock TSRMLS_CC);
      kafka_sock->stream = NULL;
      kafka_sock->mode   = ATOMIC;
    }
    
    // Wait for a while before trying to reconnect
    kafka_sock_connect(kafka_sock TSRMLS_CC); /* reconnect */
    if(kafka_sock->stream) { /*  check for EOF again. */
      eof = php_stream_eof(kafka_sock->stream);
    }
  }
    return 0;
}

/**
 * kafka_sock_create
 */
PHPAPI KFKSock* kafka_sock_create(char *host, int host_len, unsigned short port,double timeout, int persistent, char *persistent_id, zend_bool lazy_connect)
{
  // std::cout <<"kafka_sock_create::1::" <<std::endl;  
  KFKSock *kafka_sock;
  // kafka_sock         = ecalloc(1, sizeof(KFKSock));
  kafka_sock = static_cast<KFKSock*>(calloc(1,sizeof(KFKSock)));
        
  kafka_sock->stream = NULL;
  kafka_sock->host   = estrndup(host, host_len);
  kafka_sock->port    = port;
  kafka_sock->timeout = timeout;
  kafka_sock->lazy_connect = lazy_connect;
  kafka_sock->status = KAFKA_SOCK_STATUS_DISCONNECTED;
  kafka_sock->persistent = persistent;

  if(persistent_id) {
  	size_t persistent_id_len = strlen(persistent_id);
  	// kafka_sock->persistent_id = ecalloc(persistent_id_len + 1, 1);
  	kafka_sock->persistent_id = static_cast<char*>(calloc(persistent_id_len + 1,1));
  	memcpy(kafka_sock->persistent_id, persistent_id, persistent_id_len);
  } else { 	
  	kafka_sock->persistent_id = NULL;
  }

  memcpy(kafka_sock->host, host, host_len);
  kafka_sock->host[host_len] = '\0';  
  kafka_sock->read_timeout = timeout; 
  kafka_sock->serializer = KAFKA_SERIALIZER_NONE;  
  kafka_sock->mode = ATOMIC;
  return kafka_sock;
}

// /**
//  * kafka_sock_connect
//  */
PHPAPI int kafka_sock_connect(KFKSock *kafka_sock TSRMLS_DC)
{
  struct timeval tv, read_tv, *tv_ptr = NULL;
  char *host = NULL, *persistent_id = NULL, *errstr = NULL;
  int host_len, err = 0;
  int tcp_flag = 1;

  php_netstream_data_t *sock;  

  if (kafka_sock->stream != NULL) {
    kafka_sock_disconnect(kafka_sock TSRMLS_CC);
  }

  tv.tv_sec  = (time_t)kafka_sock->timeout;
  tv.tv_usec = (int)((kafka_sock->timeout - tv.tv_sec) * 1000000);
  if(tv.tv_sec != 0 || tv.tv_usec != 0) {
  	tv_ptr = &tv;
  }

  read_tv.tv_sec  = (time_t)kafka_sock->read_timeout;
  read_tv.tv_usec = (int)((kafka_sock->read_timeout - read_tv.tv_sec) * 1000000);
    
  if(kafka_sock->host[0] == '/' && kafka_sock->port < 1) {
    host_len = spprintf(&host, 0, "unix://%s", kafka_sock->host);
  } else {
    if(kafka_sock->port == 0)
    	kafka_sock->port = 9092;
    host_len = spprintf(&host, 0, "%s:%d", kafka_sock->host, kafka_sock->port);
  }

  if (kafka_sock->persistent) {
    if (kafka_sock->persistent_id) {
      spprintf(&persistent_id, 0, "KFK:%s:%s", host, kafka_sock->persistent_id);
    } else {
      spprintf(&persistent_id, 0, "KFK:%s:%f", host, kafka_sock->timeout);
    }
  }  
    
  kafka_sock->stream = php_stream_xport_create(host, host_len, ENFORCE_SAFE_MODE,STREAM_XPORT_CLIENT| STREAM_XPORT_CONNECT,persistent_id, tv_ptr, NULL, &errstr, &err);
  if (persistent_id) {
    efree(persistent_id);
  }
  efree(host);
  if (!kafka_sock->stream) {
  	efree(errstr);
  	return -1;
  }

  /* set TCP_NODELAY */
  sock = (php_netstream_data_t*)kafka_sock->stream->abstract;
  setsockopt(sock->socket, IPPROTO_TCP, TCP_NODELAY, (char *) &tcp_flag, sizeof(int));
  php_stream_auto_cleanup(kafka_sock->stream);

  if(tv.tv_sec != 0 || tv.tv_usec != 0) {
  	php_stream_set_option(kafka_sock->stream, PHP_STREAM_OPTION_READ_TIMEOUT,0, &read_tv);
  }
  php_stream_set_option(kafka_sock->stream,PHP_STREAM_OPTION_WRITE_BUFFER,PHP_STREAM_BUFFER_NONE, NULL);
  kafka_sock->status = KAFKA_SOCK_STATUS_CONNECTED;
  return 0;
}



/**
 * kafka_sock_server_open  
 */
PHPAPI int kafka_sock_server_open(KFKSock *kafka_sock, int force_connect TSRMLS_DC)
{
  int res = -1;
  switch (kafka_sock->status) {
	case KAFKA_SOCK_STATUS_DISCONNECTED:
	  return kafka_sock_connect(kafka_sock TSRMLS_CC);
	case KAFKA_SOCK_STATUS_CONNECTED:
		res = 0;
		break;
    case KAFKA_SOCK_STATUS_UNKNOWN:
      if (force_connect > 0 && kafka_sock_connect(kafka_sock TSRMLS_CC) < 0) {
      	res = -1;
      } else {
      	res = 0;
      	kafka_sock->status = KAFKA_SOCK_STATUS_CONNECTED;
      }
      break;
    }
    return res;
}

// /**
//  *  kafka_sock_disconnect
//  */
PHPAPI int kafka_sock_disconnect(KFKSock *kafka_sock TSRMLS_DC)
{
  // std::cout <<"kafka_sock_disconnect::1" <<std::endl;
  if (kafka_sock == NULL) {
  	return 1;
  }

  if (kafka_sock->stream != NULL) {
  	if (!kafka_sock->persistent) {
  		/*Notification Server, let me disconnect*/
  		// kafka_sock_write(kafka_sock, "i need disconnect", sizeof("i need disconnect") - 1 TSRMLS_CC);
  	}
  	kafka_sock->status = KAFKA_SOCK_STATUS_DISCONNECTED;
  	if(kafka_sock->stream && !kafka_sock->persistent) { /* still valid after the write */
	  php_stream_close(kafka_sock->stream);
	}
	kafka_sock->stream = NULL;
	return 1;
  }
  return 0;
}

/**
 *  sock_write
 */
PHPAPI int kafka_sock_write(KFKSock *kafka_sock,unsigned char* buffer, size_t sz TSRMLS_DC)
{
	// std::cout <<"kafka_sock_write::1::" <<std::endl;
	if(kafka_sock && kafka_sock->status == KAFKA_SOCK_STATUS_DISCONNECTED) {
		zend_throw_exception(kafka_exception_ce, "Connection closed", 0 TSRMLS_CC);
		return -1;
	}

	if(-1 == kafka_check_eof(kafka_sock TSRMLS_CC)) {
    return -1;
  }
  char sendMsg[sz]; 
  memcpy(sendMsg,buffer,sz); 
  return php_stream_write(kafka_sock->stream, sendMsg,sz);
}

// // /**
// //  *  kafka_free_socket
// //  */
PHPAPI void kafka_free_socket(KFKSock *kafka_sock)
{
	// std::cout <<"kafka_free_socket::1::" <<std::endl;
  if(kafka_sock->persistent_id) {
    efree(kafka_sock->persistent_id);
  }
  efree(kafka_sock->host);
  efree(kafka_sock);
}

