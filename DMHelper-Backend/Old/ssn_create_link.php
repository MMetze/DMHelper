<?php
	session_start();
	include_once "src/includes.php";
	include_once "config.inc.php";
	include_once "src/ext_XMLSerialzer.php";

  # connect to db
	$dbc= new db();
	$pf= $dbc->pre;

	# initialize POST vars
	$user= isset($_POST["user"])?$_POST["user"]:NULL;
	$password= isset($_POST["password"])?$_POST["password"]:NULL;
	$session= isset($_POST["session"])?str_replace("-","",$_POST["session"]):NULL;
	$_SESSION["user"]= isset($_SESSION["user"])?$_SESSION["user"]:(object) array("ID" => -1, "username" => NULL );

	# check for active session
	if( !isset($_SESSION["logged_in"]) || $_SESSION["user"]->username != $user ) {
		session_unset();
		$_SESSION["logged_in"]= false;
	}

	if( $user==NULL || $password==NULL || $session==NULL ) {
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

		$dbc->query= "DELETE FROM $pf"."vsessioncode WHERE sessionid=?;";
		$dbc->bind= "s";
		$dbc->prepare();
		$dbc->values= array( hex2bin($session) );
		$dbc->bind();
		$dbc->execute();

		#$dbc->query= 'INSERT INTO dmh_vsessioncode ( code, session ) VALUES ( ' . randomASCII() . ', ?)';
		$codeok= false;
		while( !$codeok ) {
			$code= randomASCII();
			$dbc->query= "SELECT 1 FROM $pf"."vsessioncode WHERE code=? LIMIT 1;";
			$dbc->bind= "s";
			$dbc->prepare();
			$dbc->values= array( $code );
			$dbc->bind();
			$dbc->execute();
			if( $dbc->result->num_rows==0 ) {
				$codeok= true;
			}
		}

		$dbc->query= "INSERT INTO $pf"."vsessioncode( code, sessionid ) VALUES ( ?, ? );";
		$dbc->bind= "ss";
		$dbc->prepare();
		$dbc->values= array( $code, hex2bin($session) );
		$dbc->bind();
		$dbc->execute();

		$values= array( "session" => myUUID::hex2uuid($session), "code" => $code );

	} else {
		$values= array( "error" => "not a session id");
	}
	echo XMLSerializer::GenerateValidXmlFromArray($values);

?>
