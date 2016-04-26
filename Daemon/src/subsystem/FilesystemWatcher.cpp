#include "FilesystemWatcher.h"

#include "../util/LoggerMacros.hpp"

namespace titanic {
namespace subsystem {

util::Logger FilesystemWatcher::Logger("titanic.subsystem.filesystemwatcher");

FilesystemWatcher::FilesystemWatcher() {
	LOG_TRC(Logger, "Constructing " << name());
}

FilesystemWatcher::~FilesystemWatcher() {
	LOG_TRC(Logger, "Destructing " << name());
}

const char*
FilesystemWatcher::name() const {
	return "FilesystemWatcher";
}

void
FilesystemWatcher::initialize(Poco::Util::Application& app) {
	LOG_INFO(Logger, "Initializing " << name());
}

void
FilesystemWatcher::uninitialize() {
	LOG_INFO(Logger, "Uninitializing " << name());
}

}
}