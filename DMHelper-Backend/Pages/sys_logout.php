<?php
	include_once "config.inc.php";

	session_start();
?>

<?php
	unset($_SESSION["uid"]);
	unset($_SESSION["user"]);
	unset($_SESSION);
	header("location: index.php?view=login");
?>
