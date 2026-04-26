FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build gdb clang-format \
    python3-dev python3-pytest python3-jupyter-core jupyter-notebook python3-matplotlib \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
