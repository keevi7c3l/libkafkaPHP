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
  | Author:  iliaojie@gmail.com                                          |
  +----------------------------------------------------------------------+
*/

/* $Id$ */
extern "C" {
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"	
#include <zend_exceptions.h>
// #ifdef PHP_SESSION
// #include "ext/session/php_session.h"
// ps_module ps_mod_libkafkas;
// #endif
}
#include "common.h"
#include "php_libkafkas.h"
#include "libkafkas_sock.h"

#include <iostream>
#include <string>
#include <cstring>
#include <libkafka/Message.h>
#include <libkafka/produce/ProduceRequest.h>
using namespace std;
using std::string;
using namespace LibKafka;

/*defined structure in the union*/
zend_class_entry *kafka_ce;
zend_class_entry *kafka_exception_ce;
zend_class_entry *spl_ce_RuntimeException = NULL;

/* True global resources - no need for thread safety here */
int le_libkafkas;

/* If you declare any globals in php_libkafkas.h uncomment this:*/
ZEND_DECLARE_MODULE_GLOBALS(libkafkas)

/*defined libkafkas class function*/
static zend_function_entry libkafkas_functions[] = {
	PHP_ME(KafkaPHP, __construct, NULL, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
  PHP_ME(KafkaPHP, connect, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(KafkaPHP, pconnect, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(KafkaPHP, close, NULL, ZEND_ACC_PUBLIC)  
  PHP_ME(KafkaPHP, writeMsg, NULL, ZEND_ACC_PUBLIC)  
	{NULL, NULL, NULL}
};


/* {{{ libkafkas_module_entry */
zend_module_entry libkafkas_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"libkafkas",
	// libkafkas_functions,
	NULL,
	PHP_MINIT(libkafkas),
	PHP_MSHUTDOWN(libkafkas),
	PHP_RINIT(libkafkas),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(libkafkas),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(libkafkas),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_LIBKAFKAS
BEGIN_EXTERN_C()
ZEND_GET_MODULE(libkafkas)
END_EXTERN_C()
#endif
/*
  +----------------------------------------------------------------------+
  |                                                                      |
  +----------------------------------------------------------------------+
  |                                                                      |
  +----------------------------------------------------------------------+
*/

/* register key list*/
PHPAPI zend_class_entry *kafka_get_exception_base(int root TSRMLS_DC)
{
#if HAVE_SPL
  if (!root) {
    if (!spl_ce_RuntimeException) {
      zend_class_entry **pce;
      if (zend_hash_find(CG(class_table), "runtimeexception",sizeof("RuntimeException"), (void **) &pce) == SUCCESS) {
        spl_ce_RuntimeException = *pce;
        return *pce;
      }
    } else {
      return spl_ce_RuntimeException;
    }
  }
#endif
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 2)
  return zend_exception_get_default();
#else
  return zend_exception_get_default(TSRMLS_C);
#endif
}

/**
 * kafka_destructor
 */
static void kafka_destructor(zend_rsrc_list_entry * rsrc TSRMLS_DC)
{
    KFKSock *kafka_sock = (KFKSock *) rsrc->ptr;
    // kafka_sock_disconnect(kafka_sock TSRMLS_CC);
    kafka_free_socket(kafka_sock);
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(libkafkas)
{
	zend_class_entry kafka_class_entry;
	zend_class_entry kafka_exception_class_entry;
	    
	/* register KafkaPHP class */
	INIT_CLASS_ENTRY(kafka_class_entry, "KafkaPHP", libkafkas_functions);
  kafka_ce = zend_register_internal_class(&kafka_class_entry TSRMLS_CC);

	/* Exception class */
  INIT_CLASS_ENTRY(kafka_exception_class_entry, "KafkaPHPException", NULL);
  kafka_exception_ce = zend_register_internal_class_ex(&kafka_exception_class_entry, kafka_get_exception_base(0 TSRMLS_CC), NULL TSRMLS_CC );

  /* register Process number */
  le_libkafkas = zend_register_list_destructors_ex( kafka_destructor,  NULL, le_name, module_number);

  // #ifdef PHP_SESSION
  // php_session_register_module(&ps_mod_libkafkas);
  // #endif
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(libkafkas)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(libkafkas)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(libkafkas)
{

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(libkafkas)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "libkafkas support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/*
  +----------------------------------------------------------------------+
  |                                                                      |
  +----------------------------------------------------------------------+
  |                                                                      |
  +----------------------------------------------------------------------+
*/

/**
 * get socket
 */
PHPAPI int getSock(zval *id, KFKSock **kafka_sock TSRMLS_DC, int isThrow)
{
  zval **socket;
  int resource_type;

  if (Z_TYPE_P(id) != IS_OBJECT || zend_hash_find(Z_OBJPROP_P(id), "socketKFK", sizeof("socketKFK"), (void **) &socket) == FAILURE) {
    std::cout <<"getSock::2:: sock this process list Does not exist,Throw an exception" <<std::endl;
      if(!isThrow) {
        zend_throw_exception(kafka_exception_ce, "sock this process list Does not exist", 0 TSRMLS_CC);
      }
      return -1;
  }

  *kafka_sock = (KFKSock *) zend_list_find(Z_LVAL_PP(socket), &resource_type);
  if (!*kafka_sock || resource_type != le_libkafkas) {
    std::cout <<"getSock::3::not sock,Throw an exception" <<std::endl;
    if(!isThrow) {
      zend_throw_exception(kafka_exception_ce, "sock server Does not exist", 0 TSRMLS_CC);
    }
    return -1;
  }

  if ((*kafka_sock)->lazy_connect)
  {
      (*kafka_sock)->lazy_connect = 0;
      if (kafka_sock_server_open(*kafka_sock, 1 TSRMLS_CC) < 0) {
        return -1;
      }
  }
  return Z_LVAL_PP(socket);
}



/* {{{ proto KafkaPHP KafkaPHP::__construct()
    Public constructor */
PHP_METHOD(KafkaPHP,__construct)
{
	// std::cout <<"n1::" <<std::endl;
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
      RETURN_FALSE;
  }
}
/* }}} */

/* {{{ proto KafkaPHP KafkaPHP::__destruct()
    Public Destructor
 */
PHP_METHOD(KafkaPHP,__destruct) {
  if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    RETURN_FALSE;
  }

  // Grab our socket
  KFKSock *kafka_sock;
  if (getSock(getThis(), &kafka_sock TSRMLS_CC, 1) < 0) {
    RETURN_FALSE;
  }

  // If we think we're in MULTI mode, send a discard
  if(kafka_sock->mode == MULTI) {
    // Discard any multi commands, and free any callbacks that have been queued
    // send_discard_static(kafka_sock TSRMLS_CC);
    // free_reply_callbacks(getThis(), kafka_sock);
  }
}



/* {{{ proto boolean KafkaPHP::connect(string host, int port [, double timeout [, long retry_interval]])
 */
PHP_METHOD(KafkaPHP, connect)
{
	// std::cout <<"c1::" <<std::endl;
	if (connectKFK(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0) == FAILURE) {
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ proto boolean KafkaPHP::pconnect(string host, int port [, double timeout])
 */
PHP_METHOD(KafkaPHP, pconnect)
{
  // std::cout <<"p1::" <<std::endl;
	if (connectKFK(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1) == FAILURE) {
		RETURN_FALSE;
	} else {    
	 	// reset multi/exec state if there is one. 
	 	KFKSock *kafka_sock;
	 	if (getSock(getThis(), &kafka_sock TSRMLS_CC, 0) < 0) {      
	 		RETURN_FALSE;
	 	}
    RETURN_TRUE;
	}
}
/* }}} */

PHPAPI int connectKFK(INTERNAL_FUNCTION_PARAMETERS, int persistent) {
  zval *object;
  zval **socket;

  char *host = NULL;
  int host_len, id;  
  long port;
  double timeout = 0.0;
  char *persistent_id = NULL;//重要标识，POOL
  int persistent_id_len = -1;
  

  KFKSock *kafka_sock;

  #ifdef ZTS
    persistent = 0;
  #endif
    //////,&retry_interval,&persistent_id, &persistent_id_len
  if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|ldsl", &object, kafka_ce, &host, &host_len, &port, &timeout,&persistent_id, &persistent_id_len) == FAILURE) {
    return FAILURE;
  }

  if (timeout < 0L || timeout > INT_MAX) {
    zend_throw_exception(kafka_exception_ce, "Invalid timeout", 0 TSRMLS_CC);
    return FAILURE;
  }

  if(port == 0 && host_len && host[0] != '/') { /* not unix socket, set to default value */
    port = 9092;
  }
  // std::cout <<"connectKFK::1"  <<std::endl;

  if (getSock(object, &kafka_sock TSRMLS_CC, 1) > 0) {
    if (zend_hash_find(Z_OBJPROP_P(object), "socketKFK",sizeof("socketKFK"), (void **) &socket) == FAILURE)
    {
      //maybe there is a socket but the id isn't known.. what to do?
    } else {
      zend_list_delete(Z_LVAL_PP(socket)); // the refcount should be decreased and the detructor called 
    }
  }

  kafka_sock = kafka_sock_create(host, host_len, port, timeout, persistent,  persistent_id, 0);
  if (kafka_sock_server_open(kafka_sock, 1 TSRMLS_CC) < 0) {
    kafka_free_socket(kafka_sock);
    return FAILURE;
  }

#if PHP_VERSION_ID >= 50400
  id = zend_list_insert(kafka_sock, le_libkafkas TSRMLS_CC);
#else
  id = zend_list_insert(kafka_sock, le_libkafkas);
#endif
  add_property_resource(object, "socketKFK", id);
  return SUCCESS;
}


/* {{{ proto boolean KafkaPHP::close()
 */
PHP_METHOD(KafkaPHP, close)
{
  // std::cout <<"c1::" <<std::endl;  
  zval *object;
  KFKSock *kafka_sock = NULL;

  if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O",&object, kafka_ce) == FAILURE) {
      RETURN_FALSE;
  }

  if (getSock(object, &kafka_sock TSRMLS_CC, 0) < 0) {
      RETURN_FALSE;
  }

  if (kafka_sock_disconnect(kafka_sock TSRMLS_CC)) {
      RETURN_TRUE;
  }

  RETURN_FALSE;
}
/* }}} */

Message* createMessage(const char * value, const char *key = "");
ProduceRequest* createProduceRequest(string topic_name, Message **messageArray, int messageArraySize);
/* {{{ proto boolean KafkaPHP::writeMsg(string topicName, string msg)
 */
PHP_METHOD(KafkaPHP, writeMsg)
{
  // std::cout <<"w1::" <<std::endl;
  zval *object;

  char *topicName = NULL;
  char *msg = NULL;
  int argc = ZEND_NUM_ARGS();
  int topicName_len;
  int msg_len;
  KFKSock *kafka_sock = NULL;

  if (zend_parse_parameters(argc TSRMLS_CC, "ss", &topicName, &topicName_len, &msg, &msg_len) == FAILURE) {
    RETURN_FALSE;
  }
  
  /* get socket */
  if (getSock(getThis(), &kafka_sock TSRMLS_CC, 0) < 0) {
      RETURN_FALSE;
  }

  Message *m = createMessage(msg);
  Message **messageArray = &m;

  ProduceRequest *request = createProduceRequest(topicName, messageArray, 1);  
  unsigned char *buffers = request->toWireFormat();

  int numBytesSent = kafka_sock_write(kafka_sock, buffers,request->size() TSRMLS_CC);

  if (numBytesSent>-1)
  {
    RETURN_TRUE;
  }
  RETURN_FALSE;
  
}
/* }}} */

vector<Message*> messageVector;
MessageSet* messageSet;
ProduceMessageSet** produceMessageSetArray;
TopicNameBlock<ProduceMessageSet>** produceTopicArray;

Message* createMessage(const char * value, const char *key)
{
  // these will be updated as the message is prepared for production
  const static int crc = 1001;
  const static signed char magicByte = -1;
  const static signed char attributes = 0; // last three bits must be zero to disable gzip compression

  unsigned char *v = new unsigned char[strlen(value)];
  memcpy(v, value, strlen(value));

  unsigned char *k = new unsigned char[strlen(key)];
  memcpy(k, key, strlen(key));

  return new Message(crc, magicByte, attributes, strlen(key), (unsigned char *)k, strlen(value), (unsigned char *)v, 0, true);
}

MessageSet* createMessageSet(Message **messageArray, int messageArraySize)
{
  int messageSetSize = 0;
  messageVector.clear();

  for (int i = 0 ; i < messageArraySize ; i++)
  {
    messageVector.push_back(messageArray[i]);
    // sizeof(offset) + sizeof(messageSize) + messageSize
    messageSetSize += sizeof(long int) + sizeof(int) + messageArray[i]->getWireFormatSize(false);
  }

  return new MessageSet(messageSetSize, messageVector, true);
}

ProduceMessageSet* createProduceMessageSet(Message **messageArray, int messageArraySize)
{
  messageSet = createMessageSet(messageArray, messageArraySize);
  int messageSetSize = messageSet->getWireFormatSize(false);
  // using partition = 0
  return new ProduceMessageSet(0, messageSetSize, messageSet, true);
}

TopicNameBlock<ProduceMessageSet>* createProduceRequestTopicNameBlock(string topic_name, Message **messageArray, int messageArraySize)
{
  const int produceMessageSetArraySize = 1;

  produceMessageSetArray = new ProduceMessageSet*[produceMessageSetArraySize];
  for (int i=0; i<produceMessageSetArraySize; i++) {
    produceMessageSetArray[i] = createProduceMessageSet(messageArray, messageArraySize);
  }
  return new TopicNameBlock<ProduceMessageSet>(topic_name, produceMessageSetArraySize, produceMessageSetArray, true);
}

ProduceRequest* createProduceRequest(string topic_name, Message **messageArray, int messageArraySize)
{
  const int correlationId = 212121;
  const string clientId = string("libkafka-test");
  const static int requiredAcks = 1;
  const static int timeout = 20;

  int produceTopicArraySize = 1;
  produceTopicArray = new TopicNameBlock<ProduceMessageSet>*[produceTopicArraySize];
  for (int i=0; i<produceTopicArraySize; i++) {
    produceTopicArray[i] = createProduceRequestTopicNameBlock(topic_name, messageArray, messageArraySize);
  }
  return new ProduceRequest(correlationId, clientId, requiredAcks, timeout, produceTopicArraySize, produceTopicArray, true);
}

// PHPAPI int combinationMsg(INTERNAL_FUNCTION_PARAMETERS,const char *key) {
//   std::cout <<"combinationMsg::1::" <<std::endl;
//   char *topicName = NULL;
//   char *msg = NULL;
//   int argc = ZEND_NUM_ARGS();
//   int topicName_len;
//   int msg_len;
//   if (zend_parse_parameters(argc TSRMLS_CC, "ss", &topicName, &topicName_len, &msg, &msg_len) == FAILURE) {
//     return -1;
//   }else{
//     const static int crc = 1001;
//     const static signed char magicByte = -1;
//     const static signed char attributes = 0; // last three bits must be zero to disable gzip compression

//     unsigned char *v = new unsigned char[strlen(msg)];
//     memcpy(v, msg, strlen(msg)); 
//     std::cout <<"combinationMsg::2::" <<std::endl;
//     unsigned char *k = new unsigned char[strlen(key)];
//     memcpy(k, key, strlen(key));
//     std::cout <<"combinationMsg::3::" <<std::endl;
//     Message *newMsg = new Message(crc, magicByte, attributes, strlen(key), (unsigned char *)k, strlen(msg), (unsigned char *)v, 0, true);
//     Message **messageArray = &newMsg;

//     ProduceRequest *request = createProduceRequest(topicName, messageArray, 1);
//     unsigned char *buffers = request->toWireFormat();

//     KFKSock *kafka_sock = NULL;
//     /* get socket */
//     if (getSock(getThis(), &kafka_sock TSRMLS_CC, 0) < 0) {
//         return -1;
//     }

//     int numBytesSent = kafka_sock_write(kafka_sock, buffers, request->size() TSRMLS_CC);
//     return numBytesSent;
//   }
// }

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
