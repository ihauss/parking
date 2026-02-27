def test_stats_ok(client):
    response = client.get("/parking/stats")

    assert response.status_code == 200
    data = response.json()

    assert data["free_places"] == 3
    assert data["occupied_places"] == 2
    assert data["total_places"] == 5


def test_stats_not_initialized(client_no_parking):
    response = client_no_parking.get("/parking/stats")

    assert response.status_code == 503
    assert response.json()["detail"] == "Parking system not initialized"