<?php

/**
 * @file
 * Defines the AgileZipBatchImporter.
 */

/**
 * Zip file importer.
 * @author adam
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