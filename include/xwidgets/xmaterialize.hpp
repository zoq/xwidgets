/***************************************************************************
* Copyright (c) 2017, Sylvain Corlay and Johan Mabille                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XWIDGETS_MATERIALIZE_HPP
#define XWIDGETS_MATERIALIZE_HPP

#include <utility>

#include "nlohmann/json.hpp"

#include "xwidgets_config.hpp"

namespace nl = nlohmann;

namespace xw
{
    /****************************
     * xmaterialize declaration *
     ****************************/

    template <template <class> class B, class... P>
    class xmaterialize final : public B<xmaterialize<B, P...>>
    {
    public:

        using self_type = xmaterialize<B, P...>;
        using base_type = B<self_type>;

        template <class... A>
        xmaterialize(A&&...);

        template <class... A>
        static xmaterialize initialize(A&&...);

        ~xmaterialize();

        xmaterialize(const xmaterialize&);
        xmaterialize& operator=(const xmaterialize&);

        xmaterialize(xmaterialize&&);
        xmaterialize& operator=(xmaterialize&&);

        xmaterialize<B, P...> finalize() &&;

    private:

        template <class... A>
        xmaterialize(bool open, A&&...);
    };

    /*******************************
     * xmaterialize implementation *
     *******************************/

    template <template <class> class B, class... P>
    template <class... A>
    inline xmaterialize<B, P...>::xmaterialize(A&&... args)
        : xmaterialize(true, std::forward<A>(args)...)
    {
    }

    // private constructor determining whether the comm should be open
    template <template <class> class B, class... P>
    template <class... A>
    inline xmaterialize<B, P...>::xmaterialize(bool op, A&&... args)
        : base_type(std::forward<A>(args)...)
    {
        if (op)
        {
            this->open();
        }
    }

    template <template <class> class B, class... P>
    template <class... A>
    inline auto xmaterialize<B, P...>::initialize(A&&... args) -> xmaterialize
    {
        return xmaterialize(false, std::forward<A>(args)...);
    }

    template <template <class> class B, class... P>
    inline xmaterialize<B, P...>::~xmaterialize()
    {
        if (!this->moved_from())
        {
            this->close();
        }
    }

    template <template <class> class B, class... P>
    inline xmaterialize<B, P...>::xmaterialize(const xmaterialize& rhs)
        : base_type(rhs)
    {
        this->open();
    }

    template <template <class> class B, class... P>
    inline xmaterialize<B, P...>& xmaterialize<B, P...>::operator=(const xmaterialize& rhs)
    {
        this->close();
        base_type::operator=(rhs);
        this->open();
        return *this;
    }

    template <template <class> class B, class... P>
    inline xmaterialize<B, P...>::xmaterialize(xmaterialize&&) = default;

    template <template <class> class B, class... P>
    inline xmaterialize<B, P...>& xmaterialize<B, P...>::operator=(xmaterialize&& rhs) = default;

    template <template <class> class B, class... P>
    inline xmaterialize<B, P...> xmaterialize<B, P...>::finalize() &&
    {
        return reinterpret_cast<typename xmaterialize<B, P...>::base_type&&>(*this);
    }

    /**********************************************************
     * Specialization of mime_bundle_repr for Jupyter Widgets *
     **********************************************************/

    template <template <class> class B, class... P>
    nl::json mime_bundle_repr(const xmaterialize<B, P...>& val)
    {
        nl::json mime_bundle;

        // application/vnd.jupyter.widget-view+json
        nl::json widgets_json;
        widgets_json["version_major"] = XWIDGETS_PROTOCOL_VERSION_MAJOR;
        widgets_json["version_minor"] = XWIDGETS_PROTOCOL_VERSION_MINOR;
        widgets_json["model_id"] = val.id();
        mime_bundle["application/vnd.jupyter.widget-view+json"] = std::move(widgets_json);

        // text/plain
        mime_bundle["text/plain"] = "A Jupyter widget";
        return mime_bundle;
    }

    /*************
     * Generator *
     *************/  

    template <template <class> class B, class... P, class... A>
    inline xmaterialize<B, P...> generator(A&&... args)
    {
        return xmaterialize<B, P...>::initialize(std::forward<A>(args)...);
    }
}

#endif
