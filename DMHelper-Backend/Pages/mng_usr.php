<script type="text/javascript">
function checkform( a, b, empty=false ) {
	var field1= document.getElementById( a );
	var field2= document.getElementById( b );

	if( field1.value.length > 0 ) {
		if ( field1.value==field2.value ) {
			return true;
		}
		alert('Passwort nicht gleich!');
	  return false;
  }

	if( field1.value=='' && empty ) {
		return true;
	}

  alert('Passwort darf nicht leer sein');
  return false;
}
</script>

<?php
  $query= "SELECT IDh, username, email, surname, forename, created, lastlogin, disabled FROM $pf"."user ORDER BY username";
  $users= $dbc->myQuery($query);
?>

<div id="Contentwrap">
	<!-- invisible area to make anchor jumps honor the header -->
	<span class="anchor" id="tab-list"></span><span class="anchor" id="tab-new"></span><span class="anchor" id="tab-edit"></span>
  Benutzerverwaltung
  <div id="tabs1">
    <ul>
      <li><a href="#tab-list">Liste</a></li>
      <li><a href="#tab-new">Neu</a></li>
      <?php if( isset($_GET["user"]) ) { ?>
        <li><a href="#tab-edit">bearbeiten</a></li>
      <?php } ?>
    </ul>


		<div id="tab-list">

      <table>
        <tr>
          <th>Benutzer</td>
          <th>eMail</td>
          <th>Name</td>
          <th>Erstellt</td>
          <th>zuletzt aktiv</td>
          <th>deaktiviert</td>
        </tr>

        <?php
          while( $row= $users->fetch_object() )  {
            ?>
            <tr>
              <td><a href="?view=mngusr&user=<?php echo $row->IDh; ?>#tab-edit"><?php echo $row->username; ?></a></td>
              <td><?php echo $row->email; ?></td>
              <td><?php echo $row->surname . ", " . $row->forename; ?></td>
              <td><?php echo mydatetime( $row->created ); ?></td>
              <td><?php echo mydatetime( $row->lastlogin ); ?></td>
              <td><?php if( !is_null($row->disabled) ) echo date( "d.m.Y", strtotime($row->disabled) ); ?></td>
            </tr>
          <?php
        } # endwhile
        ?>
      </table>
    </div>

    <div id="tab-new">
      <form name="add_user" action="mng_usr_add_post.php" method="post" onSubmit="return checkform( 'password1', 'password2' )">
        <table>
          <tr>
            <td>Username</td>
            <td><input name="username" type="text" maxlength="32" required /></td> <!-- ajax check if user and/or email already in use! -->
          </tr>
          <tr>
            <td>eMail</td>
            <td><input name="email" type="text" maxlength="32" required /></td>
          </tr>
          <tr>
            <td>Name</td>
            <td><input name="surname" type="text" placeholder="Nachname" required /><br /><input name="forename" type="text" placeholder="Vorname" required /></td>
          </tr>
          <tr>
            <td>Passwort</td>
            <td><input name="password1" id="password1" type="password" /><br /><input name="password2" id="password2" type="password" /></td>
          </tr>
          <tr>
            <td colspan="2" style="text-align: right;"><input type="submit" value="ok" /></td>
          </tr>
        </table>
      </form>
    </div>


      <?php if( isset($_GET["user"]) ) { ?>
        <?php
          $dbc->query="SELECT IDh, username, email, surname, forename, created, lastlogin, disabled FROM $pf"."user WHERE ID=?;";
          $dbc->bind= "s";
          $dbc->prepare();
          $dbc->values= array( hex2bin(str_replace("-","",$_GET["user"]) ) );
          $dbc->bind();
          $result= $dbc->execute();
          $user= $result->fetch_object();
        ?>
        <div id="tab-edit">
					<form name="edit_user" action="mng_usr_edit_post.php" method="post" onSubmit="return checkform( 'password3', 'password4', true )">
						<input type="hidden" name="userid" value="<?php echo $user->IDh; ?>" />
	          <table>
	            <tr>
	              <td>Username</td>
	              <td><input type="text" name="username" value="<?php echo $user->username; ?>" placeholder="Username" required /></td>
	            </tr>
	            <tr>
	              <td>eMail</td>
	              <td><input type="text" name="email" value="<?php echo $user->email; ?>" placeholder="eMail" required /></td>
	            </tr>
	            <tr>
	              <td>Name</td>
	              <td><input type="text" name="surname" value="<?php echo $user->surname; ?>" placeholder="Nachname" required /><br/><input type="text" name="forename" value="<?php echo $user->forename; ?>" placeholder="Vorname" required /></td>
	            </tr>
	            <tr>
	              <td>Erstellt</td>
	              <td><?php echo mydatetime( $user->created ); ?></td>
	            </tr>
	            <tr>
	              <td>Aktivität</td>
	              <td><?php echo mydatetime( $user->lastlogin ); ?></td>
	            </tr>
							<tr>
		            <td>Passwort</td>
		            <td><input name="password1" id="password3" type="password" /><br /><input name="password2" id="password4" type="password" /></td>
		          </tr>
	            <tr>
	              <td>Deaktiviert</td>
	              <td><input type="date" name="disabled" value="<?php if( !is_null($user->disabled) ) echo  date( 'Y-m-d', strtotime( $user->disabled )); ?>" /></td>
	            </tr>
							<tr>
								<td colspan="2" style="text-align: right;"><input type="submit" value="speichern" /></td>
							</tr>
	          </table>
					</form>
        </div>
      <?php } ?>

    </div>

</div>
