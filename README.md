# Chat-Cli

`chat-cli` is a **learning-focused, decentralized command-line chat application written in C**.  
The goal of this project is to **better understand the principles of**: networking, systems programming, and system design.

---

## 🎯 Project Goals

The main objectives of this project are:

- Build a peer-to-peer chat system in **C**
- Understand **networking**
- Explore **decentralized communication models**
- Design a simple **federated lookup / rendezvous mechanism**
- Gain hands-on experience with **systems programming**

This is a **learning experiment**, and many components are intentionally implemented “from scratch” to better understand how they work.

---

## 🏛️ High-Level Architecture

At a conceptual level, the system consists of:

### 1. CLI Client
- Runs in a terminal
- Manages user interaction
- Sends and receives messages
- Maintains local state (identity, known peers, keys, etc.)

### 2. Peer-to-Peer Communication
- Once peers discover each other, messages are exchanged **directly**
- No central server is involved in message delivery

### 3. Federated Lookup (Rendezvous Server)
- A small, simple server assists with:
  - Mapping user identities to network addresses
  - Helping peers find each other
- The server does **not** relay messages
- Once peers connect, communication is fully peer-to-peer

This model reflects how many real-world systems balance decentralization with practicality.

---

## 🖧 Decentralization Model

- No central message relay
- No centralized conversation storage
- Minimal infrastructure for discovery only

---

## 🔨 Build & Run
⚠️ Meant for POSIX-compliant operating systems, currently only tested on Linux. 

> Dependencies:
```
sqlite3 openssl
```

> Clone the project
```sh
git clone https://github.com/AtesIsf/chat-cli
cd chat-cli/
```

> Make and run the client
```sh
make keygen
make
./chat-cli <username>
```

> (Optional, not intended for client use) Make and run the lookup server
```sh
make lookup-keygen
make lookup
./lookup
```

> Cleanup binaries
```sh
make clean
```

---

## 🐳 Testing with Docker

The project includes a Docker environment to simulate a real network with multiple users and a lookup server.

### 1. Build the Docker Image
Ensure you have Docker and Docker Compose installed.
```sh
# Terminal 1
sudo docker-compose build --no-cache
```

### 2. Start the Lookup Server
Run the lookup server in the foreground to monitor registration and fetch requests:
```sh
sudo docker-compose up lookup
```

### 3. Run Interactive Clients
Open separate terminal windows for each user:
```sh
# Terminal 2
sudo docker-compose run user1
```

```sh
# Terminal 3
sudo docker-compose run user2

```sh
# Terminal 4
sudo docker-compose run user3
```

### 4. Cleanup
To stop all containers and remove the virtual network:
```sh
sudo docker-compose down --remove-orphans
```

---

## ⚠️ Disclaimer

This project is for **educational purposes only**.

Do **not** use this code to protect real communications or sensitive data.

