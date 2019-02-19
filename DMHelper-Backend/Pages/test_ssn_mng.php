<html>
<body>
  <form name="test" action="ssn_mng.php" method="post">
    <input name="user" type="text" value="c.turner" /><br />
    <input name="password" type="password" value="" /><br />
    <input name="name" type="text"/>
    <input name="ok" type="submit" value="erzeugen" />
    <input name="action" type="hidden" value="create" />
  </form>

  <form name="test" action="ssn_mng.php" method="post">
    <input name="user" type="text" value="c.turner" /><br />
    <input name="password" type="password" value="" /><br />
    <input name="name" type="text"/><br />
    <input name="session" type="session" value="7B3AA550-649A-4D51-920E-CAB465616995" placeholder="sessionid" /><br />
    <input name="ok" type="submit" value="owner?" />
    <input name="action" type="hidden" value="isowner" />
  </form>

  <form name="test" action="ssn_mng.php" method="post">
    <input name="user" type="text" value="c.turner" /><br />
    <input name="password" type="password" value="" /><br />
    <input name="name" type="text" placeholder="newname" /><br />
    <input name="session" type="session" value="7B3AA550-649A-4D51-920E-CAB465616995" placeholder="sessionid" /><br />
    <input name="ok" type="submit" value="rename" />
    <input name="action" type="hidden" value="rename" />
  </form>

  <form name="test" action="ssn_mng.php" method="post">
    <input name="user" type="text" value="c.turner" /><br />
    <input name="password" type="password" value="" /><br />
    <input name="session" type="session" value="" placeholder="sessionid" /><br />
    <input name="ok" type="submit" value="remove" />
    <input name="action" type="hidden" value="remove" />
  </form>

  <form name="test" action="ssn_mng.php" method="post">
    <input name="user" type="text" value="c.turner" /><br />
    <input name="password" type="password" value="" /><br />
    <input name="session" type="session" value="" placeholder="sessionid" /><br />
    <input name="ok" type="submit" value="renew" />
    <input name="action" type="hidden" value="renew" />
  </form>

  <form name="test" action="ssn_mng.php" method="post">
    <input name="user" type="text" value="c.turner" /><br />
    <input name="password" type="password" value="" /><br />
    <input name="session" type="session" value="" placeholder="sessionid" /><br />
    <input name="ok" type="submit" value="close" />
    <input name="action" type="hidden" value="close" />
  </form>

  <form name="test" action="ssn_mng.php" method="post">
    <input name="user" type="text" value="c.turner" /><br />
    <input name="password" type="password" value="" /><br />
    <input name="session" type="session" value="" placeholder="sessionid" /><br />
    <input name="ok" type="submit" value="members" />
    <input name="action" type="hidden" value="member" />
  </form>

</body>
</html>
