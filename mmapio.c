/*********************************************************************************
 *
 *       Copyright (C) 2025 Ichiro Kawazome
 *       All rights reserved.
 * 
 *       Redistribution and use in source and binary forms, with or without
 *       modification, are permitted provided that the following conditions
 *       are met:
 * 
 *         1. Redistributions of source code must retain the above copyright
 *            notice, this list of conditions and the following disclaimer.
 * 
 *         2. Redistributions in binary form must reproduce the above copyright
 *            notice, this list of conditions and the following disclaimer in
 *            the documentation and/or other materials provided with the
 *            distribution.
 * 
 *       THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *       "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *       LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *       A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 *       OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *       SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *       LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *       DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *       THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *       (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *       OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 ********************************************************************************/
#include <Python.h>
#include <sys/mman.h>
#include <unistd.h>

#define MODULE_NAME         "mmapio"
#define OBJECT_NAME         "MemoryMappedIO"
#define MODULE_VERSION      "0.0.1"
#define MODULE_AUTHOR       "Ichiro Kawazome"
#define MODULE_AUTHOR_EMAIL "ichiro_k@ca2-so-net.ne.jp"
#define MODULE_LICENSE      "BSD 2-Clause"
#define MODULE_DESCRIPTION  "Memory Mapped I/O Module"

typedef struct {
    PyObject_HEAD
    void*        addr;
    Py_ssize_t   size;
} mmapio_object;

#define DEFINE_MMAPIO_CHECK_OFFSET_METHOD(type)                                   \
static int mmapio_check_offset_ ## type(mmapio_object* self, PY_LONG_LONG offset) \
{                                                                                 \
    if ((offset < 0) || (offset >= self->size)) {                                 \
        PyErr_SetString(PyExc_ValueError, "Offset exceeds mapped region");        \
        return -1;                                                                \
    }                                                                             \
    if ((offset % sizeof(type)) != 0) {                                           \
        PyErr_SetString(PyExc_ValueError, "Offset violates " #type " alignment"); \
        return -1;                                                                \
    }                                                                             \
    return 0;                                                                     \
}

DEFINE_MMAPIO_CHECK_OFFSET_METHOD(uint8_t)
DEFINE_MMAPIO_CHECK_OFFSET_METHOD(uint16_t)
DEFINE_MMAPIO_CHECK_OFFSET_METHOD(uint32_t)
DEFINE_MMAPIO_CHECK_OFFSET_METHOD(uint64_t)

#define DEFINE_MMAPIO_RW_METHOD(name,type)                                 \
static PyObject* mmapio_read_ ## name(mmapio_object* self, PyObject* args) \
{                                                                          \
    PY_LONG_LONG  offset;                                                  \
    type          data;                                                    \
    if (!PyArg_ParseTuple(args, "L", &offset)) {                           \
        return NULL;                                                       \
    }                                                                      \
    if (mmapio_check_offset_ ## type(self, offset) != 0) {                 \
        return NULL;                                                       \
    }                                                                      \
    data = *(type *)(self->addr + offset);                                 \
    return PyLong_FromUnsignedLongLong((unsigned PY_LONG_LONG)(data));     \
}                                                                          \
static PyObject* mmapio_write_ ## name(mmapio_object* self, PyObject* args)\
{                                                                          \
    PY_LONG_LONG           offset;                                         \
    unsigned PY_LONG_LONG  data;                                           \
    unsigned PY_LONG_LONG  mask = (unsigned PY_LONG_LONG)(~(type)0);       \
    if (!PyArg_ParseTuple(args, "LK", &offset, &data)) {                   \
        return NULL;                                                       \
    }                                                                      \
    if (mmapio_check_offset_ ## type(self, offset) != 0) {                 \
        return NULL;                                                       \
    }                                                                      \
    *(type *)(self->addr + offset) = (type)(data & mask);                  \
    Py_RETURN_NONE;                                                        \
}                                                                          \

DEFINE_MMAPIO_RW_METHOD(byte  ,uint8_t )
DEFINE_MMAPIO_RW_METHOD(half  ,uint16_t)
DEFINE_MMAPIO_RW_METHOD(word  ,uint32_t)
DEFINE_MMAPIO_RW_METHOD(quad  ,uint64_t)
DEFINE_MMAPIO_RW_METHOD(uint8 ,uint8_t )
DEFINE_MMAPIO_RW_METHOD(uint16,uint16_t)
DEFINE_MMAPIO_RW_METHOD(uint32,uint32_t)
DEFINE_MMAPIO_RW_METHOD(uint64,uint64_t)

static PyMethodDef mmapio_methods[] = {
    {"read_byte"   ,(PyCFunction)mmapio_read_byte   , METH_VARARGS, "Read Byte from Memrory Mapped I/O" },
    {"read_half"   ,(PyCFunction)mmapio_read_half   , METH_VARARGS, "Read 16bit from Memrory Mapped I/O"},
    {"read_word"   ,(PyCFunction)mmapio_read_word   , METH_VARARGS, "Read 32bit from Memrory Mapped I/O"},
    {"read_quad"   ,(PyCFunction)mmapio_read_quad   , METH_VARARGS, "Read 64bit from Memrory Mapped I/O"},
    {"write_byte"  ,(PyCFunction)mmapio_write_byte  , METH_VARARGS, "Write Byte to Memrory Mapped I/O"  },
    {"write_half"  ,(PyCFunction)mmapio_write_half  , METH_VARARGS, "Write 16bit to Memrory Mapped I/O" },
    {"write_word"  ,(PyCFunction)mmapio_write_word  , METH_VARARGS, "Write 32bit to Memrory Mapped I/O" },
    {"write_quad"  ,(PyCFunction)mmapio_write_quad  , METH_VARARGS, "Write 64bit to Memrory Mapped I/O" },
    {"read_uint8"  ,(PyCFunction)mmapio_read_uint8  , METH_VARARGS, "Read Byte from Memrory Mapped I/O" },
    {"read_uint16" ,(PyCFunction)mmapio_read_uint16 , METH_VARARGS, "Read 16bit from Memrory Mapped I/O"},
    {"read_uint32" ,(PyCFunction)mmapio_read_uint32 , METH_VARARGS, "Read 32bit from Memrory Mapped I/O"},
    {"read_uint64" ,(PyCFunction)mmapio_read_uint64 , METH_VARARGS, "Read 64bit from Memrory Mapped I/O"},
    {"write_uint8" ,(PyCFunction)mmapio_write_uint8 , METH_VARARGS, "Write Byte to Memrory Mapped I/O"  },
    {"write_uint16",(PyCFunction)mmapio_write_uint16, METH_VARARGS, "Write 16bit to Memrory Mapped I/O" },
    {"write_uint32",(PyCFunction)mmapio_write_uint32, METH_VARARGS, "Write 32bit to Memrory Mapped I/O" },
    {"write_uint64",(PyCFunction)mmapio_write_uint64, METH_VARARGS, "Write 64bit to Memrory Mapped I/O" },
    {NULL}
};


static PyObject*
mmapio_object_new(PyTypeObject* type, PyObject* args, PyObject* kwdict)
{
    mmapio_object* self;
    self = (mmapio_object*)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->addr = NULL;
        self->size   = 0;
    }
    return (PyObject*)self;
}

