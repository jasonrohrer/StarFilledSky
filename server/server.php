<?php



global $fs_version;
$fs_version = "1";



// edit settings.php to change server' settings
include( "settings.php" );




// no end-user settings below this point




// enable verbose error reporting to detect uninitialized variables
error_reporting( E_ALL );



// page layout for web-based setup
$setup_header = "
<HTML>
<HEAD><TITLE>Star-filled Sky Flag Server Web-based setup</TITLE></HEAD>
<BODY BGCOLOR=#FFFFFF TEXT=#000000 LINK=#0000FF VLINK=#FF0000>

<CENTER>
<TABLE WIDTH=75% BORDER=0 CELLSPACING=0 CELLPADDING=1>
<TR><TD BGCOLOR=#000000>
<TABLE WIDTH=100% BORDER=0 CELLSPACING=0 CELLPADDING=10>
<TR><TD BGCOLOR=#EEEEEE>";

$setup_footer = "
</TD></TR></TABLE>
</TD></TR></TABLE>
</CENTER>
</BODY></HTML>";





// ensure that magic quotes are OFF
// we hand-filter all _REQUEST data with regexs before submitting it to the DB
if( get_magic_quotes_gpc() ) {
    // force magic quotes to be added
    $_GET     = array_map( 'fs_stripslashes_deep', $_GET );
    $_POST    = array_map( 'fs_stripslashes_deep', $_POST );
    $_REQUEST = array_map( 'fs_stripslashes_deep', $_REQUEST );
    $_COOKIE  = array_map( 'fs_stripslashes_deep', $_COOKIE );
    }
    





// all calls need to connect to DB, so do it once here
fs_connectToDatabase();

// close connection down below (before function declarations)


// testing:
//sleep( 5 );


// general processing whenver server.php is accessed directly


fs_checkForFlush();



// grab POST/GET variables
$action = fs_requestFilter( "action", "/[A-Z_]+/i" );

$debug = fs_requestFilter( "debug", "/[01]/" );


$remoteIP = "";
if( isset( $_SERVER[ "REMOTE_ADDR" ] ) ) {
    $remoteIP = $_SERVER[ "REMOTE_ADDR" ];
    }



if( $action == "version" ) {
    global $fs_version;
    echo "$fs_version";
    }
else if( $action == "show_log" ) {
    fs_showLog();
    }
else if( $action == "clear_log" ) {
    fs_clearLog();
    }
else if( $action == "show_data" ) {
    fs_showData();
    }
else if( $action == "place_flag" ) {
    fs_placeFlag();
    }
else if( $action == "get_flags" ) {
    fs_getFlags();
    }
else if( $action == "logout" ) {
    fs_logout();
    }
else if( $action == "fs_setup" ) {
    global $setup_header, $setup_footer;
    echo $setup_header; 

    echo "<H2>Star-filled Sky Flag Server Web-based Setup</H2>";

    echo "Creating tables:<BR>";

    echo "<CENTER><TABLE BORDER=0 CELLSPACING=0 CELLPADDING=1>
          <TR><TD BGCOLOR=#000000>
          <TABLE BORDER=0 CELLSPACING=0 CELLPADDING=5>
          <TR><TD BGCOLOR=#FFFFFF>";

    fs_setupDatabase();

    echo "</TD></TR></TABLE></TD></TR></TABLE></CENTER><BR><BR>";
    
    echo $setup_footer;
    }
else if( preg_match( "/server\.php/", $_SERVER[ "SCRIPT_NAME" ] ) ) {
    // server.php has been called without an action parameter

    // the preg_match ensures that server.php was called directly and
    // not just included by another script
    
    // quick (and incomplete) test to see if we should show instructions
    global $tableNamePrefix;
    
    // check if our "games" table exists
    $tableName = $tableNamePrefix . "games";
    
    $exists = fs_doesTableExist( $tableName );
        
    if( $exists  ) {
        echo "Star-filled Sky Flag Server database setup and ready";
        }
    else {
        // start the setup procedure

        global $setup_header, $setup_footer;
        echo $setup_header; 

        echo "<H2>Star-filled Sky Flag Server Web-based Setup</H2>";
    
        echo "Flag Server will walk you through a " .
            "brief setup process.<BR><BR>";
        
        echo "Step 1: ".
            "<A HREF=\"server.php?action=fs_setup\">".
            "create the database tables</A>";

        echo $setup_footer;
        }
    }



// done processing
// only function declarations below

fs_closeDatabase();







/**
 * Creates the database tables needed by seedBlogs.
 */
