//
// Created by Hubert Klonowski on 25/03/2025.
//

#include "EngineDebug.h"

#include <iostream>
#include <filesystem>

#include "Helpers/Util.h"
#include "Signal/Signals.h"
#include <mutex>


std::mutex logMutex;


EngineDebug &EngineDebug::GetInstance() {
    static EngineDebug instance;
    return instance;
}

EngineDebug::~EngineDebug() {
    if (logFile.is_open()) {
        DisableFileLogging();
    }
}


std::vector<std::string> EngineDebug::GetLogFiles() {
    std::vector<std::string> logFiles;
    if (std::filesystem::exists(LOGS_DIR)) {
        for (const auto &entry : std::filesystem::directory_iterator(LOGS_DIR)) {
            if (entry.is_regular_file() && entry.path().extension() == ".log") {
                logFiles.push_back(entry.path().string());
            }
        }
    }
    return logFiles;
}

void EngineDebug::deleteOldestLogFileIfNeeded() {
    std::vector<std::string> logFiles = GetLogFiles();

    if (logFiles.size() >= MAX_LOGS) {
        // Sort files by creation time (oldest first)
        std::sort(logFiles.begin(), logFiles.end(), [](const std::string &a, const std::string &b) {
            return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
        });

        // Delete the oldest file
        std::filesystem::remove(logFiles.front());
    }
}

std::string EngineDebug::sanitizeFileName(const std::string &fileName) {
    std::string sanitized = fileName;
    for (char &c: sanitized) {
        if (c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            c = '_'; // Zamieñ niedozwolone znaki na `_`
        }
    }
    return sanitized;
}



void EngineDebug::EnableFileLogging() {
    if (!std::filesystem::exists(LOGS_DIR)) {
        if (std::filesystem::create_directories(LOGS_DIR)) {
            std::cout << "Directory created: " << LOGS_DIR << std::endl;
        } else {
            std::cerr << "Failed to create directory: " << LOGS_DIR << std::endl;
        }
    }

    deleteOldestLogFileIfNeeded();

    std::string logFilename = sanitizeFileName(Util::GetCurrentDateTime() + ".log");
    std::filesystem::path logFilePath = std::filesystem::path(LOGS_DIR) / logFilename;

    logFile.open(logFilePath, std::ios::app); // Open file in append mode
    if (logFile.is_open()) {
        PrintInfo("Logging to file enabled.");
    } else {
        PrintError("Failed to open log file.");
    }
}

void EngineDebug::DisableFileLogging() {
    PrintInfo("Logging disabled.");
    logFile.close();
}


void EngineDebug::Print(LogLevel level, std::string message) {
    std::string logEntry = "[" + Util::GetCurrentDateTime() + "] ";
    switch (level) {
        case LogLevel::INFO:
            logEntry += message;
            break;
        case LogLevel::WARNING:
            logEntry += "[WARN] " + message;
            break;
        case LogLevel::ERR:
            logEntry += "[ERROR] " + message;
            break;
        case LogLevel::DEBUG:
            logEntry += "[DEBUG] " + message;
            break;
        default:
            logEntry += "[MSG] " + message;
            break;
    }

    // Synchronizowany dostêp do wspólnych zasobów
    {
        std::lock_guard<std::mutex> lock(logMutex);
        logBufferLevels.push_back(level);
        logBuffer.push_back(logEntry);
    }

    // Print to console
    std::cout << logEntry << std::endl;
    

    // Write to file if logging is enabled
    if (logFile.is_open()) {
        logFile << logEntry << std::endl;
    }

    Signals::Console_ScrollToBottom.emit();
}

void EngineDebug::PrintWarning(std::string message) {
    Print(LogLevel::WARNING, message);
}
void EngineDebug::PrintInfo(std::string message) {
    Print(LogLevel::INFO, message);
}
void EngineDebug::PrintDebug(std::string message) {
    Print(LogLevel::DEBUG, message);
}
void EngineDebug::PrintError(std::string message) {
    Print(LogLevel::ERR, message);
}
