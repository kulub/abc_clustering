#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <random>
#include <vector>
#include <array>

#include "problems.h"
#include "colonies.h"

#ifndef VECTOR_DIM
#define VECTOR_DIM static_cast<size_t>(2)
#endif


struct BeeColony {
	PyObject_HEAD
	ABCFuzzyClustering<VECTOR_DIM>* colony_impl;
	std::vector<std::array<double, VECTOR_DIM>>* vectors;
};

static PyTypeObject BeeColonyType = {
	PyVarObject_HEAD_INIT(NULL, 0)
};

struct ModBeeColony {
	PyObject_HEAD
	ModABCFuzzyClustering<VECTOR_DIM>* colony_impl;
	std::vector<std::array<double, VECTOR_DIM>>* vectors;
};

static PyTypeObject ModBeeColonyType = {
	PyVarObject_HEAD_INIT(NULL, 0)
};

static PyObject* test(PyObject* self, PyObject* args){
	long a, b;

	if (!PyArg_ParseTuple(args, "ll", &a, &b)) {
		return nullptr;
	}
	return PyLong_FromLong(a * b);
}

static PyMethodDef ABCMethods[] = {
	{"test",  test, METH_VARARGS,
	 "Multiply 2 longs."},
	{NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef abcmodule = {
	PyModuleDef_HEAD_INIT,
	"abc_plusplus",   /* name of module */
	"Artificial Bee Colony bindings", /* module documentation, may be NULL */
	-1,       /* size of per-interpreter state of the module,
				 or -1 if the module keeps state in global variables. */
	ABCMethods
};

static PyObject* BeeColony_new(PyTypeObject* type, PyObject* args, PyObject *kwds) {
	BeeColony* self;
	self = (BeeColony*)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->colony_impl = nullptr;
		self->vectors = nullptr;
	}

	return (PyObject*)self;
}

static int BeeColony_init(BeeColony* self, PyObject* args) {
	size_t population;
	size_t limit;
	
	size_t n_clusters;
	PyObject* vectors;

	if (!PyArg_ParseTuple(args, "OKKK", &vectors, &n_clusters, &population, &limit)) {
		return -1;
	}

	if (!PySequence_Check(vectors)) {
		return -1;
	}

	self->vectors = new std::vector<std::array<double, VECTOR_DIM>>();

	Py_ssize_t n_vectors = PySequence_Size(vectors);
	self->vectors->resize(n_vectors);
	for (Py_ssize_t vector_idx = 0; vector_idx < n_vectors; ++vector_idx) {
		PyObject* vector = PySequence_GetItem(vectors, vector_idx);

		if (!PySequence_Check(vector)) {
			return -1;
		}
		Py_ssize_t vector_dim = PySequence_Size(vector);
		if (vector_dim != VECTOR_DIM) {
			return -1;
		}

		for (size_t dimension = 0; dimension < VECTOR_DIM; ++dimension) {
			PyObject* value = PySequence_GetItem(vector, dimension);
			if (!PyObject_TypeCheck(value, &PyFloat_Type)) {
				return -1;
			}

			(*self->vectors)[vector_idx][dimension] = PyFloat_AsDouble(value);
		}
	}

	FuzzyClusteringParams<VECTOR_DIM> params;
	params.n_clusters = n_clusters;
	params.vectors = self->vectors;
	self->colony_impl = new ABCFuzzyClustering<VECTOR_DIM>(params, population, limit, ClassicMixingStrategy<FuzzyClustering<VECTOR_DIM>>(), std::mt19937_64());

	return 0;
}

static void BeeColony_dealloc(BeeColony* self) {
	delete self->colony_impl;
	delete self->vectors;
}

static PyObject* ABC_optimize(BeeColony* self, PyObject* args) {
	size_t cycles;

	if (!PyArg_ParseTuple(args, "K", &cycles)) {
		return nullptr;
	}

	self->colony_impl->optimize(cycles);

	FuzzyClustering<VECTOR_DIM> solution = self->colony_impl->get_champion().get_state();
	PyObject* result = PyList_New(solution.get_n_clusters());

	if (result == nullptr) {
		return nullptr;
	}

	for (size_t cluster_idx = 0; cluster_idx < solution.get_n_clusters(); ++cluster_idx) {
		PyObject* cluster_weights = PyList_New(self->vectors->size());

		if (cluster_weights == nullptr) {
			return nullptr;
		}

		if (PyList_SetItem(result, cluster_idx, cluster_weights) < 0) {
			return nullptr;
		}

		for (size_t vector_idx = 0; vector_idx < self->vectors->size(); ++vector_idx) {
			PyObject* value = PyFloat_FromDouble(solution.get_value()[cluster_idx * self->vectors->size() + vector_idx]);
			if (value == nullptr) {
				return nullptr;
			}

			if (PyList_SetItem(cluster_weights, vector_idx, value) < 0) {
				return nullptr;
			}
		}
	}

	return result;
}

static PyMethodDef BeeColony_methods[] = {
	{"optimize", (PyCFunction)ABC_optimize, METH_VARARGS,
	 "Runs the algorithm and returns the best solution"
	},
	{NULL}  /* Sentinel */
};

static PyObject* ModBeeColony_new(PyTypeObject* type, PyObject* args, PyObject *kwds) {
	ModBeeColony* self;
	self = (ModBeeColony*)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->colony_impl = nullptr;
		self->vectors = nullptr;
	}

	return (PyObject*)self;
}

