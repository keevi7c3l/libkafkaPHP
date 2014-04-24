<?php
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
'port' => '9092',
);
$gKafka = new tpKafka();
$gKafka->connect( $config );
var_dump($gKafka->writeMsg("test","q1w2e3qqqqqq123456"));
?>
