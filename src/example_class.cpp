#include "example_class.h"
#include "godot_cpp/variant/variant.hpp"

void ExampleClass::_bind_methods() {
	godot::ClassDB::bind_method(D_METHOD("print_type", "variant"), &ExampleClass::print_type);
}

void ExampleClass::print_type(const Variant &p_variant) const {
	print_line(vformat("Type: %s", Variant::get_type_name(p_variant.get_type())));
}
