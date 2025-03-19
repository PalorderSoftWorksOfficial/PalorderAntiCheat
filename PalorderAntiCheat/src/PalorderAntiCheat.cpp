#include <iostream>
#include <windows.h>
#include <fstream>
#include <psapi.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <filesystem>

// Function to get the current directory of the executable
std::string getCurrentDir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}

// Structure to hold process information
struct ProcessInfo {
    DWORD pid;
    std::string name;
    std::string behavior;  // Description of suspicious behavior
};

// Function to check for suspicious processes and memory access
void scanForSuspiciousProcesses(std::vector<ProcessInfo>& flaggedProcesses) {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return;
    }

    if (Process32First(hProcessSnap, &pe32)) {
        do {
            // Check for suspicious behavior (e.g., memory access, DLL injection, etc.)
            std::string processName(pe32.szExeFile);
            bool isSuspicious = false;

            // Example condition: Process name matching common cheat tools
            if (processName == "cheatengine.exe" || processName == "injector.exe") {
                isSuspicious = true;
            }

            if (isSuspicious) {
                ProcessInfo suspiciousProcess = {pe32.th32ProcessID, processName, "Suspicious process detected"};
                flaggedProcesses.push_back(suspiciousProcess);
                TerminateProcess(OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID), 0);
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);
}

// Function to write flagged processes to the config file
void logSuspiciousProcesses(const std::vector<ProcessInfo>& flaggedProcesses, const std::string& configFile) {
    std::ofstream config(configFile, std::ios::app);
    if (config.is_open()) {
        for (const auto& process : flaggedProcesses) {
            config << "Process Name: " << process.name << std::endl;
            config << "Process ID: " << process.pid << std::endl;
            config << "Behavior: " << process.behavior << std::endl;
            config << "-----------------------" << std::endl;
        }
        config.close();
    } else {
        std::cerr << "Failed to open config file!" << std::endl;
    }
}

// Function to check if the config file is intact
bool checkConfigFileIntegrity(const std::string& configFile) {
    std::ifstream file(configFile);
    if (!file) {
        std::cerr << "Config file is missing or corrupt!" << std::endl;
        return false;
    }
    // Add further checks here to verify the integrity of the config file
    return true;
}

// Function to launch the game and load the anti-cheat DLL
void launchAntiCheatAndGame() {
    // Get the current directory where the EXE is located
    std::string currentDir = getCurrentDir();

    // Paths to the game, DLL, and config file (relative to the current directory)
    std::string gamePath = currentDir + "\\UnityGame.exe";  // Path to Unity game executable
    std::string dllPath = currentDir + "\\PalorderAnticheat.dll";  // Path to anti-cheat DLL
    std::string configFile = currentDir + "\\suspicious_config.txt";  // Path to config file

    // Check if the config file is intact
    if (!checkConfigFileIntegrity(configFile)) {
        std::cerr << "Config file has been tampered with!" << std::endl;
        return;
    }

    std::cout << "Launching Anti-Cheat and Game..." << std::endl;

    // Load the anti-cheat DLL
    HMODULE hDll = LoadLibraryA(dllPath.c_str());
    if (hDll == NULL) {
        std::cerr << "Failed to load Anti-Cheat DLL!" << std::endl;
        return;
    }

    typedef void (*StartAntiCheat)();
    StartAntiCheat startAntiCheat = (StartAntiCheat)GetProcAddress(hDll, "startAntiCheat");
    if (startAntiCheat != nullptr) {
        startAntiCheat();  // Initialize the anti-cheat
    }

    // Scan for suspicious processes
    std::vector<ProcessInfo> flaggedProcesses;
    scanForSuspiciousProcesses(flaggedProcesses);

    // Log flagged processes
    logSuspiciousProcesses(flaggedProcesses, configFile);

    // Launch the game
    PROCESS_INFORMATION piGame;
    STARTUPINFO siGame;
    ZeroMemory(&siGame, sizeof(siGame));
    ZeroMemory(&piGame, sizeof(piGame));

    if (CreateProcess(gamePath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &siGame, &piGame)) {
        std::cout << "Game started successfully." << std::endl;
    } else {
        std::cerr << "Failed to start the game." << std::endl;
    }

    // Wait for the game to exit
    WaitForSingleObject(piGame.hProcess, INFINITE);

    // Once the game exits, unload the anti-cheat DLL
    FreeLibrary(hDll);
    std::cout << "Game finished, unloading Anti-Cheat." << std::endl;
}

int main() {
    // Launch anti-cheat and game
    launchAntiCheatAndGame();

    return 0;
}
