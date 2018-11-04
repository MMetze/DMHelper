<?php

# Gets files from Server
# POST Data:
# user -> string (USERNAME)
# password -> string (PASSWORD)
# files[] -> array of md5 sums of files to get
# returns XML filenodes <md5><name><base64 encoded binary stream>
#

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
	$files= isset($_POST["files"])?$_POST["files"]:NULL;
	$_SESSION["user"]= isset($_SESSION["user"])?$_SESSION["user"]:(object) array("ID" => -1, "username" => NULL );

	# check for active session
	if( !isset($_SESSION["logged_in"]) || $_SESSION["user"]->ID != $user ) {
		session_unset();
		$_SESSION["logged_in"]= false;
	}

	$msg= new message("file_pull");

	if( $user==NULL || $password==NULL || $files==NULL) {
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

	}

	$data= new stdClass();
	if( password_verify($password, $_SESSION["user"]->pass) ) {
		$_SESSION["logged_in"]= true;
		if( strtolower( $_SERVER[ 'REQUEST_METHOD' ] ) == 'post' && !empty( $files ) ) {

			$dbc->query="SELECT ID, name, data FROM $pf"."file WHERE ID=? AND user=? LIMIT 1;";
			$dbc->bind= "ss";
			$dbc->prepare();

			foreach( $files as &$id ) {
				if( $id["md5"] ) {
					# Convert MD5 to binary
					$dbc->values= array( hex2bin($id["md5"]), $user->ID );
					$dbc->bind();

					# execute query
					$result= $dbc->execute();

					if( $result->num_rows>0 ) {
					# get file objects and store in array
						$file= $result->fetch_object();
						$file->ID= md5($file->data);
						$file->data= base64_encode($file->data);

						# encapsulate datastream in XML
						$msg->data[]= $file;
					}
				}
			}
		}
	}
	echo $msg;

/*
So, as a response to comments. The correct way of storing a 36-char UUID as binary(16) is to perform the insert in a manner like:

INSERT INTO sometable (UUID) VALUES (UNHEX(REPLACE("3f06af63-a93c-11e4-9797-00505690773f", "-","")))
UNHEX because an UUID is already a hexed value. We trim (REPLACE) the dashes in the statement to bring the length down to 32 ASCII characters (our 16 bytes represented as HEX). You can do this at any point before storing it, obviously, so it doesn't have to be handled by the database.

You may retrieve the UUID like this:

SELECT HEX(UUID) FROM sometable;
Just in case someone comes across this thread and is unsure how this works.

And remember: If you're selecting a row using the UUID, use UNHEX() on the condition:

SELECT * FROM sometable WHERE UUID = UNHEX('3f06af63a93c11e4979700505690773f');
And not HEX()on the column:

SELECT * FROM sometable WHERE HEX(UUID) = '3f06af63a93c11e4979700505690773f';
The second solution, while it works, requires that MySQL HEXes all UUIDs before it can determine which rows match. It's very inefficient.

*/
?>
