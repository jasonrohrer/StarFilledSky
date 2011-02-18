<?php

$donation = "0.00";
if( isset( $_REQUEST[ "donation" ] ) ) {
    $donation = $_REQUEST[ "donation" ];
    }

$donationInCents = round( 100 * $donation );



if( $donationInCents < 0 ) {    
    $donationInCents = 0;
    }

// FastSpring tags must be >= 1
// add 1 here, and subtract it later in FastSpring script

$donationInCents += 1;



header( "Location:https://sites.fastspring.com/jasonrohrer/instant/starfilledskyticket?tags=donation_amount=$donationInCents");



?>