#pragma once

#include <typeinfo>

#include "acmacs-base/rjson.hh"

#include "acmacs-base/color.hh"
#include "acmacs-draw/size.hh"

// ----------------------------------------------------------------------

namespace rjson
{
    class field_container_parent
    {
     public:
        virtual inline ~field_container_parent() {}

        virtual value& get_ref(std::string aFieldName, value&& aDefault) = 0;
        virtual const value& get_ref(std::string aFieldName, value&& aDefault) const = 0;
        virtual object& get_ref_to_object(std::string aFieldName) = 0;
        virtual void set_field(std::string aFieldName, value&& aValue) = 0;

    }; // class field_container_parent

      // ----------------------------------------------------------------------

    class field_container_toplevel : public field_container_parent
    {
     public:
        inline void use_json(rjson::value&& aData) { mData = std::move(aData); }

        inline value& get_ref(std::string aFieldName, value&& aDefault) override { return mData.get_ref(aFieldName, std::forward<value>(aDefault)); }
        inline const value& get_ref(std::string aFieldName, value&& aDefault) const override { return mData.get_ref(aFieldName, std::forward<value>(aDefault)); }
        inline object& get_ref_to_object(std::string aFieldName) override { return mData.get_ref_to_object(aFieldName); }
        inline void set_field(std::string aFieldName, value&& aValue) override { mData.set_field(aFieldName, std::forward<value>(aValue)); }

        inline std::string to_json() const { return mData.to_json(); }

     private:
        mutable rjson::value mData = rjson::object{};

    }; // class field_container_toplevel

      // ----------------------------------------------------------------------

    class field_container_child : public field_container_parent
    {
     public:
        inline field_container_child(field_container_parent& aParent, std::string aFieldName)
            : mParent{aParent}, mFieldName{aFieldName} {}

        inline object& get_ref_to_object() { return mParent.get_ref_to_object(mFieldName); }
        inline const object& get_ref_to_object() const { return mParent.get_ref_to_object(mFieldName); }
        inline object& get_ref_to_object(std::string aFieldName) override { return get_ref_to_object().get_ref_to_object(aFieldName); }

        inline value& get_ref(std::string aFieldName, value&& aDefault) override { return get_ref_to_object().get_ref(aFieldName, std::forward<value>(aDefault)); }
        inline const value& get_ref(std::string aFieldName, value&& aDefault) const override { return get_ref_to_object().get_ref(aFieldName, std::forward<value>(aDefault)); }

        inline void set_field(std::string aFieldName, value&& aValue) override { get_ref_to_object().set_field(aFieldName, std::forward<value>(aValue)); }

        inline std::string to_json() const { return get_ref_to_object().to_json(); }

     private:
        field_container_parent& mParent;
        std::string mFieldName;

    }; // class field_container_child

      // ----------------------------------------------------------------------

    template <typename Element> class array_field_container_child
    {
     public:
        class iterator
        {
         public:
            inline iterator& operator++() { ++iter; return *this;}
            inline iterator operator++(int) { iterator retval = *this; ++(*this); return retval;}
            inline bool operator==(iterator other) const { return iter == other.iter; }
            inline bool operator!=(iterator other) const { return !(*this == other); }
            inline Element operator*() { /* std::cerr << "DEBUG: " << *iter << DEBUG_LINE_FUNC << '\n'; */ return Element{*iter}; }

            using difference_type = array::iterator::difference_type;
            using value_type = Element;
            using pointer = Element*;
            using reference = Element&;
            using iterator_category = array::iterator::iterator_category;

         private:
            array::iterator iter;
            inline iterator(array::iterator aIter) : iter(aIter) {}

            friend class array_field_container_child<Element>;
        };

        inline array_field_container_child(field_container_parent& aParent, std::string aFieldName)
            : mParent{aParent}, mFieldName{aFieldName} {}

        inline array& get_ref_to_array() { return std::get<array>(mParent.get_ref(mFieldName, array{})); }
        inline const array& get_ref_to_array() const { return std::get<array>(mParent.get_ref(mFieldName, array{})); }
        inline size_t size() const { return get_ref_to_array().size(); }
        inline bool empty() const { return get_ref_to_array().empty(); }
        inline iterator begin() const { return get_ref_to_array().begin(); }
        inline iterator end() const { return get_ref_to_array().end(); }
        inline iterator begin() { return get_ref_to_array().begin(); }
        inline iterator end() { return get_ref_to_array().end(); }

        inline Element emplace_back()
            {
                auto& ar = get_ref_to_array();
                ar.insert(object{});
                return Element{*ar.rbegin()};
            }

     private:
        field_container_parent& mParent;
        std::string mFieldName;

    }; // class array_field_container_child<>

    class array_field_container_child_element : public field_container_parent
    {
     public:
        inline array_field_container_child_element(const value& aData) : mData{aData} {}

        inline value& get_ref(std::string aFieldName, value&& aDefault) override { return const_cast<value&>(mData).get_ref(aFieldName, std::forward<value>(aDefault)); }
        inline const value& get_ref(std::string aFieldName, value&& aDefault) const override { return const_cast<value&>(mData).get_ref(aFieldName, std::forward<value>(aDefault)); }
        inline object& get_ref_to_object(std::string aFieldName) override { return const_cast<value&>(mData).get_ref_to_object(aFieldName); }
        inline void set_field(std::string aFieldName, value&& aValue) override { const_cast<value&>(mData).set_field(aFieldName, std::forward<value>(aValue)); }

