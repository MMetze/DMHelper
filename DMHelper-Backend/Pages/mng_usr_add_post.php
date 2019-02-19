<?php
	session_start();
	include_once "src/includes.php";
	include_once "config.inc.php";

	$dbc= new db();
	$pf= $dbc->pre;
#	iterate_form();
	
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

	$dbc->query= "SELECT ID FROM $pf"."user WHERE username LIKE ? OR email LIKE ? LIMIT 1;";
	$dbc->bind= "ss";
	$dbc->prepare();
	$dbc->values= array( $username, $email );
	$dbc->bind();
	$result= $dbc->execute();
	
	if( $result->num_rows > 0 ) {
		header("location: index.php?view=mngusr&alert=Name oder eMail bereits vergeben#tab-new");
	} else {
		$uuid= myUUID::v4();
		$dbc->query= "INSERT INTO $pf"."user ( ID, IDh, username, email, surname, forename, pass ) VALUES ( ?, ?, ?, ?, ?, ?, ? );";
		$dbc->bind= "sssssss";
		$dbc->prepare();
		$dbc->values= array( hex2bin($uuid), $uuid, $username, $email, $surname, $forename, password_hash( $password1, PASSWORD_DEFAULT ) );
		$dbc->bind();
		$dbc->execute();
	}
	header("location: index.php?view=mngusr");
	exit();
?>
