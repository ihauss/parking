def test_render_ok(client):
    response = client.get("/parking/render")

    assert response.status_code == 200
    data = response.json()

    assert "places" in data
    assert "transform" in data

    assert len(data["places"]) == 1

    place = data["places"][0]
    assert len(place["coords"]) == 4
    assert isinstance(place["state"], int)

    transform = data["transform"]
    assert transform["valid"] is True
    assert len(transform["matrix"]) == 2


def test_render_not_initialized(client_no_parking):
    response = client_no_parking.get("/parking/render")

    assert response.status_code == 503
    assert response.json()["detail"] == "Parking system not initialized"