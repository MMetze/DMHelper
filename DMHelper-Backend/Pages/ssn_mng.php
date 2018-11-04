<?php

/*
		Management interface for sessions

		ssn_mng.php
		POST Vars
		user -> str, req
		password -> str, req
		session -> str, UUID, opt
		action -> str, req
		name -> str, opt

		returns XML
		<status> -> tells if last action was successful or not
		<error> -> in case of error, holds the kind of error
		- action create (creates new session)
		  <session> -> Session UUID
			<code> -> invite code for players
		- action isowner (checks if user is owner of session)
		  <owner> -> true/false (1, NULL)
		- action rename (renames session)
		  <session> -> Session UUID
			<renamed> -> new Session namespace
		- action remove (deletes session)
			<session> -> Session UUID
			<removed> -> 'ok' on success
		- action renew (creates new invite code, invalidates old one)
			<session> -> Session UUID
			<code> -> invite code
		- action close (invalidates invite codes, does not create a new one)
			<session> -> Session UUID
		- action member (returns members of session)
			<session> -> Session UUID
			<members> -> <data>* -> 	<username>
														  	<surname>
														  	<forename>

*/
	session_start();
	include_once "src/includes.php";
	include_once "config.inc.php";
	include_once "src/class_session.php";
	include_once "src/class_message.php";

  # connect to db
	$dbc= new db();
	$pf= $dbc->pre;

	# initialize POST vars
	$user= $_POST["user"]??NULL;
	$password= $_POST["password"]??NULL;
	$session= isset($_POST["session"])?str_replace("-","",$_POST["session"]):NULL;
	$action= $_POST["action"]??NULL;
	$name= $_POST["name"]??NULL;

	$msg= new message("ssn_general");

	$_SESSION["user"]= isset($_SESSION["user"])?$_SESSION["user"]:(object) array("ID" => -1, "username" => NULL );


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
		if( myUUID::is_valid($session) ) {
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
		}
		# 2Do: matrix to allow more than one user the ownage of a session, right now its just 1 user.

	}

	# if everything is in order, ...
	if( password_verify($password, $_SESSION["user"]->pass) ) {

		$ssn= new session();
		if( myUUID::is_valid($session) ) {
			if( !$ssn->get( $session ) ) {
				$msg->error= "Could not load session";
			}
		}

		switch( $action ) {
			case "create": # create Session
				$msg->mode= "ssn_create";
				$ssn->create( $_SESSION["user"]->ID, $name );
				if( myUUID::is_valid($ssn->IDh) ) {
					$msg->data["session"]= $ssn->IDh;
					$msg->data["code"]= $ssn->code;
				} else {
					$values= array ( "status" => "error", "error" => "could not create UUID" );
					$msg->error= "Could not create UUID";
				}
				break;

			case "isowner": # check if user is owner
				$msg->mode= "ssn_isowner";
				$msg->data["session"]= $ssn->IDh;
				$msg->data["user"]= myUUID::hex2uuid(bin2hex($_SESSION["user"]->ID));
				if( $_SESSION["user"]->ID==$ssn->owner ) {
					$msg->data["isowner"]= TRUE;
				} else {
					$msg->data["isowner"]= FALSE;
				}
				break;

			case "rename": # rename session
				$msg->mode= "ssn_rename";
				if( $_SESSION["user"]->ID==$ssn->owner ) {
					$ssn->name= $name;
					$msg->data= $ssn->name;
				} else {
					$msg->error= "Could not rename session";
				}
				break;

			case "remove": # remove session
			$msg->mode= "ssn_remove";
				if( $_SESSION["user"]->ID==$ssn->owner ) {
					$ssn->remove();
					$msg->data["session"]= $ssn->IDh;
					$msg->data["action"]= "removed";
				} else {
					$msg->error= "Could not delete session";
				}
				break;

			case "renew": # renew invite code
				$msg->mode= "ssn_renew";
				if( $_SESSION["user"]->ID==$ssn->owner ) {
					$ssn->code= true;
					$msg->data["session"]= $ssn->IDh;
					$msg->data["code"]= $ssn->code;
				} else {
					$msg->error= "Could not renew code";
				}
				break;

			case "close": # disable session join
				if( $_SESSION["user"]->ID==$ssn->owner ) {
					$ssn->code= false;
					$msg->data["session"]= $ssn->IDh;
					$msg->data["code"]= FALSE;
				} else {
					$msg->error= "Could not close session";
				}
				break;

			case "member": # get member array for session
				$msg->mode= "ssn_members";
				if( $_SESSION["user"]->ID==$ssn->owner ) {
					$msg->data["session"]= $ssn->IDh;
					$msg->data["members"]= $ssn->members;
					break;
				}

			}

		echo $msg;
		exit();
	}

?>
