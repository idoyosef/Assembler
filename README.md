# 💻 C Assembler – Final Project (Maman 14) 💻
This is the **Final Project (Maman 14)** for the **Laboratory in Systems Programming (20465)** course at the **Open University of Israel**, completed in **semester 2024B**. It is a fully functional **assembler** written in **C** for a semi-imaginary assembly language, following the requirements outlined in the official course booklet. The purpose of this project is to simulate the operation of a real system-level program and gain experience in developing a medium-scale modular software system.

___

## 📖 Table of Contents
- [Project Structure](#-project-structure)
- [Getting Started](#-getting-started)
- [Usage](#-usage)
- [Testing](#-testing)
- [Documentation](#-documentation)

___

## 🧩 Project Structure

The project is organized into multiple **`.c`** and **`.h`** files, each responsible for a specific part of the assembler.  
The assembler operates in **three main stages**:

1. **Pre-Processing**  
2. **First Pass**  
3. **Second Pass**  

These stages form the **core algorithm** of the assembler.

### **Pre-Processing**
Reads the source file line by line, detects **macro definitions**, stores them in a macro table, and replaces macro calls in the source file before further processing.

### **First Pass**
Scans through the preprocessed code, builds the **symbol table** (labels and their addresses), and calculates memory locations.

### **Second Pass**
Generates the final **machine code**, replacing operation mnemonics with binary equivalents and label references with their corresponding addresses.


<details closed><summary>📂 Project Structure</summary>

```bash
repo
├── include/
│   ├── definitions.h
│   ├── macros.h
│   ├── passes.h
│   ├── tokens.h
├── obj/
│   ├── macros.o
│   ├── main.o
│   ├── passes.o
│   ├── tokens.o
├── src/
│   ├── macros.c
│   ├── main.c
│   ├── passes.c
│   ├── tokens.c
├── Makefile
├── assembler
├── error.as
├── ps.as
└── ps1.as

3 directories, 17 files
```
</details>

---
## ⚡ Getting Started

### ✔️ Prerequisites

Before you begin, ensure you have the following installed:  
> - **GCC compiler**  
> - **Ubuntu (recommended)** or any Unix-like system  

The project must compile with:  
```bash
gcc -Wall -ansi -pedantic
```

### 📦 Installation

1. Clone the Labratory-C-Final-Project repository:
```sh
git clone https://github.com/idoyosef/Assembler.git
```

2. Change to the project directory:
```sh
cd Assembler
```

3. Compile using the Makefile:
```sh
make
```

## 🔧 Usage

Run the assembler with: `./assembler ps.as ps1.as ...`

For each source file, the assembler will produce the appropriate output files (.ob, .ent, .ext) in the same directory.

___

## 🧪 Testing

Multiple assembly language input files are included to showcase different instructions, addressing methods, and to verify the assembler’s ability to detect and report errors.

___ 

## 📚 Documentation

Each function in the codebase is documented with clear header comments describing its role and usage. Key variables are explained, and detailed inline comments are provided throughout to enhance understanding of the program’s flow.

___
## 🎉 Ido Yosef and Yohay Shabatayev
