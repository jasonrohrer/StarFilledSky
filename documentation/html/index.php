<?php

// no caching
header('Cache-Control: no-store, no-cache, must-revalidate');
header('Cache-Control: post-check=0, pre-check=0', FALSE);
header('Pragma: no-cache');

error_reporting( E_ALL );


$pathToRoot = "";

include( "header.php" );


$deadline = "2011-03-13 20:59:59";
//$deadline = "2011-03-10 23:59:59";





function getTimeLeft() {
    global $deadline;
    
    
    date_default_timezone_set( "America/New_York" );

    $deadlineTimestamp = strtotime( $deadline );

    $currentTimestamp = time();

    
    if( $currentTimestamp < $deadlineTimestamp ) {
        return $deadlineTimestamp - $currentTimestamp;
        }
    else {
        return 0;
        }
    }



function showTimeLeft( $inTimeLeft ) {
            
    $d = $inTimeLeft;

    $hours = (int)( $d / 3600 );

    $seconds = (int)( $d % 3600 );
    $minutes = (int)( $seconds / 60 );
    $seconds = (int)( $seconds % 60 );
            
    $days = (int)( $hours / 24 );
    $hours = (int)( $hours % 24 );
            
        
    echo "Only <b>";

    if( $days > 1 ) {
        echo "$days days";
        }
    else {
        $hours += 24 * $days;
            
        if( $hours > 1 ) {
            echo "$hours hours";
            }
        else {
            $minutes += 60 * $hours;

            if( $minutes > 1 ) {
                echo "$minutes minutes";
                }
            else {
                $seconds += 60 * $minutes;

                echo "$seconds seconds";
                }
            }
        }
    echo "</b> left to name your donation";
    }







?>





<?php

function showDownloadForm() {
    ?>
      Access your download:<br>
            <FORM ACTION="ticketServer/server.php" METHOD="post">
    <INPUT TYPE="hidden" NAME="action" VALUE="show_downloads">
      Enter Code:   
    <INPUT TYPE="text" NAME="ticket_id" MAXLENGTH=10 SIZE=10>
    <INPUT TYPE="Submit" VALUE="Next">
    </FORM>
      
<?php
    }



function showPayLinks( $inSimple ) {
    $referring_page = "";
    
    if( isset( $_SERVER['HTTP_REFERER'] ) ) {

        $numMatches = preg_match(
            "#(http|https)://([\w-]+\.)+[\w-]+(/[\w- ./?%&=]*)?#",
            $_SERVER['HTTP_REFERER'], $matches );

        $url = "";
        
        if( $numMatches == 1 ) {
            $url = $matches[0];
            }
            
        
        $referring_page = urlencode( $url );
        }
    
    

    ?>
 <center>
      <center><table border=0><tr><td> 
<ul> 
      <li>Unlimited downloads
      <li>Access to all future updates
      <li>Tech support included
      <li>Support me and my family directly<br>(so I can make more games)
      </ul>
</td></tr></table>

      <font size=5>Available now for $12</font><br><br>
      
      <a href="https://sites.fastspring.com/jasonrohrer/instant/starfilledskyticket?referrer=<?php echo $referring_page;?>">
      <img src="fs_cards.png" width=280 height=45 border=0><?php
      if( !$inSimple ) {

          echo "<br>";
          
          echo "<img src=\"fs_button05.png\" width=210 height=60 border=0>";
          }
?>
      </a>
      </center>
<?php
    }



function showPayForm( $inSimple ) {
    ?>
<center><table border=0><tr><td> 
<ul> 
      <li>Unlimited downloads
      <li>Access to all future updates
      <li>Tech support included
      <li>Support me and my family directly<br>(so I can make more games)
      </ul>
</td></tr></table></center>                                          
      <font size=6>What's it worth to you?</font><br><br>                           
 <center>
      
      <form action="nameYourDonation.php" METHOD="post">
      <table border=0>
      <tr><td>Name your donation:</td><td>$</td><td><input type="text" name="donation" VALUE="10.00" SIZE=7 MAXLENGTH=20></td></tr>
      <tr><td align=right>+</td><td>$</td><td>1.75</td></tr>
      </table>
      <br>

      <input type="image" src="fs_cards.png" width=280 height=45 title="Buy Downloads and Donate"><br>
      <input type="image" src="fs_button05.png" width=210 height=60 title="Buy Downloads and Donate">
      </form>
      </center>
<?php
    }



