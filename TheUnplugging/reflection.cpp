#include "reflection.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

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

	struct TypeDescriptor_GlmVec2 : TypeDescriptor {
		TypeDescriptor_GlmVec2() : TypeDescriptor{ "glm::vec2", sizeof(glm::vec2) } {}
	
		virtual void dump(const void* obj, int) const override {
			auto ptr = (const glm::vec2*)obj;
			std::cout << "glm::vec2{" << ptr->x << "," << ptr->y << "}";
		}
	};

	template<>
	TypeDescriptor* getPrimitiveDescriptor<glm::vec2>() {
		static TypeDescriptor_GlmVec2 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Mat4 : TypeDescriptor {
		TypeDescriptor_Mat4() : TypeDescriptor{ "glm::mat4", sizeof(glm::vec3)} {}
		virtual void dump(const void* obj, int) const override {
			std::cout << "glm::mat4";
		}
	};

	template<>
	TypeDescriptor* getPrimitiveDescriptor<glm::mat4>() {
		static TypeDescriptor_Mat4 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_GlmQuat : TypeDescriptor {
		TypeDescriptor_GlmQuat() : TypeDescriptor{ "glm::quat", sizeof(glm::quat) } {}

		virtual void dump(const void* obj, int) const override {
			auto ptr = (const glm::quat*)obj;
			auto euler = glm::eulerAngles(*ptr);
			std::cout << "quat{" << glm::degrees(euler.x) << " " << glm::degrees(euler.y) << " " << glm::degrees(euler.z) << std::endl;
		}
	};

	template<>
	TypeDescriptor* getPrimitiveDescriptor<glm::quat>() {
		static TypeDescriptor_GlmQuat typeDesc;
		return &typeDesc;
	}
} // namespace reflect