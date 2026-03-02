# Use a modern Ubuntu base
FROM ubuntu:24.04

# Avoid interactive prompts during apt install
ENV DEBIAN_FRONTEND=noninteractive

# Install gcc-14 and dependencies
RUN apt-get update && apt-get install -y \
    gcc-14 \
    build-essential \
    libsqlite3-dev \
    libssl-dev \
    openssl \
    && rm -rf /var/lib/apt/lists/*

# Point the 'gcc' command to gcc-14 so the Makefile works without changes
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100

WORKDIR /app

# The .dockerignore handles skipping host binaries
COPY . .

# Build both the client and the lookup server
RUN mkdir -p bin && make clean && make chat-cli lookup

RUN make keygen && make lookup-keygen

CMD ["./chat-cli"]
