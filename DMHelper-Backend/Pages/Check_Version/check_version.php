<?php
# Define class with functions for logging

include_once "src/ext_XMLSerialzer.php";
include_once "src/ext_UUID.php";

class version {
  public $major;
  public $minor;
  public $notes;

  public $uuid;

  public function __construct() {
    $this->uuid= isset($_POST["UUID"])?$_POST["UUID"]:NULL;
    if( !UUID::is_valid($this->uuid) ) {
      $this->uuid= NULL;
    }
  }

  public function create_answer() {
    echo XMLSerializer::generateValidXmlFromArray( (array)$this );
    return;
  }

  public function log() {
    $fp= fopen('checks.csv','a');
    $log= date('Y-m-d H:i:s') . ";" . $this->uuid . ";\n";
    fwrite( $fp, $log );

    fclose( $fp );
  }
}

$query= new version();

$query->major= "1.6";
$query->minor= "1.6.4";
$query->notes= "Initial Release of DMHelper v 1.6\\nMARKUP";

$query->create_answer();
$query->log();

?>
