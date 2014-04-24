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
$jsonStr =array();
$jsonStr[] = array ("type"=>"Error","level"=>9,"host"=>"http://localhost:9090","msgContent"=>"Two pure-php implementations of the json protocol that might be of use, e.g. for php 4 installs where adding extensions is not an option or because of extr");
$jsonStr[] = array ("type"=>"Warning","level"=>9,"host"=>"http://localhost:9090","msgContent"=>"Two pure-php implementations of the json protocol that might be of use, e.g. for php 4 installs where adding extensions is not an option or because of extr");
$jsonStr[] = array ("type"=>"Info","level"=>9,"host"=>"http://localhost:9090","msgContent"=>"Two pure-php implementations of the json protocol that might be of use, e.g. for php 4 installs where adding extensions is not an option or because of extr");
$jsonStr[] = array ("type"=>"debug","level"=>9,"host"=>"http://localhost:9090","msgContent"=>"Two pure-php implementations of the json protocol that might be of use, e.g. for php 4 installs where adding extensions is not an option or because of extr");
$jsonStr[] = array ("type"=>"message","level"=>1,"host"=>"http://localhost:9090","msgContent"=>"Two pure-php implementations of the json protocol that might be of use, e.g. for php 4 installs where adding extensions is not an option or because of extr");
$jsonStr[] = array ("type"=>"viewinfo","level"=>1,"host"=>"http://localhost:9090","msgContent"=>"Two pure-php implementations of the json protocol that might be of use, e.g. for php 4 installs where adding extensions is not an option or because of extr");


$number = rand(0,5);
var_dump($gKafka->writeMsg("test",json_encode($jsonStr[$number])));

?>
