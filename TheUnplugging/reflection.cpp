#include "reflection.h"
#include <glm/vec3.hpp>

#define PRIMITIVE_TYPE_DESCRIPTOR(type) \
struct TypeDescriptor_##type : TypeDescriptor { \
	TypeDescriptor_##type() : TypeDescriptor{ #type , sizeof(type) } {}; \
	virtual void dump(const void* obj, int) const override { \
		std::cout << *(const type*)obj << std::endl; \
	} \
}; \
\
template<> \
TypeDescriptor* getPrimitiveDescriptor<type>() { \
	static TypeDescriptor_##type typeDesc; \
	return &typeDesc;  \
} 

namespace reflect {

	//--------------------------------------------------------
	// A type descriptor for int
	//--------------------------------------------------------

	PRIMITIVE_TYPE_DESCRIPTOR(float);
	PRIMITIVE_TYPE_DESCRIPTOR(bool);
	PRIMITIVE_TYPE_DESCRIPTOR(int);
	
	using UINT = unsigned int;
	PRIMITIVE_TYPE_DESCRIPTOR(UINT);
	//--------------------------------------------------------
	// A type descriptor for std::string
	//--------------------------------------------------------

	struct TypeDescriptor_StdString : TypeDescriptor {
		TypeDescriptor_StdString() : TypeDescriptor{ "std::string", sizeof(std::string) } {
		}
		virtual void dump(const void* obj, int /* unused */) const override {
			std::cout << "std::string{\"" << *(const std::string*) obj << "\"}";
		}
	};

	template <>
	TypeDescriptor* getPrimitiveDescriptor<std::string>() {
		static TypeDescriptor_StdString typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_GlmVec3 : TypeDescriptor {
		TypeDescriptor_GlmVec3() : TypeDescriptor{ "glm::vec3", sizeof(glm::vec3) } {

		}
		virtual void dump(const void* obj, int) const override {
			auto ptr = (const glm::vec3*)obj;
			std::cout << "glm::vec3{" << ptr->x << "," << ptr->y << "," << ptr->z << "}";
		}
	};

	template<>
	TypeDescriptor* getPrimitiveDescriptor<glm::vec3>() {
		static TypeDescriptor_GlmVec3 typeDesc;
		return &typeDesc;
	}

} // namespace reflect