function fs_setupDatabase() {
    global $tableNamePrefix;
    

    $tableName = $tableNamePrefix . "server_globals";
    if( ! fs_doesTableExist( $tableName ) ) {

        // this table contains general info about the server
        // use INNODB engine so table can be locked
        $query =
            "CREATE TABLE $tableName( " .
            "last_flush_time DATETIME NOT NULL ) ENGINE = INNODB;";

        $result = fs_queryDatabase( $query );

        echo "<B>$tableName</B> table created<BR>";
        }
    else {
        echo "<B>$tableName</B> table already exists<BR>";
        }


    
    $tableName = $tableNamePrefix . "log";
    if( ! fs_doesTableExist( $tableName ) ) {

        // this table contains general info about the server
        // use INNODB engine so table can be locked
        $query =
            "CREATE TABLE $tableName(" .
            "entry TEXT NOT NULL, ".
            "entry_time DATETIME NOT NULL );";

        $result = fs_queryDatabase( $query );

        echo "<B>$tableName</B> table created<BR>";
        }
    else {
        echo "<B>$tableName</B> table already exists<BR>";
        }

    
    
    $tableName = $tableNamePrefix . "flags";
    if( ! fs_doesTableExist( $tableName ) ) {

        // this table contains general info about each game
        // use INNODB engine so table can be locked
        $query =
            "CREATE TABLE $tableName(" .
            "level_number INT NOT NULL," .
            "level_seed INT UNSIGNED NOT NULL," .
            "creation_date DATETIME NOT NULL," .
            "change_date DATETIME NOT NULL," .
            "change_ip_address CHAR(15) NOT NULL," .
            "change_count INT UNSIGNED NOT NULL," .
            "view_date DATETIME NOT NULL," .
            "view_count INT UNSIGNED NOT NULL," .
            "flag_a CHAR(9) NOT NULL," .
            "flag_b CHAR(9) NOT NULL," .
            "PRIMARY KEY( level_number, level_seed ) ) ENGINE = INNODB;";

        $result = fs_queryDatabase( $query );

        echo "<B>$tableName</B> table created<BR>";
        }
    else {
        echo "<B>$tableName</B> table already exists<BR>";
        }
    }



function fs_showLog() {
    fs_checkPassword( "show_log" );

    echo "[<a href=\"server.php?action=show_data" .
         "\">Main</a>]<br><br><br>";
    
    global $tableNamePrefix;
    
    $query = "SELECT * FROM $tableNamePrefix"."log ORDER BY entry_time DESC;";
    $result = fs_queryDatabase( $query );
    
    $numRows = mysql_numrows( $result );

    echo "<a href=\"server.php?action=clear_log\">".
        "Clear log</a>";
        
    echo "<hr>";
    
    echo "$numRows log entries:<br><br><br>\n";
    
    
    for( $i=0; $i<$numRows; $i++ ) {
        $time = mysql_result( $result, $i, "entry_time" );
        $entry = mysql_result( $result, $i, "entry" );
        
        echo "<b>$time</b>:<br>$entry<hr>\n";
        }
    }



function fs_clearLog() {

    fs_checkPassword( "clear_log" );

    echo "[<a href=\"server.php?action=show_data" .
         "\">Main</a>]<br><br><br>";
    
    global $tableNamePrefix;

    $query = "DELETE FROM $tableNamePrefix"."log;";
    $result = fs_queryDatabase( $query );
    
    if( $result ) {
        echo "Log cleared.";
        }
    else {
        echo "DELETE operation failed?";
        }
    }






