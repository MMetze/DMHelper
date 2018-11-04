<?php

/*
		Management interface for session vodes

		ssn_ass.php
		POST Vars
		user -> str, req
		password -> str, req
		code -> str, req
		action -> str, req

		returns XML
		<status> -> tells if last action was successful or not
		<error> -> in case of error, holds the kind of error
		- action join (join existing session using code)
		  <session> -> Session UUID
*/

	session_start();
	include_once "src/includes.php";
	include_once "config.inc.php";
	include_once "src/class_message.php";

  # connect to db
	$dbc= new db();
	$pf= $dbc->pre;

	# initialize POST vars
	$user= $_POST["user"]??NULL;
	$password= $_POST["password"]??NULL;
	$action= $_POST["action"]??NULL;
	$code= $_POST["code"]??NULL;

	$_SESSION["user"]= isset($_SESSION["user"])?$_SESSION["user"]:(object) array("ID" => -1, "username" => NULL );

	$msg= new message('ssn_ass');

	# check for active session
	if( !isset($_SESSION["logged_in"]) || $_SESSION["user"]->username != $user ) {
		session_unset();
		$_SESSION["logged_in"]= false;
	}

	if( $user==NULL || $password==NULL || $action==NULL ) {
		$msg->error= "Received data incomplete";
		echo $msg;
		exit();
	}

  if( !$_SESSION["logged_in"] ) {

		# Verify User
		$dbc->query= "SELECT ID, pass, username FROM $pf"."user WHERE username=?;";
		$dbc->bind= "s";
		$dbc->prepare();
		$dbc->values= array( $user );
		$dbc->bind();
		$result= $dbc->execute();

		$user= $result->fetch_object();
		$_SESSION["user"]= $user;

	}

	# if everything is in order, ...
	if( password_verify($password, $_SESSION["user"]->pass) ) {

		switch( $action ) {

			case "join":
				$dbc->query= "SELECT sessionid FROM $pf"."vsessioncode WHERE code=?;";
				$dbc->bind= "s";
				$dbc->prepare();
				$dbc->values= array( $code );
				$dbc->bind();
				$result= $dbc->execute();

				if( $result->num_rows==1 ) {
					$row=$result->fetch_object();
					$dbc->query= "DELETE FROM $pf"."vusersession WHERE session=? AND user=?;";
					$dbc->bind= "ss";
					$dbc->values= array( $row->sessionid, $_SESSION["user"]->ID );
					$dbc->prepare();
					$dbc->bind();
					$dbc->execute();

					$dbc->query= "INSERT INTO $pf"."vusersession ( session, user ) VALUES ( ? ,? );";
					$dbc->prepare();
					$dbc->bind();
					$dbc->execute();
					$retval= $dbc->db->affected_rows;

					if( $retval ) {
						$msg->data= myUUID::hex2uuid(bin2hex($row->sessionid));
					} else {
						$msg->error= "Error joining session";
					}
				} else {
					$msg->error= "Invalid code";
				}
				break;

			}

		echo $msg;
		exit();
	}

?>
