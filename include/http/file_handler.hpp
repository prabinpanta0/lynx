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
        private:

        fs::path base_dir_;
        bool enable_directory_listing_{false};
        size_t max_file_size_{ 10 * 1024 * 1024 };

        [[nodiscard]] http_response serve_file(
            const fs::path& path,
            const request_context& ctx
        ) const;

        http_response serve_directory(
            const fs::path& path,
            const request_context& ctx
        ) const;
        
        bool is_not_modified(
            const fs::path& path,
            const request_context& ctx 
        ) const;
        
        void add_cache_headers(
            http_response& resp,
            const fs::path& path
        ) const;

        public:

        explicit file_handler( fs::path base_dir );

        void set_directory_listing( bool enabled )
        {
            enable_directory_listing_ = enabled;
        }
        
        void set_max_file_size( size_t bytes )
        {
            max_file_size_ = bytes;
        }

    };
}