function fs_placeFlag() {
    $level_number = fs_requestFilter( "level_number", "/[0-9]+/", "" );

    $level_seed = fs_requestFilter( "level_seed", "/[0-9]+/", "" );

    $spot = fs_requestFilter( "spot", "/[AB]/", "" );

    
    $flag = fs_requestFilter( "flag", "/[A-F0-9]+/i", "" );

    $flag = strtoupper( $flag );

    
    $sig = fs_requestFilter( "sig", "/[A-F0-9]+/i", "" );    

    $sig = strtoupper( $sig );


    if( $level_number == "" ||
        $level_seed == "" ||
        $spot == "" ||
        strlen( $flag ) != 9 ) {

        echo "REJECTED";
        return;
        }


    // verify sig
            
    global $sharedSecret;
            
    $trueSig =
        sha1( $level_number . $level_seed . $spot . $flag . $sharedSecret );
    
    $trueSig = strtoupper( $trueSig );


    if( $trueSig != $sig ) {
        echo "REJECTED";
        return;
        }
    
    // else sig good
    
    global $tableNamePrefix, $remoteIP;

    
    // disable autocommit so that FOR UPDATE actually works
    fs_queryDatabase( "SET AUTOCOMMIT = 0;" );
    

    /*
            "CREATE TABLE $tableName(" .
            "level_number INT NOT NULL," .
            "level_seed INT UNSIGNED NOT NULL," .
            "creation_date DATETIME NOT NULL," .
            "change_date DATETIME NOT NULL," .
            "change_ip_address CHAR(15) NOT NULL," .
            "change_count INT UNSIGNED NOT NULL," .
            "view_date DATETIME NOT NULL," .
            "view_count INT UNSIGNED NOT NULL," .
            "flag_a CHAR(9) NOT NULL," .
            "flag_b CHAR(9) NOT NULL," .
            "PRIMARY KEY( level_number, level_seed ) ) ENGINE = INNODB;";
    */

    
    $query = "SELECT * FROM $tableNamePrefix"."flags ".
        "WHERE level_number = '$level_number' AND level_seed = '$level_seed' ".
        "FOR UPDATE;";
    $result = fs_queryDatabase( $query );

    $numRows = mysql_numrows( $result );

    if( $numRows == 1 ) {
        $row = mysql_fetch_array( $result, MYSQL_ASSOC );
        
        $old_flag_a = $row[ "flag_a" ];

        if( $spot == "A" && $old_flag_a != "BLANKFLAG" ) {
            // already a permanent flag here

            // unlock rows that were locked by FOR UPDATE above
            fs_queryDatabase( "COMMIT;" );
            
            fs_queryDatabase( "SET AUTOCOMMIT = 1;" );

            echo "REJECTED";
            return;
            }

        $change_count = $row[ "change_count" ];
        $change_count ++;
        
        
        $flagClause;

        if( $spot == "A" ) {
            $flagClause = "flag_a = '$flag'";
            }
        else {
            $flagClause = "flag_b = '$flag'";
            }

        
        
        
        $query = "UPDATE $tableNamePrefix"."flags SET " .
            "change_date = CURRENT_TIMESTAMP, " .
            "change_ip_address = '$remoteIP', " .
            "change_count = '$change_count', " .
            "$flagClause " .
            "WHERE level_number = '$level_number' AND ".
            "level_seed = '$level_seed';";
        
        $result = fs_queryDatabase( $query );


        echo "OK";
        }
    else {
        // doesn't exist yet

        $flagClause;

        if( $spot == "A" ) {
            $flagClause = "'$flag', 'BLANKFLAG'"; 
           }
        else {
            $flagClause = "'BLANKFLAG', '$flag'";
            }
        
        $query = "INSERT INTO $tableNamePrefix". "flags VALUES ( " .
            "'$level_number', '$level_seed', ".
            "CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, ".
            "'$remoteIP', '1', CURRENT_TIMESTAMP, '0', ".
            "$flagClause );";

        $result = fs_queryDatabase( $query );

        echo "OK";
        }

    /*
http://localhost/jcr13/game10_flag/server.php?action=place_flag&level_number=5&level_seed=234890902&spot=A&flag=FA89&sig=ba48acc4f3603c16f5629404470dbad0eaaec7cd
     */

    // unlock rows that were locked by FOR UPDATE above
    fs_queryDatabase( "COMMIT;" );
    
    fs_queryDatabase( "SET AUTOCOMMIT = 1;" );
    }






function fs_getFlags() {
    $level_number = fs_requestFilter( "level_number", "/[0-9]+/", "" );

    $level_seed = fs_requestFilter( "level_seed", "/[0-9]+/", "" );

    if( $level_number == "" ||
        $level_seed == "" ) {

        echo "REJECTED";
        return;
        }

    
    global $tableNamePrefix, $remoteIP;

    
    // disable autocommit so that FOR UPDATE actually works
    fs_queryDatabase( "SET AUTOCOMMIT = 0;" );
    

    /*
            "CREATE TABLE $tableName(" .
            "level_number INT NOT NULL," .
            "level_seed INT UNSIGNED NOT NULL," .
            "creation_date DATETIME NOT NULL," .
            "change_date DATETIME NOT NULL," .
            "change_ip_address CHAR(15) NOT NULL," .
            "change_count INT UNSIGNED NOT NULL," .
            "view_date DATETIME NOT NULL," .
            "view_count INT UNSIGNED NOT NULL," .
            "flag_a CHAR(9) NOT NULL," .
            "flag_b CHAR(9) NOT NULL," .
            "PRIMARY KEY( level_number, level_seed ) ) ENGINE = INNODB;";
    */

    
    $query = "SELECT * FROM $tableNamePrefix"."flags ".
        "WHERE level_number = '$level_number' AND level_seed = '$level_seed' ".
        "FOR UPDATE;";
    $result = fs_queryDatabase( $query );

    $numRows = mysql_numrows( $result );

    if( $numRows == 1 ) {
        $row = mysql_fetch_array( $result, MYSQL_ASSOC );
        
        $flag_a = $row[ "flag_a" ];
        $flag_b = $row[ "flag_b" ];

        
        $view_count = $row[ "view_count" ];
        $view_count ++;
        
        
        
        $query = "UPDATE $tableNamePrefix"."flags SET " .
            "view_date = CURRENT_TIMESTAMP, " .
            "view_count = '$view_count' " .
            "WHERE level_number = '$level_number' AND ".
            "level_seed = '$level_seed';";
        
        $result = fs_queryDatabase( $query );


        echo "$flag_a $flag_b";
        }
    else {
        // doesn't exist yet

        echo "BLANKFLAG BLANKFLAG";
        }

    /*
http://localhost/jcr13/game10_flag/server.php?action=get_flags&level_number=5&level_seed=234890902
     */

    // unlock rows that were locked by FOR UPDATE above
    fs_queryDatabase( "COMMIT;" );
    
    fs_queryDatabase( "SET AUTOCOMMIT = 1;" );
    }





