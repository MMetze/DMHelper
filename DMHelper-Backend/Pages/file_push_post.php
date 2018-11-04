<?php

  # Saves File on server in SQL
	# POST Data:
	# user -> string (USERNAME)
	# password -> string (PASSWORD)
	# myFile[] -> array with files, multiple files are allowed. Standard HTML Post file type
	# returns XML with md5 hashes and filenames for every successfully uploaded files.
	#
	# Files already in the database (md5 check) will not be stored again (for the same user).
	#
	# Limitation: Filesize cannot exceed MEDIUMBLOB (16MB). PHP and SQL has to be configured to accept larger files.

  # debug mode
	$debug= isset($_GET["debug"])?$_GET["debug"]:false;
	if( $debug=='1' ) {
		echo "Debug: <br />";
		echo "<pre>" . var_dump($_POST) . "</pre><br />";
		echo "<pre>" . var_dump($_FILES) . "</pre><br />";
		exit();
	}

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

	# check for active session
	if( !isset($_SESSION["logged_in"]) || $_SESSION["user"]->ID != $user ) {
		session_unset();
		$_SESSION["logged_in"]= false;
	}

	$msg= new message("file_push");

	if( $user==NULL || $password==NULL || $_FILES==NULL ) {
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

	if( password_verify($password, $_SESSION["user"]->pass) ) {
		$_SESSION["logged_in"]= true;
		if( strtolower( $_SERVER[ 'REQUEST_METHOD' ] ) == 'post' && !empty( $_FILES ) ) {

	#		if( $$_SERVER['DOCUMENT_ROOT']=="c:/xampp/htdocs" ) {
	#			$uploads_dir= 'C:/xampp/Git/DM-Helper-Backend/Pages/files';
	$uploads_dir= str_replace( pathinfo(__FILE__, PATHINFO_FILENAME) . ".php", "", __FILE__) . "files/";
	#		} else {
	#			$uploads_dir= "/files";
	#		}

			# check if file is already in DB
			$dbc->query= "SELECT ID FROM $pf"."file WHERE ID=? AND user=? LIMIT 1;";
			$dbc->bind= "ss";
			$dbc->prepare();

			foreach( $_FILES["myFile"]["error"] as $key => $error ) {
				if( $error==UPLOAD_ERR_OK ) {
				$md5= md5_file( $_FILES["myFile"]["tmp_name"][$key], TRUE );

				$dbc->values= array( $md5, $_SESSION["user"]->ID );
				$dbc->bind();
				$result= $dbc->execute();

					if( $result->num_rows>0 ) {
						$msg->data[]= array( "name" => $_FILES["myFile"]["name"][$key], "id" => md5_file( $_FILES["myFile"]["tmp_name"][$key] ) );
						unset($_FILES["myFile"]["error"]["$key"]);
					} else {
						$_FILES["myFile"]["md5"][$key]= $md5;
					}
				}
			}


			# Store binary data in database
			$dbc->query= "INSERT INTO $pf"."file ( ID, user, name, time, data ) VALUES ( ?, ?, ?, NOW(), ? );";
			$dbc->bind= "sssb";
			$dbc->prepare();
			foreach ($_FILES["myFile"]["error"] as $key => $error) {
				$null= NULL; # placeholder to bind for binary
				if ($error == UPLOAD_ERR_OK) {
				$tmp_name = $_FILES["myFile"]["tmp_name"][$key];
				$name = basename($_FILES["myFile"]["name"][$key]);
				$md5= md5_file($tmp_name, TRUE);
				$md5str= md5_file($tmp_name);

				# open and read file
				$handle= fopen( $tmp_name, "rb" );
				if( isset($handle) ) {
					$data= fread($handle, filesize($tmp_name));
				}

				if( !is_null($data) ) {
					$dbc->values= array( $md5, $_SESSION["user"]->ID, $name, $null );
					$dbc->bind();
					# send $data to placeholder
					$dbc->send_long_data(3, $data );
					$dbc->execute();
				}
				$msg->data[]= array( "name" => $name, "id" => $md5str );
				#move_uploaded_file($tmp_name, $uploads_dir. $_SESSION["user"]->ID ."-".$name );

				}
			}


	if( isset($msg->data) )
		echo $msg;

	}
}


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
