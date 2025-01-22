from setuptools import setup, Extension
from os import path
import re

package_name    = "mmapio"
package_sources = ["mmapio.c"]

root_dir = path.abspath(path.dirname(__file__))
with open(path.join(root_dir, package_sources[0])) as f:
    source       = f.read()
    version      = re.search(r'#define\s+MODULE_VERSION\s+\"(.+?)\"'     , source).group(1)
    license      = re.search(r'#define\s+MODULE_LICENSE\s+\"(.+?)\"'     , source).group(1)
    author       = re.search(r'#define\s+MODULE_AUTHOR\s+\"(.+?)\"'      , source).group(1)
    author_email = re.search(r'#define\s+MODULE_AUTHOR_EMAIL\s+\"(.+?)\"', source).group(1)
    description  = re.search(r'#define\s+MODULE_DESCRIPTION\s+\"(.+?)\"' , source).group(1)

assert version
assert license
assert author
assert author_email
assert description

module = Extension(
    package_name             ,
    sources = package_sources,
)

setup(
    name=package_name,
    version=version,
    description=description,
    long_description="Memory Mapped I/O Module",
    author=author,
    author_email=author_email,
    license=license,
    ext_modules=[module],
)
