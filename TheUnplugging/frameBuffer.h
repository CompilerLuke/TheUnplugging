#pragma once

#include "ecs.h"
#include <vector>
#include <glm/vec4.hpp>

enum DepthBufferSettings { DepthComponent24 };
enum InternalColorFormat { Rgb16f };
enum ColorFormat { Rgb };
enum TexelType { Float_Texel };
enum Filter { Nearest, Linear };
enum Wrap { ClampToBorder, Repeat };

struct AttachmentSettings {
	ID tex_id;
	InternalColorFormat internal_format = Rgb16f;
	ColorFormat external_format = Rgb;
	TexelType texel_type = Float_Texel;
	Filter min_filter = Nearest;
	Filter mag_filter = Nearest;
	Wrap wrap_s = ClampToBorder;
	Wrap wrap_t = ClampToBorder;

	AttachmentSettings(ID);
};

struct FramebufferSettings {
	unsigned int width;
	unsigned int height;
	DepthBufferSettings depth_buffer;
	AttachmentSettings* depth_attachment;
	std::vector<AttachmentSettings> color_attachmens;
};

struct Framebuffer {
	unsigned int fbo;
	unsigned int rbo;
	unsigned int width;
	unsigned int height;

	Framebuffer(World&, FramebufferSettings&);

	void bind();
	void clear_color(glm::vec4);
	void clear_depth(glm::vec4);
	void unbind();
};