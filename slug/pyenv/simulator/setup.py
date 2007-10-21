from distutils.core import setup
import py2exe, pygame, ode

setup(
    name = 'ProgPractice',
    description = 'Student Robotics Programming Practice System',
    version = '0.1',

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