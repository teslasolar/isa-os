# ════════════════════════════════════════════════════════════════════════
# ISA-OS Docker Image
# Build ISA-OS and run in QEMU
# ════════════════════════════════════════════════════════════════════════

FROM ubuntu:22.04

LABEL maintainer="ISA-OS Project"
LABEL description="Industrial Automation Operating System"
LABEL version="1.0"

# Prevent interactive prompts during build
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Install build dependencies
RUN apt-get update && apt-get install -y \
    nasm \
    gcc \
    gcc-multilib \
    binutils \
    make \
    qemu-system-x86 \
    python3 \
    python3-pip \
    git \
    vim \
    curl \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Create workspace
WORKDIR /isa-os

# Copy source code
COPY . .

# Build ISA-OS
RUN make clean && make

# Generate index
RUN python3 scripts/generate_index.py || true

# Expose serial port for web terminal
EXPOSE 4444

# Default command: run ISA-OS in QEMU with serial on TCP
CMD ["qemu-system-i386", \
     "-drive", "file=build/isa-os.img,format=raw", \
     "-serial", "tcp:0.0.0.0:4444,server,nowait", \
     "-nographic"]
