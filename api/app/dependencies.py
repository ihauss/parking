"""
FastAPI dependencies.

Provides access to shared application services such as the ParkingSystem.
"""

from fastapi import HTTPException
from app.runtime import runtime
from smart_parking_core import ParkingSystem


def get_parking_system() -> ParkingSystem:
    """
    Retrieve the global ParkingSystem instance.

    This dependency ensures that the system has been properly initialized
    before handling any request.

    Returns:
        ParkingSystem: The initialized parking system instance

    Raises:
        HTTPException: If the system is not initialized (503 Service Unavailable)
    """
    if not runtime.initialized or runtime.parking_system is None:
        raise HTTPException(
            status_code=503,
            detail="Parking system not initialized"
        )

    return runtime.parking_system