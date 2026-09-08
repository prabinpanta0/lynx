#pragma once
#include "http/http_response.hpp"
#include "http/request_context.hpp"
#include "http/http_request.hpp"
#include <filesystem>
#include <string>


namespace http
{
    class file_handler
    {
        fs::path base_dir_;
        bool enable_directory_listing_{false};
        size_t max_file_size{ 10 * 1024 * 1024 };

        [[nodiscard]] http_response serve_file(
            const fs::path& path,
            const request_context& ctx
        ) const;

    };
}