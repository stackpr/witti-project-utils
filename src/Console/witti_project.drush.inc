<?php
/*
 * This file is part of the Witti Project Utils package.
 *
 * (c) Greg Payne
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

use Witti\Utils\ArgValidate;
use Witti\Project\Console\Helper;
use Splash\Splash;

/**
 * Implementation of hook_drush_command().
 *
 * You can SYMLINK this file to any of the following:
 *     1. A .drush folder in your HOME folder.
 *     2. Anywhere in a folder tree below an active module on your site.
 *     3. /usr/share/drush/commands (configurable)
 *     4. In an arbitrary folder specified with the --include option.
 *     5. Drupal's /drush or /sites/all/drush folders.
 */
function witti_project_drush_command() {
  $items = array();
  $items['witti-project-build'] = array(
    'description' => 'Build a witti project.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(
      'project_dir' => 'The path of the project directory to build.',
    ),
  );
  $items['witti-project-replace'] = array(
    'description' => 'Replace text recursively through a project.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(
      'root_dir' => 'The path of the root directory in which to perform replacements (defaults to current directory).',
    ),
    'options' => array(
      'escape' => array(
        'description' => 'Convert special chars (\\n,\\r,\\t) in both regex and replacement.',
        'example-value' => 'nrt',
      ),
      'exclude-files' => array(
        'description' => 'Regular expression for which files to exclude.',
        'example-value' => '/Test\.php$/',
      ),
      'ext' => array(
        'description' => 'Pipe-delimited list of file extensions to consider.',
        'example-value' => 'php|inc (default)',
      ),
      'include-files' => array(
        'description' => 'Regular expression for which files to include.',
        'example-value' => '/php$/',
      ),
      'progress' => array(
        'description' => 'Display progress as files are checked.',
      ),
      'stats' => array(
        'description' => 'Display replacement summary after completing the replacements.',
      ),
      // Global options that should be shown for usability
      'simulate' => array(
        'description' => 'Simulate the replacement but do not make the replacement.',
        'example-value' => 'y',
      ),
    ),
    'examples' => array(
      'Simple replacement in current directory' => 'witti-project replace "/old/" "new"',
    ),
  );
  $items['witti-project-help'] = array(
    'description' => 'Display witti project help.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  return $items;
}

function witti_project_drush_init() {
  // Examine the current command and return quickly for anything not in this file.
  $cmd = drush_get_command();
  if ($cmd["commandfile"] !== 'witti_project') {
    return;
  }

  // Load ALL of the classes.
  require_once dirname(dirname(__DIR__)) . '/autoload.php.dist';
}

function drush_witti_project_build($build_dir = NULL) {
  if (!isset($build_dir)) {
    drush_print("USAGE: witti-project build [build_dir]");
    return;
  }
  if ($build_dir[0] !== '/') {
    $build_dir = drush_cwd() . '/' . $build_dir;
  }

  // Attempt to apply the license block.
  $lb = new \Witti\Project\Builder\Helper\FileLicenseBlock($build_dir);
  $php_files = new \Witti\Project\Builder\Files\BuilderFiles($build_dir);
  foreach ($php_files as $php_file) {
    drush_print($php_file);
    $lb->parseFile($php_file)->update();
  }
}

function drush_witti_project_replace($match = '', $replace = '', $root_dir = NULL) {
  if ($match === '') {
    drush_print("USAGE: witti-project replace '@preg@' 'replace' [root_dir|.]");
    return;
  }
  if (!isset($root_dir)) {
    $root_dir = drush_cwd();
  }
  elseif ($root_dir[0] !== '/') {
    $root_dir = drush_cwd() . '/' . $root_dir;
  }

  try {
    $match = ArgValidate::getRegexPattern($match);
  } catch (\Exception $e) {
    drush_print(dt('Error: ') . $e->getMessage());
    return;
  }

  // Build the list of files.
  $ext = trim(drush_get_option('ext'));
  $files = new \Witti\Project\Builder\Files\BuilderFiles($root_dir, $ext);
  $files = Splash::go()->append($files);

  // Apply additional options.
  $include = drush_get_option('include-files');
  if (strlen($include) >= 3) {
    $files = $files->regex($include);
  }
  $exclude = drush_get_option('exclude-files');
  if (strlen($exclude) >= 3) {
    $files = $files->inverseregex($exclude);
  }
  $files = $files->unique();

  // Adjust the handling of whitespace.
  $chars = drush_get_option('escape', 'nrt');
  foreach (str_split($chars, 1) as $char) {
    $char = "\\$char";
    $escaped = eval("return \"$char\";");
    $match = str_replace($char, $escaped, $match);
    $replace = str_replace($char, $escaped, $replace);
  }

  // Initialize the stats.
  $stats = array(
    'files.total' => array(
      'title' => 'Total Files',
      'stat' => 0,
    ),
    'files.match' => array(
      'title' => 'Matching Files',
      'stat' => 0,
    ),
    'replacements' => array(
      'title' => 'Total Replacements',
      'stat' => 0,
    ),
  );

  // Build the progress meter.
  $progress = NULL;
  $show_progress = drush_get_option('progress')
    && !drush_get_context('DRUSH_VERBOSE');
  if ($show_progress) {
    $progress = Helper::getProgressHelper($files->count());
  }
  foreach ($files as $file) {
    $stats['files.total']['stat']++;
    $msg = '';

    // Open the file and perform replacements.
    $count = 0;
    $dat = file_get_contents($file);
    $dat = preg_replace($match, $replace, $dat, -1, $count);
    if ($count != 0) {
      if (!drush_get_context('DRUSH_SIMULATE')) {
        file_put_contents($file, $dat);
      }
      $stats['files.match']['stat']++;
      $stats['replacements']['stat'] += $count;
    }

    // Progress update.
    if (isset($progress)) {
      $progress->advance();
    }
    elseif (drush_get_context('DRUSH_VERBOSE')) {
      drush_print($file);
      if (!empty($msg)) {
        drush_print($msg, 3);
      }
    }
  }
  if (isset($progress)) {
    $progress->finish();
  }


  $show_stats = drush_get_option('stats');
  if ($show_stats) {
    foreach ($stats as $stat) {
      drush_print(sprintf('%20s: %s', $stat['title'], $stat['stat']));
    }
  }
}

function drush_witti_project_help($help_topic = NULL) {
  switch ($help_topic) {
    default:
      drush_print("Use this command to access help topics about Witti project utils.");
      break;
  }
}
