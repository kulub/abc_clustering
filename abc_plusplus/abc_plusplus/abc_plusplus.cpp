/*
Python bindings. Every specialization needs to be its own Python type, defined separately along with all of its methods.

The Python API only allows clustering of vectors of specific dimensionality. This dimensionality is specified by the VECTOR_DIM
preprocessor macro and defaults to 2.
*/

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <random>
#include <vector>
#include <array>

#include "abc.h"

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

struct TournamentBeeColony {
	PyObject_HEAD
	TournamentABCFuzzyClustering<VECTOR_DIM>* colony_impl;
	std::vector<std::array<double, VECTOR_DIM>>* vectors;
};

static PyTypeObject TournamentBeeColonyType = {
	PyVarObject_HEAD_INIT(NULL, 0)
};

struct TournamentModBeeColony {
	PyObject_HEAD
	TournamentModABCFuzzyClustering<VECTOR_DIM>* colony_impl;
	std::vector<std::array<double, VECTOR_DIM>>* vectors;
};

static PyTypeObject TournamentModBeeColonyType = {
	PyVarObject_HEAD_INIT(NULL, 0)
};


static PyMethodDef ABCMethods[] = {
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef abcmodule = {
	PyModuleDef_HEAD_INIT,
	"abc_plusplus",
	"Artificial Bee Colony bindings",
	-1,
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
	self->colony_impl = new ABCFuzzyClustering<VECTOR_DIM>(params, population, limit, ClassicMixingStrategy<FuzzyClustering<VECTOR_DIM>>(), RouletteSelectionStrategy(), std::mt19937_64());

	return 0;
}

static void BeeColony_dealloc(BeeColony* self) {
	delete self->colony_impl;
	delete self->vectors;
}

static PyObject* ABC_fit(BeeColony* self, PyObject* args) {
	size_t cycles;

	if (!PyArg_ParseTuple(args, "K", &cycles)) {
		return nullptr;
	}

	self->colony_impl->optimize(cycles);

	Py_RETURN_NONE;
}

static PyObject* ABC_score(BeeColony* self, PyObject* args) {
	return PyFloat_FromDouble(self->colony_impl->get_champion().get_fitness());
}

static PyObject* ABC_optimize(BeeColony* self, PyObject* args) {
	Py_DECREF(ABC_fit(self, args));

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
	{"fit", (PyCFunction)ABC_fit, METH_VARARGS,
	 "Runs the algorithm, without returning anything"
	},
	{"score", (PyCFunction)ABC_score, METH_VARARGS,
	 "Returns the score of the best solution"
	},
	{NULL}
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
	self->colony_impl = new ModABCFuzzyClustering<VECTOR_DIM>(params, population, limit, DEMixingStrategy<FuzzyClustering<VECTOR_DIM>>(f, mr), RouletteSelectionStrategy(), std::mt19937_64());

	return 0;
}

static void ModBeeColony_dealloc(ModBeeColony* self) {
	delete self->colony_impl;
	delete self->vectors;
}

static PyObject* ModABC_fit(ModBeeColony* self, PyObject* args) {
	size_t cycles;

	if (!PyArg_ParseTuple(args, "K", &cycles)) {
		return nullptr;
	}

	self->colony_impl->optimize(cycles);

	Py_RETURN_NONE;
}

static PyObject* ModABC_score(ModBeeColony* self, PyObject* args) {
	return PyFloat_FromDouble(self->colony_impl->get_champion().get_fitness());
}

static PyObject* ModABC_optimize(ModBeeColony* self, PyObject* args) {
	Py_DECREF(ModABC_fit(self, args));

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
	{"fit", (PyCFunction)ModABC_fit, METH_VARARGS,
	 "Runs the algorithm, without returning anything"
	},
	{"score", (PyCFunction)ModABC_score, METH_VARARGS,
	 "Returns the score of the best solution"
	},
	{NULL}
};

static PyObject* TournamentBeeColony_new(PyTypeObject* type, PyObject* args, PyObject *kwds) {
	TournamentBeeColony* self;
	self = (TournamentBeeColony*)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->colony_impl = nullptr;
		self->vectors = nullptr;
	}

	return (PyObject*)self;
}

static int TournamentBeeColony_init(TournamentBeeColony* self, PyObject* args) {
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
	self->colony_impl = new TournamentABCFuzzyClustering<VECTOR_DIM>(params, population, limit, ClassicMixingStrategy<FuzzyClustering<VECTOR_DIM>>(), TournamentSelectionStrategy(), std::mt19937_64());

	return 0;
}

static void TournamentBeeColony_dealloc(TournamentBeeColony* self) {
	delete self->colony_impl;
	delete self->vectors;
}

static PyObject* TournamentABC_fit(TournamentBeeColony* self, PyObject* args) {
	size_t cycles;

	if (!PyArg_ParseTuple(args, "K", &cycles)) {
		return nullptr;
	}

	self->colony_impl->optimize(cycles);

	Py_RETURN_NONE;
}

static PyObject* TournamentABC_score(TournamentBeeColony* self, PyObject* args) {
	return PyFloat_FromDouble(self->colony_impl->get_champion().get_fitness());
}

