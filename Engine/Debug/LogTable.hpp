#pragma once

#include "Core/Types.hpp"

#include "Debug/Log.hpp"

#include <array>
#include <string>
#include <string_view>

namespace Engine::Debug
{
    struct LogTable
    {
        static constexpr u8 MaxColumns = 8;

        std::array<std::string, MaxColumns> Headers{};
        std::array<std::string, MaxColumns> Values{};

        u8 ColumnIndex = 0;
        u8 ColumnCount = 0;
    };

    inline void LogTableBegin(LogTable& table, u8 columns)
    {
        constexpr u8 maxColumns = Engine::Debug::LogTable::MaxColumns;
        ASSERT(columns <= maxColumns, "LogTable only supports a maximum of {} columns!", maxColumns);
        table.ColumnCount = columns;
    }

    inline void LogTableAddColumn(LogTable& table, std::string_view header, std::string_view value)
    {
        ASSERT(table.ColumnIndex <= table.ColumnCount, "Tried to add to many columns to the table!");
        table.Headers.at(table.ColumnIndex) = header;
        table.Values.at(table.ColumnIndex)  = value;
        table.ColumnIndex++;
    }

    inline void LogTablePrint(const LogTable& table)
    {
        ASSERT(table.ColumnIndex == table.ColumnCount,
               "LogTable expected {} columns but got {}!",
               table.ColumnCount,
               table.ColumnIndex);

        constexpr std::string_view               logPrefixPadding = "       "; // Same width as "[INFO] "
        std::array<size_t, LogTable::MaxColumns> widths{};

        // Compute the visible width required by each column
        for (size_t i = 0; i < table.ColumnCount; ++i)
        {
            widths[i] = std::max(table.Headers[i].size(), table.Values[i].size());
        }

        size_t tableWidth = 1;

        // Compute the full table width including separators and padding
        for (size_t i = 0; i < table.ColumnCount; ++i)
        {
            tableWidth += widths[i] + 3;
        }

        fmt::print(stdout, fg(fmt::color::green), "{}{:-^{}}\n", logPrefixPadding, "", tableWidth);
        fmt::print(stdout, fg(fmt::color::green), "{}", logPrefixPadding);

        // Print the table header row.
        for (size_t i = 0; i < table.ColumnCount; ++i)
        {
            fmt::print(stdout, fg(fmt::color::green), "| {:^{}} ", table.Headers[i], widths[i]);
        }

        fmt::print(stdout, fg(fmt::color::green), "|\n");
        fmt::print(stdout, fg(fmt::color::green), "{}", logPrefixPadding);

        // Print the separator row using the computed column widths
        for (size_t i = 0; i < table.ColumnCount; ++i)
        {
            fmt::print(stdout, fg(fmt::color::green), "| {:-^{}} ", "", widths[i]);
        }

        fmt::print(stdout, fg(fmt::color::green), "|\n");
        fmt::print(stdout, fg(fmt::color::green), "{}", logPrefixPadding);

        // Print the table value row
        for (size_t i = 0; i < table.ColumnCount; ++i)
        {
            fmt::print(stdout, fg(fmt::color::green), "| {:^{}} ", table.Values[i], widths[i]);
        }

        fmt::print(stdout, fg(fmt::color::green), "|\n");
        fmt::print(stdout, fg(fmt::color::green), "{}{:-^{}}\n", logPrefixPadding, "", tableWidth);
    }
}

#define LOG_TABLE_BEGIN(columns)                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        Engine::Debug::LogTable logTable{};                                                                            \
        Engine::Debug::LogTableBegin(logTable, columns);

#define LOG_TABLE_COLUMN(header, fmtString, ...)                                                                       \
    Engine::Debug::LogTableAddColumn(logTable, header, fmt::format(fmtString __VA_OPT__(, ) __VA_ARGS__));

#define LOG_TABLE_END()                                                                                                \
    Engine::Debug::LogTablePrint(logTable);                                                                            \
    }                                                                                                                  \
    while (false)
