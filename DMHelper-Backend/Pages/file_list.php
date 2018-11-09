<div id="Contentwrap">
  <?php
    $file= new myFile($dbc, 'file', $pf);

    $file_list= $file->fileList();

    if( $file_list ){
      foreach( $file_list as $row ) {
        echo "ID: ". $row["ID"] . " | md5: ". $row["md5"] . " | Name: " . $row["name"] . " | Besitzer: " . $row["user"] . " | Size: " . human_filesize( $row["size"] ) . "<br />";
      }
    } else {
      echo "Keine Dateien vorhanden<br />";
    }
   ?>
</div>