function fs_generateFlagHTML( $inLink, $inFlagString ) {
    $colorMap = array( "0" => "#DF2D00",
                       "1" => "#31DD09",
                       "2" => "#3838BB",
                       "3" => "#FFF5DA",
                       "4" => "#363636",
                       "5" => "#FFDD00",
                       "6" => "#4CD0D0",
                       "7" => "#D400D3",
                       "8" => "#E68700",
                       "9" => "#7B19B1",
                       "A" => "#5AFF8B",
                       "B" => "#808080",
                       "C" => "#7F4F00",
                       "D" => "#1F7301",
                       "E" => "#71001B",
                       "F" => "#FF9FDA" );
    
    $flag_chars = str_split( $inFlagString );
                
    $flagHTML =
        "<table border=0 ".
        "bgcolor=black cellspacing=0 cellpadding=0>";
                
    for( $y=0; $y<3; $y++ ) {
        $flagHTML = $flagHTML . "<tr>";
        for( $x=0; $x<3; $x++ ) {
            $cellChar = $flag_chars[ $y * 3 + $x ];
            
            if( array_key_exists( $cellChar, $colorMap ) ) {
                
                $color = $colorMap[ $cellChar ];
            
                $flagHTML =
                    $flagHTML .
                    "<td bgcolor=$color>";

                if( $inLink != "" ) {
                    $flagHTML = $flagHTML . "<a href=\"$inLink\">";
                    }
            
                $flagHTML = $flagHTML .
                    "<img border=0 width=12 height=12 src=\"flagBlank.png\">";

                if( $inLink != "" ) {
                    $flagHTML = $flagHTML . "</a>";
                    }
            
                $flagHTML = $flagHTML . "</td>";
                }
            }
        $flagHTML = $flagHTML . "</tr>";
        }
    $flagHTML = $flagHTML . "</table>";

    return $flagHTML;
    }



function fs_logout() {

    fs_clearPasswordCookie();

    echo "Logged out";
    }





