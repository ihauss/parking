class FakePlace:
    def __init__(self):
        self.coords = [(0,0),(10,0),(10,10),(0,10)]
        self.state = 1


class FakeSnapshot:
    def __init__(self):
        self.places = [FakePlace()]
        self.num_occupied = 1
        self.num_places = 5
        self.has_affine = True
        self.affine = [1,0,0,0,1,0]