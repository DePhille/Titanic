#include "Artefact.h"

#include <Poco/File.h>
#include <Poco/Path.h>

#include "Directory.h"
#include "../exception/NotFound.hpp"
#include "../exception/InvalidInput.hpp"
#include "../exception/InsufficientPrivileges.hpp"
#include "../util/LoggerMacros.hpp"

namespace titanic {
namespace artefact {

util::Logger Artefact::Log("titanic.artefact");

Artefact::Artefact(const Poco::File& path) : _directory(NULL), _path(path) {
	LOG_DBG(Log, "Artefact created on [" + path.path() + "]");
}

Artefact::~Artefact() {
	reset();
	LOG_DBG(Log, "Artefact destroyed");
}

void
Artefact::initialize() {
	LOG_DBG(Log, "Initializing artefact");

	// Validate input directory
	if (!_path.exists()) {
		throw exception::NotFound(Poco::Path(_path.path()));
	}
	else if (!_path.isDirectory()) {
		throw exception::InvalidInput("directory", "a directory");
	}
	else if (!_path.canRead() || !_path.canWrite()) {
		throw exception::InsufficientPrivileges(Poco::Path(_path.path()));
	}

	// Construct root directory object
	reset();
	_directory = new Directory(Poco::File(_path.path()));

	LOG_DBG(Log, "Artefact initialized");
}

void
Artefact::scan() {
	if (!_directory) {
		throw exception::Exception("Not initialized");
	}

	_directory->scan();
}

unsigned long
Artefact::countFiles() const {
	if (_directory)
		return _directory->countFiles();
	else
		return 0;
}

unsigned long
Artefact::countDirectories() const {
	if (_directory)
		return _directory->countDirectories();
	else
		return 0;
}

unsigned long
Artefact::countAll() const {
	if (_directory)
		return _directory->countAll();
	else
		return 0;
}

void
Artefact::reset() {
	if (_directory)
		delete _directory;
	_directory = NULL;
}

}
}