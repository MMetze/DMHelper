<html>
<body>
  <form name="test" action="file_push_post.php" method="post" enctype="multipart/form-data">
    <input name="user" type="text" value="c.turner" /><br />
    <input name="password" type="password" value="" /><br />

    <input type="file" name="myFile[]" multiple />

    <input name="ok" type="submit" value="senden" />
  </form>
  <form name="test" action="file_push_post.php?debug=1" method="post" enctype="multipart/form-data">
    <input name="user" type="text" value="c.turner" /><br />
    <input name="password" type="password" value="" /><br />

    <input type="file" name="myFile[]" multiple />

    <input name="ok" type="submit" value="debug" />
  </form>

</body>
</html>
