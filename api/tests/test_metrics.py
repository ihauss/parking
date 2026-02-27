def test_metrics_ok(client):
    response = client.get("/metrics")

    assert response.status_code == 200
    data = response.json()

    assert "fps" in data
    assert "last_latency_ms" in data

    assert isinstance(data["fps"], (int, float))
    assert isinstance(data["last_latency_ms"], (int, float))

    assert data["fps"] == 27.5
    assert data["last_latency_ms"] == 18.3

def test_metrics_not_initialized(client_no_parking):
    response = client_no_parking.get("/metrics")

    assert response.status_code == 503
    assert response.json()["detail"] == "Parking system not initialized"

def test_metrics_internal_error(client, monkeypatch):
    def crash():
        raise RuntimeError("boom")

    monkeypatch.setattr(
        client.app.state.parking_,
        "get_stats",
        crash
    )

    response = client.get("/metrics")

    assert response.status_code == 500
    assert response.json()["detail"] == "Internal server error"