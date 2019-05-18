<?php

$license = "CNDP3POOLEANGUS";

$emails = array("admin@fanshaweonline.ca", "angus@fanshaweonline.ca", "oscar@fanshaweonline.ca");
$emailCount = count($emails);
$passwords = array("pass!word0", "p001e", "0sc4r");
$passCount = count($passwords);

$email = null;
$pass = null;

if($_SERVER["REQUEST_METHOD"] == "POST"){
    
    $args = array(
        'email' => FILTER_SANITIZE_EMAIL,
        'pass' => FILTER_SANITIZE_STRING
    );

    #echo "hello";
    #$inputs = filter_input_array(INPUT_POST, $args);
    #$email = $inputs[0];
    #$pass = $inputs[1];
    $email = filter_input(INPUT_POST, "email", FILTER_SANITIZE_EMAIL);
    $pass = filter_input(INPUT_POST, "pass");

    #echo $email;
    #echo "<br/>";
    #echo $pass;

    if(!empty($email) && !empty($pass))
    {
        #echo $email;
        #echo "<br/>";
        #echo $pass;

        for($i = 0; $i < $emailCount; $i++) {
            if($email == $emails[$i])
            {
                if($pass == $passwords[$i])
                {
                    echo $license;
                    return;
                }
            }
        }
        echo "INVALID";
        return;
    }
    else {
        echo $email;
        echo "Error: missing an email or password!";
        return;
    }
}
echo "sorry!";