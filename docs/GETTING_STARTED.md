# Run pcbtech in VS Code

The current version compiles and runs engineering tests in a terminal. It does not open a PCB game window yet. A passing test run checks the implemented code; it does not certify a physical board.

These steps use Windows with Ubuntu in WSL and GCC. WSL runs Linux tools on Windows. GCC compiles the C++ source. VS Code provides the editor and terminal. CMake is optional for this route.

## 1. Check Ubuntu

1. Open the Windows Start menu.
2. Open **Ubuntu**.

Expected result: a Linux terminal opens. If Ubuntu is missing, follow [Microsoft's WSL installation guide](https://learn.microsoft.com/en-us/windows/wsl/install), then return here. Record the exact error if Ubuntu opens but fails.

## 2. Prepare the compiler

In the Ubuntu terminal, run these commands one at a time:

```bash
sudo apt update
sudo apt install -y build-essential git
g++ --version
```

Expected result: the final command prints the compiler version. The current source has been tested with GCC 13 on Ubuntu 24.04. It requires C++20 and 128-bit integer support; native MSVC cannot build the current geometry code as written.

## 3. Add WSL to VS Code

1. Open VS Code.
2. Click the **Extensions** button on the left.
3. Search for **WSL**.
4. Select the **WSL** extension published by **Microsoft**.
5. Click **Install**, or **Enable** if it is disabled.
6. Reload VS Code if prompted.

Expected result: the WSL extension is installed and enabled before opening the project from Ubuntu. The optional Microsoft **C/C++** extension adds editing and debugging support. It does not install the compiler.

## 4. Open the project

If the project is already downloaded, open its folder in Ubuntu and run only `code .`.

In the Ubuntu terminal, run:

```bash
git clone https://github.com/1freetech/pcbtech.git
cd pcbtech
code .
```

Expected result: VS Code opens a folder containing `src`, `include`, `tests`, and `scripts`. The bottom-left status bar identifies the Ubuntu WSL connection.

If Restricted Mode appears, use **Manage** to trust the project folder you downloaded.

Only if the Ubuntu connection is missing:

1. Click the remote connection button in the bottom-left corner.
2. Select **Reopen Folder in WSL**.
3. Choose Ubuntu if prompted.

If the connection fails, keep the exact error message for troubleshooting.

## 5. Run the tests

1. Click **Terminal** in the top menu.
2. Click **New Terminal**.
3. Check that the terminal is in the folder containing `scripts/validate.sh`.
4. Run this command:

```bash
sh scripts/validate.sh
```

The terminal may be quiet while compiling. A successful run ends with:

```text
pcbtech component gate: PASS
```

The script stops on a failed build or test. Keep the full error text when asking for help. If it says the script cannot be found, use **File > Open Folder** to open the project folder containing `scripts`, then open a new terminal there.

After a successful build, rerun the component demonstration with:

```bash
./build/pcbtech_validate
```

This last command runs only the component demonstration. Use `sh scripts/validate.sh` to rebuild and run the complete test suite.

Official setup references: [VS Code with WSL](https://code.visualstudio.com/docs/remote/wsl), [C++ with WSL](https://code.visualstudio.com/docs/cpp/config-wsl), and [Workspace Trust](https://code.visualstudio.com/docs/editing/workspaces/workspace-trust).
