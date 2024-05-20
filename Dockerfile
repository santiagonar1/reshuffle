FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

RUN apt update -y && apt upgrade -y
RUN apt install -y build-essential \
    cmake openmpi-bin \
    libopenmpi-dev \
    python3 \
    python3-pip \
    python3-venv \
    python3-virtualenv \
    gdb \
    ninja-build

RUN pip install conan --break-system-packages
