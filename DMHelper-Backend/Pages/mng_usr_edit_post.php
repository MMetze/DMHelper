<?php
	session_start();
	include_once "src/includes.php";
	include_once "config.inc.php";

	$dbc= new db();
	$pf= $dbc->pre;
	#iterate_form();
	#exit();

	include_once "sys_validate_login.php";
?>

<?php
foreach( $_POST as $key => $value ) {
	$a= $key;
	if( !is_array($value) ) {
		$$a= sanitizeString($value);
		if( strlen($$a)==0 ) {
			$$a= NULL;
		}
	}
}
$userid= hex2bin( str_replace("-","",$userid) );

	# add existing user catch
	$dbc->query= "SELECT ID FROM $pf"."user WHERE username LIKE ? LIMIT 1;";
	$dbc->bind= "s";
	$dbc->prepare();
	$dbc->values= array( $username );
	$dbc->bind();
	$result= $dbc->execute();

	if( $result->num_rows == 1 ) {
		// $dbc->query= "UPDATE $pf"."user SET username=? WHERE ID=?;";
		// $dbc->bind= "ss";
		// $dbc->prepare();
		// $dbc->values= array( $username, hex2bin($userid) );
		// $dbc->bind();
		// $dbc->execute();


		$dbc->query= "UPDATE $pf"."user SET username=?, email=?, surname=?, forename=?, ";
		$dbc->bind= "ssss";
		if( isset($password1) ) {
			$dbc->query.= "pass=?, ";
			$dbc->bind.= "s";
			$dbc->values= array( $username, $email , $surname, $forename, password_hash( $password1, PASSWORD_DEFAULT ), $disabled, $userid );
		} else {
			$dbc->values= array( $username, $email , $surname, $forename, $disabled, $userid );
		}
		$dbc->query.= "disabled=? WHERE ID=?;";
		$dbc->bind.= "ss";
		$dbc->prepare();
		$dbc->bind();

		$dbc->execute();
	}
	header("location: index.php?view=mngusr");
	exit();
?>
