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

    $_SESSION["user"]= $_SESSION["user"]??NULL;
    $files= $_POST["fileids"]??NULL;
    $action= $_POST["action"]??NULL;
    $id= $_POST["ID"]??NULL;

    # check for active session
    if( !isset($_SESSION["logged_in"]) || $_SESSION["user"]->ID != $user ) {
  		session_unset();
  		$_SESSION["logged_in"]= false;
  	}

    $msg= new message("file");

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
      $file= new myFile($dbc, 'file', $pf );
      $file->user= $_SESSION["user"]->ID;

      switch( $action ) {
        case "exists":
          $msg->mode= "file_exists";
          $msg->data= $file->exists( $files, $_SESSION["user"]->ID );
          break;

        case "list":
          $msg->mode= "file_list";
          $file_list= $file->fileList($_SESSION["user"]->ID);
          if( isset($file_list) ) {
            $msg->data= $file_list;
          }
          break;

        case "pull":
          $msg->mode= "file_pull";
          if( strtolower( $_SERVER[ 'REQUEST_METHOD' ] ) == 'post' && !empty( $files ) ) {
            foreach( $files AS $pfile => $key ) {
              $file->ID= $key["ID"];
              $msg->data[]= $file();
            }
          }
          break;

          case "update":
          $msg->mode= "file_update";
          if( !empty( $files ) ){
            $tmpFile= $file->exists( $id, $user );
          }
          break;
      }

      echo $msg;

    }
   ?>
