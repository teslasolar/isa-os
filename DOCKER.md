# ISA-OS Docker Guide

Run ISA-OS in Docker for easy deployment and testing.

## Quick Start

### Pull from GitHub Container Registry

```bash
docker pull ghcr.io/teslasolar/isa-os:latest
docker run -it -p 4444:4444 ghcr.io/teslasolar/isa-os:latest
```

### Using Docker Compose (Recommended)

```bash
git clone https://github.com/teslasolar/isa-os.git
cd isa-os
docker-compose up
```

Then open browser to: **http://localhost:7681**

## Build Locally

```bash
# Clone repository
git clone https://github.com/teslasolar/isa-os.git
cd isa-os

# Build Docker image
docker build -t isa-os:local .

# Run container
docker run -it -p 4444:4444 isa-os:local
```

## Access Methods

### 1. Serial Port (TCP)

Connect to QEMU serial console:

```bash
nc localhost 4444
```

Or use telnet:

```bash
telnet localhost 4444
```

### 2. Web Terminal (docker-compose)

```bash
docker-compose up
```

Open browser: **http://localhost:7681**

### 3. Interactive Shell

```bash
docker exec -it isa-os /bin/bash
# Then inside container:
make run
```

## Configuration

### Environment Variables

```bash
docker run -it \
  -e ISA_OS_DEBUG=1 \
  -p 4444:4444 \
  ghcr.io/teslasolar/isa-os:latest
```

### Volume Mounts

Mount custom config:

```bash
docker run -it \
  -v $(pwd)/config:/isa-os/config \
  -p 4444:4444 \
  ghcr.io/teslasolar/isa-os:latest
```

## Docker Compose Services

```yaml
services:
  isa-os:          # Main OS in QEMU
  web-terminal:    # Web-based terminal (port 7681)
```

### Start Services

```bash
# Start all services
docker-compose up

# Start in background
docker-compose up -d

# View logs
docker-compose logs -f

# Stop services
docker-compose down
```

## Development Workflow

### 1. Build and Test

```bash
# Build image
docker build -t isa-os:dev .

# Run with volume mount for live development
docker run -it \
  -v $(pwd):/isa-os \
  -p 4444:4444 \
  isa-os:dev \
  bash

# Inside container, rebuild:
make clean && make && make run
```

### 2. Debugging

```bash
# Run with GDB server
docker run -it \
  -p 4444:4444 \
  -p 1234:1234 \
  isa-os:dev \
  qemu-system-i386 \
    -drive file=build/isa-os.img,format=raw \
    -serial tcp:0.0.0.0:4444,server,nowait \
    -s -S
```

Then connect with GDB:

```bash
gdb
(gdb) target remote localhost:1234
(gdb) continue
```

## CI/CD

GitHub Actions automatically builds and publishes Docker images on:
- Push to main/master
- Version tags (v1.0, v1.1, etc.)
- Pull requests (build only, no publish)

### Image Tags

```bash
# Latest from main branch
ghcr.io/teslasolar/isa-os:latest

# Specific version
ghcr.io/teslasolar/isa-os:v1.0

# Branch name
ghcr.io/teslasolar/isa-os:main
```

## Troubleshooting

### Port Already in Use

```bash
# Check what's using port 4444
lsof -i :4444

# Use different port
docker run -it -p 5555:4444 isa-os:latest
```

### Container Won't Start

```bash
# Check logs
docker logs isa-os

# Run interactively
docker run -it isa-os:latest /bin/bash
```

### QEMU Issues

```bash
# Check QEMU version
docker run isa-os:latest qemu-system-i386 --version

# Test image manually
docker run -it isa-os:latest \
  qemu-system-i386 -drive file=build/isa-os.img,format=raw -nographic
```

## Production Deployment

### Docker Swarm

```bash
docker stack deploy -c docker-compose.yml isa-os
```

### Kubernetes

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: isa-os
spec:
  replicas: 1
  selector:
    matchLabels:
      app: isa-os
  template:
    metadata:
      labels:
        app: isa-os
    spec:
      containers:
      - name: isa-os
        image: ghcr.io/teslasolar/isa-os:latest
        ports:
        - containerPort: 4444
```

## Security

### Run as Non-Root

Add to Dockerfile:

```dockerfile
RUN useradd -m isa-os
USER isa-os
```

### Read-Only Filesystem

```bash
docker run -it --read-only \
  -v /tmp \
  isa-os:latest
```

## Performance

### CPU Limits

```bash
docker run -it \
  --cpus="2" \
  --memory="1g" \
  isa-os:latest
```

### Monitoring

```bash
# Resource usage
docker stats isa-os

# Inspect container
docker inspect isa-os
```

## Registry

### GitHub Container Registry

Images hosted at: `ghcr.io/teslasolar/isa-os`

### Docker Hub (Alternative)

```bash
docker pull teslasolar/isa-os:latest
```

## Examples

### Run Multiple Instances

```bash
# Instance 1
docker run -d --name isa-os-1 -p 4444:4444 isa-os:latest

# Instance 2
docker run -d --name isa-os-2 -p 4445:4444 isa-os:latest

# Instance 3
docker run -d --name isa-os-3 -p 4446:4444 isa-os:latest
```

### Custom Configuration

```bash
# Mount custom YAML config
docker run -it \
  -v $(pwd)/my-config.yaml:/isa-os/config/isa-os.yaml \
  -p 4444:4444 \
  isa-os:latest
```

### Automated Testing

```bash
# Run tests in CI
docker run --rm isa-os:latest make test

# Check build
docker run --rm isa-os:latest make clean build
```

## Resources

- Dockerfile: `Dockerfile`
- Compose file: `docker-compose.yml`
- CI/CD: `.github/workflows/docker-build.yml`
- Registry: https://github.com/teslasolar/isa-os/pkgs/container/isa-os

---

**Need Help?**
- GitHub Issues: https://github.com/teslasolar/isa-os/issues
- Documentation: See README.md
