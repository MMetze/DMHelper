<div id="Contentwrap">
	<?php
		if( isset($_SESSION["user"]->ID) ) {
			echo "Bereits eingeloggt. <a href=\"sys_logout.php\">Ausloggen?</a>";
		} else {
			?>
			<form action="sys_login_post.php" method="post">
				<table style="border: none">
					<tr style="border: none">
						<td style="border: none">User:</td>
						<td style="border: none"><input type="text" size="24" maxlength="24" name="username" autofocus><br /></td>
					</tr>
					<tr style="border: none; background: none">
						<td style="border: none">Pass:</td>
						<td style="border: none"><input type="password" size="24" maxlength="24" name="password"><br /></td>
					</tr>
					<tr style="border: none">
						<td align=center colspan="2" style="border: none"><input type="submit" value="Login"></td>
					</tr>
				</table>

			</form>
			<?php
		}
	 ?>
</div>
