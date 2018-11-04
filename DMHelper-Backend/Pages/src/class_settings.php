<?php

class settings {
	private $dbc;

	function __construct($dbc) {

		$this->dbc= $dbc;
		$query= "SELECT * FROM " . $this->dbc->pre . "settings;";
		$result= $this->dbc->myQuery($query);

		while( $row= mysqli_fetch_array($result) ) {
			$_SESSION[$row[0]]= $row[1];
		}
	}

	function __destruct() {
		return;
	}

	public function delete($key) {
		$query= "DELETE FROM " . $this->dbc->pre . "settings WHERE setting='$key';";
		$this->dbc->myQuery($query);
	}

	public function get($key) {
		$query= "SELECT value FROM " . $this->dbc->pre . "settings WHERE setting='$key';";
		$result= $this->dbc->myQuery($query);

		$row= mysqli_fetch_object($result);
		return $row->value;
	}

	public function set($key, $value) {
		$key= sanitizeString($key);
		$value= sanitizeString($value);

		if( isset($_SESSION[$key]) ) {
			$query= "UPDATE " . $this->dbc->pre . "settings SET value='$value' WHERE setting='$key';";
		} else {
			$query= "INSERT INTO " . $this->dbc->pre . "settings ( setting, value ) VALUES ( '$key', '$value' );";
		}

		$this->dbc->myQuery($query);
	}

}

?>