function fs_showData() {
    // call several of these global so they can be accessed properly
    // inside the sub-functions we define below
    global $skip, $search, $order_by, $graph_interval;
    
    
    fs_checkPassword( "show_data" );

    global $tableNamePrefix, $remoteIP;
    

    echo "<table width='100%' border=0><tr>".
        "<td>[<a href=\"server.php?action=show_data" .
            "\">Main</a>]</td>".
        "<td align=right>[<a href=\"server.php?action=logout" .
            "\">Logout</a>]</td>".
        "</tr></table><br><br><br>";


    $graph_interval = fs_requestFilter( "graph_interval", "/[0-9]+/", "30" );


    $skip = fs_requestFilter( "skip", "/[0-9]+/", "0" );



    $order_by = fs_requestFilter( "order_by", "/[A-Z_]+/i", "change_date" );

    
    global $flagsPerPage;    


    $search = fs_requestFilter( "search", "/[A-Z0-9_@. \-]+/i", "" );

    
    $keywordWhereClause = "";
    $keywordAndClause = "";
    $searchDisplay = "";
    
    if( $search != "" ) {
        
        /*
        $keywordClause = "WHERE ( level_number LIKE '%$search%' " .
            "OR level_seed LIKE '%$search%' ".
            "OR change_ip_address LIKE '%$search%' ".
            "OR flag_a LIKE '%$search%' ".
            "OR flag_b LIKE '%$search%' ) ";
        */

        // watch for flag_ prefix, which indicates flag display should be
        // used, but this should not be passed into LIKE clause
        $flagCount = 0;
        
        $searchString = preg_replace( "/flag_/", "", $search, -1, $flagCount );
        
        
        // switch to exact matches to avoid surprising cross-matches
        // (searching for level 27 was matching IP address 127.0.0.1)
        $keywordWhereClause = "WHERE ( level_number LIKE '$searchString' " .
            "OR level_seed LIKE '$searchString' ".
            "OR change_ip_address LIKE '$searchString' ".
            "OR flag_a LIKE '$searchString' ".
            "OR flag_b LIKE '$searchString' ) ";

        
        $keywordAndClause =
            preg_replace( "/WHERE/", "AND", $keywordWhereClause );


        if( $flagCount == 1 ) {
            $searchDisplay = " matching " .
                fs_generateFlagHTML( "", $searchString );
            }
        else {
            $searchDisplay = " matching <b>$search</b>";
            }
        
        }


    

    // first, count results
    $query = "SELECT COUNT(*) FROM $tableNamePrefix".
        "flags $keywordWhereClause;";

    $result = fs_queryDatabase( $query );
    $totalFlags = mysql_result( $result, 0, 0 );

    
             
    $query = "SELECT * FROM $tableNamePrefix"."flags $keywordWhereClause".
        "ORDER BY $order_by DESC ".
        "LIMIT $skip, $flagsPerPage;";
    $result = fs_queryDatabase( $query );
    
    $numRows = mysql_numrows( $result );

    $startSkip = $skip + 1;
    
    $endSkip = $startSkip + $flagsPerPage - 1;

    if( $endSkip > $totalFlags ) {
        $endSkip = $totalFlags;
        }
    $showingDisplay = " (showing $startSkip - $endSkip)";
    if( $totalFlags <= 1 ) {
        $showingDisplay = "";
        }
    



    // form for searching flags
?>
        <hr>
            <FORM ACTION="server.php" METHOD="post">
    <INPUT TYPE="hidden" NAME="action" VALUE="show_data">
    <INPUT TYPE="text" MAXLENGTH=40 SIZE=20 NAME="search"
             VALUE="<?php echo $search;?>">
    <INPUT TYPE="Submit" VALUE="Search">
    </FORM>
        <hr>
<?php

    $recordWord = "records";

    if( $totalFlags == 1 ) {
        $recordWord = "record";
        }
    
    echo "$totalFlags flag $recordWord" .$searchDisplay .
        "$showingDisplay:<br>\n";

    
    $nextSkip = $skip + $flagsPerPage;

    $prevSkip = $skip - $flagsPerPage;
    
    if( $prevSkip >= 0 ) {
        echo "[<a href=\"server.php?action=show_data" .
            "&skip=$prevSkip&search=$search".
            "&order_by=$order_by".
            "&graph_interval=$graph_interval\">Previous Page</a>] ";
        }
    if( $nextSkip < $totalFlags ) {
        echo "[<a href=\"server.php?action=show_data" .
            "&skip=$nextSkip&search=$search".
            "&order_by=$order_by".
            "&graph_interval=$graph_interval\">Next Page</a>]";
        }

    /*
            "CREATE TABLE $tableName(" .
            "level_number INT NOT NULL," .
            "level_seed INT UNSIGNED NOT NULL," .
            "creation_date DATETIME NOT NULL," .
            "change_date DATETIME NOT NULL," .
            "change_ip_address CHAR(15) NOT NULL," .
            "change_count INT UNSIGNED NOT NULL," .
            "view_date DATETIME NOT NULL," .
            "view_count INT UNSIGNED NOT NULL," .
            "flag_a CHAR(9) NOT NULL," .
            "flag_b CHAR(9) NOT NULL," .
            "PRIMARY KEY( level_number, level_seed ) ) ENGINE = INNODB;";
    */

    
    echo "<br><br>";
    
    echo "<table border=1 cellpadding=5>\n";


    function orderLink( $inOrderBy, $inLinkText ) {
        global $skip, $search, $order_by;
        if( $inOrderBy == $order_by ) {
            // already displaying this order, don't show link
            return "<b>$inLinkText</b>";
            }

        // else show a link to switch to this order
        return "<a href=\"server.php?action=show_data" .
            "&search=$search&skip=$skip&order_by=$inOrderBy\">$inLinkText</a>";
        }

    echo "<tr>\n";
    echo "<td>".orderLink( "level_number", "Level Number" )."</td>\n";
    echo "<td>Level seed</td>\n";
    echo "<td>Temporary flag</td>\n";
    echo "<td>Permanent flag</td>\n";
    echo "<td>".orderLink( "creation_date", "Created" )."</td>\n";
    echo "<td>".orderLink( "change_date", "Changed" )."</td>\n";
    echo "<td>Changed by</td>\n";
    echo "<td>".orderLink( "change_count", "Changes" )."</td>\n";
    echo "</tr>\n";
    

    function searchLink( $inString, $inLinkText ) {
        return "<a href=\"server.php?action=show_data" .
            "&search=$inString\">$inLinkText</a>";
        }

    for( $i=0; $i<$numRows; $i++ ) {
        $level_number = mysql_result( $result, $i, "level_number" );
        $level_seed = mysql_result( $result, $i, "level_seed" );
        $creation_date = mysql_result( $result, $i, "creation_date" );
        $change_date = mysql_result( $result, $i, "change_date" );
        $change_ip_address = mysql_result( $result, $i, "change_ip_address" );
        $change_count = mysql_result( $result, $i, "change_count" );

        $flag_a = mysql_result( $result, $i, "flag_a" );
        $flag_b = mysql_result( $result, $i, "flag_b" );

        

        $flags[0] = $flag_a;
        $flags[1] = $flag_b;

        $flagHTML[0] = "";
        $flagHTML[1] = "";
        
        for( $f=0; $f<2; $f++ ) {
            
            $flagHTML[$f] = searchLink( "$flags[$f]",
                                        "(blank)" );

            if( $flags[$f] != "BLANKFLAG" ) {

                $flagHTML[$f] = fs_generateFlagHTML(
                    "server.php?action=show_data" .
                    "&search=flag_$flags[$f]",
                    $flags[$f] );
                }
            }
        
        
        


        
        echo "<tr>\n";
        echo "<td>".searchLink( $level_number, $level_number )."</td>\n";
        echo "<td>".searchLink( $level_seed, $level_seed )."</td>\n";
        echo "<td align=center>$flagHTML[1]</td>\n";
        echo "<td align=center>$flagHTML[0]</td>\n";
        echo "<td>$creation_date</td>\n";
        echo "<td>$change_date</td>\n";
        echo "<td>".
            searchLink( $change_ip_address, $change_ip_address )."</td>\n";
        
        echo "<td align=right>$change_count</td>\n";
        echo "</tr>\n";
        
        }
    echo "</table>";


    echo "<hr>";

    // links for showing more of graph
    $intervalMap = array( "30" => "30 days",
                          "60" => "2 months",
                          "90" => "3 months",
                          "365" => "1 year",
                          "730" => "2 years",
                          "1825" => "5 years",
                          "36500" => "100 years",
                          "365000" => "1000 years",
                          "730000" => "2000 years" );

    $currentIntervalName = $intervalMap[ $graph_interval ];
    
    
    echo "<a name=\"graph\"></a>".
        "Changes $searchDisplay over the last $currentIntervalName:<br><br>";

    echo "<table border=1><tr><td><table border=0 cellpadding=10>";

    
    $query =
        "select DATE_FORMAT( change_date, '%Y-%M-%e' ), count(change_date) ".
        "from $tableNamePrefix"."flags ".
        "WHERE change_date > ".
        "DATE_SUB( CURDATE(), INTERVAL $graph_interval day ) ".
        "$keywordAndClause ".
        "GROUP BY DATE_FORMAT( change_date, 'Y-%M-%e' ) ".
        "ORDER BY change_date;";
    $result = fs_queryDatabase( $query );
        
    $numRows = mysql_numrows( $result );

    $maxCount = 0;

    for( $i=0; $i<$numRows; $i++ ) {
        $count = mysql_result( $result, $i, 1 );
        if( $maxCount < $count ) {
            $maxCount = $count;
            }
        }
    

    // find an appropriate bin size, given data span
    $binFactor = 1;
    
    while( $maxCount / $binFactor > 50 ) {
        $binFactor ++;
        }
    
        
    
    for( $i=0; $i<$numRows; $i++ ) {
        $formattedDate = mysql_result( $result, $i, 0 );
        $count = mysql_result( $result, $i, 1 );

        // bin the data
        $countBins = $count / $binFactor;
        
        
        echo "<tr><td>$formattedDate</td>".
            "<td bgcolor=\"#DDDDDD\" align=right>$count</td><td>|";
        for( $j=0; $j<$countBins; $j++ ) {
            echo "o";
            }
        echo "|</td></tr>\n";
        }
    if( $numRows == 0 ) {
        echo "<tr><td>none</td></tr>\n";
        }
    
    echo "</table></td></tr></table>\n";

    foreach( $intervalMap as $interval => $name ) {

        // hide link for current display
        if( $interval != $graph_interval ) {
            
            echo "[<a href=\"server.php?action=show_data" .
                "&skip=$skip&search=$search".
                "&order_by=$order_by".
                "&graph_interval=$interval#graph\">$name</a>] ";
            }
        }
        
    
    echo "<hr>";
    
    
    echo "<a href=\"server.php?action=show_log\">".
        "Show log</a>";
    echo "<hr>";
    echo "Generated for $remoteIP\n";

    }








