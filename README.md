# Character Device Driver

A simple Linux kernel **character device** driver developed for educational purposes. The module demonstrates how to:

* Register a character device dynamically and expose it under `/dev` (e.g. `/dev/resul`).
* Support **read**, **write**, **poll/select** (non-blocking I/O) operations.
* Implement **ioctl** commands to retrieve buffer size, clear the internal buffer, and reverse stored data.
* Use a **kernel timer** and **wait-queue** to notify user-space when data is ready.
* Handle basic memory management and user–kernel data transfers (`copy_to_user`, `copy_from_user`).

---

## Directory Structure

```
CharacterDevice.c   # Kernel module source
README.md           # You are here
```

---

## Building & Loading

1. Ensure you have the correct kernel headers installed:
   ```bash
   sudo apt install build-essential linux-headers-$(uname -r)
   ```
2. Build the module:
   ```bash
   make
   ```
3. Load the module:
   ```bash
   sudo insmod CharacterDevice.ko
   ```
4. Verify it is loaded:
   ```bash
   dmesg | tail
   ls -l /dev/resul
   ```

---

## Usage

```bash
# Write data
echo "Hello kernel!" > /dev/resul

# Read data (will block until timer expires)
cat /dev/resul

# Example ioctl usage (get buffer size)
./ioctl-example /dev/resul GET_SIZE
```

> **Note**: Example user-space tools are left as an exercise – see the IOCTL definitions in `CharacterDevice.c`.

---

## Cleaning Up

```bash
sudo rmmod CharacterDevice
make clean
```

---

## License

This project is released under the **GPL-2.0** license, the same license used by the Linux kernel.
