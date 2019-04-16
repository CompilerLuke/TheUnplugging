#include "reflection.h"
#include <glm/vec3.hpp>

namespace reflect {

	//--------------------------------------------------------
	// A type descriptor for int
	//--------------------------------------------------------

	struct TypeDescriptor_Int : TypeDescriptor {
		TypeDescriptor_Int() : TypeDescriptor{ "int", sizeof(int) } {
		}
		virtual void dump(const void* obj, int /* unused */) const override {
			std::cout << "int{" << *(const int*)obj << "}";
		}
	};

	template <>
	TypeDescriptor* getPrimitiveDescriptor<int>() {
		static TypeDescriptor_Int typeDesc;
		return &typeDesc;
	}

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