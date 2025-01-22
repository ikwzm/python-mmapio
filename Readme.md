python-mmapio
==================================================================================

# Overview

## Introduction of python-mmapio

A Python extension module written in C for low-level memory-mapped I/O accesses.

The high-level nature of the python interpreter often creates inconveniences when you want to do low-level memory-mapped I/O accesses.
For example, if you try to read a 32-bit word using the read method of the mmap module in arm64 architecture, the cpu cache gets in the way and does not access the word as expected.

python-mmapio solves these problems by being written in C.

# Installation

## From source:

### 1. Install required tools:

 * GCC or Clang compiler
 * Python development headers (e.g., `python3-dev` on Linux)

### 2. Clone the repository:

```console
shell$ git clone https://github.com/ikwzm/python-mmapio.git
shell$ cd python-mmapio
```

### 3. Build and install the module:

```console
shell$ python3 setup.py build_ext --inplace
shell$ python3 setup.py install
```

# Usage

## Importing the module:

```python
import mmapio
```

## Example 1: Basic Usage

```console
shell$ python3
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

## Example 2: Use uio.py

```python:uio.py
import mmapio
import mmap
import os
import glob
import re

class Uio:
    """A simple uio class"""

    @staticmethod
    def find_device_file(device_name):
        device_file = None
        r = re.compile("/sys/class/uio/(.*)/name")
        for uio_device_name_file in glob.glob("/sys/class/uio/uio*/name"):
            f = open(uio_device_name_file, "r")
            uio_device_name = f.readline().strip()
            if uio_device_name == device_name:
                m = r.match(uio_device_name_file)
                device_file = m.group(1)
            f.close()
        return device_file
        
    def __init__(self, name):
        self.name        = name
        self.device_name = Uio.find_device_file(self.name)
        self.device_file = os.open('/dev/%s' % self.device_name, os.O_RDWR | os.O_SYNC)
        self.memmap_dict = {}

    def sys_class_file_name(self, name):
        return '/sys/class/uio/%s/%s' % (self.device_name, name)

    def read_class_integer(self, name):
        file_name = self.sys_class_file_name(name)
        with open(file_name, "r") as file:
            value = file.readline().strip()
        if   value.startswith("0x") or value.startswith("0X"):
            return int(value, 16)
        elif value.isdigit():
            return int(value, 10)
        raise ValueError("Invalid value %s in %s " % (value, file_name))
        
    def get_map_addr(self, index=0):
        return self.read_class_integer('maps/map%d/addr'   % index)

    def get_map_size(self, index=0):
        return self.read_class_integer('maps/map%d/size'   % index)

    def get_map_offset(self, index=0):
        return self.read_class_integer('maps/map%d/offset' % index)

    def get_map_info(self, index=0):
        map_addr   = self.get_map_addr(index)
        map_size   = self.get_map_size(index)
        map_offset = self.get_map_offset(index)
        return dict(addr=map_addr, size=map_size, offset=map_offset)
        
    def irq_on(self):
        os.write(self.device_file, bytes([1, 0, 0, 0]))

    def irq_off(self):
        os.write(self.device_file, bytes([0, 0, 0, 0]))
        
    def wait_irq(self):
        os.read(self.device_file, 4)

    def regs(self, index=0, offset=0, length=None):
        if index in self.memmap_dict.keys():
            memmap_info = self.memmap_dict[index]
            memmap      = memmap_info['memmap']
            mmap_offset = memmap_info['offset']
            mmap_size   = memmap_info['size']
            mmap_addr   = memmap_info['addr']
        else:
            page_size   = os.sysconf("SC_PAGE_SIZE")
            map_info    = self.get_map_info(index)
            mmap_addr   = map_info['addr']
            mmap_offset = ((map_info['addr'] + map_info['offset'])        ) % page_size
            mmap_size   = ((map_info['size'] + page_size - 1) // page_size) * page_size
            memmap      = mmap.mmap(self.device_file,
                                    mmap_size,
                                    mmap.MAP_SHARED,
                                    mmap.PROT_READ | mmap.PROT_WRITE,
                                    index*page_size)
            memmap_info = {'memmap': memmap, 'size': mmap_size, 'addr': mmap_addr, 'offset': mmap_offset}
            self.memmap_dict[index] = memmap_info
        regs_offset = mmap_offset + offset
        if   length == None:
            regs_length = mmap_size - regs_offset
        elif regs_offset + length <= mmap_size:
            regs_length = length
        else:
            raise ValueError("region range error")
        return mmapio.MemoryMappedIO(memmap, regs_offset, regs_length)
```

```console
shell$ python3
Python 3.10.12 (main, Nov  6 2024, 20:22:13) [GCC 11.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> from uio import Uio
>>> uio = Uio('dma-controller@60010000')
>>> regs = uio.regs()
>>> regs.write_word(0x64,0xDEADBEAF)
>>> print("0x%08X" % regs.read_word(0x64))
0xDEADBEAF
>>> exit()
```

# License

This project is licensed under the BSD 2-Clause License. See the [LICENSE](./LICENSE) file for details.
