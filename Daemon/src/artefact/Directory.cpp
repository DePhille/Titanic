#include "Directory.h"

#include <Poco/DirectoryIterator.h>
#include "../util/LoggerMacros.hpp"

namespace titanic {
namespace artefact {

util::Logger Directory::Log("artefact.directory");

Directory::Directory(const Poco::File& directoryPath) : _directoryPath(directoryPath) {
}

Directory::~Directory() {
	reset();
}

void
Directory::scan() {
	reset();

	Poco::DirectoryIterator dirIt(_directoryPath);
	Poco::DirectoryIterator dirEnd;
	for (; dirIt != dirEnd; ++dirIt) {
		if (dirIt->isFile()) {
			File* foundFile = new File(*dirIt);
			_files.push_back(foundFile);
		} else if (dirIt->isDirectory()) {
			Directory* foundDir = new Directory(*dirIt);
			foundDir->scan();
			_subDirectories.push_back(foundDir);
		} else {
			LOG_WARN(Log, "Object is not a file or directory [" << dirIt->path() << "]");
		}
	}

}

unsigned long
Directory::countFiles() const {
	unsigned long fileCount = _files.size();
	std::vector<Directory*>::const_iterator dirIt = _subDirectories.begin();
	for (; dirIt != _subDirectories.end(); ++dirIt) {
		fileCount += (*dirIt)->countFiles();
	}
	return fileCount;
}

unsigned long
Directory::countDirectories() const {
	unsigned long dirCount = _subDirectories.size();
	std::vector<Directory*>::const_iterator dirIt = _subDirectories.begin();
	for (; dirIt != _subDirectories.end(); ++dirIt) {
		dirCount += (*dirIt)->countDirectories();
	}
	return dirCount;
}

unsigned long
Directory::countAll() const {
	unsigned long nodeCount = _subDirectories.size() + _files.size();
	std::vector<Directory*>::const_iterator dirIt = _subDirectories.begin();
	for (; dirIt != _subDirectories.end(); ++dirIt) {
		nodeCount += (*dirIt)->countAll();
	}
	return nodeCount;
}

void
Directory::reset() {
	// Clear directories
	std::vector<Directory*>::iterator aDirIt = _subDirectories.begin();
	for (; aDirIt != _subDirectories.end(); ++aDirIt) {
		if (*aDirIt)
			delete *aDirIt;
		*aDirIt = NULL;
	}
	_subDirectories.clear();

	// Clear files
	std::vector<File*>::iterator aFileIt = _files.begin();
	for (; aFileIt != _files.end(); ++aFileIt) {
		if (*aFileIt)
			delete *aFileIt;
		*aFileIt = NULL;
	}
	_files.clear();
}

}
}