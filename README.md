# Smart Parking
## Overview
## Demo
## Installation
## Usage

The application can be built and executed entirely through Docker.
Command-line arguments are forwarded directly to the executable, preserving the standard `argc / argv` behavior.

---

### Command-line arguments

| Argument | Description |
|--------|-------------|
| `--headless` | Run the application without displaying the video window |
| `--rec` | Record the processed video |
| `--output <filename>.mp4` | Set a custom output video name |

---

### Build the Docker image

```bash
docker build -t smart_parking .
```

### Run the application
The app launch mode with docker need to be headless. If you want to monitor the result you can record or give docker access to visual environement.

```bash
docker run --rm smart_parking --headless
```

### Run the application (Visual Linux)
```bash
xhost -local:docker
```

```bash
docker run --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  smart_parking
```

### Run with command-line arguments

```bash
docker run --rm smart_parking --headless --rec --output result.mp4
```

**Run tests** :
Unit tests are executed during the Docker build stage to ensure code correctness.

To manually run tests inside a development container, you can use the build stage image:

```bash
docker run -it --rm --entrypoint bash smart_parking
```

```bash
cd build
ctest
```

## Performance
## Architecture
## Technical Choices
## Testing
## Challenges
## Limitations
## Future Work
## Conclusion
