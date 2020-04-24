--TEST--
ffmpeg persistent movie test
--SKIPIF--
<?php extension_loaded('ffmpeg') or die("ffmpeg extension not loaded"); ?>
--FILE--
<?php
ini_set('ffmpeg.allow_persistent', '1');
$mov = new ffmpeg_movie(dirname(__FILE__) . '/test_media/robot.avi', 1);
$mov2 = new ffmpeg_movie(dirname(__FILE__) . '/test_media/robot.avi', 1);
printf("ffmpeg getDuration(): %0.2f\n", $mov->getDuration());
printf("ffmpeg getDuration(): %0.2f\n", $mov2->getDuration());
?>
--EXPECT--
ffmpeg getDuration(): 9.64
ffmpeg getDuration(): 9.64
