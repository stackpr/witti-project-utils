<?php
namespace Witti\Project\Console;

class Helper {
  /**
   * @return ProgressHelper
   */
  static public function getProgressHelper($total) {
    $output = new \Symfony\Component\Console\Output\ConsoleOutput;
    $progress = new \Symfony\Component\Console\Helper\ProgressHelper;
    $progress->setBarCharacter('<comment>=</comment>');
    $progress->setProgressCharacter('|');
    $progress->setEmptyBarCharacter(' ');
    $progress->setBarWidth(50);
    $progress->start($output, $total);
    return $progress;
  }

}