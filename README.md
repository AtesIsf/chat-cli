# Chat-Cli

`chat-cli` is a **learning-focused, decentralized command-line chat application written in C**.  
The goal of this project is to **better understand the principles of**: networking, cryptography, systems programming, and system design.

---

## 🎯 Project Goals

The main objectives of this project are:

- Build a peer-to-peer chat system in **C**
- Understand **networking**
- Explore **decentralized communication models**
- Implement **public-key cryptography for learning purposes**
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

### 3. Identity & Cryptography
- Cryptography is implemented manually **for educational purposes**
- Asymmetric cryptography is used for:
  - Authentication
  - Key exchange

### 4. Federated Lookup (Rendezvous Server)
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

> Clone the project
```sh
git clone https://github.com/AtesIsf/chat-cli
cd chat-cli/
```

> Make and run the client
```sh
make
./chat-cli
```

> (Optional, not intended for client use) Make and run the lookup server
```sh
make lookup
./lookup
```

> Cleanup binaries
```sh
make clean
```

---

## 🛈 Project Status

This project is **actively evolving**.

- APIs are unstable
- Architecture may change
- Files and modules will be refactored frequently

Expect breaking changes.

---

## ⚠️ Disclaimer

This project is for **educational purposes only**.

Do **not** use this code to protect real communications or sensitive data.

