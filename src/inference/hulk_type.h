#ifndef HULK_INFERENCE_TYPE_H
#define HULK_INFERENCE_TYPE_H

#include <string>
#include <memory>

namespace Hulk {

    // Representación de los tipos inferidos para el AST.
    // Usamos una clase con Kind y std::string para soporte de objetos.
    class HulkType {
    public:
        enum class Kind {
            Number,
            String,
            Boolean,
            Void,
            Object,
            Unknown,
            Error
        };

        HulkType(); // Default: Unknown
        explicit HulkType(Kind kind);
        HulkType(Kind kind, std::string name);

        static HulkType make_number() { return HulkType(Kind::Number); }
        static HulkType make_string() { return HulkType(Kind::String); }
        static HulkType make_boolean() { return HulkType(Kind::Boolean); }
        static HulkType make_void() { return HulkType(Kind::Void); }
        static HulkType make_unknown() { return HulkType(Kind::Unknown); }
        static HulkType make_error() { return HulkType(Kind::Error); }
        static HulkType make_object(const std::string& name) { return HulkType(Kind::Object, name); }

        Kind kind() const { return kind_; }
        const std::string& name() const { return name_; }

        bool is_error() const { return kind_ == Kind::Error; }
        bool is_unknown() const { return kind_ == Kind::Unknown; }

        bool operator==(const HulkType& other) const;
        bool operator!=(const HulkType& other) const;

        std::string to_string() const;

    private:
        Kind kind_;
        std::string name_; // Solo usado si kind_ == Object
    };

} // namespace Hulk

#endif // HULK_INFERENCE_TYPE_H
