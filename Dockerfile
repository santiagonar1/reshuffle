FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# TODO: this needs to be updated before setting up the CI in GitHub. We are missing GCC-14 and Clang 19
RUN apt update -y && apt upgrade -y
RUN apt install -y build-essential \
    cmake openmpi-bin \
    libopenmpi-dev \
    python3 \
    python3-pip \
    python3-venv \
    python3-virtualenv \
    gdb \
    ninja-build \
    libopenblas-dev liblapacke-dev \
    libscalapack-mpi-dev \
    libomp-dev

RUN pip install conan --break-system-packages
