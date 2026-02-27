"""
Global application state.

This will later host:
- Parking core instance
- Camera states
- Metrics collectors
"""

class AppState:
    def __init__(self):
        self.initialized = False

state = AppState()