// check if we should flush stale games from the database
// do this every 5 minutes
function fs_checkForFlush() {

    // FOR NOW:
    // no flush operation
    // but leave this stub here so we can add one easily later
    return;
    

    
    global $tableNamePrefix;

    $tableName = "$tableNamePrefix"."server_globals";
    
    if( !fs_doesTableExist( $tableName ) ) {
        return;
        }
    
    
    fs_queryDatabase( "SET AUTOCOMMIT = 0;" );

    
    $query = "SELECT last_flush_time FROM $tableName ".
       "WHERE last_flush_time < SUBTIME( CURRENT_TIMESTAMP, '0 0:05:0.000' ) ".
        "FOR UPDATE;";

    $result = fs_queryDatabase( $query );

    if( mysql_numrows( $result ) > 0 ) {

        // last flush time is old


        // trigger a flush
        

        /*
          example code from Between:
        
        global $tableNamePrefix;

        $gamesTable = $tableNamePrefix . "games";
        $columnsTable = $tableNamePrefix . "columns";
    

        // Games that haven't been fully joined yet
        //   need to be checked by the creator (player 1) every 5 seconds.
        // If they linger 20 seconds without being checked, they are ignored
        //   by strangers looking to join.
        // If they linger 40 seconds without being checked, we drop them.
        $query = "DELETE $gamesTable, $columnsTable ".
            "FROM $gamesTable JOIN $columnsTable ".
            "WHERE player_2_ready = '0' ".
            "AND $gamesTable.touch_date < ".
            "    SUBTIME( CURRENT_TIMESTAMP, '0 0:00:40.00' ) ".
            "AND $gamesTable.game_id = $columnsTable.game_id;";
    

        $result = fs_queryDatabase( $query );

        $numRowsRemoved = mysql_affected_rows();

        fs_log( "Flush operation on unstarted games removed $numRowsRemoved".
                " rows." );
        

        // games that have been joined / started can linger for a long
        // time with no server action (while players are thinking, etc).

        // also, players can quit playing and resume playing later
        // (hmmm... do we want to support this?)

        // for now, give them 2 hours of idle time

        $query = "DELETE $gamesTable, $columnsTable ".
            "FROM $gamesTable JOIN $columnsTable ".
            "WHERE player_2_ready = '1' ".
            "AND $gamesTable.touch_date < ".
            "    SUBTIME( CURRENT_TIMESTAMP, '0 2:00:0.00' ) ".
            "AND $gamesTable.game_id = $columnsTable.game_id;";
    

        $result = fs_queryDatabase( $query );


        $numRowsRemoved = mysql_affected_rows();

        fs_log( "Flush operation on started games removed $numRowsRemoved".
                " rows." );

        global $enableLog;
        
        if( $enableLog ) {
            // count remaining games for log
            $query = "SELECT COUNT(*) FROM $gamesTable;";

            $result = fs_queryDatabase( $query );

            $count = mysql_result( $result, 0, 0 );

            fs_log( "After flush, $count games left." );
                        }
        
        // set new flush time

        $query = "UPDATE $tableName SET " .
            "last_flush_time = CURRENT_TIMESTAMP;";
    
        $result = fs_queryDatabase( $query );
        */
    
        }

    fs_queryDatabase( "COMMIT;" );

    fs_queryDatabase( "SET AUTOCOMMIT = 1;" );
    }






