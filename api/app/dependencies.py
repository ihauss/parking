"""
FastAPI dependencies.

Provides access to shared application services.
"""

from fastapi import HTTPException
from app.runtime import runtime
from smart_parking_core import ParkingSystem


def get_parking_system() -> ParkingSystem:
    """
    Dependency that returns the global ParkingSystem instance.
    Ensures it has been initialized.
    """
    if not runtime.initialized or runtime.parking_system is None:
        raise HTTPException(status_code=503, detail="Parking system not initialized")

    return runtime.parking_system