     private:
        const value& mData;
    };

      // ----------------------------------------------------------------------

    template <typename FValue> class field_get_set
    {
     public:
        inline field_get_set(field_container_parent& aParent, std::string aFieldName, FValue&& aDefault) : mParent{aParent}, mFieldName{aFieldName}, mDefault{std::move(aDefault)} {}
        inline field_get_set(field_container_parent& aParent, std::string aFieldName, const FValue& aDefault) : mParent{aParent}, mFieldName{aFieldName}, mDefault{aDefault} {}

        inline const rjson_type<FValue>& get_value_ref() const
            {
                try {
                    return std::get<rjson_type<FValue>>(get_ref());
                }
                catch (std::bad_variant_access& /*err*/) {
                    std::cerr << "ERROR: cannot convert json to " <<  typeid(rjson_type<FValue>).name() << " (" << typeid(FValue).name() << "): " << get_ref() << '\n';
                    throw;
                }
            }

          // inline field_get_set<FValue>& operator = (FValue&& aValue) { mParent.set_field(mFieldName, to_value(aValue)); return *this; }
        inline field_get_set<FValue>& operator = (const FValue& aValue) { mParent.set_field(mFieldName, to_value(aValue)); return *this; }
        inline field_get_set<FValue>& operator = (const field_get_set<FValue>& aSource) { return operator=(static_cast<FValue>(aSource)); }

        inline bool empty() const { return static_cast<FValue>(*this).empty(); }

          // to be specialized for complex types
        inline operator FValue() const { return get_value_ref(); }

     private:
        field_container_parent& mParent;
        std::string mFieldName;
        FValue mDefault;

        // inline value& get_ref() { return mParent.get_ref(mFieldName, to_value(mDefault)); }
        inline const value& get_ref() const { return mParent.get_ref(mFieldName, to_value(mDefault)); }
        // inline rjson_type<FValue>& get_value_ref() { return std::get<rjson_type<FValue>>(get_ref()); }

    }; // class field_get_set<>

      // ----------------------------------------------------------------------
      // double: can be extracted from rjson::number and rjson::integer
      // ----------------------------------------------------------------------

    template <> inline field_get_set<double>::operator double() const
    {
        if (auto ptr_n = std::get_if<number>(&get_ref()))
            return *ptr_n;
        else if (auto ptr_i = std::get_if<integer>(&get_ref()))
            return *ptr_i;
        else {
            std::cerr << "ERROR: cannot convert json to double (from rjson::number or rjson::integer): " << get_ref() << '\n';
            throw std::bad_variant_access{};
        }
    }

      // ----------------------------------------------------------------------
      // Color
      // ----------------------------------------------------------------------

    template <> struct content_type<Color> { using type = string; };

    template <> inline field_get_set<Color>::operator Color() const { return static_cast<std::string>(get_value_ref()); }

      // ----------------------------------------------------------------------
      // Size
      // ----------------------------------------------------------------------

    template <> struct content_type<Size> { using type = array; };

    template <> inline field_get_set<Size>::operator Size() const
    {
        const auto& ar = get_value_ref(); // std::get<array>(get_ref());
        return {std::get<number>(ar[0]), std::get<number>(ar[1])};
    }

    template <> inline value to_value<Size>(const Size& aSize)
    {
        return array{number{aSize.width}, number{aSize.height}};
    }

      // ----------------------------------------------------------------------
      // TextStyle
      // ----------------------------------------------------------------------

    template <> struct content_type<TextStyle> { using type = object; };

    template <> inline field_get_set<TextStyle>::operator TextStyle() const
    {
        const auto& obj = get_value_ref(); // std::get<object>(get_ref());
        TextStyle style;
        try { style.font_family(obj.get_field<std::string>("family")); } catch (object::field_not_found&) {}
        try { style.slant(obj.get_field<std::string>("slant")); } catch (object::field_not_found&) {}
        try { style.weight(obj.get_field<std::string>("weight")); } catch (object::field_not_found&) {}
        return style;
    }

    template <> inline value to_value<TextStyle>(const TextStyle& aTextStyle)
    {
        return object{
            {"family", string{aTextStyle.font_family()}},
            {"slant", string{aTextStyle.slant_as_stirng()}},
            {"weight", string{aTextStyle.weight_as_stirng()}}
        };
    }

      // ----------------------------------------------------------------------
      // std::vector<std::string>
      // ----------------------------------------------------------------------

    template <> struct content_type<std::vector<std::string>> { using type = array; };

    template <> inline field_get_set<std::vector<std::string>>::operator std::vector<std::string>() const
    {
        const auto& ar = get_value_ref();
        std::vector<std::string> result{ar.size()};
        std::transform(ar.begin(), ar.end(), result.begin(), [](const rjson::value& elt) -> std::string { return std::get<rjson::string>(elt); });
        return result;
    }

    template <> inline value to_value<std::vector<std::string>>(const std::vector<std::string>& aData)
    {
        array ar;
        for (const auto& elt: aData)
            ar.insert(string{elt});
        return ar;
    }

} // namespace rjson

// ----------------------------------------------------------------------

template <typename FValue> inline std::ostream& operator<<(std::ostream& out, const rjson::field_get_set<FValue>& aValue)
{
    return out << static_cast<FValue>(aValue);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
