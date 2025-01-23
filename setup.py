from setuptools import setup, find_packages, Extension
from os import path
import re

package_name = "mmapio"

root_dir = path.abspath(path.dirname(__file__))
with open(path.join(root_dir, package_name, "__init__.py")) as f:
    source       = f.read()
    version      = re.search(r'__version__\s*=\s*[\'\"](.+?)[\'\"]'     , source).group(1)
    license      = re.search(r'__license__\s*=\s*[\'\"](.+?)[\'\"]'     , source).group(1)
    author       = re.search(r'__author__\s*=\s*[\'\"](.+?)[\'\"]'      , source).group(1)
    author_email = re.search(r'__email__\s*=\s*[\'\"](.+?)[\'\"]'       , source).group(1)
    description  = re.search(r'__description__\s*=\s*[\'\"](.+?)[\'\"]' , source).group(1)
          
assert version
assert license
assert author
assert author_email
assert description

mmapio_module = Extension(
    f'{package_name}.mmapio',
    sources = [f'{package_name}/mmapio.c'],
)

setup(
    name=package_name,
    version=version,
    description=description,
    long_description="Memory Mapped I/O Package",
    author=author,
    author_email=author_email,
    license=license,
    ext_modules=[mmapio_module],
    packages=find_packages()
)
