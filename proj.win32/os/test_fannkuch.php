<?php

function fannkuch($n)
{
  $p = array();
  $q = array();
  $s = array();
  $sign = 1;
  $maxflips = $sum = 0;
  for($i=1; $i<=$n; $i++) $p[$i] = $q[$i] = $s[$i] = $i;
  for(;;){
    // Copy and flip.
    $q1 = $p[1];				// Cache 1st element.
    if($q1 != 1){
      for($i=2; $i<=$n; $i++) $q[$i] = $p[$i];		// Work on a copy.
      $flips = 1;
      for(;;){
		$qq = $q[$q1];
		if($qq == 1){				// ... until 1st element is 1.
		  $sum += $sign*$flips;
		  if($flips > $maxflips){
			$maxflips = $flips;
		  } // New maximum?
		  break;
		}
		$q[$q1] = $q1;
		if($q1 >= 4){
		  $i = 2; $j = $q1 - 1;
		  for(;;){ $tmp = $q[$i]; $q[$i] = $q[$j]; $q[$j] = $tmp; if(++$i >= --$j) break; }
		}
		$q1 = $qq; $flips++;
      }
    }
    // Permute.
    if($sign == 1){
      $tmp = $p[2]; $p[2] = $p[1]; $p[1] = $tmp; $sign = -1;	// Rotate 1<-2.
    }else{
      $tmp = $p[2]; $p[2] = $p[3]; $p[3] = $tmp; $sign = 1;	// Rotate 1<-2 and 1<-2<-3.
      for($i = 3;; $i++){
		$sx = $s[$i];
		if($sx != 1){ $s[$i] = $sx-1; break; }
		if($i == $n) return array($sum, $maxflips);	// Out of permutations.
		$s[$i] = $i;
		// Rotate 1<-...<-i+1.
		$t = $p[1]; for($j = 1; $j <= $i; $j++){ $p[$j] = $p[$j+1]; } $p[$i+1] = $t;
      }
    }
  }
}

function getTimeSec(){
    list($usec, $sec) = explode(" ",microtime());
    return ($usec + $sec);
}

$n = isset($argv[1]) ? $argv[1] : 5;
echo "n: $n\n";
$start_time = getTimeSec();
$r = fannkuch($n);
$sum = $r[0]; $flips = $r[1];
echo("$sum\nPfannkuchen($n) = $flips\n"
	. "time = ".(getTimeSec() - $start_time)."\n");

