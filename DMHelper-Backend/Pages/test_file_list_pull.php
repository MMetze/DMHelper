<?php
include_once "src/xt_UUID.php";
 ?>

<html>
<body>
  <form name="test" action="file.php" method="post" enctype="multipart/form-data">
    <input name="user" type="text" value="c.turner" /><br />
    <input name="password" type="password" value="" /><br />
    <input name="action" type="hidden" value="list" /><br />

    <input name="ok" type="submit" value="test" />
  </form>

  <form name="test" action="file.php" method="post" enctype="multipart/form-data">
    <input name="user" type="text" value="c.turner" /><br />
    <input name="password" type="password" value="" /><br />
    <input name="fileids[]" type="text" value="<?php echo myUUID::hex2uuid("33646139303834626534316634613361"); ?>" /><br />
    <input name="fileids[]" type="text" value="<?php #echo myUUID::hex2uuid("25b939c597a98aa5aba30a09837c7065"); ?>" /><br />
    <input name="ok" type="submit" value="test" />
    <input name="action" value="exists" type="hidden" />
  </form>

</body>
</html>
