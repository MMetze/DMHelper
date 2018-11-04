<?php
include_once "ext_UUID.php";
class myUUID extends UUID {
  public static function v4(){
    $uuid= parent::v4();
    $uuid= str_replace("-","", $uuid);
    return $uuid;
  }

  public static function v4bin() {
    return hex2bin(self::v4());
  }

  public static function hex2uuid( $uuid ) {
    $retval= substr_replace($uuid, "-", 20, 0);
    $retval= substr_replace($retval, "-", 16, 0);
    $retval= substr_replace($retval, "-", 12, 0);
    $retval= substr_replace($retval, "-", 8, 0);
    return $retval;
  }

  public static function uuid2bin( $uuid ) {
    $retval= str_replace("-","",$uuid);
    $retval= hex2bin($retval);
    return $retval;
  }
}
 ?>
