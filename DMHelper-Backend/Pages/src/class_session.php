<?php

include_once "xt_UUID.php";

class session {
	private $dbc;

	private $ID;
	private $IDh;
	private $owner;
	private $name;
	private $payload;
	private $last;
	private $code;

	private $members;

	private $pf;

	function __construct( &$dbc= NULL) {
		if( isset($dbc) ) {
			$this->dbc= $dbc;
		} else {
			global $dbc;
			$this->dbc= &$dbc;
		}

		$this->pf= $this->dbc->pre;
	}

	function __destruct() {
		return;
	}

	function __get( $prop ) {
		return $this->$prop;
	}

	function __set( $name, $value ) { # set name or code
		if( property_exists( $this, $name ) ) {
			switch( $name ) {
				case "name":
					$this->dbc->query= "UPDATE $this->pf"."session SET name=? WHERE ID=? LIMIT 1;";
					$this->dbc->bind= "ss";
					$this->dbc->prepare();
					$this->dbc->values= array( $value, $this->ID );
					$this->dbc->bind();
					$this->dbc->execute();

					if( $this->dbc->db->affected_rows > 0 ) {
						$this->name= $value;
					}
					break;

				case "code":
					if( $value ) {
						$this->createCode();
					} elseif ( !$value ) {
						$this->removeCode();
					}
			}
		}
	}

	private function object2class( $row ) {	# iterate through object and store in members
		foreach($row as $key => $value) {
    	if( property_exists( $this, $key ) ) {
				$this->$key= $value;
			}
		}
	}

	private function createCode() { # create invite code and store in Database
		$this->removeCode();

		$codeok= false;
		while( !$codeok ) {
			$code= randomASCII();
			$this->dbc->query= "SELECT 1 FROM $this->pf"."vsessioncode WHERE code=? LIMIT 1;";
			$this->dbc->bind= "s";
			$this->dbc->prepare();
			$this->dbc->values= array( $code );
			$this->dbc->bind();
			$this->dbc->execute();
			if( $this->dbc->result->num_rows==0 ) {
				$codeok= true;
				$this->dbc->query= "INSERT INTO $this->pf"."vsessioncode ( code, sessionid ) VALUES ( ?, ? );";
				$this->dbc->bind= "ss";
				$this->dbc->prepare();
				$this->dbc->values= array ( $code, $this->ID );
				$this->dbc->bind();
				$this->dbc->execute();
			}
		}
		$this->code= $code;
	}

	private function removeCode() { # delete invite code from database
		$this->dbc->query= "DELETE FROM $this->pf"."vsessioncode WHERE sessionid=?;";
		$this->dbc->bind= "s";
		$this->dbc->prepare();
		$this->dbc->values= array( $this->ID );
		$this->dbc->bind();
		$this->dbc->execute();
	}

	public function get( $session ) { # get session by hex ID and store in instance
		# get session info from database
		$this->dbc->query= "SELECT ID, IDh, owner, name, payload, last FROM $this->pf"."session WHERE ID=? LIMIT 1;";
		$this->dbc->bind= "s";
		$this->dbc->prepare();
		$this->dbc->values= array( hex2bin($session) );
		$this->dbc->bind();
		$result= $this->dbc->execute();
		$retval= $result->num_rows;
		$row= $result->fetch_object();

		# store session info in instance
		$this->object2class( $row );

		# get players for sessionid
		$this->dbc->query= "SELECT username, surname, forename FROM $this->pf"."vusersession AS vUS LEFT JOIN $this->pf"."user AS U ON vUS.user=U.ID WHERE vUS.session=? AND vUS.user != ?;";
		$this->dbc->bind= "ss";
		$this->dbc->prepare();
		$this->dbc->values= array ( $this->ID, $this->owner );
		$this->dbc->bind();
		$result= $this->dbc->execute();

		while( $row= $result->fetch_object() ) {
			$this->members[]= $row;
		}

		# return success / fail
		if( $retval>0 ) {
			return true;
		} else {
			return false;
		}
	}

	public function create($user, $name) { # create session and invite code
		$this->ID= myUUID::v4();
		$this->IDh= myUUID::hex2uuid($this->ID);

		$this->dbc->query= "INSERT INTO $this->pf"."session ( ID, owner, name ) VALUES ( ?, ?, ? );";
		$this->dbc->bind= "sss";
		$this->dbc->prepare();
		$this->dbc->values= array( hex2bin($this->ID), $user, $name );
		$this->dbc->bind();

		$this->dbc->execute();

		$this->createCode();

		return $this->IDh;
	}

	public function remove() { # remove session from all tables
		if( myUUID::is_valid($this->IDh ) ) {
			$this->dbc->query= "DELETE FROM $this->pf"."vsessioncode WHERE sessionid=?;";
			$this->dbc->bind= "s";
			$this->dbc->prepare();
			$this->dbc->values= array( $this->ID );
			$this->dbc->bind();
			$this->dbc->execute();

			$this->dbc->query= "DELETE FROM $this->pf"."vusersession WHERE session=?;";
			$this->dbc->prepare();
			$this->dbc->bind();
			$this->dbc->execute();

			$this->dbc->query= "DELETE FROM $this->pf"."session WHERE ID=?;";
			$this->dbc->prepare();
			$this->dbc->bind();
			$this->dbc->execute();
		}
	}
}

?>
