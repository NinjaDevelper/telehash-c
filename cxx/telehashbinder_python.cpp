/*
 * Copyright (c) 2015, Shinya Yagyu
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its 
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
*/

#include "StorjTelehash.hpp"
#include <Python.h>
#include "bytesobject.h"

#ifndef LOG
void *logging(const char *file, int line, const char *function, 
               const char * format, ...)
{
  char buffer[256];
  va_list args;
  va_start (args, format);
  vsnprintf (buffer, 256, format, args);
  fprintf(stderr,"%s:%d %s() %s\n", file, line, function, buffer);
  fflush(stderr);
  va_end (args);
  return NULL;

}
#define LOG(fmt, ...)  logging(__FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#endif

char *EvaluatePyObject(PyObject *obj, char *str){
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    if(!PyCallable_Check(obj)){
        LOG("not callable object");
    }
    PyObject *arglist = Py_BuildValue("(s)", str);
    PyObject *result = PyObject_CallObject(obj, arglist);
        
    char *buffer=NULL;
    char *rbuf=NULL;
    PyObject *ascii=NULL;
    if(result){
        if(PyUnicode_Check(result)){
            ascii=PyUnicode_AsASCIIString(result);
            buffer=PyBytes_AsString(ascii);
            rbuf=(char *)malloc(strlen(buffer)+1);
            strcpy(rbuf,buffer);
            Py_XDECREF(ascii);
        }else{
            if(PyBytes_Check(result)){
                buffer=PyBytes_AsString(result);
                rbuf=(char *)malloc(strlen(buffer));
                strcpy(rbuf,buffer);
            }
        }
    }else{
        PyErr_PrintEx(1);
    }

    Py_XDECREF(arglist);
    Py_XDECREF(result);
    PyGILState_Release(gstate);
    return rbuf;
}
    
class ChannelHandlerImpl : public ChannelHandler{
private:
    PyObject *pyHandler;
public:
    /**
     * create myself with handlers.
     * 
     * @param h channels.
     */
	ChannelHandlerImpl(PyObject *pyHandler){
        Py_XINCREF(pyHandler);
        this->pyHandler=pyHandler;
	}
    
    /**
     * handle one packet.
     * 
     * @param json one packet described in json.
     * @return a json packet that should be sent back. not be sent if NULL.
     */
	char* handle(char *json){
        if(!pyHandler) return NULL;
        return EvaluatePyObject(pyHandler, json);
	}
    
    ~ChannelHandlerImpl(){
         Py_XDECREF(pyHandler);
    }
};

class ChannelHandlerFactoryImpl : public ChannelHandlerFactory{
private:
    PyObject *pyFactory;
public:
    ChannelHandlerFactoryImpl(PyObject *pyFactory){
        Py_XINCREF(pyFactory); 
        this->pyFactory=pyFactory;
    }
    /**
     * return myself which is associated channel name.
     * 
     * @param name channel name.
     * @return myself associated the channel name.
     */
	ChannelHandler* createInstance(string name){
        PyGILState_STATE gstate = PyGILState_Ensure();
        if(!PyCallable_Check(pyFactory)){
            LOG("factory is not callable object\n");
        } else {
            PyObject *arglist = Py_BuildValue("(s)", name.c_str());
            PyObject *pyHandler = PyObject_CallObject(pyFactory, arglist);
            Py_XDECREF(arglist); 
            if(!pyHandler){
                PyErr_PrintEx(1);
            } else {
                if(pyHandler == Py_None){
                }else{
                    if(PyCallable_Check(pyHandler)){
                        PyGILState_Release(gstate);
   		                return new ChannelHandlerImpl(pyHandler);
                    }
                }
            }
        }
        LOG("cannot create instance\n");
        PyGILState_Release(gstate);
        return NULL;
	}
    
    ~ChannelHandlerFactoryImpl(){
        Py_XDECREF(pyFactory);
    }
};

static PyObject *telehashbinder_init(PyObject *self, PyObject *args){
    int port=0;
    PyObject *pyFactory=NULL;
    
    if (! PyEval_ThreadsInitialized()) {
        PyEval_InitThreads();
    }
    if (!PyArg_ParseTuple(args, "iO", &port,&pyFactory)){
        return NULL;
    }
    if (!PyCallable_Check(pyFactory)) {
        PyErr_SetString(PyExc_TypeError, "factory must be callable");
        return NULL;
    }

    ChannelHandlerFactoryImpl *f=
        new ChannelHandlerFactoryImpl(pyFactory);
    StorjTelehash *m=new StorjTelehash(port,*f);
    PyObject *p=PyCapsule_New(m, NULL,NULL);
    return p;
}

