# Capstone Project 06: Multithreaded Chunked File Transfer Protocol

---

## 🎯 Overview

Transferring large files across networks reliably requires handling chunking, checksum verification, out-of-order reassembly, and transfer resumption.

This project implements a chunked file transfer protocol from scratch:
1. **Chunk Framing & Metadata**: Every chunk is packed with a binary header containing `file_id`, `chunk_index`, `total_chunks`, `payload_size`, and CRC32 payload checksum.
2. **End-to-End Integrity Verification**: Uses CRC32 to detect transmission corruption on both a per-chunk and whole-file basis.
3. **Pipeline Transfer Engine**: Transmits file chunks across stream sockets with progress reporting (bytes transferred, percentage, elapsed time, throughput in MB/s).
4. **Reassembly Verification**: Validates that destination byte-streams match the original source file with zero corrupted bytes.

---

## 🚀 Build and Run

```bash
make
make run
```
