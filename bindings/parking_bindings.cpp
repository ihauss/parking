#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "smart_parking/ParkingSystem.h"
#include "smart_parking/Frame.h"

namespace py = pybind11;

//
// --- numpy -> cv::Mat ---
//

static Frame numpyToFrame(const py::array& array) {

    if (array.ndim() != 3)
        throw std::runtime_error("Frame must be 3D (H, W, C)");

    if (array.shape(2) != 3)
        throw std::runtime_error("Frame must have 3 channels");

    if (array.dtype().kind() != 'u' || array.dtype().itemsize() != 1)
        throw std::runtime_error("Frame must be uint8");

    if (!(array.flags() & py::array::c_style))
        throw std::runtime_error("Frame must be C-contiguous");

    int height = array.shape(0);
    int width  = array.shape(1);

    cv::Mat mat(height, width, CV_8UC3, const_cast<void*>(array.data()));

    return Frame(mat.clone()); // clone for safety
}

//
// --- Module ---
//

PYBIND11_MODULE(smart_parking_core, m) {

    //
    // --- PlaceState enum ---
    //
    py::enum_<PlaceState>(m, "PlaceState")
        .value("INIT_STATE", PlaceState::INIT_STATE)
        .value("FREE", PlaceState::FREE)
        .value("TRANSITION_IN", PlaceState::TRANSITION_IN)
        .value("OCCUPIED", PlaceState::OCCUPIED)
        .value("TRANSITION_OUT", PlaceState::TRANSITION_OUT)
        .export_values();


    //
    // --- RenderPlace ---
    //
    py::class_<RenderPlace>(m, "RenderPlace")
        .def_property_readonly("coords",
            [](const RenderPlace& p) {
                py::list pts;
                for (const auto& pt : p.coords) {
                    pts.append(py::make_tuple(pt.x, pt.y));
                }
                return pts;
            })
        .def_readonly("state", &RenderPlace::state);


    //
    // --- RenderSnapshot ---
    //
    py::class_<RenderSnapshot>(m, "RenderSnapshot")
        .def_readonly("places", &RenderSnapshot::places)
        .def_readonly("num_occupied", &RenderSnapshot::numOccupied)
        .def_readonly("num_places", &RenderSnapshot::numPlaces)
        .def_readonly("has_affine", &RenderSnapshot::hasAffine)
        .def_property_readonly("affine",
            [](const RenderSnapshot& s) {
                py::tuple t(6);
                for (size_t i = 0; i < 6; ++i)
                    t[i] = s.affine[i];
                return t;
            });

    py::class_<ParkingSystem>(m, "ParkingSystem")
        .def(py::init<>())

        .def("add_camera",
            [](ParkingSystem& self,
               const std::string& id,
               const std::string& json_path,
               py::array reference_array)
            {
                Frame ref = numpyToFrame(reference_array);
                self.addCamera(id, json_path, ref.data);
            })

        .def("remove_camera",
            &ParkingSystem::removeCamera)

        .def("restart_camera",
            [](ParkingSystem& self,
               const std::string& id,
               const std::string& json_path,
               py::array reference_array)
            {
                Frame ref = numpyToFrame(reference_array);
                self.restartCamera(id, json_path, ref.data);
            })

        .def("process_frame",
            [](ParkingSystem& self,
               const std::string& id,
               py::array frame_array)
            {
                Frame frame = numpyToFrame(frame_array);

                py::gil_scoped_release release;  // important
                self.processFrame(id, frame);
            })

        .def("get_stats",
            &ParkingSystem::getStats)

        .def("list_cameras",
            &ParkingSystem::listCameras)

        .def("is_healthy",
            &ParkingSystem::isHealthy)

        .def("get_snapshot", &ParkingSystem::getSnapshot)

        .def("get_cam_state_str", &ParkingSystem::getStateString)

        .def("get_last_update_seconds", &ParkingSystem::getLastUpdateSeconds);
}