static PyObject *telehashbinder_openChannel(PyObject *self, 
                                                PyObject *args){
    PyObject *cobj=NULL;
    char *location=NULL;
    char *channelName=NULL;
    PyObject *handler=NULL;
    if (!PyArg_ParseTuple(args, "OssO",&cobj,&location,&channelName,&handler)){
        return NULL;
    }
    ChannelHandlerImpl *h=new ChannelHandlerImpl(handler);
    StorjTelehash *m=(StorjTelehash *)PyCapsule_GetPointer(cobj,NULL);
    m->openChannel(location,channelName,*h);
    Py_RETURN_NONE;
}

static PyObject *telehashbinder_start(PyObject *self, PyObject *args){
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject *cobj=NULL;
    if (!PyArg_ParseTuple(args, "O",&cobj)){
        return NULL;
    }
    StorjTelehash *m=(StorjTelehash *)PyCapsule_GetPointer(cobj,NULL);
    Py_BEGIN_ALLOW_THREADS
        m->start();
    Py_END_ALLOW_THREADS
    PyGILState_Release(gstate);
    Py_RETURN_NONE;
}

static PyObject *telehashbinder_setStopFlag(PyObject *self, PyObject *args){
    PyObject *cobj=NULL;
    int flag=0;
    if (!PyArg_ParseTuple(args, "Oi",&cobj,&flag)){
        return NULL;
    }
    StorjTelehash *m=(StorjTelehash *)PyCapsule_GetPointer(cobj,NULL);
    m->setStopFlag(flag);
    Py_RETURN_NONE;
}

static PyObject *telehashbinder_finalize(PyObject *self, 
                                             PyObject *args){
    PyObject *cobj=NULL;
    if (!PyArg_ParseTuple(args, "O",&cobj)){
        return NULL;
    }
    StorjTelehash *m=(StorjTelehash *)PyCapsule_GetPointer(cobj,NULL);
    ChannelHandlerFactoryImpl *h=(ChannelHandlerFactoryImpl *)
                                           m->getChannelHandlerFactory();
    delete h;
    delete m;
    Py_RETURN_NONE;
}

static PyObject *telehashbinder_getMyLocation(PyObject *self, 
                                             PyObject *args){
    PyObject *cobj=NULL;
    if (!PyArg_ParseTuple(args, "O",&cobj)){
        return NULL;
    }
    StorjTelehash *m=(StorjTelehash *)PyCapsule_GetPointer(cobj,NULL);
    string loc;
    m->getMyLocation(loc);
    PyObject *p= Py_BuildValue("s",loc.c_str());
    return p;
}

static PyObject *telehashbinder_getMyId(PyObject *self, 
                                             PyObject *args){
    PyObject *cobj=NULL;
    if (!PyArg_ParseTuple(args, "O",&cobj)){
        return NULL;
    }
    StorjTelehash *m=(StorjTelehash *)PyCapsule_GetPointer(cobj,NULL);
    string id;
    m->getMyId(id);
    PyObject *p= Py_BuildValue("s",id.c_str());
    return p;
}

static PyObject *telehashbinder_ping(PyObject *self, 
                                                PyObject *args){
    PyObject *cobj=NULL;
    char *location=NULL;
    if (!PyArg_ParseTuple(args, "Os",&cobj,&location)){
        return NULL;
    }
    StorjTelehash *m=(StorjTelehash *)PyCapsule_GetPointer(cobj,NULL);
    m->ping(location);
    Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
    {"init", telehashbinder_init, METH_VARARGS, 
        "initialize"},
    {"open_channel", telehashbinder_openChannel, METH_VARARGS,
         "open a channel"},
    {"ping", telehashbinder_ping, METH_VARARGS,
         "ping"},    {"start", telehashbinder_start, METH_VARARGS,
         "start receiving loop"},
    {"set_stopflag", telehashbinder_setStopFlag, METH_VARARGS,
         "stop receiving loop"},
    {"finalize", telehashbinder_finalize, METH_VARARGS
        , "finalize C object."},
    {"get_my_location", telehashbinder_getMyLocation, METH_VARARGS
        , "get my location."},
   {"get_my_id", telehashbinder_getMyId, METH_VARARGS
        , "get my id."},
    {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef modules = {
   PyModuleDef_HEAD_INIT,
   "telehashbinder",   /* name of module */
   "Messaging Layer in Telehash", /* module documentation, may be NULL */
   -1,       /* size of per-interpreter state of the module,
                or -1 if the module keeps state in global variables. */
   methods
   };

extern "C" PyMODINIT_FUNC PyInit_telehashbinder(void){
     return PyModule_Create(&modules);
}

#else
extern "C" PyMODINIT_FUNC
inittelehashbinder(void) {
    (void) Py_InitModule("telehashbinder", methods);
}

#endif
