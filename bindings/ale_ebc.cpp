#include "ale_ebc.hpp"
#include "ale_interface.hpp"
#include "nlohmann/json.hpp"
#include "pybind11_json/pybind11_json.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace ale_ebc;
using nlohmann::json;

PYBIND11_MODULE(ale_ebc,m){
    py::class_<Ale_Ebc>(m,"Ale_Ebc")
        .def(py::init<>())
        .def(py::init<const json& >())
        .def("reset",[](Ale_Ebc &self,vector <int> &observations, long long _seed) {
            bool rv;
            rv = self.reset(observations,_seed);
            return make_tuple(rv,observations);
        })
        .def("reset",[](Ale_Ebc &self,vector < vector<int> > &observations, long long _seed){
            bool rv;
            rv = self.reset(observations,_seed);
            return make_tuple(rv,observations); 
        })
        .def("step",[](Ale_Ebc &self,const double action,vector <int> &observations){
            bool rv;
            double reward;
            rv = self.step(action,observations,reward);
            return make_tuple(rv,observations,reward);
        })
        .def("step",[](Ale_Ebc &self,const double action,vector < vector <int> > &observations){ 
            bool rv;
            double reward;
            rv = self.step(action,observations,reward);
            return make_tuple(rv,observations,reward);
        });
}