static int
mmapio_object_init(mmapio_object* self, PyObject* args, PyObject* kwdict)
{
    PyObject*    buf;
    Py_ssize_t   offset = 0;
    Py_ssize_t   length = 0;
    Py_buffer    view;
    void*        buf_ptr;
    Py_ssize_t   buf_size;
    static char* kwlist[] = {"buffer", "offset", "length", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwdict, "O|nn", kwlist, &buf, &offset, &length))  {
        return -1;
    }

    if (Py_TYPE(buf)->tp_as_buffer &&
        Py_TYPE(buf)->tp_as_buffer->bf_releasebuffer) {
        buf = PyMemoryView_FromObject(buf);
        if (buf == NULL) {
            return -1;
        }
    } else {
        Py_INCREF(buf);
    }

    if (PyObject_GetBuffer(buf, &view, PyBUF_SIMPLE) < 0) {
        Py_DECREF(buf);
        return -1;
    }
    buf_ptr  = (void*)view.buf;
    buf_size = view.len;
    PyBuffer_Release(&view);

    if ((offset < 0) || (offset > buf_size)) {
        PyErr_Format(PyExc_ValueError,
                     "offset must be non-negative and no greater than buffer " \
                     "length (%lld)", buf_size);
        Py_DECREF(buf);
        return -1;
    }
    if (length == 0) {
        length = buf_size - offset;
    }
    if (offset+length > buf_size) {
        PyErr_Format(PyExc_ValueError,
                     "offset+length must be no greater than buffer " \
                     "length (%lld)", buf_size);
        Py_DECREF(buf);
        return -1;
    }
    self->addr = buf_ptr  + offset;
    self->size = length;

    Py_DECREF(buf);
    return 0;
}

static void
mmapio_object_dealloc(mmapio_object* self)
{
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyTypeObject mmapio_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = MODULE_NAME "." OBJECT_NAME,
    .tp_basicsize = sizeof(mmapio_object),
    .tp_dealloc   = (destructor)mmapio_object_dealloc,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_doc       = "Memory Mapped I/O Object \n\n"
                    OBJECT_NAME "(buffer, offset, length) \n"
                    "    Create a new " OBJECT_NAME " instance \n"
                    "    buffer: buffer object \n"
                    "    offset: map offset (default=0)\n"
                    "    length: map size (default=buffer.length-offset)",
    .tp_methods   = mmapio_methods,
    .tp_init      = (initproc)mmapio_object_init,
    .tp_new       = mmapio_object_new,
};

static struct PyModuleDef mmapio_module = {
    PyModuleDef_HEAD_INIT,
    MODULE_NAME,
    MODULE_DESCRIPTION "\n"
    "License: " MODULE_LICENSE "\n"
    "Author:  " MODULE_AUTHOR  "\n"
    "Version: " MODULE_VERSION,
    -1,
    NULL,
};

PyMODINIT_FUNC PyInit_mmapio(void) {
    PyObject* m;

    if (PyType_Ready(&mmapio_type) < 0) {
        return NULL;
    }

    m = PyModule_Create(&mmapio_module);
    if (m == NULL) {
        return NULL;
    }

    Py_INCREF(&mmapio_type);
    if (PyModule_AddObject(m, OBJECT_NAME, (PyObject*)&mmapio_type) < 0) {
        Py_DECREF(&mmapio_type);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
