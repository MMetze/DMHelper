<?php
#
#  Message Class to generate standard XML response for DMHelper and DMHelper mysql_client_encoding
#  Version
#  0.1.0 -> initial release
#  0.1.1 -> Added action 17 File exists
#

include_once "ext_XMLSerialzer.php";

class message {

  private
    $version,
    $timestamp,
    $mode,
    $state,
    $error;

  public
    $data;

	function __construct( $mode=NULL ) {
    $this->version= "0.1.2";
    $this->setMode($mode);
    $this->state= 0;
	}

	function __destruct() {
		return;
	}

	function __get( $prop ) {
		return $this->$prop;
	}

  function __toString() {
    $this->timestamp= date( 'Y-m-d H:i:s' );
    $vars= get_object_vars($this);
    $array= array();
    foreach( $vars AS $key => $value ) {
      $array[$key]= $value;
    }
    return XMLSerializer::generateValidXmlFromArray($array);
  }

	function __set( $name, $value ) { # set name or code
		if( property_exists( $this, $name ) ) {
			switch( $name ) {
				case "mode":
          $this->setMode($value);
          break;

        case "state":
          $this->state= $value;
          break;

        case "error":
          $this->state= -1;
          $this->error= $value;
          break;

        case "data":
          $this->data= $value;
          break;
			}
		}
	}

  private function setMode( $value ) {
    switch( $value ) {
      case "dm_push":
        $this->mode["int"]= 1;
        $this->mode["text"]= "DM Push";
        break;

      case "dm_push_pull":
        $this->mode["int"]= 2;
        $this->mode["text"]= "DM Push with Pull";
        break;

      case "pl_pull":
        $this->mode["int"]= 3;
        $this->mode["text"]= "Player Pull";
        break;

      case "pl_pull_push":
        $this->mode["int"]= 4;
        $this->mode["text"]= "Player Pull with Push";
        break;

      case "ssn_ass":
        $this->mode["int"]= 5;
        $this->mode["text"]= "Session assignment";
        break;

      case "ssn_create":
        $this->mode["int"]= 6;
        $this->mode["text"]= "Create session";
        break;

      case "ssn_isowner":
        $this->mode["int"]= 7;
        $this->mode["text"]= "Is session owner";
        break;

      case "ssn_rename":
        $this->mode["int"]= 8;
        $this->mode["text"]= "Rename session";
        break;

      case "ssn_remove":
        $this->mode["int"]= 9;
        $this->mode["text"]= "Remove session";
        break;

      case "ssn_renew":
        $this->mode["int"]= 10;
        $this->mode["text"]= "Renew session code";
        break;

      case "ssn_close":
        $this->mode["int"]= 11;
        $this->mode["text"]= "Close session";
        break;

      case "ssn_members":
        $this->mode["int"]= 12;
        $this->mode["text"]= "Memberlist";
        break;

      case "ssn_general":
        $this->mode["int"]= 13;
        $this->mode["text"]= "Session general";
        break;

      case "file_push":
        $this->mode["int"]= 14;
        $this->mode["text"]= "File push";
        break;

      case "file_pull":
        $this->mode["int"]= 15;
        $this->mode["text"]= "File pull";
        break;

      case "file_list":
        $this->mode["int"]= 16;
        $this->mode["text"]= "File list";
        break;

      case "file_exists":
        $this->mode["int"]= 17;
        $this->mode["text"]= "File exists";
        break;

    }
  }
}

?>
