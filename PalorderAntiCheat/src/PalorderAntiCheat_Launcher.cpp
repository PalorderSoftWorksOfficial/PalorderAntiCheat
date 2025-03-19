#include <iostream>
#include <windows.h>
#include <string>
#include <filesystem>

// Function to get the current directory of the executable
std::string getCurrentDir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}

void launchGameAndAntiCheat() {
    // Get the current directory where the launcher EXE is located
    std::string currentDir = getCurrentDir();

    // Paths to the anti-cheat executable and the Unity game executable (relative to current directory)
    std::string antiCheatPath = currentDir + "\\PalorderAnticheat.exe";  // Path to Anti-Cheat EXE
    std::string gamePath = currentDir + "\\UnityGame.exe";  // Path to Unity game executable

    // First, run the anti-cheat executable
    std::cout << "Starting Palorder Anti-Cheat..." << std::endl;
    PROCESS_INFORMATION antiCheatProcessInfo;
    STARTUPINFO antiCheatStartupInfo;
    ZeroMemory(&antiCheatStartupInfo, sizeof(STARTUPINFO));
    ZeroMemory(&antiCheatProcessInfo, sizeof(PROCESS_INFORMATION));
    
    if (CreateProcess(antiCheatPath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &antiCheatStartupInfo, &antiCheatProcessInfo)) {
        std::cout << "Anti-Cheat is running." << std::endl;
    } else {
        std::cerr << "Failed to start anti-cheat." << std::endl;
        return;
    }

    // Next, run the Unity game executable
    std::cout << "Starting Unity Game..." << std::endl;
    PROCESS_INFORMATION gameProcessInfo;
    STARTUPINFO gameStartupInfo;
    ZeroMemory(&gameStartupInfo, sizeof(STARTUPINFO));
    ZeroMemory(&gameProcessInfo, sizeof(PROCESS_INFORMATION));
    
    if (CreateProcess(gamePath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &gameStartupInfo, &gameProcessInfo)) {
        std::cout << "Game is running." << std::endl;
    } else {
        std::cerr << "Failed to start the game." << std::endl;
    }

    // Wait for the game process to exit (so we can clean up the anti-cheat)
    WaitForSingleObject(gameProcessInfo.hProcess, INFINITE);

    // Once the game is finished, close the anti-cheat process
    TerminateProcess(antiCheatProcessInfo.hProcess, 0);
    std::cout << "Game finished, shutting down anti-cheat." << std::endl;

    CloseHandle(gameProcessInfo.hProcess);
    CloseHandle(gameProcessInfo.hThread);
    CloseHandle(antiCheatProcessInfo.hProcess);
    CloseHandle(antiCheatProcessInfo.hThread);
}

int main() {
    // Launch the game and anti-cheat with the relative paths
    launchGameAndAntiCheat();

    return 0;
}
