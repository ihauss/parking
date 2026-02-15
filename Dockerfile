# =========================
# Build stage
# =========================
FROM debian:bookworm AS build

ENV DEBIAN_FRONTEND=noninteractive

# ---- Install build dependencies ----
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libopencv-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy entire project
COPY . .

# ---- Configure and build ----
RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/opencv4 \
 && cmake --build build -j$(nproc)

# ---- Run tests (fail build if tests fail) ----
RUN ctest --test-dir build --output-on-failure


# =========================
# Runtime stage
# =========================
FROM debian:bookworm

ENV DEBIAN_FRONTEND=noninteractive

# ---- Install only runtime dependencies (lighter than -dev) ----
RUN apt-get update && apt-get install -y \
    libopencv-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*


WORKDIR /app

# ---- Copy embedded executable ----
COPY --from=build /app/build/embedded/smart_parking_embedded /usr/local/bin/smart_parking

# ---- Copy runtime assets ----
COPY files /app/files

# ---- Default entrypoint ----
ENTRYPOINT ["smart_parking"]
CMD []
