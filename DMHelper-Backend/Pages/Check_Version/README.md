Server response for version checking.

To get a reply (XML) call URL/check_version.php.
If nothing is provided, it will return the latest version and a $notes field,
which any textual information, eg the notes itself, or a link to the notes on an
external storage (URL).

Optional POST values
UUID
If UUID is posted, it will be stored in the challenge log for instance identification.

version
If version is posted, the release note for this, and every newer, version is part of
the XML response.

Example for requesting an update-path
  <?xml version="1.0" encoding="UTF-8" ?>
    <nodes>
      <version>1.6.0</version>
        <notes>&lt;ul&gt;&lt;li&gt;full MacOS support (and a commitment to keep this going!)&lt;li&gt;the battle dialog now integrated in the main window&lt;li&gt;a new camera tool inside the battle view giving you full control over publishing&lt;li&gt;and even better DnD Beyond import integration!</notes>
        <update>
          <version>1.6.1</version>
            <notes>Engineering Release</notes>
              <update>
                <version>1.6.2</version>
                  <notes>&lt;a href=&quot;https://dmh.wwpd.de&quot;&gt;Update Notes&lt;/a&gt;</notes>
          <update>
        </update>
      </update>
    </update>
  </nodes>
