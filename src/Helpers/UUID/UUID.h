#ifndef UUID_H
#define UUID_H

#include <string>
#include <random>
#include <ostream>
#include <sstream>

/// <summary>
/// Utility class for generating UUIDs (Universal Unique Identifiers).
/// </summary>
class UUID {
	
	public:

		/// <summary>
        /// Generates a random UUID in the format xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx.
		/// </summary>
		/// <returns>Random UUID (string)</returns>
		static std::string generateUUID() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dis(0, 15); // Hexadecimal digits

            std::ostringstream uuid_stream;
            uuid_stream << std::hex;

            for (int i = 0; i < 8; ++i)
                uuid_stream << dis(gen); // Random 8 hex characters
            uuid_stream << "-";
            for (int i = 0; i < 4; ++i)
                uuid_stream << dis(gen); // Random 4 hex characters
            uuid_stream << "-";
            for (int i = 0; i < 4; ++i)
                uuid_stream << dis(gen); // Random 4 hex characters
            uuid_stream << "-";
            for (int i = 0; i < 4; ++i)
                uuid_stream << dis(gen); // Random 4 hex characters
            uuid_stream << "-";
            for (int i = 0; i < 12; ++i)
                uuid_stream << dis(gen); // Random 12 hex characters

            return uuid_stream.str();
        };
};
#endif // UUID_H
