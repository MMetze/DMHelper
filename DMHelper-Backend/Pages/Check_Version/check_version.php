<?php

#include XMLSerializer and UUID generator class (links inside)
include_once "src/ext_XMLSerialzer.php";
include_once "src/ext_UUID.php";

#create class for data
class version {
  public $major;
  public $minor;
  public $notes;

  public $uuid;

# constructor, fetches UUID from POST and checks for validity
  public function __construct() {
    $this->uuid= isset($_POST["UUID"])?$_POST["UUID"]:NULL;
    if( !UUID::is_valid($this->uuid) ) {
      $this->uuid= NULL;
    }
  }

# Returns public data as XML
  public function create_answer() {
    echo XMLSerializer::generateValidXmlFromArray( (array)$this );
    return;
  }

# Writes check for update to file, including UUID if provided
  public function log() {
    $fp= fopen('checks.csv','a');
    $log= date('Y-m-d H:i:s') . ";" . $this->uuid . ";\n";
    fwrite( $fp, $log );

    fclose( $fp );
  }
}

# instanciate class version
$query= new version();

# set members to version values
$query->major= "1.6";
$query->minor= "1.6.4";
$query->notes= "Initial Release of DMHelper v 1.6\\nMARKUP";

# call create_answer to generate xml
$query->create_answer();

# log version check to file
$query->log();

?>
