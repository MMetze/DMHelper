<?php

include_once "xt_UUID.php";

class myFile {
  private
    $ID,
    $name,
    $user,
    $data,
    $dbc,
    $table,
    $pf;


  public function __construct($dbc, $table, $pf) {
    # constructor fills all vars with NULL
    $init= get_object_vars($this);
    foreach($init AS $key => $value ) {
      $key= NULL;
    }

    if( isset($dbc) ) {
      $this->dbc= $dbc;
      $this->table= sanitizeString($table);
      $this->pf= $pf;
    } else {
      return "Database Connection required";
    }
  }

  function __set( $name, $value ) { # set name or code
    if( property_exists($this, $name) ) {
      switch( $name ) {
        case "ID":
          $this->getFile( $value );
          break;

        case "user":
          $tmp= myUUID::hex2uuid(bin2hex($value));
          if( myUUID::is_valid($tmp) ) {
            $this->user= $value;
          }
      }
    }
  }

  function __get( $prop ) {
    return $this->$prop;
  }

  function __invoke() {
    $tmp= array( "md5" => $this->ID, "name" => $this->name, "data" => base64_encode($this->data) );
    return $tmp;
  }

  public static function withArray(Array $row) {
    # creates empty instance of self
    $instance= new static();

    # gets all vars of instance
    $pop= get_object_vars($instance);

    # iterates all given values of submitted array
    foreach( $pop AS $key => $value ) {
      if( isset( $row["$key"]) ) { #if submitted value is not NULL
        if( property_exists( 'myFile', $key ) ) { # checks if class has member with key
          $instance->$key= $row["$key"];	# assigns value of array to class instance member
        }
      }
    }
    return $instance;	# returns class
  }

  public function fileList($user=NULL) {
    $this->dbc->query= "SELECT files.ID AS ID, files.name AS name,";
    if( !isset($user) ){
      $this->dbc->query.= " users.username AS user,";
    }
    $this->dbc->query.= "LENGTH(files.data) AS size FROM $this->pf"."$this->table AS files JOIN $this->pf"."user AS users ON files.user=users.ID";
    if( isset($user) ) {
      $this->dbc->query.= " WHERE users.ID='$user'";
    }
    $this->dbc->query.= " ORDER BY files.name;";
    $result= $this->dbc->myQuery($this->dbc->query);

    while( $row= $result->fetch_assoc() ) {
      if( !isset($user) ) {
        $retval[]= array( 'md5' => (bin2hex($row["ID"])), 'name' => $row["name"], 'user' => $row["user"], 'size' => $row["size"] );
      } else {
        $retval[]= array( 'md5' => (bin2hex($row["ID"])), 'name' => $row["name"], 'size' => $row["size"] );
      }
    }
    return isset($retval)?$retval:NULL;
  }

  public function exists( $files, $user ) {
    $this->dbc->bind= "";
    $this->dbc->query= "SELECT ID, name FROM $this->pf"."file WHERE ID IN (";
    foreach( $files as $file => $key ) {
      $this->dbc->query.= "?, ";
      $this->dbc->bind.= "s";

      $files[$file]= str_replace( "-", "", hex2bin($key) );
    }
    $this->dbc->query= rtrim($this->dbc->query, ", ") . ") AND user=?;";
    $this->dbc->bind.= "s";
    $this->dbc->prepare();
    $this->dbc->values= $files;
    $this->dbc->values[]= $user;
    $this->dbc->bind();
    $return= $this->dbc->execute();

    while( $row= $return->fetch_assoc() ) {
      $row["ID"]= bin2hex($row["ID"]);
      $retval[]= $row;
    }
    return $retval;
  }

  private function getFile( $ID ) {
    $this->dbc->query= "SELECT ID, name, data FROM $this->pf"."file WHERE ID=? AND user=? ORDER BY name LIMIT 1;";
    $this->dbc->bind= "ss";
    $this->dbc->prepare();
    $this->dbc->values= array( hex2bin($ID), $this->user);
    if( $this->dbc->values[0] ){
      $this->dbc->bind();
      $this->dbc->execute();
    }

    if( $this->dbc->result->num_rows>0 ) {
      $row= $this->dbc->result->fetch_object();
      $this->name= $row->name;
      $this->data= $row->data;
      $this->ID= $ID;
    }
  }

}

?>
