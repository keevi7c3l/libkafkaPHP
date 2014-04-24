libkafkaPHP
===========

kafka PHP API. 

Dependencies
============
 adobe-research/libkafka
 
 https://github.com/adobe-research/libkafka
Installation
============
```
phpize
./configure --with-php-config=/usr/local/php/bin/php-config LDFLAGS=-L/usr/local/lib
make
make install
```
Examples
========
```<?php
class tpKafka extends KafkaPHP
{
	public function __construct( )
	{
	}
	public function connect( $config )
	{
		parent::pconnect( $config['host'], $config['port'] );
	}
}


$config = array (
'host' => '192.168.20.200',
'port' => '9093',
);
$gKafka = new tpKafka();
$gKafka->connect( $config );
var_dump($gKafka->writeMsg("test","q1w2e3qqqqqq123456"));
?>
```