// general-purpose functions down here, many copied from seedBlogs

/**
 * Connects to the database according to the database variables.
 */  
function fs_connectToDatabase() {
    global $databaseServer,
        $databaseUsername, $databasePassword, $databaseName;
    
    
    mysql_connect( $databaseServer, $databaseUsername, $databasePassword )
        or fs_fatalError( "Could not connect to database server: " .
                       mysql_error() );
    
	mysql_select_db( $databaseName )
        or fs_fatalError( "Could not select $databaseName database: " .
                       mysql_error() );
    }


 
/**
 * Closes the database connection.
 */
function fs_closeDatabase() {
    mysql_close();
    }



/**
 * Queries the database, and dies with an error message on failure.
 *
 * @param $inQueryString the SQL query string.
 *
 * @return a result handle that can be passed to other mysql functions.
 */
function fs_queryDatabase( $inQueryString ) {

    $result = mysql_query( $inQueryString )
        or fs_fatalError( "Database query failed:<BR>$inQueryString<BR><BR>" .
                       mysql_error() );

    return $result;
    }



/**
 * Checks whether a table exists in the currently-connected database.
 *
 * @param $inTableName the name of the table to look for.
 *
 * @return 1 if the table exists, or 0 if not.
 */
function fs_doesTableExist( $inTableName ) {
    // check if our table exists
    $tableExists = 0;
    
    $query = "SHOW TABLES";
    $result = fs_queryDatabase( $query );

    $numRows = mysql_numrows( $result );


    for( $i=0; $i<$numRows && ! $tableExists; $i++ ) {

        $tableName = mysql_result( $result, $i, 0 );
        
        if( $tableName == $inTableName ) {
            $tableExists = 1;
            }
        }
    return $tableExists;
    }



function fs_log( $message ) {
    global $enableLog, $tableNamePrefix;

    $slashedMessage = addslashes( $message );
    
    if( $enableLog ) {
        $query = "INSERT INTO $tableNamePrefix"."log VALUES ( " .
            "'$slashedMessage', CURRENT_TIMESTAMP );";
        $result = fs_queryDatabase( $query );
        }
    }



/**
 * Displays the error page and dies.
 *
 * @param $message the error message to display on the error page.
 */
