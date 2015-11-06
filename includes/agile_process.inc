<?php

/**
 * @file
 * Migration forms and handlers.
 */

/**
 * Form to define parameters for importing files.
 *
 * @param array $form
 *   Drupal Form
 * @param array $form_state
 *   Drupal from state
 *
 * @return array
 *   Drupal form
 */
function agile_import_wiki_form($form, &$form_state) {
  $form = array();
  $form['source_dir'] = array(
    '#type' => 'textfield',
    '#title' => t("CSV Path"),
    '#description' => t('Full path to CSV'),
  );
  $form['namespace'] = array(
    '#type' => 'textfield',
    '#title' => t("Namespace"),
    '#description' => t('Namespace for created collection'),
    '#default_value' => 'yale',
  );

  $form['collection'] = array(
    '#type' => 'textfield',
    '#title' => t("Collection"),
    '#description' => t('PID of collection to which new objects are being added'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Create Objects"),
  );
  return $form;
}

/**
 * Validation handler.
 *
 * @param array $form
 *   Drupal Form
 * @param array $form_state
 *   Drupal form state
 */
function agile_import_wiki_form_validate($form, &$form_state) {
  if (!is_dir($form_state['values']['source_dir'])) {
    form_set_error('source_dir', t('Not a valid file path.'));
  }
  if (!islandora_object_load($form_state['values']['collection'])) {
    form_set_error('collection', t('Collection does not exist.'));
  }
}

/**
 * Submit handler.
 *
 * @param array $form
 *   Drupal Form
 * @param array $form_state
 *   Drupal form state
 */
function agile_import_wiki_form_submit($form, &$form_state) {
  $path = $form_state['values']['source_dir'];
  $records = agile_wiki_read_csv($path);
  $batch = array(
    'title' => t('Adding Objects'),
    'operations' => array(),
    'file' => drupal_get_path('module', 'agile_wiki') . '/includes/agile_process.inc',
    'progress_message' => t('@current of @total operations completed.'),
  );

  foreach ($records as $record) {
    $batch['operations'][] = array('agile_batch_add_record', array(
        'record' => $record,
        'namespace' => $form_state['values']['namespace'],
        'collection_pid' => $form_state['values']['collection'],
      ),
    );
  }
  batch_set($batch);
  batch_process();
}

/**
 * Creates object from array.
 *
 * @global user $user
 *
 * @param array $record
 *   Associative array of required values
 * @param string $namespace
 *   Namespace of ingested object
 * @param syring $collection_pid
 *   PID of collection
 * @param string $source_path
 *   File Path
 */
function agile_batch_add_wiki($record, $namespace, $collection_pid) {
  global $user;
  $tuque = new IslandoraTuque();
  $object = $tuque->repository->constructObject($namespace);
  $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $collection_pid);
  $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:sp_html_snippet');
  $object->label = $record['title'];
  $object->owner = $user->name;

  // Create and add MODS
  $mods = agile_wiki_build_mods($record);
  $mods_ds = $object->constructDatastream('MODS', 'M');
  $mods_ds->setContentFromString($mods);
  $mods_ds->label = 'MODS';
  $mods_ds->mimetype = 'text/xml';
  $object->ingestDatastream($mods_ds);
  // Add OBJ datastream.
//  $file = agile_wiki_get_associated_file($record['page_id']);  //ROSIE!!!  Write this function!!!
//  $ds = $object->constructDatastream('OBJ', 'M');
//  $ds->setContentFromFile($file, FALSE);
//  $ds->label = 'OBJ';
//  $ds->mimetype = "text/html";
//  $object->ingestDatastream($ds);
  islandora_add_object($object);
}

function agile_wiki_read_csv($path) {
  ini_set("auto_detect_line_endings", "1");
  $pages = array();
  if (($handle = fopen($path, "r")) !== FALSE) {
    while (($data = fgetcsv($handle, 10000, ",")) !== FALSE) {
      if (!isset($header)) {
        $header = $data;
        continue;
      }
      for ($counter = 0; $counter < count($data); $counter++) {
        $page[$header[$counter]] = $data[$counter];
      }
      $pages[] = $page;
    }
    fclose($handle);
  }
  return $pages;
}

function agile_wiki_get_terms($records) {
  module_load_include('module', 'taxonomy');
  $temporals = array();
  $topics = array();
  foreach ($records as $record) {
    $terms = explode('|', $record['categories']);
    foreach ($terms as $term) {
      if (is_int($term)) {
        $temporals[] = $term;
      }
      else {
        $topics = $term;
      }
    }
  }
  // Build vocabularies
  $temporal = taxonomy_vocabulary_machine_name_load('temporal');
  if (!$temporal->vid) {
    taxonomy_vocabulary_save((object) array(
          'name' => 'Temporal',
          'machine_name' => 'temporal',
    ));
    $temporal = taxonomy_vocabulary_machine_name_load('temporal');
  }
  $temporals = array_unique($temporals);
  foreach ($temporals as $term) {
    taxonomy_term_save((object) array(
          'name' => $term,
          'vid' => $temporal->vid,
    ));
  }
  $topic = taxonomy_vocabulary_machine_name_load('topic');
  if (!$topic->vid) {
    taxonomy_vocabulary_save((object) array(
          'name' => 'Topic',
          'machine_name' => 'topic',
    ));
    $topic = taxonomy_vocabulary_machine_name_load('topic');
  }
  $topics = array_unique($topics);
  foreach ($topics as $term) {
    taxonomy_term_save((object) array(
          'name' => $term,
          'vid' => $topicl->vid,
    ));
  }
}

function agile_wiki_build_mods($record) {
  $xml = new DomDocument('1.0', 'UTF-8');
  $xml->preserveWhiteSpace = false;
  $xml->formatOutput = true;
  $mods_uri = "http://www.loc.gov/mods/v3";
  //root.
  $root = $xml->createElementNS($mods_uri, 'mods:mods');
  $xml->appendChild($root);
  $titleInfo_node = $xml->createElementNS($mods_uri, "mods:titleInfo");
  $root->appendChild($titleInfo_node);
  $title_node = $xml->createElementNS($mods_uri, 'mods:title', $record['title']);
  $titleInfo_node->appendChild($title_node);

  $authors = explode('|', $record['authors']);
  foreach ($authors as $author) {
    agile_wiki_add_author($author);
  }


  return $xml->saveXML();
}

function agile_wiki_add_author($xml, $name) {
  $mods_uri = "http://www.loc.gov/mods/v2";
  $root = $xml->documentElement;
  // name.
  $name_node = $xml->createElementNS($mods_uri, 'mods:name');
  $name_node->setAttribute('type', 'personal');
  $root->appendChild($name_node);
  //namePart
  $name_part = $xml->createElementNS($mods_uri, 'mods:namePart', $name);
  $name_node->appendChild($name_part);
  //role
  $role_node = $xml->createElementNS($mods_uri, 'mods:role');
  $name_node->appendChild($role_node);
  $roleTerm_node = $xml->createElementNS($mods_uri, 'mods:roleTerm', "Author");
  $roleTerm_node->setAttribute('authority', 'marcrelator');
  $roleTerm_node->setAttribute('type', 'text');
  $role_node->appendChild($roleTerm_node);
}
