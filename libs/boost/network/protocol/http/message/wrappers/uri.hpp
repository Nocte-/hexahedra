#ifndef BOOST_NETWORK_PROTOCOL_HTTP_MESSAGE_WRAPPERS_URI_HPP_20100620
#define BOOST_NETWORK_PROTOCOL_HTTP_MESSAGE_WRAPPERS_URI_HPP_20100620

// Copyright 2010 (c) Dean Michael Berris.
// Copyright 2010 (c) Sinefunc, Inc.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/network/uri/http/uri.hpp>

namespace boost { namespace network { namespace http {

    template <class Tag>
    struct basic_request;

    namespace impl {
        template <class Tag>
        struct uri_wrapper {
            basic_request<Tag> const & message_;
            uri_wrapper(basic_request<Tag> const & message)
                : message_(message) {}
            typedef typename basic_request<Tag>::string_type string_type;
            operator string_type() {
                return message_.uri().raw();
            }
            operator boost::network::uri::basic_uri<tags::http_default_8bit_tcp_resolve> () {
                return message_.uri();
            }
        };
    }

    template <class Tag> inline
    impl::uri_wrapper<Tag>
    uri(basic_request<Tag> const & request) {
        return impl::uri_wrapper<Tag>(request);
    }

} // namespace http

} // namespace network

} // nmaespace boost

#endif // BOOST_NETWORK_PROTOCOL_HTTP_MESSAGE_WRAPPERS_URI_HPP_20100620
