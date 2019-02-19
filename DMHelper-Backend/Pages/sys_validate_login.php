<?php

	if(!isset($_SESSION["uid"]) || bin2hex($_SESSION["uid"])==0 ) {
				header("location: ?view=login");
				exit();
	} else {
		# check if user has right to log in
		$dbc->query= "SELECT 1 FROM $pf"."user WHERE ID=? AND disabled IS NULL LIMIT 1;";
		$dbc->bind= "s";
		$dbc->prepare();
		$dbc->values= array( $_SESSION["uid"]);
		$dbc->bind();
		$result= $dbc->execute();

		#log out user
		if( $result->num_rows==0 ) {
			$_SESSION["uid"]= 0;
			header("location: index.php");
		}
	}

?>
