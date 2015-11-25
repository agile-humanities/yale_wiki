<?php

/**
 * @file
 * Defines the AgileZipBatchImporter.
 */

/**
 * Zip file importer.
 * @author adam
 *
 */
class AgileZipBatchImporter extends ZipBatchImporter {

  protected $itemClass = 'AgileZipBatchImportObject';

  /**
   * Get an identifier we can use for an object.
   *
   * @param AbstractFedoraObject $object
   *   The object to be later processed and added to Fedora.
   *
   * @return string
   *   An id to use in the back-end (Fedora).
   */
  protected function getId(AbstractFedoraObject $object) {
    dpm($this);
    // Command line argument overrides Collection Policy.
    if (isset($this->parameters['namespace'])) {
      $namespace = $this->parameters['namespace'];
    }
    else {
      $namespace = $this->determineNamespace($object);
    }
    $namespace = 'foobar';
    // TODO: Implement some form of caching, so we can get multiple at a time.
    return $this->connection->repository->getNextIdentifier($namespace);
  }


}

class AgileZipBatchImportObject extends ZipBatchImportObject
{

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
    if ($this->id === NULL) {
      $mods = $this->getMODS();
      if ($mods) {
        $mods_doc = new DOMDocument();
        $mods_doc->loadXML($mods);
        $mods_xpath = new DOMXPath($mods_doc);
        $mods_xpath->registerNamespace('m', 'http://www.loc.gov/mods/v3');

        $string = $mods_xpath->evaluate('string(//m:mods/m:identifier[@type="local"]/text())');
        $this->id = 'foobar1:' . $string;
      }
    }

    return $this->id;
  }

}

class AgileIslandoraImporterBatchObject extends IslandoraImporterBatchObject {

  public function batchProcess()
  {
    $this->id = $this->importerObjectInstance->getIdFromMods();
    parent::batchProcess();
  }

}