static int ModBeeColony_init(ModBeeColony* self, PyObject* args) {
	size_t population;
	size_t limit;

	double f;
	double mr;

	size_t n_clusters;
	PyObject* vectors;

	if (!PyArg_ParseTuple(args, "OKKKdd", &vectors, &n_clusters, &population, &limit, &f, &mr)) {
		return -1;
	}

	if (!PySequence_Check(vectors)) {
		return -1;
	}

	self->vectors = new std::vector<std::array<double, VECTOR_DIM>>();

	Py_ssize_t n_vectors = PySequence_Size(vectors);
	self->vectors->resize(n_vectors);
	for (Py_ssize_t vector_idx = 0; vector_idx < n_vectors; ++vector_idx) {
		PyObject* vector = PySequence_GetItem(vectors, vector_idx);

		if (!PySequence_Check(vector)) {
			return -1;
		}
		Py_ssize_t vector_dim = PySequence_Size(vector);
		if (vector_dim != VECTOR_DIM) {
			return -1;
		}

		for (size_t dimension = 0; dimension < VECTOR_DIM; ++dimension) {
			PyObject* value = PySequence_GetItem(vector, dimension);
			if (!PyObject_TypeCheck(value, &PyFloat_Type)) {
				return -1;
			}

			(*self->vectors)[vector_idx][dimension] = PyFloat_AsDouble(value);
		}
	}

	FuzzyClusteringParams<VECTOR_DIM> params;
	params.n_clusters = n_clusters;
	params.vectors = self->vectors;
	self->colony_impl = new ModABCFuzzyClustering<VECTOR_DIM>(params, population, limit, DEMixingStrategy<FuzzyClustering<VECTOR_DIM>>(f, mr), std::mt19937_64());

	return 0;
}

static void ModBeeColony_dealloc(ModBeeColony* self) {
	delete self->colony_impl;
	delete self->vectors;
}

static PyObject* ModABC_optimize(ModBeeColony* self, PyObject* args) {
	size_t cycles;

	if (!PyArg_ParseTuple(args, "K", &cycles)) {
		return nullptr;
	}

	self->colony_impl->optimize(cycles);

	FuzzyClustering<VECTOR_DIM> solution = self->colony_impl->get_champion().get_state();
	PyObject* result = PyList_New(solution.get_n_clusters());

	if (result == nullptr) {
		return nullptr;
	}

	for (size_t cluster_idx = 0; cluster_idx < solution.get_n_clusters(); ++cluster_idx) {
		PyObject* cluster_weights = PyList_New(self->vectors->size());

		if (cluster_weights == nullptr) {
			return nullptr;
		}

		if (PyList_SetItem(result, cluster_idx, cluster_weights) < 0) {
			return nullptr;
		}

		for (size_t vector_idx = 0; vector_idx < self->vectors->size(); ++vector_idx) {
			PyObject* value = PyFloat_FromDouble(solution.get_value()[cluster_idx * self->vectors->size() + vector_idx]);
			if (value == nullptr) {
				return nullptr;
			}

			if (PyList_SetItem(cluster_weights, vector_idx, value) < 0) {
				return nullptr;
			}
		}
	}

	return result;
}

static PyMethodDef ModBeeColony_methods[] = {
	{"optimize", (PyCFunction)ModABC_optimize, METH_VARARGS,
	 "Runs the algorithm and returns the best solution"
	},
	{NULL}  /* Sentinel */
};

PyMODINIT_FUNC PyInit_abc_plusplus(void) {
	BeeColonyType.tp_name = "abc_plusplus.ArtificialBeeColony";
	BeeColonyType.tp_doc = "Simple Artificial Bee Colony";
	BeeColonyType.tp_basicsize = sizeof(BeeColony);
	BeeColonyType.tp_itemsize = 0;
	BeeColonyType.tp_flags = Py_TPFLAGS_DEFAULT;
	BeeColonyType.tp_new = BeeColony_new;
	BeeColonyType.tp_init = (initproc)BeeColony_init;
	BeeColonyType.tp_dealloc = (destructor)BeeColony_dealloc;
	BeeColonyType.tp_methods = BeeColony_methods;

	ModBeeColonyType.tp_name = "abc_plusplus.ModArtificialBeeColony";
	ModBeeColonyType.tp_doc = "Modified Artificial Bee Colony";
	ModBeeColonyType.tp_basicsize = sizeof(ModBeeColony);
	ModBeeColonyType.tp_itemsize = 0;
	ModBeeColonyType.tp_flags = Py_TPFLAGS_DEFAULT;
	ModBeeColonyType.tp_new = ModBeeColony_new;
	ModBeeColonyType.tp_init = (initproc)ModBeeColony_init;
	ModBeeColonyType.tp_dealloc = (destructor)ModBeeColony_dealloc;
	ModBeeColonyType.tp_methods = ModBeeColony_methods;

	PyObject *module;
	if (PyType_Ready(&BeeColonyType) < 0) {
		return nullptr;
	}

	if (PyType_Ready(&ModBeeColonyType) < 0) {
		return nullptr;
	}

	module = PyModule_Create(&abcmodule);
	if (module == nullptr) {
		return nullptr;
	}

	Py_INCREF(&BeeColonyType);
	if (PyModule_AddObject(module, "ArtificialBeeColony", (PyObject*)&BeeColonyType) < 0) {
		Py_DECREF(&BeeColonyType);
		Py_DECREF(module);
		return nullptr;
	}

	Py_INCREF(&ModBeeColonyType);
	if (PyModule_AddObject(module, "ModArtificialBeeColony", (PyObject*)&ModBeeColonyType) < 0) {
		Py_DECREF(&ModBeeColonyType);
		Py_DECREF(module);
		return nullptr;
	}

	return module;
}
