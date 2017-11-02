<?php
    include('espaperjson.php');

    date_default_timezone_set("Europe/Zurich");

    $output = $_GET["output"];
    if (!isset($output) || $output === "json") {
      $isJson = true;
    } else {
      $isJson = false;
    }
    if ($isJson) {
    	header('Content-Type: application/json');
    } else {
      header('Content-Type: image/svg+xml');
    }
  	$battery = $_GET['battery'] / 1024.0;
  	$voltage = round(($battery + 0.083) * 13 / 3.0, 2);
  	if ($voltage > 4.2) {
  		$percentage = 100;
  	} else if ($voltage < 3) {
  		$percentage = 0;
  	} else {
  		$percentage = round(($voltage - 3.2) * 100 / (4.2 - 3.0));
  	}

    $canvas = new Canvas();

    $canvas->setJson($isJson);
    if ($isJson) {
      echo "{ \"meta\": { \"height\": 128, \"width\": 296 }, \"commands\": [";
    } else {
      echo "<svg width=\"296px\" height=\"128px\" viewBox=\"0 0 296 128\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">";
      $canvas->meteocons();
      $canvas->moonphases();
    }
    //$canvas->downloadFile("http://www.squix.org/blog/espaper/SquixLogo.png.bin", "SquixLogo.png.bin", 0);
    //$canvas->downloadFile("http://www.squix.org/blog/espaper/sunny.bmp.bin", "sunny.bmp.bin", 0);
    $canvas->setColor(0);
    $canvas->downloadFile("http://www.squix.org/blog/espaper/TimesRegular30.mxf", "TimesRegular30.mxf", 0);
		$canvas->fillBuffer(1);
		$canvas->setFont("ArialMT_Plain_10");
		$canvas->setTextAlignment("LEFT");
		$canvas->drawString(2, -2, "Updated: ".date("Y-m-d H:i:s"));
		$canvas->drawLine(0, 11, 400, 11);
		$canvas->setTextAlignment("RIGHT");
		$canvas->drawString(374, -1, $voltage. "V ".$percentage."%");
		$canvas->drawRect(374, 0, 18, 10);
		$canvas->drawRect(393, 2, 1, 6);
		$canvas->fillRect(376, 2, round(14 * $percentage / 100), 6);
    $canvas->setFontFile("TimesRegular30.mxf");
    $canvas->setTextAlignment("LEFT");
    $canvas->drawString(0, 20, "Hello world");
    //$canvas->drawImage(100, 20, "SquixLogo.png.bin");
    //$canvas->drawImage(10, 64, "sunny.bmp.bin");
    //$canvas->drawBmpFromFile(0, 12, "cross.bmp");


    $canvas->setFont("ArialMT_Plain_10");
		$canvas->setTextAlignment("CENTER");
		$canvas->drawString(48, 116, "CONFIG+RST");
		$canvas->drawString(245, 116, "UPDATE");
		$canvas->drawLine(0, 116, 296, 116);
		$canvas->drawLine(98, 116, 98, 128);
		$canvas->drawLine(196, 116, 196, 128);
		$canvas->commit();
    if ($isJson) {
      echo "]}";
    } else {
      echo "</svg>";
    }

		?>
