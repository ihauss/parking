# =========================
# Build stage
# =========================
FROM debian:bookworm AS build

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libopencv-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j$(nproc)

# =========================
# Runtime stage
# =========================
FROM debian:bookworm

ENV DEBIAN_FRONTEND=noninteractive

# ---- Install runtime dependencies ----
RUN apt-get update && apt-get install -y \
    libopencv-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# ---- Set workdir ----
WORKDIR /app

# ---- Copy executable ----
COPY --from=build /app/build/smart_parking /usr/local/bin/smart_parking

# ---- Copy runtime assets ----
COPY files /app/files

# ---- Copy output assets ----
COPY output /app/output

ENTRYPOINT ["smart_parking"]
CMD []