from fastapi import APIRouter, Request

router = APIRouter()


@router.get("/health")
def health_check(request: Request):
    """
    System health check.
    """
    parking = request.app.state.parking
    return {
        "parking_loaded": parking is not None
    }


@router.post("/init")
def initialize_system():
    """
    Initialize parking system.
    """
    return {"detail": "Not implemented"}
