#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "smart_parking/ParkingSystem.h"
#include "smart_parking/Frame.h"

namespace py = pybind11;

//
// --- numpy -> cv::Mat ---
//

/**
 * @brief Converts a NumPy array into a safe smart_parking::Frame.
 *
 * Expected format:
 *  - shape: (H, W, 3)
 *  - dtype: uint8
 *  - contiguous (C-style)
 *  - RGB packed
 *
 * A clone is performed to ensure memory safety (Python buffer may be released).
 */
static smart_parking::Frame numpyToFrame(const py::array& array) {

    // --- Shape validation ---
    if (array.ndim() != 3)
        throw std::runtime_error(
            "Invalid frame shape: expected (H,W,3), got (" +
            std::to_string(array.ndim()) + " dims)"
        );

    if (array.shape(2) != 3)
        throw std::runtime_error("Frame must have 3 channels");

    // --- Type validation ---
    if (array.dtype().kind() != 'u' || array.dtype().itemsize() != 1)
        throw std::runtime_error("Frame must be uint8");

    // --- Memory layout validation ---
    if (!(array.flags() & py::array::c_style))
        throw std::runtime_error("Frame must be C-contiguous");

    if (array.strides(2) != 1)
        throw std::runtime_error("Channel stride must be 1");

    if (array.strides(1) != 3)
        throw std::runtime_error("Row must be packed RGB");

    int height = array.shape(0);
    int width  = array.shape(1);

    // Create cv::Mat view over numpy buffer
    cv::Mat mat(height, width, CV_8UC3, const_cast<void*>(array.data()));

    // IMPORTANT: clone to detach from Python memory
    return smart_parking::Frame(mat.clone());
}

//
// --- Module ---
//

PYBIND11_MODULE(smart_parking_core, m) {

    //
    // --- PlaceState enum ---
    //
    py::enum_<smart_parking::PlaceState>(m, "PlaceState")
        .value("INIT_STATE", smart_parking::PlaceState::INIT_STATE)
        .value("FREE", smart_parking::PlaceState::FREE)
        .value("TRANSITION_IN", smart_parking::PlaceState::TRANSITION_IN)
        .value("OCCUPIED", smart_parking::PlaceState::OCCUPIED)
        .value("TRANSITION_OUT", smart_parking::PlaceState::TRANSITION_OUT)
        .export_values();


    //
    // --- RenderPlace ---
    //
    py::class_<smart_parking::RenderPlace>(m, "RenderPlace")

        /**
         * @brief Returns polygon coordinates as list of (x,y)
         */
        .def_property_readonly("coords",
            [](const smart_parking::RenderPlace& p) {
                py::list pts;
                for (const auto& pt : p.coords) {
                    pts.append(py::make_tuple(pt.x, pt.y));
                }
                return pts;
            })

        /**
         * @brief Current logical state
         */
        .def_readonly("state", &smart_parking::RenderPlace::state);


    //
    // --- RenderSnapshot ---
    //
    py::class_<smart_parking::RenderSnapshot>(m, "RenderSnapshot")

        .def_readonly("places", &smart_parking::RenderSnapshot::places)
        .def_readonly("num_occupied", &smart_parking::RenderSnapshot::numOccupied)
        .def_readonly("num_places", &smart_parking::RenderSnapshot::numPlaces)
        .def_readonly("has_affine", &smart_parking::RenderSnapshot::hasAffine)

        /**
         * @brief Returns affine transform as tuple (a,b,c,d,e,f)
         */
        .def_property_readonly("affine",
            [](const smart_parking::RenderSnapshot& s) {
                py::tuple t(6);
                for (size_t i = 0; i < 6; ++i)
                    t[i] = s.affine[i];
                return t;
            });


    //
    // --- ParkingSystem ---
    //
    py::class_<smart_parking::ParkingSystem>(m, "ParkingSystem")
        .def(py::init<>())

        //
        // --- Camera management ---
        //
        .def("add_camera",
            [](smart_parking::ParkingSystem& self,
               const std::string& id,
               const std::string& json_path,
               py::array reference_array)
            {
                smart_parking::Frame ref = numpyToFrame(reference_array);
                self.addCamera(id, json_path, ref.data);
            })

        .def("remove_camera",
            &smart_parking::ParkingSystem::removeCamera)

        .def("restart_camera",
            [](smart_parking::ParkingSystem& self,
               const std::string& id,
               const std::string& json_path,
               py::array reference_array)
            {
                smart_parking::Frame ref = numpyToFrame(reference_array);
                self.restartCamera(id, json_path, ref.data);
            })

        .def("list_cameras",
            &smart_parking::ParkingSystem::listCameras)

        //
        // --- Processing ---
        //
        .def("process_frame",
            [](smart_parking::ParkingSystem& self,
               const std::string& id,
               py::array frame_array)
            {
                smart_parking::Frame frame = numpyToFrame(frame_array);

                // 🔥 VERY IMPORTANT: release GIL for heavy CV + async
                py::gil_scoped_release release;
                self.processFrame(id, frame);
            })

        //
        // --- Retrieval ---
        //
        .def("get_snapshot",
            &smart_parking::ParkingSystem::getSnapshot)

        .def("get_stats",
            &smart_parking::ParkingSystem::getStats)

        //
        // --- Monitoring ---
        //
        .def("is_healthy",
            &smart_parking::ParkingSystem::isHealthy)

        .def("get_cam_state_str",
            &smart_parking::ParkingSystem::getStateString)

        .def("get_last_update_seconds",
            &smart_parking::ParkingSystem::getLastUpdateSeconds);
}