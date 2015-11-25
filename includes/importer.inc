<?php

/**
 * @file
 * Defines the AgileZipBatchImporter.
 */

/**
 * Zip file importer.
 * @author adam
 *
 * This class lets you select "Agile Zip Batch Importer" from the dropdown.
 */
class AgileZipBatchImporter extends ZipBatchImporter {

  protected $itemClass = 'AgileZipBatchImportObject';

  /**
   * Get an identifier we can use for an object.
   * See AgileIslandoraImporterBatchObject::getChosenId().
   *
   * @param AbstractFedoraObject $object
   *   The object to be later processed and added to Fedora.
   *
   * @return string
   *   An id to use in the back-end (Fedora).
   */
  protected function getId(AbstractFedoraObject $object) {
    return $object->getChosenId();
  }

}

/*
 * This is the itemClass specified above. Need it to select the WrapperClass.
 *
 */
class AgileZipBatchImportObject extends ZipBatchImportObject
{
  protected $pid;

  /**
   * Constructor.
   */
  protected function __construct($source) {
    parent::__construct($source);
  }

  public function getWrapperClass() {
    return 'AgileIslandoraImporterBatchObject';
  }
  /**
   * Get the ID from local identifier in MODS.
   *
   *
   * @return string
   *   A string containing the id (eventually used as the pid).
   */
  public function getIdFromMods() {
    if ($this->pid === NULL) {
      $mods = $this->getMODS();
      if ($mods) {
        $mods_doc = new DOMDocument();
        $mods_doc->loadXML($mods);
        $mods_xpath = new DOMXPath($mods_doc);
        $mods_xpath->registerNamespace('m', 'http://www.loc.gov/mods/v3');

        $string = $mods_xpath->evaluate('string(//m:mods/m:identifier[@type="local"]/text())');
        $this->pid = 'yale:' . $string;
      }
    }

    return $this->pid;
  }
}

class AgileIslandoraImporterBatchObject extends IslandoraImporterBatchObject {

  /**
   * Constructor.
   */
  public function __construct(IslandoraTuque $connection, IslandoraImportObject $object, $preprocessor_parameters = array()) {
    parent::__construct($connection, $object, $preprocessor_parameters);
  }

  public function getChosenId() {
    return $this->importerObjectInstance->getIdFromMods();
  }

}