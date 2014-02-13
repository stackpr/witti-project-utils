<?php
/*
 * This file is part of the Witti Project Utils package.
 *
 * (c) Greg Payne
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace Witti\Project\Builder\Files;

class BuilderFiles extends \FilterIterator {
  protected $root;
  protected $stats = array(
    'added' => 0,
    'updated' => 0,
    'skipped' => 0,
  );
  public function __construct($root, $ext = 'php|inc') {
    $this->root = $root;
    $it = new \RecursiveDirectoryIterator($root, \FilesystemIterator::SKIP_DOTS);
    $it = new \RecursiveIteratorIterator($it);
    $it = new \RegexIterator($it, "@\.(?:$ext)$@s");
    $regex = '';
    parent::__construct($it);
  }

  public function accept() {
    $test = substr($this->current(), strlen($this->root));
    if (preg_match("@(?:/\.git|/\.svn/|^/vendor/)@s", $test) || !is_file($this->current())) {
      return FALSE;
    }
    return TRUE;
  }

}