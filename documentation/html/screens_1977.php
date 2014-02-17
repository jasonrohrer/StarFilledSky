<html>

<head>
<title>Inside a Star-filled Sky</title>
</head>

<body bgcolor="#333333" text="#FFFFFF"
      link="#FD8109" alink="#ffff00" vlink="#FD8109">

<center>



<?php

$screenFiles = glob( "screens_1977/screen*.png" );

foreach( $screenFiles as $f ) {
    echo "<img width=\"640\" height=\"480\" src=\"$f\"><br><br><br><br>\n";
    }
?>

<script src="//static.getclicky.com/js" type="text/javascript"></script>
<script type="text/javascript">try{ clicky.init(100708526); }catch(e){}</script>
<noscript><p><img alt="Clicky" width="1" height="1" src="//in.getclicky.com/100708526ns.gif" /></p></noscript> 


<?php include( "footer.php" ); ?>