function showLogo( $inImageFile, $inText ) {

    echo "<table border=0><tr><td align=center>
          <img src=\"$inImageFile\"><br>
          <font size=1>$inText</font>
          </td></tr></table>";
    }


?>


<center>
<font size=6>Inside a Star-filled Sky</font><br>
an infinite, recursive, tactical shooter for one player by
<a href="http://hcsoftware.sf.net/jason-rohrer">Jason Rohrer</a><br>
<br>

[<a href="intro.php">intro</a>] --
[<a href="screens.php">screenshots</a>] --
[<a href="bulletPoints.php">bullet points</a>] -- 
[<a href="reviews.php">reviews</a>] 
</center>



<br>
<br>

<center>

<font size=6>9/10, Editor's Choice</font><br>
<font size=5>"A beautiful, violent crumb from infinity's table"</font> --<a href="http://pc.ign.com/articles/115/1154253p1.html">IGN Review</a>

    <br>
    <br>
<br>


<font size=4>
"Eloquently translates the incomprehensible concept of infinity into game form"</font><br>
<font size=5>"Alternately maddening and sublime"</font> --<a href="http://www.gamepro.com/article/reviews/218679/indie-of-the-week-inside-a-star-filled-sky-review/">GamePro Review</a>
<br>
<br>
    
<font size=5>
"It really opens up at the higher levels into something amazing."</font><br>
--Frank Lantz [<a href="lantzQuote.php">read full quote</a>]
<br>
<br>

<font size=5>
"Remarkably tactical"</font> --Alex Meer, <a href="http://www.rockpapershotgun.com/2011/02/23/impressions-inside-a-star-filled-sky/">RPS First Impressions</a>
<br>
<br>

<font size=5>
"Extremely clever"</font>
--Graham Smith, <a href="http://www.pcgamer.com/2011/02/13/inside-a-star-filled-sky-preview/">PC Gamer Preview</a><br><br>
<br>

<!--
<font size=5>
"Cripplingly deep"</font>
--Andrew Webster <a href="http://arstechnica.com/gaming/reviews/2011/02/universe-too-large-ars-reviews-indie-pc-inside-a-star-filled-sky.ars?comments=1">Ars Technica Review</a>
<br>
<br>
-->



<iframe title="YouTube video player" width="640" height="390" src="http://www.youtube.com/embed/pQaIAhHJvAw?rel=0" frameborder="0" allowfullscreen></iframe>

<!--
old youtube code:
<object width="640" height="390"><param name="movie" value="http://www.youtube.com/v/pQaIAhHJvAw?fs=1&amp;hl=en_US&amp;rel=0"></param><param name="allowFullScreen" value="true"></param><param name="allowscriptaccess" value="always"></param><embed src="http://www.youtube.com/v/pQaIAhHJvAw?fs=1&amp;hl=en_US&amp;rel=0" type="application/x-shockwave-flash" allowscriptaccess="always" allowfullscreen="true" width="640" height="390"></embed></object>
-->

</center>
<br>


<br>


    <?php /*

<a name="order"></a>
<font size=5>Name Your Donation (for one last weekend)</font><br>
<table border=0 width="100%" cellpadding=5><tr><td bgcolor="#222222">

<center>
<font size=5 color=red>
<?php

$timeLeft = getTimeLeft();
if( $timeLeft > 0 ) {
    
    showTimeLeft( $timeLeft );
    }
else {
    echo "Name Your Donation is about to end...";
    }

?>
</font>
</center>


<!--<font color="#ffff00"><i>For a limited time only</i></font>-->
<br>

You can download the game right now for $1.75 plus <b>whatever donation you can afford to give</b>.<br>
<br>

The minimum price of $1.75 covers payment processing fees and download bandwidth.

</td></tr></table>

          */
?>

    
<br>

<center>
<table border=0><tr>
<td><?php showLogo( "noDRM.png", "No DRM" ); ?></td>
<td><?php showLogo( "noTie.png", "No middle-person" ); ?></td>
<td><?php showLogo( "crossPlatform.png", "Cross-platform" ); ?></td>
<td><?php showLogo( "openSource.png", "Open Source" ); ?></td>
</tr></table>                                  
</center>                          
                                  