static PyObject* TournamentABC_optimize(TournamentBeeColony* self, PyObject* args) {
	Py_DECREF(TournamentABC_fit(self, args));

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

static PyMethodDef TournamentBeeColony_methods[] = {
	{"optimize", (PyCFunction)TournamentABC_optimize, METH_VARARGS,
	 "Runs the algorithm and returns the best solution"
	},
	{"fit", (PyCFunction)TournamentABC_fit, METH_VARARGS,
	 "Runs the algorithm, without returning anything"
	},
	{"score", (PyCFunction)TournamentABC_score, METH_VARARGS,
	 "Returns the score of the best solution"
	},
	{NULL}
};

static PyObject* TournamentModBeeColony_new(PyTypeObject* type, PyObject* args, PyObject *kwds) {
	TournamentModBeeColony* self;
	self = (TournamentModBeeColony*)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->colony_impl = nullptr;
		self->vectors = nullptr;
	}

	return (PyObject*)self;
}

static int TournamentModBeeColony_init(TournamentModBeeColony* self, PyObject* args) {
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
	self->colony_impl = new TournamentModABCFuzzyClustering<VECTOR_DIM>(params, population, limit, DEMixingStrategy<FuzzyClustering<VECTOR_DIM>>(f, mr), TournamentSelectionStrategy(), std::mt19937_64());

	return 0;
}

static void TournamentModBeeColony_dealloc(TournamentModBeeColony* self) {
	delete self->colony_impl;
	delete self->vectors;
}

static PyObject* TournamentModABC_fit(TournamentModBeeColony* self, PyObject* args) {
	size_t cycles;

	if (!PyArg_ParseTuple(args, "K", &cycles)) {
		return nullptr;
	}

	self->colony_impl->optimize(cycles);

	Py_RETURN_NONE;
}

static PyObject* TournamentModABC_score(TournamentModBeeColony* self, PyObject* args) {
	return PyFloat_FromDouble(self->colony_impl->get_champion().get_fitness());
}

static PyObject* TournamentModABC_optimize(TournamentModBeeColony* self, PyObject* args) {
	Py_DECREF(TournamentModABC_fit(self, args));

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

static PyMethodDef TournamentModBeeColony_methods[] = {
	{"optimize", (PyCFunction)TournamentModABC_optimize, METH_VARARGS,
	 "Runs the algorithm and returns the best solution"
	},
	{"fit", (PyCFunction)TournamentModABC_fit, METH_VARARGS,
	 "Runs the algorithm, without returning anything"
	},
	{"score", (PyCFunction)TournamentModABC_score, METH_VARARGS,
	 "Returns the score of the best solution"
	},
	{NULL} 
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

	TournamentBeeColonyType.tp_name = "abc_plusplus.TournamentArtificialBeeColony";
	TournamentBeeColonyType.tp_doc = "Artificial Bee Colony with Tournament selection strategy";
	TournamentBeeColonyType.tp_basicsize = sizeof(TournamentBeeColony);
	TournamentBeeColonyType.tp_itemsize = 0;
	TournamentBeeColonyType.tp_flags = Py_TPFLAGS_DEFAULT;
	TournamentBeeColonyType.tp_new = TournamentBeeColony_new;
	TournamentBeeColonyType.tp_init = (initproc)TournamentBeeColony_init;
	TournamentBeeColonyType.tp_dealloc = (destructor)TournamentBeeColony_dealloc;
	TournamentBeeColonyType.tp_methods = TournamentBeeColony_methods;

	TournamentModBeeColonyType.tp_name = "abc_plusplus.TournamentModArtificialBeeColony";
	TournamentModBeeColonyType.tp_doc = "Modified Artificial Bee Colony with Tournament selection strategy";
	TournamentModBeeColonyType.tp_basicsize = sizeof(TournamentModBeeColony);
	TournamentModBeeColonyType.tp_itemsize = 0;
	TournamentModBeeColonyType.tp_flags = Py_TPFLAGS_DEFAULT;
	TournamentModBeeColonyType.tp_new = TournamentModBeeColony_new;
	TournamentModBeeColonyType.tp_init = (initproc)TournamentModBeeColony_init;
	TournamentModBeeColonyType.tp_dealloc = (destructor)TournamentModBeeColony_dealloc;
	TournamentModBeeColonyType.tp_methods = TournamentModBeeColony_methods;

	PyObject *module;
	if (PyType_Ready(&BeeColonyType) < 0) {
		return nullptr;
	}

	if (PyType_Ready(&ModBeeColonyType) < 0) {
		return nullptr;
	}

	if (PyType_Ready(&TournamentBeeColonyType) < 0) {
		return nullptr;
	}

	if (PyType_Ready(&TournamentModBeeColonyType) < 0) {
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

	Py_INCREF(&TournamentBeeColonyType);
	if (PyModule_AddObject(module, "TournamentArtificialBeeColony", (PyObject*)&TournamentBeeColonyType) < 0) {
		Py_DECREF(&TournamentBeeColonyType);
		Py_DECREF(module);
		return nullptr;
	}

	Py_INCREF(&TournamentModBeeColonyType);
	if (PyModule_AddObject(module, "TournamentModArtificialBeeColony", (PyObject*)&TournamentModBeeColonyType) < 0) {
		Py_DECREF(&TournamentModBeeColonyType);
		Py_DECREF(module);
		return nullptr;
	}

	return module;
}
