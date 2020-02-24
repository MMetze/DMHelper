<?php

#include XMLSerializer and UUID generator class (links inside)
include_once "src/ext_XMLSerialzer.php";
include_once "src/ext_UUID.php";

#create class for data
class version {
  public $version;
  public $notes;

  public $update;

  private $uuid;

# constructor, fetches UUID from POST and checks for validity
  public function __construct() {
    $this->uuid= isset($_POST["UUID"])?$_POST["UUID"]:NULL;
    if( !UUID::is_valid($this->uuid) ) {
      $this->uuid= NULL;
    }
  }

# Returns public data as XML
  public function create_answer() {
    echo XMLSerializer::generateValidXmlFromArray( array( "version" => $this->version, "notes" => $this->notes , "update" => $this->update ) ). PHP_EOL;
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
$v160= new version();
$v161= new version();
$v162= new version();


# set members to version values
$v160->version= "1.6.0";
$v160->notes= "<ul><li>full MacOS support (and a commitment to keep this going!)<li>the battle dialog now integrated in the main window<li>a new camera tool inside the battle view giving you full control over publishing<li>and even better DnD Beyond import integration!";

$v160->update= &$v161;
$v161->version= "1.6.1";
$v161->notes= "Engineering Release";

$v161->update= &$v162;
$v162->version= "1.6.2";
$v162->notes= '<a href="https://dmh.wwpd.de">Update Notes</a>';

# set latest version
$latest= &$v162;

$challengev= isset($_POST["version"])?$_POST["version"]:NULL;

switch ( $challengev ) {
  case "v1.6.0":
    $v160->create_answer();
    $v160->log();
    break;

  default:
    $latest->create_answer();
    $latest->log();
    break;
}

?>
