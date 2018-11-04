<?php
class db {
	public $db;
	public $pre;
	public $query;
	public $bind;
	public $values;
	public $params;
	public $result;

	private $stmt;

	// Open SQL connection using parameters stored in config.inc.php
  function __construct() {
  		global $config;

  			$this->db = mysqli_connect($config["db_host"],$config["db_user"],$config["db_pass"], $config["db_name"]);
  			if (!$this->db) {
  				die("Database Connection Error: " . mysqli_error($this->db));
  			}

  			/*$db_select = mysql_select_db($config["db_name"], $this->db);
  			if (!$db_select) {
  				die("Database selection failed: " .mysql_error());
  			};*/
  			$this->pre= $config["db_prefix"];

  			/* change character set to utf8 */
  			mysqli_set_charset($this->db, "utf8");
  }

	// Close SQL connection when class object is discarded
  function __destruct() {
  		mysqli_close($this->db);
  	}

	// Run query on selected database
  public function myQuery($query, $uid=0) {

		$defNULL= array( "'NULL'", "''" );
		$query= str_replace($defNULL, "NULL", $query);

		$result = mysqli_query($this->db, $query);

		if (!$result) {
			$message = "Ung&uuml;ltige Abfrage: " . mysqli_error($this->db) . "\n";
			$message .= "Query: " . $query;
			die($message);
		}
		return $result;
	}

	// Run query on selected database
  public function myQueryObject($query, $uid=0) {

		$defNULL= array( "'NULL'", "''" );
		$query= str_replace($defNULL, "NULL", $query);

		$result = $this->db->query($query);

		if (!$result) {
			$message = "Ung&uuml;ltige Abfrage: " . mysqli_error($this->db) . "\n";
			$message .= "Query: " . $query;
			die($message);
		}
		return $result;
	}

  public function mysqli_exec_batch ($p_query, $p_transaction_safe = true) {
  	if ($p_transaction_safe) {
  		  $p_query = 'START TRANSACTION;' . $p_query . '; COMMIT;';
  	};
  	$query_split = preg_split ("/[;]+/", $p_query);
  	foreach ($query_split as $command_line) {
  		$command_line = trim($command_line);
  		if ($command_line != '') {
  			$query_result = mysqli_query($this->db, $command_line);
  				if ($query_result == 0) {
  					break;
  				};
  			};
  		};
		return $query_result;
	}

  public function fetch_all($result) {
  	$values= array();
  	while( $row= mysqli_fetch_row($result) ) {
  		array_push( $values, $row[0]);
  	}
  	return $values;
  }

  public function prepare() {
  	# prepare statement
  	if(!($this->stmt= $this->db->prepare($this->query)) ) {
  		echo "Prepare failed: (" . $this->db->errno . ") " . $this->db->error;
  	}
  }

  public function bind() {

  	# store reference to parameter type as first element in $params
  	$params[]= & $this->bind;

    # get all elements of values and store references in $params
  	$n= count($this->values);
  	for( $i=0; $i<$n; $i++ ) {
  		$params[]= & $this->values[$i];
  	}

  	# call 'bind-param', member of $this->stmt (DBC) and supply all parameters in array
  	call_user_func_array(array($this->stmt, 'bind_param'), $params );
  }

  public function execute() {
  	# execute statement
  	if( !$this->stmt->execute() ) {
  		echo "Execute failed: (" . $this->stmt->errno . ") " . $this->db->error;
  	}
  	$this->result= $this->stmt->get_result();
  	return $this->result;
  }

	public function send_long_data( $in, $data ) {
		$this->stmt->send_long_data( $in, $data );
	}

	public function affected_rows() {
		return $this->db->affected_rows();
	}
}
?>
