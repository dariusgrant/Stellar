#pragma once

#include <map>
#include <any>
#include <typeindex>

namespace stlr {
    /// <summary>
    /// A map containing extensions for Vulkan object crreation
    /// and querying.
    /// </summary>
    class ExtensionMap {
    private:
        std::map<std::size_t, std::any> map;
        void* start = nullptr;
        void** end = nullptr;

    public:
        template <typename Extension>
        ExtensionMap( const Extension& extension ) {
            add( extension );
        }

        template <typename Extension, typename... Extensions>
        ExtensionMap( const Extension& first, const Extensions&... rest ) {
            add( first );
            add( rest... );
        }

        /// <summary>
        /// Adds an extension to the map. The extension must be a
        /// valid extension for the associated Vulkan object per the
        /// specification.
        /// </summary>
        /// <typeparam name="Extension">The extension's type.</typeparam>
        /// <param name="extension">The extension to add.</param>
        /// <returns>The extension map with the added extension.</returns>
        template <typename Extension>
        ExtensionMap& add( const Extension& extension ) {
            auto typeIndex = std::type_index( typeid(Extension) );
            auto hash = typeIndex.hash_code();
            map[hash] = extension;
            Extension& obj = std::any_cast<Extension&>( map[hash] );

            if( start == nullptr ) {
                start = &obj;
            }
            else {
                *end = &obj;
            }

            /// const_cast necessary as some Vulkan create info structures
            /// have const pNext structures.
            end = const_cast<void**>(&obj.pNext);

            return *this;
        }

        /// <summary>
        /// Adds extensions to the map. The extensions must be a
        /// valid extension for the associated Vulkan object per the
        /// specification.
        /// </summary>
        /// <typeparam name="Extension">The first extension type.</typeparam>
        /// <typeparam name="...Extensions">The rest of the extensions' types.</typeparam>
        /// <param name="first"> The first extension to add.</param>
        /// <param name="...rest">The rest of the extensions to add, if applicable.</param>
        /// <returns>The extension map with the added extensions.</returns>
        template <typename Extension, typename... Extensions>
        ExtensionMap& add( const Extension& first, const Extensions&... rest ) {
            add( first );
            add( rest... );
            return *this;
        }

        /// <summary>
        /// Gets an extension from the extension map if it exists.
        /// </summary>
        /// <typeparam name="Extension">The type of extension to get.</typeparam>
        /// <returns>A pointer to the extension if it exists. Otherwise null pointer.</returns>
        template <typename Extension>
        constexpr Extension* get() const {
            const auto& res = find<Extension>();
            return res != map.end() ? std::any_cast<Extension*>(&res->second) : nullptr;
        }

        /// <summary>
        /// Gets the chain containing all the extensions. Necessary for
        /// passing into creation and query structures.
        /// </summary>
        /// <returns>A void pointer that links all the extensions.</returns>
        constexpr void* get_chain() const {
            return start;
        }


    private:
        /// <summary>
        /// Finds the extension based on its type.
        /// </summary>
        /// <typeparam name="Extension">The type of extension to find.</typeparam>
        /// <returns>An iterator at the extension if it exists. Otherwise, past-the-end iterator.</returns>
        template <class Extension>
        std::map<std::size_t, std::any>::iterator find() {
            return map.find( typeid(Extension).hash_code() );
        }
    };
}
