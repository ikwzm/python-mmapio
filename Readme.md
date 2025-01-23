python-mmapio
==================================================================================

Overview
----------------------------------------------------------------------------------

### Introduction of python-mmapio

A Python extension package written in C for low-level memory-mapped I/O accesses.

The high-level nature of the python interpreter often creates inconveniences when you want to do low-level memory-mapped I/O accesses.
For example, if you try to read a 32-bit word using the read method of the mmap module in arm64 architecture, the cpu cache gets in the way and does not access the word as expected.

python-mmapio solves these problems by being written in C.

Installation
----------------------------------------------------------------------------------

### From source:

#### 1. Install required tools:

 * GCC or Clang compiler
 * Python development headers (e.g., `python3-dev` on Linux)

#### 2. Clone the repository:

```console
shell$ git clone https://github.com/ikwzm/python-mmapio.git
shell$ cd python-mmapio
```

#### 3. Build mmapio.mmapio module

```console
shell$ python3 setup.py build_ext --inplace
```

#### 4. Install mmapio package

```console
shell$ sudo python3 setup.py install
```

Usage
----------------------------------------------------------------------------------

### Importing the package

```python
import mmapio
```

### Example 1: Using mmapio.MemoryMappedIO

```console
shell# python3
Python 3.10.12 (main, Nov  6 2024, 20:22:13) [GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import os
>>> import mmap
>>> from mmapio import MemoryMappedIO
>>> device_file = os.open('/dev/uio0', os.O_RDWR | os.O_SYNC)
>>> device_mmap = mmap.mmap(device_file, 0x1000, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE, 0)
>>> regs = MemoryMappedIO(device_mmap, 0x0000, 0x1000)
>>> print("0x%08X" % regs.read_word(0x00))
0x00020064
>>> regs.write_word(0x64,0xDEADBEAF)
>>> print("0x%08X" % regs.read_word(0x64))
0xDEADBEAF
>>> exit()
```

### Example 2: Using mmapio.Uio 

```console
shell# python3
Python 3.10.12 (main, Nov  6 2024, 20:22:13) [GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> from mmapio import Uio
>>> uio = Uio('dma-controller@60010000')
>>> regs = uio.regs()
>>> regs.write_word(0x64,0xDEADBEAF)
>>> print("0x%08X" % regs.read_word(0x64))
0xDEADBEAF
>>> exit()
```

License
----------------------------------------------------------------------------------

This project is licensed under the BSD 2-Clause License. See the [LICENSE](./LICENSE) file for details.
