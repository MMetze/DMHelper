<?php
	session_start();
	include_once "src/includes.php";
	include_once "config.inc.php";
	include_once "src/class_message.php";

  # connect to db
	$dbc= new db();
	$pf= $dbc->pre;

	# initialize POST vars
	$user= isset($_POST["user"])?$_POST["user"]:NULL;
	$password= isset($_POST["password"])?$_POST["password"]:NULL;
	$session= isset($_POST["session"])?str_replace("-","",$_POST["session"]):NULL;
	$payload= isset($_POST["payload"])?$_POST["payload"]:NULL;
	$fetch= isset($_POST["fetch"])?$_POST["fetch"]:false;
	$_SESSION["user"]= isset($_SESSION["user"])?$_SESSION["user"]:(object) array("ID" => -1, "username" => NULL );

	if( $fetch=="true" ) {
		$fetch= true;
	} else {
		$fetch= false;
	}

	$msg= new message("dm_push");

	# Dunno what happens, but I have to define seperately
	if( !isset($_SESSIO["user"]->username) ) {
		$_SESSION["user"]->username= NULL;
	}

	# check for active session
	if( !isset($_SESSION["logged_in"]) || $_SESSION["user"]->username != $user ) {
		session_unset();
		$_SESSION["logged_in"]= false;
	}

	if( $user==NULL || $password==NULL || $session==NULL || $payload==NULL ) {
		$msg->state= -1;
		$msg->error= "Received data incomplete";
		#$retval= array(  "status" => "error", "error" => "received data incomplete" );
		#echo XMLSerializer::GenerateValidXmlFromArray($retval);
		echo $msg;
		exit();
	}

  if( !$_SESSION["logged_in"] ) {
		$_SESSION["owner"]= false;

		# Verify User
		$dbc->query= "SELECT ID, pass, username FROM $pf"."user WHERE username=?;";
		$dbc->bind= "s";
		$dbc->prepare();
		$dbc->values= array( $user );
		$dbc->bind();
		$result= $dbc->execute();

		$user= $result->fetch_object();
		$_SESSION["user"]= $user;

		# Verify User<->Session
		$dbc->query= "SELECT owner AS ID FROM $pf"."session WHERE ID=?;";
		$dbc->bind= "s";
		$dbc->prepare();
		$dbc->values= array( hex2bin($session) );
		$dbc->bind();
		$ssql= $dbc->execute();

		if( $ssql->num_rows > 0 ){
			$owner= $ssql->fetch_object();
			$_SESSION["owner"]= $owner->ID;
		}
		# 2Do: matrix to allow more than one user the ownage of a session, right now its just 1 user.

	}

	# if everything is in order, ...
	if( password_verify($password, $_SESSION["user"]->pass) && $_SESSION["owner"]==$_SESSION["user"]->ID ) {
		# send payload to database
		$_SESSION["logged_in"]= true;
		$dbc->query= "UPDATE $pf"."session SET payload=?, last=NOW() WHERE ID=?;";
		$dbc->bind= "ss";
		$dbc->prepare();
		$dbc->values= array( $payload, hex2bin($session) );
		$dbc->bind();
		$dbc->execute();

		# fetch player payloads
		if( $fetch ) {
			$dbc->query=" SELECT u.username, vUS.time, vUS.payload FROM $pf"."vusersession AS vUS JOIN $pf"."user AS u ON vUS.user=u.ID WHERE session=?;";
			$dbc->bind= "s";
			$dbc->prepare();
			$dbc->values= array( hex2bin($session) );
			$dbc->bind();
			$result= $dbc->execute();
			while( $row= $result->fetch_object() ) {
				$payloads[]= array( "username" => $row->username, "time" => $row->time, "payload" => $row->payload );
			}

			if( isset($payloads) ) {
				$msg->data= $payloads;
				echo $msg;
				#echo XMLSerializer::generateValidXmlFromArray($payloads, 'player payloads', 'payload');
			}

		}
	} else {
		$msg->state= -1;
		$msg->error= "No data found";
		echo $msg;

		session_unset();
	}

?>
