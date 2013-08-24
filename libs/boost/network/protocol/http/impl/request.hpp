
//          Copyright Dean Michael Berris 2007,2009,2010.
//          Copyright Michael Dickey 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef __NETWORK_PROTOCOL_HTTP_REQUEST_IMPL_20070908_1_HPP__
#define __NETWORK_PROTOCOL_HTTP_REQUEST_IMPL_20070908_1_HPP__

#include <boost/network/protocol/http/message.hpp>
#include <boost/network/protocol/http/message/header.hpp>

#include <boost/fusion/container/map.hpp>
#include <boost/fusion/sequence/intrinsic/at_key.hpp>
#include <boost/fusion/sequence/intrinsic/value_at_key.hpp>

#include <boost/network/uri/http/uri.hpp>
#include <boost/network/traits/vector.hpp>

#include <boost/network/protocol/http/message/async_message.hpp>
#include <boost/network/support/is_async.hpp>
#include <boost/network/protocol/http/support/sync_only.hpp>

#include <boost/cstdint.hpp>

namespace boost { namespace network {

    /** Specialize the traits for the http_server tag. */
    template <>
    struct headers_container<http::tags::http_server> :
        vector<http::tags::http_server>::apply<http::request_header<http::tags::http_server>::type>
    {};

    template <>
    struct headers_container<http::tags::http_async_server> :
        vector<http::tags::http_async_server>::apply<http::request_header<http::tags::http_async_server>::type>
    {};

namespace http {

    /** request.hpp
      *
      * This file implements the basic request object required
      * by the HTTP client implementation. The basic_request
      * object encapsulates a URI which is parsed at runtime.
      */

    template <class Tag>
    struct basic_request : public basic_message<Tag>
    {

        mutable boost::network::uri::http::basic_uri<Tag> uri_;
        typedef basic_message<Tag> base_type;

    public:
        typedef typename sync_only<Tag>::type tag;
        typedef typename string<tag>::type string_type;
        typedef boost::uint16_t port_type;

        explicit basic_request(string_type const & uri_)
        : uri_(uri_)
        { }

        void uri(string_type const & new_uri) {
            uri_ = new_uri;
        }

        basic_request()
        : base_type()
        { }

        basic_request(basic_request const & other)
        : base_type(other), uri_(other.uri_)
        { }

        basic_request & operator=(basic_request rhs) {
            rhs.swap(*this);
            return *this;
        }

        void swap(basic_request & other) {
            using boost::network::uri::swap;
            base_type & base_ref(other);
            basic_request<Tag> & this_ref(*this);
            base_ref.swap(this_ref);
            swap(other.uri_, this->uri_);
        }

        string_type const host() const {
            return uri_.host();
        }

        port_type port() const {
            return uri::port_us(uri_);
        }

        string_type const path() const {
            return uri_.path();
        }

        string_type const query() const {
            return uri_.query();
        }

        string_type const anchor() const {
            return uri_.fragment();
        }

        string_type const protocol() const {
            return uri_.scheme();
        }

        void uri(string_type const & new_uri) const {
            uri_ = new_uri;
        }

        boost::network::uri::http::basic_uri<Tag> const & uri() const {
            return uri_;
        }

    };

    /** This is the implementation of a POD request type
     *  that is specificially used by the HTTP server
     *  implementation. This fully specializes the
     *  basic_request template above to be
     *  primarily and be solely a POD for performance
     *  reasons.
     *
     *  Reality check: This is not a POD because it contains a non-POD
     *  member, the headers vector. :(
     */
    template <class Tag>
    struct not_quite_pod_request_base {
        typedef Tag tag;
        typedef typename string<Tag>::type string_type;
        typedef typename request_header<Tag>::type header_type;
        typedef typename vector<Tag>::
            template apply<header_type>::type
            vector_type;
        typedef vector_type headers_container_type;
        typedef boost::uint16_t port_type;
        mutable string_type source;
        mutable string_type method;
        mutable string_type destination;
        mutable boost::uint8_t http_version_major;
        mutable boost::uint8_t http_version_minor;
        mutable vector_type headers;
        mutable string_type body;

        void swap(not_quite_pod_request_base & r) const {
            using std::swap;
            swap(method, r.method);
            swap(source, r.source);
            swap(destination, r.destination);
            swap(http_version_major, r.http_version_major);
            swap(http_version_minor, r.http_version_minor);
            swap(headers, r.headers);
            swap(body, r.body);
        }
    };

    template <>
    struct basic_request<tags::http_async_server>
    : not_quite_pod_request_base<tags::http_async_server>
    {};

    template <>
    struct basic_request<tags::http_server>
    : not_quite_pod_request_base<tags::http_server>
    {};

    template <class R>
    struct ServerRequest;

    BOOST_CONCEPT_ASSERT((ServerRequest<basic_request<tags::http_async_server> >));
    BOOST_CONCEPT_ASSERT((ServerRequest<basic_request<tags::http_server> >));

    template <class Tag>
    inline void swap(basic_request<Tag> & lhs, basic_request<Tag> & rhs) {
        lhs.swap(rhs);
    }

} // namespace http

    namespace http { namespace impl {

        template <>
        struct request_headers_wrapper<tags::http_server> {
            basic_request<tags::http_server> const & request_;
            request_headers_wrapper(basic_request<tags::http_server> const & request_)
            : request_(request_) {}
            typedef headers_container<tags::http_server>::type headers_container_type;
            operator headers_container_type () {
                return request_.headers;
            }
        };

        template <>
        struct body_wrapper<basic_request<tags::http_server> > {
            typedef string<tags::http_server>::type string_type;
            basic_request<tags::http_server> const & request_;
            body_wrapper(basic_request<tags::http_server> const & request_)
            : request_(request_) {}
            operator string_type () {
                return request_.body;
            }
        };

        template <>
        struct request_headers_wrapper<tags::http_async_server> {
            basic_request<tags::http_async_server> const & request_;
            request_headers_wrapper(basic_request<tags::http_async_server> const & request_)
            : request_(request_) {}
            typedef headers_container<tags::http_async_server>::type headers_container_type;
            operator headers_container_type () {
                return request_.headers;
            }
        };

        template <>
        struct body_wrapper<basic_request<tags::http_async_server> > {
            typedef string<tags::http_async_server>::type string_type;
            basic_request<tags::http_async_server> const & request_;
            body_wrapper(basic_request<tags::http_async_server> const & request_)
            : request_(request_) {}
            operator string_type () {
                return request_.body;
            }
        };

    } // namespace impl

} // namespace http

} // namespace network

} // namespace boost

#endif // __NETWORK_PROTOCOL_HTTP_REQUEST_IMPL_20070908_1_HPP__

