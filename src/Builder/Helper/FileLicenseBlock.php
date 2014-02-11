<?php
/*
 * This file is part of the Witti Project Utils package.
 *
 * (c) Greg Payne
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace Witti\Project\Builder\Helper;

class FileLicenseBlock {
  protected $mode = 'php';
  protected $root = NULL;
  protected $block = NULL;
  protected $path = NULL;
  protected $file = NULL;
  protected $file_parsed = NULL;

  public function __construct($root) {
    $this->root = $root;
  }

  public function parseFile($path) {
    // Load the appropriate block.
    $ext = pathinfo($path, PATHINFO_EXTENSION);
    $mode = $ext;
    if (strpos(' inc ', $mode) !== FALSE) {
      $mode = 'php';
    }
    if ($mode !== $this->mode || !isset($this->block)) {
      $this->mode = $mode;
      $block_path = $this->root . '/build/tpl/license.' . $mode . '.twig';
      if (is_file($block_path)) {
        $this->block = trim(file_get_contents($block_path));
        if (substr($this->block, 0, 3) === '/**') {
          throw new \ErrorException("The license block must NOT start with /**. Eliminate the second asterisk.");
        }
      }
      else {
        $this->block = '';
      }
    }

    // Store the info and parse the file.
    $this->path = $path;
    $this->file = file_get_contents($this->path);
    switch ($this->mode) {
      case 'php':
        $parsed = array(
          $this->file,
          NULL,
          NULL
        );
        if (substr($this->file, 0, 5) !== '<?php') {
          // Skip this file.
        }
        else {
          $parsed[0] = '<?php';
          $data = trim(substr($this->file, 5));
          if (substr($data, 0, 2) === '/*' && $data{2} !== '*') {
            $tmp = explode('*/', $data, 2);
            $parsed[1] = $tmp[0] . '*/';
            $parsed[2] = trim($tmp[1]);
          }
          else {
            $parsed[2] = $data;
          }
        }
        $this->file_parsed = $parsed;
        break;

      default:
        $this->file_parsed = array(
          $this->file,
        );
        break;
    }

    return $this;
  }

  public function update() {
    if (strlen($this->file_parsed[0]) == 5) {
      $parts = $this->file_parsed;
      $parts[1] = $this->block . "\n";
      $data = implode("\n", $parts);
      if (!is_file($this->path) || !is_writable($this->path)) {
        throw new \ErrorException("Unable to update path: " . $this->path);
      }
      file_put_contents($this->path, $data);
    }
    return $this;
  }
}