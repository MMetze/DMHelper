<?php
 ?>

<html>
<body>
  <form name="test" action="file.php" method="post" enctype="multipart/form-data">
    <input name="user" type="text" value="c.turner" /><br />
    <input name="password" type="password" value="" /><br />
    <input name="action" type="hidden" value="pull" /><br />

    <input name="fileids[][md5]" type="text" value="745f9545ee456ea5f072961c60872603" /><br />
    <input name="fileids[][md5]" type="text" value="25b939c597a98aa5aba30a09837c7065" /><br />

    <input name="ok" type="submit" value="test" />
  </form>

</body>
</html>
