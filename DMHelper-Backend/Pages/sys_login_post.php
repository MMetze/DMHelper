<?php
	include_once "src/includes.php";
	include_once "config.inc.php";

	session_start();
?>

<?php
	$dbc= new db();
	$username= sanitizeString($_POST["username"]);
	$password= $_POST["password"];

	$query= "SELECT ID, username, pass FROM $dbc->pre"."user WHERE username=? AND disabled IS NULL LIMIT 1;";
	$dbc->query= $query;
	$dbc->bind= "s";
	$dbc->prepare();
	$dbc->values= array( $username );
	$dbc->bind();
	$result= $dbc->execute();


	$row= mysqli_fetch_object($result);

	if(mysqli_num_rows($result) == 1 && ( password_verify("$password", $row->pass) ) ) {
		$_SESSION["uid"]= $row->ID;

		$dbc->query= "UPDATE $dbc->pre"."user SET lastlogin=NOW() WHERE ID=?;";
		$dbc->bind= "s";
		$dbc->prepare();
		$dbc->values= array( $row->ID );
		$dbc->bind();
		$dbc->execute();

		header("location: index.php");
		exit();
	} else {
		echo "Benutzername und/oder Passwort falsch.<br />";
		echo 'Zur&uuml;ck zum <a href="index.php">Login</a><br />';
	}
?>
