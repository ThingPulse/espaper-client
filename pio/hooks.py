import os
import errno
from shutil import copyfile


Import("env")


# process release build flags
my_flags = env.ParseFlags(env['BUILD_FLAGS'])
#defines = {k: v for (k, v) in my_flags.get("CPPDEFINES")}

# name release target
#RELEASE_NAME = defines.get("RELEASE_NAME")
#RELEASE_DIR = defines.get("RELEASE_DIR")
#RELEASE_TARGET = "/".join([RELEASE_DIR, RELEASE_NAME + ".hex"])


for key in my_flags['CPPDEFINES']:
    if isinstance(key, list) and key[0] == 'CLIENT_VERSION':
        CLIENT_VERSION = key[1]
        print("Building client version {}.".format(CLIENT_VERSION))

# get environment vars for built target
PROJECTBUILD_DIR = env["PROJECTBUILD_DIR"]
PIOENV = env["PIOENV"]
PROGNAME = env["PROGNAME"]
TARGET_DIR = "target"
BUILD_TARGET = "/".join([PROJECTBUILD_DIR, PIOENV, PROGNAME + ".bin"])
VERSION_FILE_DEV = "/".join([TARGET_DIR, "current-DEV.version"])
VERSION_FILE_TEST = "/".join([TARGET_DIR, "current-TEST.version"])
VERSION_FILE_PROD = "/".join([TARGET_DIR, "current-PROD.version"])
RELEASE_TARGET = "/".join([TARGET_DIR, PIOENV + "-" + CLIENT_VERSION + ".bin"])

# copy binaries to target directory
def copy2release(source, target, env):
    print("copying build {} to release {}".format(BUILD_TARGET, RELEASE_TARGET))
    ensureDirectoryExists(RELEASE_TARGET)
    copyfile(BUILD_TARGET, RELEASE_TARGET)
    writeVersionFile()

def ensureDirectoryExists(filename):
  if not os.path.exists(os.path.dirname(filename)):
    try:
      os.makedirs(os.path.dirname(filename))
    except OSError as exc: # Guard against race condition
      if exc.errno != errno.EEXIST:
        raise

def writeVersionFile():
  with open(VERSION_FILE_DEV, "w") as versionFile:
    versionFile.write("{}".format(CLIENT_VERSION))
  with open(VERSION_FILE_TEST, "w") as versionFile:
    versionFile.write("{}".format(CLIENT_VERSION))
  with open(VERSION_FILE_PROD, "w") as versionFile:
    versionFile.write("{}".format(CLIENT_VERSION))

# add post hook
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", copy2release)
