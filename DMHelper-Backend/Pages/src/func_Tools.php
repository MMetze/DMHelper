<?php
  function hasRight( $area, $user ) {
    global $dbc;
    $pf= $dbc->pre;

  /*  if( isSessionAdmin() ) {
      return true;
    }*/
    if( $user ) {
      $user= sanitizeString($user);

      $query= "SELECT 1 FROM $pf"."varearights WHERE areaID=$area AND userID=$user;";
      $result= $dbc->myQuery($query);

      if( $result->num_rows > 0 ) {
        return true;
      }
    }
    return false;
  }

  # transform sql to german date
	function mydatetime($date) {
		if( $date == NULL )
			return NULL;

		return date("d.m.Y H:i:s", strtotime($date));
	}
  #############################################################################################
	## Print Form data
	## 																							#
	#############################################################################################
	function iterate_form()
	{
		echo "<b>Values:</b><br />";
		foreach( $_POST as $key => $value ) {
			if( !is_array($value) ) {
				echo $key . " = " . $value . "<br />";
			}
		}
	}
#############################################################################################
## Takes bytecount, returns human readable string
## 																							#
#############################################################################################
function human_filesize($bytes, $decimals = 2) {
  $size = array('B','kB','MB','GB','TB','PB','EB','ZB','YB');
  $factor = floor((strlen($bytes) - 1) / 3);
  return sprintf("%.{$decimals}f", $bytes / pow(1024, $factor)) . @$size[$factor];
}

#############################################################################################
## Returns random ASCII Code
## 																							#
#############################################################################################
function randomASCII( $count= 6 ) {
  return substr(str_shuffle(str_repeat("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", $count)), 0, $count);
  #
}
?>
