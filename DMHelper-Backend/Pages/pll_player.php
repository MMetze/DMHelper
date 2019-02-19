<?php

	session_start();
	include_once "src/includes.php";
	include_once "config.inc.php";
	include_once "src/class_message.php";

  # connect to db
	$dbc= new db();
	$pf= $dbc->pre;

	# initialze POST vars
	$user= isset($_POST["user"])?$_POST["user"]:NULL;
	$password= isset($_POST["password"])?$_POST["password"]:NULL;
	$session= isset($_POST["session"])?str_replace("-","",$_POST["session"]):NULL;
	$payload= isset($_POST["payload"])?$_POST["payload"]:NULL;
	$_SESSION["user"]= isset($_SESSION["user"])?$_SESSION["user"]:(object) array("ID" => -1, "username" => NULL );

	# Dunno what happens, but I have to define seperately
	if( !isset($_SESSIO["user"]->username) ) {
		$_SESSION["user"]->username= NULL;
	}

	$msg= new message('pl_pull');

	#check for active session
	if( !isset($_SESSION["logged_in"]) || $_SESSION["user"]->username != $user ) {
		session_unset();
		$_SESSION["logged_in"]= false;
		$_session["is_player"]= false;
	}


	if( $user==NULL || $password==NULL || $session==NULL ) {
		$msg->state= -1;
		$msg->error= "Received data incomplete";
		echo $msg;

		exit();
	}

  if( !$_SESSION["logged_in"] ) {
		# Verify User
		$dbc->query= "SELECT ID, pass FROM $pf"."user WHERE username=?;";
		$dbc->bind= "s";
		$dbc->prepare();
		$dbc->values= array( $user );
		$dbc->bind();
		$result= $dbc->execute();

		$user= $result->fetch_object();
		$_SESSION["user"]= $user;

		/*
		# Verify User<->Session
		$dbc->query= "SELECT owner FROM $pf"."Session WHERE ID=?;";
		$dbc->bind= "i";
		$dbc->prepare();
		$dbc->values= array( $session );
		$dbc->bind();
		$ssql= $dbc->execute();

		$owner= $ssql->fetch_object();
		# 2Do: matrix to allow more than one user the ownage of a session, right now its just 1 user.
		*/
		#player verification
		$_SESSION["is_player"]= true;
	}
	# if everything is in order, ...
	if( password_verify($password, $_SESSION["user"]->pass) && $_SESSION["is_player"] )  {

		# get DM Payload from DB
		$_SESSION["logged_in"]= true;
		$dbc->query= "SELECT payload, last FROM $pf"."session WHERE ID=?;";
		$dbc->bind= "s";
		$dbc->prepare();
		$dbc->values= array( hex2bin($session) );
		$dbc->bind();
		$result= $dbc->execute();

		$dm_payload= $result->fetch_object();
		#echo serialize($payload);
		#echo "<br />";
		if( $result->num_rows > 0 ) {
			$msg->data= $dm_payload;
		} else {
			$msg->error= "No session available";
		}

		# if set, push PlayerPayload to DB
		if( $payload ) {
			$dbc->query="UPDATE $pf"."vusersession SET payload=? WHERE session=? AND user=? LIMIT 1;";
			$dbc->bind= "ssi";
			$dbc->prepare();
			$dbc->values= array( $payload, hex2bin($session), $_SESSION["user"]->ID );
			$dbc->bind();
			$result= $dbc->execute();
			$msg->mode= 'pl_pull_push';
		}
	} else {
		$msg->error= "User or session error";
		session_unset();
	}
	echo $msg;

?>