<center>
<table border=0 cellpadding=2><tr><td bgcolor="#222222">
<table border=0 cellpadding=5><tr><td bgcolor="#000000">
<center>                         
<font size=5 color=red>
<?php
/*
$timeLeft = getTimeLeft();
if( $timeLeft > 0 ) {
    
    showTimeLeft( $timeLeft );
    }
else {
    echo "Pre-Orders are now closed";
    }
*/
?>
</font>
</center>

<!--
<center>
<font color=red size=5>Having some trouble with the payment system.  Hold on a minute....</font>
</center>
-->

<?php
   showPayLinks( false );
   // showPayForm( false );                                  

/*
if( $timeLeft > 0 ) {
    echo "<br>";
    // only credit card button
    showPayLinks( false );
    }
else {
    //echo "<br>";
    //echo "<center>Downloads will be availble soon.</center>";
    
    showDownloadForm();

    echo "<br><center><font size=5>Orders for April 16 are open</font></center>";

    echo "<br>";
    // only credit card button
    showPayLinks( false );
    }
*/
?>
</td></tr></table>
</td></tr></table>
</center>
<br>
<br>


<font size=5>What you get</font><br>

<table border=0 width="100%" cellpadding=5><tr><td bgcolor="#222222">
Immediately after your payment is processed, you will receive an email with an access link.  You will then be able to download all of the following DRM-free distributions:
<center>
<table border=0><tr><td>
<ul>
<li>Windows build</li>
<li>MacOS build (10.2 and later, PPC/Intel)</li>
<li>Full source code bundle (which can also be compiled on GNU/Linux)</li>
</ul>
</td></tr></table>
</center>
The price also includes downloads of all future updates.<br>
<br>
You can take a look at the <a href="requirements.php">system requirements</a>.
</td></tr></table>
<br>
<br>

<center>
<table border=0 cellpadding=2><tr><td bgcolor="#222222">
<table border=0 cellpadding=5><tr><td bgcolor="#000000">
<center> 
<?php
    showDownloadForm();
?>
</center>
</td></tr></table>
</td></tr></table>
</center>

<font size=5>Credits</font><br>

<table border=0 width="100%" cellpadding=5><tr><td bgcolor="#222222">
I was able to spend eight months conceiving and working on this game because of all the generous people who bought downloads of <a href="http://sleepisdeath.net">Sleep is Death</a>.
<br>
<br>
Development was also made possible by the support of Jeff Roberts.
<br>
<br>
All design, programming, graphics, fonts, and sound by Jason Rohrer.<br>
<br>

The graphics were made with <a href="http://mtpaint.sourceforge.net/">mtPaint</a>. The <a href="http://www.libsdl.org/">SDL</a> library provides cross-platform screen, sound, and user input.  <a href="http://www.libpng.org/pub/png/libpng.html">libpng</a> and <a href="http://www.zlib.net/">zlib</a> enable PNG output.  <a href="http://www.mingw.org/">MinGW</a> was used to build the game for Windows.<br>
<br>
I am deeply indebted to the following testers who invested hundreds of hours plumbing the depths of this infinite beastie and took the time to send me feedback:<br>
<br>

Alexander Bruce,
Alexey Zubkov,
Allen,
Aloshi Aloshied,
Andrew McClure,
Ben Leatherman,
Bennett Foddy,
Chris Klimas,
Christian Knudsen,
Daniel Benmergui,
Daniel Stubbs,
Dave Evans,
Destral Minare,
Eric McQuiggan,
Farbs,
Guilherme S. Töws,
Ian Snyder,
Jason Tam,
Jasper Byrne,
Jimmy Andrews,
John Nesky,
Lauren Serafin,
Liam,
Madeleine Burleson,
Manuel Magalhães,
Mark Johns,
Michael Brough,
Michael Molinari,
Miroslav Malesevic,
Nate Kling,
Nicholas Feinberg,
Nicholas Rodine,
Quicksand Games,
Richard Lemarchand,
Rico,
Rod Humble,
Salade,
Stephen Lavelle,
Will Olthouse,
William Broom

</td></tr></table>
<br>

</center>


<?php include( "footer.php" ); ?>
