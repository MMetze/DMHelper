  <?php

    session_start();
  	include_once "src/includes.php";
  	include_once "config.inc.php";
    include_once "src/class_message.php";

    # connect to db
  	$dbc= new db();
  	$pf= $dbc->pre;

    # initialize POST vars
    $user= isset($_POST["user"])?$_POST["user"]:NULL;
    $password= isset($_POST["password"])?$_POST["password"]:NULL;

    # check for active session
    if( !isset($_SESSION["logged_in"]) || $_SESSION["user"]->ID != $user ) {
  		session_unset();
  		$_SESSION["logged_in"]= false;
  	}

    $msg= new message("file_list");

    if( $user==NULL || $password==NULL  ) {
      $msg->error= "Received data incomplete";
      echo $msg;
      exit();
    }

    if( !$_SESSION["logged_in"] ) {
      # Verify User
      $dbc->query= "SELECT ID, pass FROM $pf"."user WHERE username=?;";
      $dbc->bind= "s";
      $dbc->prepare();
      $dbc->values= array( $user );
      $dbc->bind();
      $result= $dbc->execute();

      $user= $result->fetch_object();
      $_SESSION["user"]= $user;

    }

    if( password_verify($password, $_SESSION["user"]->pass) ) {
      $file= new myFile($dbc, 'file', $pf);

      $file_list= $file->fileList($_SESSION["user"]->ID);
      if( isset($file_list) ) {
        $msg->data= $file_list;
        echo $msg;
      }
    }
   ?>
