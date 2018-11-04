<?php
	session_start();
	include_once "src/includes.php";
	include_once "config.inc.php";
	include_once "src/ext_XMLSerialzer.php";
	include_once "src/class_session.php";

  # connect to db
	$dbc= new db();
	$pf= $dbc->pre;

$tmp= session::createInvite();

	# initialize POST vars
	$user= isset($_POST["user"])?$_POST["user"]:NULL;
	$password= isset($_POST["password"])?$_POST["password"]:NULL;
	$session= isset($_POST["session"])?str_replace("-","",$_POST["session"]):NULL;
	$action= isset($_POST["action"])?$_POST["action"]:NULL;

	$_SESSION["user"]= isset($_SESSION["user"])?$_SESSION["user"]:(object) array("ID" => -1, "username" => NULL );

	# check for active session
	if( !isset($_SESSION["logged_in"]) || $_SESSION["user"]->username != $user ) {
		session_unset();
		$_SESSION["logged_in"]= false;
	}

	if( $user==NULL || $password==NULL || $session==NULL || $action==NULL ) {
		$retval= array(  "status" => "error", "error" => "received data incomplete" );
		echo XMLSerializer::GenerateValidXmlFromArray($retval);
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


		# get invite code, if is set
		if( $action=="get" ) {
			$dbc->query= "SELECT code, created FROM $pf"."vsessioncode WHERE sessionid=? LIMIT 1;";
			$dbc->bind= "s";
			$dbc->prepare();
			$dbc->values= array( hex2bin($session) );
			$dbc->bind();
			$result= $dbc->execute();

			if( $result->num_rows>0 ) {
				$row= $result->fetch_object();
				$values= array( "session" => myUUID::hex2uuid($session), "code" => $row->code, "created" => $row->created );
			} else {
				$values= array( "status" => "error", "error" => "not a session id");
			}

		} else {
			$values= array( "status" => "error", "error" => "not a session id");
		}

		# delete any existing invite code and generate new
		if( $action=="create" ){
			$dbc->query= "DELETE FROM $pf"."vsessioncode WHERE sessionid=?;";
			$dbc->bind= "s";
			$dbc->prepare();
			$dbc->values= array( hex2bin($session) );
			$dbc->bind();
			$dbc->execute();

			$code= session::createInvite();
			$dbc->query= "INSERT INTO $pf"."vsessioncode( code, sessionid ) VALUES ( ?, ? );";
			$dbc->bind= "ss";
			$dbc->prepare();
			$dbc->values= array( $code, hex2bin($session) );
			$dbc->bind();
			$dbc->execute();

			$values= array( "session" => myUUID::hex2uuid($session), "code" => $code );
		}

		if( $action=="renew" ) {

			$code= session::createInvite();
				$dbc->query= "UPDATE $pf"."vsessioncode SET code=? WHERE sessionid=?;";
				$dbc->bind= "ss";
				$dbc->prepare();
				$dbc->values= array( $code, hex2bin($session) );
				$dbc->bind();
				$dbc->execute();

				$values= array( "session" => myUUID::hex2uuid($session), "code" => $code );
		}
	}

		echo XMLSerializer::generateValidXmlFromArray($values, 'session', 'data');
		exit();
?>
