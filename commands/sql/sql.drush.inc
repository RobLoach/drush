<?php

/**
 * @file
 * Drush sql commands
 */

/**
 * Implementation of hook_drush_help().
 */
function sql_drush_help($section) {
  switch ($section) {
    case 'meta:sql:title':
      return dt('SQL commands');
    case 'meta:sql:summary':
      return dt('Examine and modify your Drupal database.');
    case 'drush:sql-sanitize':
      return dt('Run sanitization operations on the current database. You can add more sanitization to this command by implementing hook_drush_sql_sync_sanitize().');
  }
}

/**
 * Implementation of hook_drush_command().
 */
function sql_drush_command() {
  $options['database'] = array(
    'description' => 'The DB connection key if using multiple connections in settings.php.',
    'example-value' => 'key',
  );
  $db_url['db-url'] = array(
    'description' => 'A Drupal 6 style database URL.',
    'example-value' => 'mysql://root:pass@127.0.0.1/db',
  );
  $options['target'] = array(
    'description' => 'The name of a target within the specified database connection. Defaults to \'default\'.',
    'example-value' => 'key',
    // Gets unhidden in help_alter(). We only want to show this to D7 users but have to
    // declare it here since some commands do not bootstrap fully.
    'hidden' => TRUE,
  );

  $items['sql-drop'] = array(
    'description' => 'Drop all tables in a given database.',
    'arguments' => array(
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
    'options' => array(
      'yes' => 'Skip confirmation and proceed.',
      'result-file' => array(
        'description' => 'Save to a file. The file should be relative to Drupal root. Recommended.',
        'example-value' => '/path/to/file',
      ),
    ) + $options + $db_url,
    'topics' => array('docs-policy'),
  );
  $items['sql-conf'] = array(
    'description' => 'Print database connection details using print_r().',
    'hidden' => TRUE,
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
    'options' => array(
      'all' => 'Show all database connections, instead of just one.',
      'show-passwords' => 'Show database password.',
    ) + $options,
    'outputformat' => array(
      'default' => 'print-r',
      'pipe-format' => 'var_export',
      'private-fields' => 'password',
    ),
  );
  $items['sql-connect'] = array(
    'description' => 'A string for connecting to the DB.',
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
    'options' => $options + $db_url + array(
        'extra' => array(
          'description' => 'Add custom options to the mysql command.',
          'example-value' => '--skip-column-names',
        ),
      ),
    'examples' => array(
      '`drush sql-connect` < example.sql' => 'Import sql statements from a file into the current database.',
    ),
  );
  $items['sql-create'] = array(
    'description' => 'Create a database.',
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
    'examples' => array(
      'drush sql-create' => 'Create the database for the current site.',
      'drush @site.test sql-create' => 'Create the database as specified for @site.test.',
      'drush sql-create --db-su=root --db-su-pw=rootpassword --db-url="mysql://drupal_db_user:drupal_db_password@127.0.0.1/drupal_db"' =>
        'Create the database as specified in the db-url option.'
    ),
    'options' => array(
      'db-su' => 'Account to use when creating a new database. Optional.',
      'db-su-pw' => 'Password for the "db-su" account. Optional.',
    ) + $options + $db_url,
  );
  $items['sql-dump'] = array(
    'description' => 'Exports the Drupal DB as SQL using mysqldump or equivalent.',
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
    'examples' => array(
      'drush sql-dump --result-file=../18.sql' => 'Save SQL dump to the directory above Drupal root.',
      'drush sql-dump --skip-tables-key=common' => 'Skip standard tables. @see example.drushrc.php',
    ),
    'options' => array(
      'result-file' => array(
        'description' => 'Save to a file. The file should be relative to Drupal root. If --result-file is provided with no value, then date based filename will be created under ~/drush-backups directory.',
        'example-value' => '/path/to/file',
        'value' => 'optional',
      ),
      'skip-tables-key' => 'A key in the $skip_tables array. @see example.drushrc.php. Optional.',
      'structure-tables-key' => 'A key in the $structure_tables array. @see example.drushrc.php. Optional.',
      'tables-key' => 'A key in the $tables array. Optional.',
      'skip-tables-list' => 'A comma-separated list of tables to exclude completely. Optional.',
      'structure-tables-list' => 'A comma-separated list of tables to include for structure, but not data. Optional.',
      'tables-list' => 'A comma-separated list of tables to transfer. Optional.',
      'create-db' => array('hidden' => TRUE, 'description' => 'Omit DROP TABLE statements. Postgres and Oracle only.  Used by sql-sync, since including the DROP TABLE statements interfere with the import when the database is created.'),
      'data-only' => 'Dump data without statements to create any of the schema.',
      'ordered-dump' => 'Order by primary key and add line breaks for efficient diff in revision control. Slows down the dump. Mysql only.',
      'gzip' => 'Compress the dump using the gzip program which must be in your $PATH.',
    ) + $options + $db_url,
  );
  $items['sql-query'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
    'description' => 'Execute a query against a database.',
    'examples' => array(
      'drush sql-query "SELECT * FROM users WHERE uid=1"' => 'Browse user record. Table prefixes, if used, must be added to table names by hand.',
      'drush sql-query --db-prefix "SELECT * FROM {users} WHERE uid=1"' => 'Browse user record. Table prefixes are honored.  Caution: curly-braces will be stripped from all portions of the query.',
      '`drush sql-connect` < example.sql' => 'Import sql statements from a file into the current database.',
      'drush sql-query --file=example.sql' => 'Alternate way to import sql statements from a file.',
    ),
    'arguments' => array(
       'query' => 'An SQL query. Ignored if \'file\' is provided.',
    ),
    'options' => array(
      'result-file' => array(
        'description' => 'Save to a file. The file should be relative to Drupal root. Optional.',
        'example-value' => '/path/to/file',
      ),
      'file' => 'Path to a file containing the SQL to be run. Gzip files are accepted.',
      'extra' => array(
        'description' => 'Add custom options to the mysql command.',
        'example-value' => '--skip-column-names',
      ),
      'db-prefix' => 'Enable replacement of braces in your query.',
      'db-spec' => array(
        'description' => 'A database specification',
        'hidden' => TRUE, // Hide since this is only used with --backend calls.
      )
    ) + $options + $db_url,
    'aliases' => array('sqlq'),
  );

  $sanitize_options = array(
    'sanitize-password' => array(
      'description' => 'The password to assign to all accounts in the sanitization operation, or "no" to keep passwords unchanged.',
      'example-value' => 'password',
      'value' => 'required',
    ),
    'sanitize-email' => array(
      'description' => 'The pattern for test email addresses in the sanitization operation, or "no" to keep email addresses unchanged.  May contain replacement patterns %uid, %mail or %name.',
      'example-value' => 'user+%uid@localhost',
      'value' => 'required',
    ),
  );
  $items['sql-sync'] = array(
    'description' => 'Copies the database contents from a source site to a target site. Transfers the database dump via rsync.',
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
    'drush dependencies' => array('core'), // core-rsync.
    'examples' => array(
      'drush sql-sync @source @target' => 'Copy the database from the site with the alias "source" to the site with the alias "target".',
      'drush sql-sync prod dev' => 'Copy the database from the site in /sites/prod to the site in /sites/dev (multisite installation).',
    ),
    'arguments' => array(
      'source' => 'A site-alias or the name of a subdirectory within /sites whose database you want to copy from.',
      'target' => 'A site-alias or the name of a subdirectory within /sites whose database you want to replace.',
    ),
    'required-arguments' => TRUE,
    'options' => array(
      'skip-tables-key' => 'A key in the $skip_tables array. See example.drushrc.php. Optional.',
      'skip-tables-list' => 'A comma-separated list of tables to exclude completely. Optional.',
      'structure-tables-key' => 'A key in the $structure_tables array. See example.drushrc.php. Optional.',
      'structure-tables-list' => 'A comma-separated list of tables to include for structure, but not data. Optional.',
      'tables-key' => 'A key in the $tables array. Optional.',
      'tables-list' => 'A comma-separated list of tables to transfer. Optional.',
      // 'cache' => 'Skip dump if result file exists and is less than "cache" hours old. Optional; default is 24 hours.',
      // 'no-cache' => 'Do not cache the sql-dump file.',
      'no-dump' => 'Do not dump the sql database; always use an existing dump file.',
      'source-db-url' => 'Database specification for source system to dump from.',
      'source-remote-port' => 'Override sql database port number in source-db-url. Optional.',
      'source-remote-host' => 'Remote machine to run sql-dump file on. Optional; default is local machine.',
      'source-dump' => 'The destination for the dump file, or the path to the dump file when --no-dump is specified.',
      'source-database' => 'A key in the $db_url (D6) or $databases (D7+) array which provides the data.',
      'source-target' => array(
        'description' => 'A key within the SOURCE database identifying a particular server in the database group.',
        'example-value' => 'key',
        // Gets unhidden in help_alter(). We only want to show to D7+ users but have to
        // declare it here since this command does not bootstrap fully.
        'hidden' => TRUE,
      ),
      'target-db-url' => '',
      'target-remote-port' => '',
      'target-remote-host' => '',
      'target-dump' => 'A path for saving the dump file on target. Mandatory when using --no-sync.',
      'target-database' => 'A key in the $db_url (D6) or $databases (D7+) array which shall receive the data.',
      'target-target' => array(
          'description' => 'Oy. A key within the TARGET database identifying a particular server in the database group.',
          'example-value' => 'key',
          // Gets unhidden in help_alter(). We only want to show to D7+ users but have to
          // declare it here since this command does not bootstrap fully.
          'hidden' => TRUE,
      ),
      // 'temp' => 'Use a temporary file to hold dump files.  Implies --no-cache.',
      // 'dump-dir' => 'Directory to store sql dump files in when --source-dump or --target-dump are not used.',
      'create-db' => 'Create a new database before importing the database dump on the target machine.',
      'db-su' => array(
        'description' => 'Account to use when creating a new database. Optional.',
        'example-value' => 'root',
      ),
      'db-su-pw' => array(
        'description' => 'Password for the "db-su" account. Optional.',
        'example-value' => 'pass',
      ),
      // 'no-ordered-dump' => 'Do not pass --ordered-dump to sql-dump.  sql-sync orders the dumpfile by default in order to increase the efficiency of rsync.',
      'sanitize' => 'Obscure email addresses and reset passwords in the user table post-sync.',
    ),
    'sub-options' => array(
      'sanitize' => $sanitize_options + array(
        'confirm-sanitizations' => 'Prompt yes/no after importing the database, but before running the sanitizations',
      ),
    ),
    'topics' => array('docs-aliases', 'docs-policy', 'docs-example-sync-via-http', 'docs-example-sync-extension'),
  );
  $items['sql-cli'] = array(
    'description' => "Open a SQL command-line interface using Drupal's credentials.",
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
    // 'options' => $options + $db_url,
    'allow-additional-options' => array('sql-connect'),
    'aliases' => array('sqlc'),
    'examples' => array(
      'drush sql-cli' => "Open a SQL command-line interface using Drupal's credentials.",
      'drush sql-cli --extra=-A' => "Open a SQL CLI and skip reading table information.",
    ),
    'remote-tty' => TRUE,
  );
  $items['sql-sanitize'] = array(
    'description' => "Run sanitization operations on the current database.",
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
    'options' => $sanitize_options + array(
      'db-prefix' => 'Enable replacement of braces in sanitize queries.',
    ) + $db_url,
    'aliases' => array('sqlsan'),
  );
  return $items;
}

/**
 * Implements hook_drush_help_alter().
 */
function sql_drush_help_alter(&$command) {
  // Drupal 7+ only options.
  if (drush_drupal_major_version() >= 7) {
    if ($command['command'] == 'sql-sync') {
      unset($command['options']['source-target']['hidden'], $command['options']['target-target']['hidden']);
    }
    elseif ($command['commandfile'] == 'sql') {
      unset($command['options']['target']['hidden']);
    }
  }
}

/**
 * Command argument complete callback.
 *
 * @return
 *  Array of available site aliases.
 */
function sql_sql_sync_complete() {
  return array('values' => array_keys(_drush_sitealias_all_list()));
}

/**
 * Safely bootstrap Drupal to the point where we can
 * access the database configuration.
 */
function drush_sql_bootstrap_database_configuration() {
  // Under Drupal 7, if the database is configured but empty, then
  // DRUSH_BOOTSTRAP_DRUPAL_CONFIGURATION will throw an exception.
  // If this happens, we'll just catch it and continue.
  // TODO:  Fix this in the bootstrap, per http://drupal.org/node/1996004
  try {
    drush_bootstrap_max(DRUSH_BOOTSTRAP_DRUPAL_CONFIGURATION);
  }
  catch (Exception $e) {
  }
}

/**
 * Check whether further bootstrap is needed. If so, do it.
 */
function drush_sql_bootstrap_further() {
  if (!drush_get_option(array('db-url', 'db-spec'))) {
    drush_sql_bootstrap_database_configuration();
  }
}

/**
 * Command callback. Displays the Drupal site's database connection string.
 */
function drush_sql_conf() {
  drush_sql_bootstrap_database_configuration();
  if (drush_get_option('all')) {
    $sqlVersion = drush_sql_get_version();
    return $sqlVersion->getAll();
  }
  else {
    $sql = drush_sql_get_class();
    return $sql->db_spec();
  }
}

/**
 * Command callback. Emits a connect string.
 */
function drush_sql_connect() {
  drush_sql_bootstrap_further();
  $sql = drush_sql_get_class();
  return $sql->connect(FALSE);
}

/**
 * Command callback. Create a database.
 */
function drush_sql_create() {
  drush_sql_bootstrap_further();
  $sql = drush_sql_get_class();
  $db_spec = $sql->db_spec();
  // Prompt for confirmation.
  if (!drush_get_context('DRUSH_SIMULATE')) {
    // @todo odd - maybe for sql-sync.
    $txt_destination = (isset($db_spec['remote-host']) ? $db_spec['remote-host'] . '/' : '') . $db_spec['database'];
    drush_print(dt("Creating database !target. Any possible existing database will be dropped!", array('!target' => $txt_destination)));

    if (!drush_confirm(dt('Do you really want to continue?'))) {
      return drush_user_abort();
    }
  }

  return $sql->createdb();
}


/**
 * Command callback. Outputs the entire Drupal database in SQL format using mysqldump or equivalent.
 */
function drush_sql_dump() {
  drush_sql_bootstrap_further();
  $sql = drush_sql_get_class();
  return $sql->dump(drush_get_option('result-file', FALSE));
}

/**
 * Construct an array that places table names in appropriate
 * buckets based on whether the table is to be skipped, included
 * for structure only, or have structure and data dumped.
 * The keys of the array are:
 * - skip: tables to be skipped completed in the dump
 * - structure: tables to only have their structure i.e. DDL dumped
 * - tables: tables to have structure and data dumped
 *
 * @return array
 *   An array of table names with each table name in the appropriate
 *   element of the array.
 */
function drush_sql_get_table_selection() {
  // Skip large core tables if instructed.  Used by 'sql-drop/sql-dump/sql-sync' commands.
  $skip_tables = _drush_sql_get_raw_table_list('skip-tables');
  // Skip any structure-tables as well.
  $structure_tables = _drush_sql_get_raw_table_list('structure-tables');
  // Dump only the specified tables.  Takes precedence over skip-tables and structure-tables.
  $tables = _drush_sql_get_raw_table_list('tables');

  return array('skip' => $skip_tables, 'structure' => $structure_tables, 'tables' => $tables);
}

/**
 * Expand wildcard tables.
 *
 * @param array $tables
 *   An array of table names, some of which may contain wildcards (`*`).
 * @param array $db_tables
 *   An array with all the existing table names in the current database.
 * @return
 *   $tables array with wildcards resolved to real table names.
 */
function drush_sql_expand_wildcard_tables($tables, $db_tables) {
  // Table name expansion based on `*` wildcard.
  $expanded_db_tables = array();
  foreach ($tables as $k => $table) {
    // Only deal with table names containing a wildcard.
    if (strpos($table, '*') !== FALSE) {
      $pattern = '/^' . str_replace('*', '.*', $table) . '$/i';
      // Merge those existing tables which match the pattern with the rest of
      // the expanded table names.
      $expanded_db_tables += preg_grep($pattern, $db_tables);
    }
  }
  return $expanded_db_tables;
}

/**
 * Filters tables.
 *
 * @param array $tables
 *   An array of table names to filter.
 * @param array $db_tables
 *   An array with all the existing table names in the current database.
 * @return
 *   An array with only valid table names (i.e. all of which actually exist in
 *   the database).
 */
function drush_sql_filter_tables($tables, $db_tables) {
  // Ensure all the tables actually exist in the database.
  foreach ($tables as $k => $table) {
    if (!in_array($table, $db_tables)) {
      unset($tables[$k]);
    }
  }

  return $tables;
}

/**
 * Given the table names in the input array that may contain wildcards (`*`),
 * expand the table names so that the array returned only contains table names
 * that exist in the database.
 *
 * @param array $tables
 *   An array of table names where the table names may contain the
 *   `*` wildcard character.
 * @param array $db_tables
 *   The list of tables present in a database.
 * @return array
 *   An array of tables with non-existant tables removed.
 */
function _drush_sql_expand_and_filter_tables($tables, $db_tables) {
  $expanded_tables = drush_sql_expand_wildcard_tables($tables, $db_tables);
  $tables = drush_sql_filter_tables(array_merge($tables, $expanded_tables), $db_tables);
  $tables = array_unique($tables);
  sort($tables);
  return $tables;
}

/**
 * Consult the specified options and return the list of tables
 * specified.
 *
 * @param option_name
 *   The option name to check: skip-tables, structure-tables
 *   or tables.  This function will check both *-key and *-list,
 *   and, in the case of sql-sync, will also check target-*
 *   and source-*, to see if an alias set one of these options.
 * @returns array
 *   Returns an array of tables based on the first option
 *   found, or an empty array if there were no matches.
 */
function _drush_sql_get_raw_table_list($option_name) {
  foreach(array('' => 'cli', 'target-,,source-' => NULL) as $prefix_list => $context) {
    foreach(explode(',',$prefix_list) as $prefix) {
      $key_list = drush_get_option($prefix . $option_name . '-key', NULL, $context);
      foreach(explode(',', $key_list) as $key) {
        $all_tables = drush_get_option($option_name, array());
        if (array_key_exists($key, $all_tables)) {
          return $all_tables[$key];
        }
        if ($option_name != 'tables') {
          $all_tables = drush_get_option('tables', array());
          if (array_key_exists($key, $all_tables)) {
            return $all_tables[$key];
          }
        }
      }
      $table_list = drush_get_option($prefix . $option_name . '-list', NULL, $context);
      if (isset($table_list)) {
        return empty($table_list) ? array() : explode(',', $table_list);
      }
    }
  }

  return array();
}

/**
 * Command callback. Executes the given SQL query on the Drupal database.
 */
function drush_sql_query($query = NULL) {
  drush_sql_bootstrap_further();
  $filename = drush_get_option('file', NULL);
  // Enable prefix processing when db-prefix option is used.
  if (drush_get_option('db-prefix')) {
    drush_bootstrap_max(DRUSH_BOOTSTRAP_DRUPAL_DATABASE);
  }
  if (drush_get_context('DRUSH_SIMULATE')) {
    if ($query) {
      drush_print(dt('Simulating sql-query: !q', array('!q' => $query)));
    }
    else {
      drush_print(dt('Simulating sql-import from !f', array('!f' => drush_get_option('file'))));
    }
  }
  else {
    $sql = drush_sql_get_class(drush_get_option('db-spec'));
    $result = $sql->query($query, $filename, drush_get_option('result-file'));
    if (!$result) {
      return drush_set_error('DRUSH_SQL_NO_QUERY', dt('Query failed.'));
    }
    drush_print(implode("\n", drush_shell_exec_output()));
  }
  return TRUE;
}

/**
 * Drops all tables in the database.
 */
function drush_sql_drop() {
  drush_sql_bootstrap_further();
  $sql = drush_sql_get_class();
  $db_spec = $sql->db_spec();
  if (!drush_confirm(dt('Do you really want to drop all tables in the database !db?', array('!db' => $db_spec['database'])))) {
    return drush_user_abort();
  }
  $tables = $sql->listTables();
  return $sql->drop($tables);
}

function drush_sql_cli() {
  drush_sql_bootstrap_further();
  $sql = drush_sql_get_class();
  return !(bool)drush_shell_proc_open($sql->connect());
}

/**
 * Command callback. Run's the sanitization operations on the current database.
 *
 * @see hook_drush_sql_sync_sanitize() for adding custom sanitize routines.
 */
function drush_sql_sanitize() {
  drush_sql_bootstrap_further();
  if (drush_get_option('db-prefix')) {
    drush_bootstrap_max(DRUSH_BOOTSTRAP_DRUPAL_DATABASE);
  }
  drush_include(DRUSH_BASE_PATH . '/commands/sql', 'sync.sql');
  drush_command_invoke_all('drush_sql_sync_sanitize', 'default');
  $options = drush_get_context('post-sync-ops');
  if (!empty($options)) {
    if (!drush_get_context('DRUSH_SIMULATE')) {
      $messages = _drush_sql_get_post_sync_messages();
      if ($messages) {
        drush_print();
        drush_print($messages);
      }
    }
  }
  if (!drush_confirm(dt('Do you really want to sanitize the current database?'))) {
    return drush_user_abort();
  }

  $sanitize_query = '';
  foreach($options as $id => $data) {
    // Enable prefix processing when db-prefix option is used.
    if (drush_get_option('db-prefix')) {
      if (drush_drupal_major_version() >= 7) {
        $data['query'] = Database::getConnection()->prefixTables($data['query']);
      }
      else {
        $data['query'] = db_prefix_tables($data['query']);
      }
    }
    $sanitize_query .= $data['query'] . " ";
  }
  if ($sanitize_query) {
    $sql = drush_sql_get_class();
    return $sql->query($sanitize_query);
  }
}

/**
 * Call from a pre-sql-sync hook to register an sql
 * query to be executed in the post-sql-sync hook.
 * @see drush_sql_pre_sql_sync() and @see drush_sql_post_sql_sync().
 *
 * @param $id
 *   String containing an identifier representing this
 *   operation.  This id is not actually used at the
 *   moment, it is just used to fufill the contract
 *   of drush contexts.
 * @param $message
 *   String with the confirmation message that describes
 *   to the user what the post-sync operation is going
 *   to do.  This confirmation message is printed out
 *   just before the user is asked whether or not the
 *   sql-sync operation should be continued.
 * @param $query
 *   String containing the sql query to execute.  If no
 *   query is provided, then the confirmation message will
 *   be displayed to the user, but no action will be taken
 *   in the post-sync hook.  This is useful for drush modules
 *   that wish to provide their own post-sync hooks to fix
 *   up the target database in other ways (e.g. through
 *   Drupal APIs).
 */
function drush_sql_register_post_sync_op($id, $message, $query = NULL) {
  $options = drush_get_context('post-sync-ops');

  $options[$id] = array('message' => $message, 'query' => $query);

  drush_set_context('post-sync-ops', $options);
}

/**
 * Builds a confirmation message for all post-sync operations.
 *
 * @return string
 *   All post-sync operation messages concatenated together.
 */
function _drush_sql_get_post_sync_messages() {
  $messages = FALSE;

  $options = drush_get_context('post-sync-ops');
  if (!empty($options)) {
    $messages = dt('The following post-sync operations will be done on the destination:') . "\n";

    foreach($options as $id => $data) {
      $messages .= "  * " . $data['message'] . "\n";
    }
  }

  return $messages;
}

/**
 * Wrapper for drush_get_class; instantiates an driver-specific instance
 * of SqlBase class.
 *
 * @param array $db_spec
 *   If known, specify a $db_spec that the class can operate with.
 *
 * @throws \Drush\Sql\SqlException
 *
 * @return Drush\Sql\SqlBase
 */
function drush_sql_get_class($db_spec = NULL) {
  $database = drush_get_option('database', 'default');
  $target = drush_get_option('target', 'default');

  // Try a few times to quickly get $db_spec.
  if ($db_spec) {
    return drush_get_class(array('Drush\Sql\Sql', 'Drupal\Driver\Database\\' .  $db_spec['driver'] . '\Drush'), array($db_spec), array($db_spec['driver']));
  }
  elseif ($url = drush_get_option('db-url')) {
    $url =  is_array($url) ? $url[$database] : $url;
    $db_spec = drush_convert_db_from_db_url($url);
    $db_spec['db_prefix'] = drush_get_option('db-prefix');
    return drush_sql_get_class($db_spec);
  }
  elseif (($databases = drush_get_option('databases')) && (array_key_exists($database, $databases)) && (array_key_exists($target, $databases[$database]))) {
    $db_spec = $databases[$database][$target];
    return drush_sql_get_class($db_spec);
  }
  else {
    // No parameter or options provided. Determine $db_spec ourselves.
    if ($sqlVersion = drush_sql_get_version()) {
      if ($db_spec = $sqlVersion->get_db_spec()) {
        return drush_sql_get_class($db_spec);
      }
    }
  }

  throw new \Drush\Sql\SqlException('Unable to find a matching SQL Class. Drush cannot find your database connection details.');
}

/**
 * Wrapper for drush_get_class; instantiates a Drupal version-specific instance
 * of SqlVersion class.
 *
 * @return Drush\Sql\SqlVersion
 */
function drush_sql_get_version() {
  return drush_get_class('Drush\Sql\Sql', array(), array(drush_drupal_major_version())) ?: NULL;
}

/**
 * Implements hook_sql_drush_sql_sync_sanitize.
 *
 * Sanitize usernames, passwords, and sessions when the --sanitize option is used.
 * It is also an example of how to write a database sanitizer for sql sync.
 *
 * To write your own sync hook function, define mymodule_drush_sql_sync_sanitize()
 * and follow the form of this function to add your own database
 * sanitization operations via the register post-sync op function;
 * @see drush_sql_register_post_sync_op().  This is the only thing that the
 * sync hook function needs to do; sql-sync takes care of the rest.
 *
 * The function below has a lot of logic to process user preferences and
 * generate the correct SQL regardless of whether Postgres, Mysql,
 * Drupal 6/7/8 is in use.  A simpler sanitize function that
 * always used default values and only worked with Drupal 6 + mysql
 * appears in the drush.api.php.  @see hook_drush_sql_sync_sanitize().
 */
function sql_drush_sql_sync_sanitize($site) {
  $site_settings = drush_sitealias_get_record($site);
  $databases = sitealias_get_databases_from_record($site_settings);
  if (drush_get_option('db-prefix') || !empty($databases['default']['default']['prefix'])) {
    $wrap_table_name = TRUE;
  }
  else {
    $wrap_table_name = FALSE;
  }
  $user_table_updates = array();
  $message_list = array();

  // Sanitize passwords.
  $newpassword = drush_get_option(array('sanitize-password', 'destination-sanitize-password'), 'password');
  if ($newpassword != 'no' && $newpassword !== 0) {
    $major_version = drush_drupal_major_version();
    $pw_op = "";

    // In Drupal 6, passwords are hashed via the MD5 algorithm.
    if ($major_version == 6) {
      $pw_op = "MD5('$newpassword')";
    }
    // In Drupal 7, passwords are hashed via a more complex algorithm,
    // available via the user_hash_password function.
    elseif ($major_version == 7) {
      $core = DRUSH_DRUPAL_CORE;
      include_once $core . '/includes/password.inc';
      include_once $core . '/includes/bootstrap.inc';
      $hash = user_hash_password($newpassword);
      $pw_op = "'$hash'";
    }
    else {
      // D8+. Mimic Drupal's /scripts/password-hash.sh
      drush_bootstrap(DRUSH_BOOTSTRAP_DRUPAL_FULL);
      $password_hasher = \Drupal::service('password');
      $hash = $password_hasher->hash($newpassword);
      $pw_op = "'$hash'";
    }
    if (!empty($pw_op)) {
      $user_table_updates[] = "pass = $pw_op";
      $message_list[] =  "passwords";
    }
  }

  // Sanitize email addresses.
  $newemail = drush_get_option(array('sanitize-email', 'destination-sanitize-email'), 'user+%uid@localhost.localdomain');
  if ($newemail != 'no' && $newemail !== 0) {
    if (strpos($newemail, '%') !== FALSE) {
      // We need a different sanitization query for Postgres and Mysql.

      $db_driver = $databases['default']['default']['driver'];
      if ($db_driver == 'pgsql') {
        $email_map = array('%uid' => "' || uid || '", '%mail' => "' || replace(mail, '@', '_') || '", '%name' => "' || replace(name, ' ', '_') || '");
        $newmail =  "'" . str_replace(array_keys($email_map), array_values($email_map), $newemail) . "'";
      }
      elseif ($db_driver == 'mssql') {
        $email_map = array('%uid' => "' + uid + '", '%mail' => "' + replace(mail, '@', '_') + '", '%name' => "' + replace(name, ' ', '_') + '");
        $newmail =  "'" . str_replace(array_keys($email_map), array_values($email_map), $newemail) . "'";
      }
      else {
        $email_map = array('%uid' => "', uid, '", '%mail' => "', replace(mail, '@', '_'), '", '%name' => "', replace(name, ' ', '_'), '");
        $newmail =  "concat('" . str_replace(array_keys($email_map), array_values($email_map), $newemail) . "')";
      }
      $user_table_updates[] = "mail = $newmail, init = $newmail";
    }
    else {
      $user_table_updates[] = "mail = '$newemail', init = '$newemail'";
    }
    $message_list[] = 'email addresses';
  }

  if (!empty($user_table_updates)) {
    $table = $major_version >= 8 ? 'users_field_data' : 'users';
    if ($wrap_table_name) {
      $table = "{{$table}}";
    }
    $sanitize_query = "UPDATE {$table} SET " . implode(', ', $user_table_updates) . " WHERE uid > 0;";
    drush_sql_register_post_sync_op('user-email', dt('Reset !message in !table table', array('!message' => implode(' and ', $message_list), '!table' => $table)), $sanitize_query);
  }

  // Seems quite portable (SQLite?) - http://en.wikipedia.org/wiki/Truncate_(SQL)
  $table = 'sessions';
  if ($wrap_table_name) {
    $table = "{{$table}}";
  }
  $sql_sessions = "TRUNCATE TABLE {$table};";
  drush_sql_register_post_sync_op('sessions', dt('Truncate Drupal\'s sessions table'), $sql_sessions);
}
