from distutils.core import setup
import py2exe, pygame, ode
import os, shutil

ver = "0.1"

try:
    shutil.rmtree("build")
    print "Removed build directory"
except:
    print "Could not remove build directory"
    
try:
    shutil.rmtree("dist")
    print "Removed dist directory"
except:
    print "Could not remove dist directory"
    
try:
    os.remove("dist-%s.zip" % ver)
    print "Removed dist-%s.zip" % ver
except:
    print "Could not remove dist-%s.zip" % ver

setup(
    name = 'ProgPractice',
    description = 'Student Robotics Programming Practice System',
    version = ver,

    windows = [
                  {
                      'script': 'sim.py',
                  }
              ],

    options = {
                  'py2exe': {
                      'packages':'encodings',
                      'includes': 'cairo, pango, pangocairo, atk, gobject',
                  "dll_excludes": [
                        "iconv.dll","intl.dll","libatk-1.0-0.dll",
                        "libgdk_pixbuf-2.0-0.dll","libgdk-win32-2.0-0.dll",
                        "libglib-2.0-0.dll","libgmodule-2.0-0.dll",
                        "libgobject-2.0-0.dll","libgthread-2.0-0.dll",
                        "libgtk-win32-2.0-0.dll","libpango-1.0-0.dll",
                        "libpangowin32-1.0-0.dll"],
                        }
                  
              }
)

print "Copying GTK libraries into the dist folder"

shutil.copytree("gtk", "dist/gtk")
shutil.copytree("gobject", "dist/gobject")
shutil.copytree("gtkbin", "dist/gtkbin")

print "Zipping up into dist-%s.zip" % ver

import zipfile, os
from os.path import join

try:
    import zlib
    z = zipfile.ZipFile("dist-%s.zip" % ver, 'w', zipfile.ZIP_DEFLATED)
    print "Using zlib compression"
except ImportError:
    z = zipfile.ZipFile("dist-%s.zip" % ver, 'w')
    print "Unable to use zlib compression"
    
for root, dirs, files in os.walk("dist"):
    for fileName in files:
        print "Adding %s" % join(root, fileName)
        z.write(join(root, fileName))

z.close()