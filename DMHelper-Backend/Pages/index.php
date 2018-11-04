<?php
	# global execution
	session_start();
	include_once "src/includes.php";
	include_once "config.inc.php";

	# Database
	$dbc= new db();
	$pf= $dbc->pre;

	# check if user is logged in
	$view= isset($_GET["view"])?sanitizeString($_GET["view"]):NULL;
	$_SESSION["uid"]= isset($_SESSION["uid"])?$_SESSION["uid"]:NULL;

	if( $view!="login" ) {
		include_once "sys_validate_login.php";
	}

	# settings
	$settings= new settings($dbc);

	$dbc->myQuery("SET NAMES 'utf8'");

	if( $_SESSION["uid"] && ( !isset($_SESSION["user"]) || $_SESSION["uid"]!=$_SESSION["user"]->ID ) ) {
		$query= "SELECT * FROM $pf"."user where ID=?;";
		$dbc->query= $query;
		$dbc->bind= "s";
		$dbc->prepare();
		$dbc->values= array( $_SESSION["uid"] );
		$dbc->bind();
		$result= $dbc->execute();

		$_SESSION["user"]= $result->fetch_object();

	}

/*# Set Password
	$query= "UPDATE $pf"."User SET pass='" . password_hash('12345Start', PASSWORD_DEFAULT) . "' WHERE ID=?;";
	$dbc->query= $query;
	$dbc->bind= "i";
	$dbc->prepare();
	$dbc->values= array( 1 );
	$dbc->bind();
	$dbc->execute(); */




?>
<!DOCTYPE HTML>
<html>
<head>
	<title><?php echo $_SESSION["version"]; ?></title>
</head>

<link rel="stylesheet" type="text/css" href="src/style.css">

<link rel="stylesheet" href="src/jQuery/jquery-ui.min.css">
<script src="src/jQuery/external/jquery/jquery.js"></script>
<script src="src/jQuery/jquery-ui.min.js"></script>
<script>
	 $( function() {
		 $( "#tabs1" ).tabs();
		 $( "#tabs2" ).tabs();
		 $( "#tabs3" ).tabs();
		 $( "#tabs4" ).tabs();
		 $( "#tabs5" ).tabs();
	 } );
 </script>
<meta http-http-equiv="content-type" content="text/html; charset=UTF-8" /></meta>

<body>

<div id="Navigation">
	Platzalter Nav<br />
	<a href="?view=mngusr">Benutzerverwaltung</a><br />
	<a href="?view=login"><?php if(isset($_SESSION["uid"])) echo "logout"; else echo "login"; ?></a><br />

	<a href="test_poll.php">Test Poll</a><br />
	<a href="test_send.php">Test Send</a><br />
	<a href="test_file_push.php">Test File Push</a><br />
	<a href="test_file_pull.php">Test File Pull</a><br />
	<a href="test_file_list_pull.php">Test Filelist Pull</a><br />
	<a href="test_ssn_ass.php">Session Link</a><br />
	<a href="test_ssn_mng.php">Session mng</a><br />
	<a href="?view=file&action=list">Dateiliste</a><br />

	<a href="dbv/">Database Version</a>

</div>

<div id="Header">
	<?php echo $_SESSION["version"]; ?><br />

	<?php
	 	if( $_SESSION["uid"] ) {
			echo $_SESSION["user"]->username . " ist angemeldet.<br />";
		}
	?>
</div>



<?php


	switch( $view ) {
		case "login":
			include "sys_login.php";
		break;

		case "mngusr":
			include "mng_usr.php";
		break;

		case "file":
			if( $_GET["action"]=='list' )
				include "file_list.php";
		break;
	}
?>

<div id="scrollforce" style="position: absolute; top: 120%;">Scroll enabler</div>
</body>

</html>
