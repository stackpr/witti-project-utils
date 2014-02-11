<?php
/*
 * This file is part of the Witti Project Utils package.
 *
 * (c) Greg Payne
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

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
  if ($build_dir{0} !== '/') {
    $build_dir = drush_cwd() . '/' . $build_dir;
  }

  // Attempt to apply the license block.
  $lb = new \Witti\Project\Builder\Helper\FileLicenseBlock($build_dir);
  $php_files = new \Witti\Project\Builder\Files\BuilderPhpFiles($build_dir);
  foreach ($php_files as $php_file) {
    drush_print($php_file);
    $lb->parseFile($php_file)->update();
  }
}

function drush_witti_project_help($help_topic = NULL) {
  switch ($help_topic) {
    default:
      drush_print("Use this command to access help topics about Witti project utils.");
      break;
  }
}