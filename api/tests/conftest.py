# tests/conftest.py

import pytest
from fastapi.testclient import TestClient
import logging
import numpy as np
from app.main import app


class FakeParking:
    def get_stats(self):
        return {
            "free": 3,
            "occupied": 2,
            "total": 5,
            "fps": 27.5,
            "latency_ms": 18.3,
        }

    def get_render_data(self):
        coords = np.array([
            [[0, 0], [10, 0], [10, 10], [0, 10]],
        ])
        states = np.array([1])
        affine = {
            "valid": True,
            "matrix": [[1, 0, 0], [0, 1, 0]],
        }
        return coords, states, affine

    def evolve(self, frame):
        pass


@pytest.fixture
def client():
    """
    API client with a properly initialized parking system
    """
    app.state.parking_ = FakeParking()
    app.state.logger = logging.getLogger("test")
    return TestClient(app)


@pytest.fixture
def client_no_parking():
    """
    API client with parking system NOT initialized
    """
    app.state.parking_ = None
    return TestClient(app)