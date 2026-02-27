#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "smart_parking/Parking.h"

namespace py = pybind11;

cv::Mat numpy_to_mat(py::array_t<uint8_t>& input) {
    auto buf = input.request();

    if (buf.ndim != 3 || buf.shape[2] != 3) {
        throw std::runtime_error("Expected HxWx3 uint8 image");
    }

    if (!(input.flags() & py::array::c_style)) {
        throw std::runtime_error("Input array must be C-contiguous");
    }

    int rows = static_cast<int>(buf.shape[0]);
    int cols = static_cast<int>(buf.shape[1]);

    return cv::Mat(
        rows,
        cols,
        CV_8UC3,
        buf.ptr
    );
}

PYBIND11_MODULE(smart_parking_core, m) {
    m.doc() = "Smart Parking C++ core bindings";

    py::class_<Parking>(m, "Parking")
        .def(
            py::init([](const std::string& path, py::array_t<uint8_t>& ref) {
                cv::Mat reference = numpy_to_mat(ref);
                return std::make_unique<Parking>(path, reference);
            }),
            py::arg("config_path"),
            py::arg("reference_image"),
            "Create a Parking system from config and reference image"
        )

        .def(
            "evolve",
            [](Parking& self, py::array_t<uint8_t>& image) {
                cv::Mat frame = numpy_to_mat(image);
                self.evolve(frame);
            },
            py::arg("image"),
            "Process one frame"
        )

        .def(
            "get_stats",
            &Parking::getStats,
            "Return system metrics"
        )

        .def(
            "get_render_data",
            [](Parking& self) {
                const auto& places = self.getRenderData();
                const ssize_t N = static_cast<ssize_t>(places.size());

                py::array_t<int> coords(
                    std::vector<ssize_t>{N, 4, 2}
                );
                py::array_t<int> states(
                    std::vector<ssize_t>{N}
                );

                auto coords_buf = coords.mutable_unchecked<3>();
                auto states_buf = states.mutable_unchecked<1>();

                for (ssize_t i = 0; i < N; ++i) {
                    states_buf(i) = static_cast<int>(places[i].state);
                    for (int j = 0; j < 4; ++j) {
                        coords_buf(i, j, 0) = places[i].coords[j].x;
                        coords_buf(i, j, 1) = places[i].coords[j].y;
                    }
                }

                py::dict affine;
                affine["valid"] = self.hasAffine();

                if (self.hasAffine()) {
                    auto A = self.getAffine();
                    affine["matrix"] = py::array_t<double>({2, 3}, A.data());
                }

                return py::make_tuple(coords, states, affine);
            }
        )


        .def(
            "get_num_places",
            &Parking::getNumPlace
        )

        .def(
            "get_num_occupied",
            &Parking::getNumOccupied
        );
}
