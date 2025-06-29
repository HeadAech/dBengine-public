//
// Created by Hubert Klonowski on 25/03/2025.
//

#ifndef ENGINEDEBUG_H
#define ENGINEDEBUG_H
#include <chrono>
#include <fstream>

enum LogLevel {
    INFO, 
    WARNING, 
    ERR, 
    DEBUG
};

const int MAX_LOGS = 15;
const std::string LOGS_DIR = "logs";

class EngineDebug {

    std::ofstream logFile;

    void deleteOldestLogFileIfNeeded();

    std::string sanitizeFileName(const std::string& fileName);

    public:
    std::vector<LogLevel> logBufferLevels;
    std::vector<std::string> logBuffer;

    std::chrono::high_resolution_clock::time_point lastInputTime;
    std::chrono::high_resolution_clock::time_point lastFrameTime;

    /// <summary>
    /// Used to track the number of draw calls.
    /// </summary>
    int drawCalls = 0;

    /// <summary>
    /// Used to track the number of instances.
    /// </summary>
    int instancesCount = 0;

    /// <summary>
    /// Used to track the update time.
    /// </summary>
    float updateTime = 0;

    /// <summary>
    /// Used to track the input latency.
    /// </summary>
    float inputLatency = 0;

    EngineDebug() = default;
    EngineDebug(const EngineDebug&) = delete;
    EngineDebug& operator=(const EngineDebug&) = delete;

    ~EngineDebug();

    /// <summary>
    /// Returns the singleton instance of EngineDebug.
    /// </summary>
    /// <returns>Singleton (EngineDebug)</returns>
    static EngineDebug& GetInstance();

    /// <summary>
    /// Enables file logging.
    /// </summary>
    void EnableFileLogging();

    /// <summary>
    /// Disables file logging.
    /// </summary>
    void DisableFileLogging();

    /// <summary>
    /// Returns Log files from the logs directory.
    /// </summary>
    /// <returns>List of log files (std::vector)</returns>
    std::vector<std::string> GetLogFiles();

    /// <summary>
    /// Prints a debug message to the console and optionally to a log file.
    /// </summary>
    /// <param name="message">(string)</param>
    void PrintDebug(std::string message);

    /// <summary>
    /// Prints an error message to the console and optionally to a log file.
    /// </summary>
    /// <param name="message">(string)</param>
    void PrintError(std::string message);

    /// <summary>
    /// Prints a warning message to the console and optionally to a log file.
    /// </summary>
    /// <param name="message">(string)</param>
    void PrintWarning(std::string message);
    
    /// <summary>
    /// Prints an info message to the console and optionally to a log file.
    /// </summary>
    /// <param name="message">(string)</param>
    
    void PrintInfo(std::string message);
    /// <summary>
    /// Prints a message with a specific log level to the console and optionally to a log file.
    /// </summary>
    /// <param name="level">(LogLevel)</param>
    /// <param name="message">(string)</param>
    void Print(LogLevel level, std::string message);

};



#endif //ENGINEDEBUG_H
