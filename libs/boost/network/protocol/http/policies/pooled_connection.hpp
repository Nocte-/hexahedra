#ifndef BOOST_NETWORK_PROTOCOL_HTTP_POOLED_CONNECTION_POLICY_20091214
#define BOOST_NETWORK_PROTOCOL_HTTP_POOLED_CONNECTION_POLICY_20091214

//          Copyright Dean Michael Berris 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/network/protocol/http/traits/resolver_policy.hpp>

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/network/protocol/http/client/connection/sync_base.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/network/protocol/http/response.hpp>
#include <utility>

#ifndef BOOST_NETWORK_HTTP_MAXIMUM_REDIRECT_COUNT
#define BOOST_NETWORK_HTTP_MAXIMUM_REDIRECT_COUNT 5
#endif // BOOST_NETWORK_HTTP_MAXIMUM_REDIRECT_COUNT

namespace boost { namespace network { namespace http {

    template <class Tag, unsigned version_major, unsigned version_minor>
    struct pooled_connection_policy : resolver_policy<Tag>::type {
    protected:

        typedef typename string<Tag>::type string_type;
        typedef typename resolver_policy<Tag>::type resolver_base;
        typedef typename resolver_base::resolver_type resolver_type;
        typedef function<typename resolver_base::resolver_iterator_pair(resolver_type &, string_type const &, string_type const &)> resolver_function_type;
        typedef function<void(iterator_range<char const *> const &, system::error_code const &)> body_callback_function_type;
        
        void cleanup() {
            host_connection_map().swap(host_connections);
        }

        struct connection_impl {
            typedef function<shared_ptr<connection_impl>(resolver_type &,basic_request<Tag> const &,optional<string_type> const &, optional<string_type> const &)> get_connection_function;

            connection_impl(resolver_type & resolver, bool follow_redirect, string_type const & host, string_type const & port, resolver_function_type resolve, get_connection_function get_connection, bool https, optional<string_type> const & certificate_file=optional<string_type>(), optional<string_type> const & verify_path=optional<string_type>())
            : pimpl(impl::sync_connection_base<Tag,version_major,version_minor>::new_connection(resolver, resolve, https, certificate_file, verify_path))
            , resolver_(resolver)
            , connection_follow_redirect_(follow_redirect)
            , get_connection_(get_connection)
            , certificate_filename_(certificate_file)
            , verify_path_(verify_path)
            {}

            basic_response<Tag> send_request(string_type const & method, basic_request<Tag> request_, bool get_body, body_callback_function_type callback) {
                return send_request_impl(method, request_, get_body);
            }

        private:

            basic_response<Tag> send_request_impl(string_type const & method, basic_request<Tag> request_, bool get_body) {
                boost::uint8_t count = 0;
                bool retry = false;
                do {
                    if (count >= BOOST_NETWORK_HTTP_MAXIMUM_REDIRECT_COUNT)
                        boost::throw_exception(std::runtime_error("Redirection exceeds maximum redirect count."));

                    basic_response<Tag> response_;
                    // check if the socket is open first
                    if (!pimpl->is_open()) {
                        pimpl->init_socket(request_.host(), lexical_cast<string_type>(request_.port()));
                    }
                    response_ = basic_response<Tag>();
                    response_ << ::boost::network::source(request_.host());

                    pimpl->send_request_impl(method, request_);
                    boost::asio::streambuf response_buffer;

                    try {
                        pimpl->read_status(response_, response_buffer);
                    } catch (boost::system::system_error & e) {
                        if (!retry && e.code() == boost::asio::error::eof) {
                            retry = true;
                            pimpl->init_socket(request_.host(), lexical_cast<string_type>(request_.port()));
                            continue;
                        }
                        throw; // it's a retry, and there's something wrong.
                    }

                    pimpl->read_headers(response_, response_buffer);

                    if (
                        get_body && response_.status() != 304 
                        && (response_.status() != 204)
                        && not (response_.status() >= 100 && response_.status() <= 199)
                       ) {
                        pimpl->read_body(response_, response_buffer);
                    }

                    typename headers_range<basic_response<Tag> >::type connection_range = headers(response_)["Connection"];
                    if (version_major == 1 && version_minor == 1 && !empty(connection_range) && boost::begin(connection_range)->second == string_type("close")) {
                        pimpl->close_socket();
                    } else if (version_major == 1 && version_minor == 0) {
                        pimpl->close_socket();
                    }

                    if (connection_follow_redirect_) {
                        boost::uint16_t status = response_.status();
                        if (status >= 300 && status <= 307) {
                            typename headers_range<basic_response<Tag> >::type location_range = headers(response_)["Location"];
                            typename range_iterator<typename headers_range<basic_request<Tag> >::type>::type location_header = boost::begin(location_range);
                            if (location_header != boost::end(location_range)) {
                                request_.uri(location_header->second);
                                connection_ptr connection_;
                                connection_ = get_connection_(resolver_, request_, certificate_filename_, verify_path_);
                                ++count;
                                continue;
                            } else boost::throw_exception(std::runtime_error("Location header not defined in redirect response."));
                        }
                    }
                    return response_;
                } while(true);
            }

            shared_ptr<http::impl::sync_connection_base<Tag,version_major,version_minor> > pimpl;
            resolver_type & resolver_;
            bool connection_follow_redirect_;
            get_connection_function get_connection_;
            optional<string_type> certificate_filename_, verify_path_;
        };

        typedef shared_ptr<connection_impl> connection_ptr;
        
        typedef unordered_map<string_type, connection_ptr> host_connection_map;
        host_connection_map host_connections;
        bool follow_redirect_;

        connection_ptr get_connection(resolver_type & resolver, basic_request<Tag> const & request_, optional<string_type> const & certificate_filename = optional<string_type>(), optional<string_type> const & verify_path = optional<string_type>()) {
            string_type index = (request_.host() + ':') + lexical_cast<string_type>(request_.port());
            connection_ptr connection_;
            typename host_connection_map::iterator it =
                host_connections.find(index);
            if (it == host_connections.end()) {
                connection_.reset(new connection_impl(
                    resolver
                    , follow_redirect_
                    , request_.host()
                    , lexical_cast<string_type>(request_.port())
                    , boost::bind(
                        &pooled_connection_policy<Tag,version_major,version_minor>::resolve,
                        this,
                        _1, _2, _3
                        )
                    , boost::bind(
                        &pooled_connection_policy<Tag,version_major,version_minor>::get_connection,
                        this,
                        _1, _2, _3, _4
                        )
                    , boost::iequals(request_.protocol(), string_type("https"))
                    , certificate_filename
                    , verify_path
                    )
                );
                host_connections.insert(std::make_pair(index, connection_));
                return connection_;
            }
            return it->second;
        }

        pooled_connection_policy(bool cache_resolved, bool follow_redirect)
        : resolver_base(cache_resolved), host_connections(), follow_redirect_(follow_redirect) {}
        
    };

} // namespace http

} // namespace network

} // namespace boost

#endif // BOOST_NETWORK_PROTOCOL_HTTP_POOLED_CONNECTION_POLICY_20091214