function fs_fatalError( $message ) {
    //global $errorMessage;

    // set the variable that is displayed inside error.php
    //$errorMessage = $message;
    
    //include_once( "error.php" );

    // for now, just print error message
    $logMessage = "Fatal error:  $message";
    
    echo( $logMessage );

    fs_log( $logMessage );
    
    die();
    }



/**
 * Displays the operation error message and dies.
 *
 * @param $message the error message to display.
 */
function fs_operationError( $message ) {
    
    // for now, just print error message
    echo( "ERROR:  $message" );
    die();
    }


/**
 * Recursively applies the addslashes function to arrays of arrays.
 * This effectively forces magic_quote escaping behavior, eliminating
 * a slew of possible database security issues. 
 *
 * @inValue the value or array to addslashes to.
 *
 * @return the value or array with slashes added.
 */
function fs_addslashes_deep( $inValue ) {
    return
        ( is_array( $inValue )
          ? array_map( 'fs_addslashes_deep', $inValue )
          : addslashes( $inValue ) );
    }



/**
 * Recursively applies the stripslashes function to arrays of arrays.
 * This effectively disables magic_quote escaping behavior. 
 *
 * @inValue the value or array to stripslashes from.
 *
 * @return the value or array with slashes removed.
 */
function fs_stripslashes_deep( $inValue ) {
    return
        ( is_array( $inValue )
          ? array_map( 'sb_stripslashes_deep', $inValue )
          : stripslashes( $inValue ) );
    }



// this function checks the password directly from a request variable
// or via hash from a cookie.
//
// It then sets a new cookie for the next request.
//
// This avoids storing the password itself in the cookie, so a stale cookie
// (cached by a browser) can't be used to figure out the cookie and log in
// later. 
function fs_checkPassword( $inFunctionName ) {
    $password = "";
    $password_hash = "";

    $badCookie = false;
    
    
    global $accessPasswords, $tableNamePrefix, $remoteIP;

    $cookieName = $tableNamePrefix . "cookie_password_hash";

    
    if( isset( $_REQUEST[ "password" ] ) ) {
        $password = $_REQUEST[ "password" ];

        // generate a new hash cookie from this password
        $newSalt = time();
        $newHash = md5( $newSalt . $password );
        
        $password_hash = $newSalt . "_" . $newHash;
        }
    else if( isset( $_COOKIE[ $cookieName ] ) ) {
        $password_hash = $_COOKIE[ $cookieName ];
        
        // check that it's a good hash
        
        $hashParts = preg_split( "/_/", $password_hash );

        // default, to show in log message on failure
        // gets replaced if cookie contains a good hash
        $password = "(bad cookie:  $password_hash)";

        $badCookie = true;
        
        if( count( $hashParts ) == 2 ) {
            
            $salt = $hashParts[0];
            $hash = $hashParts[1];

            foreach( $accessPasswords as $truePassword ) {    
                $trueHash = md5( $salt . $truePassword );
            
                if( $trueHash == $hash ) {
                    $password = $truePassword;
                    $badCookie = false;
                    }
                }
            
            }
        }
    else {
        // no request variable, no cookie
        // cookie probably expired
        $badCookie = true;
        $password_hash = "(no cookie.  expired?)";
        }
    
        
    
    if( ! in_array( $password, $accessPasswords ) ) {

        if( ! $badCookie ) {
            
            echo "Incorrect password.";

            fs_log( "Failed $inFunctionName access with bad password" );
            }
        else {
            echo "Session expired.";
                
            fs_log( "Failed $inFunctionName access with bad cookie:  ".
                    "$password_hash" );
            }
        
        die();
        }
    else {
        // set cookie again, renewing it, expires in 24 hours
        $expireTime = time() + 60 * 60 * 24;
    
        setcookie( $cookieName, $password_hash, $expireTime, "/" );
        }
    }
 



function fs_clearPasswordCookie() {
    global $tableNamePrefix;

    $cookieName = $tableNamePrefix . "cookie_password_hash";

    // expire 24 hours ago (to avoid timezone issues)
    $expireTime = time() - 60 * 60 * 24;

    setcookie( $cookieName, "", $expireTime, "/" );
    }





/**
 * Filters a $_REQUEST variable using a regex match.
 *
 * Returns "" (or specified default value) if there is no match.
 */
function fs_requestFilter( $inRequestVariable, $inRegex, $inDefault = "" ) {
    if( ! isset( $_REQUEST[ $inRequestVariable ] ) ) {
        return $inDefault;
        }

    return fs_filter( $_REQUEST[ $inRequestVariable ], $inRegex, $inDefault );
    }


/**
 * Filters a value  using a regex match.
 *
 * Returns "" (or specified default value) if there is no match.
 */
function fs_filter( $inValue, $inRegex, $inDefault = "" ) {
    
    $numMatches = preg_match( $inRegex,
                              $inValue, $matches );

    if( $numMatches != 1 ) {
        return $inDefault;
        }
        
    return $matches[0];
    }


?>
