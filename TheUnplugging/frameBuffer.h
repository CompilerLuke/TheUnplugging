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
	unsigned int width = 0;
	unsigned int height = 0;
	DepthBufferSettings depth_buffer;
	AttachmentSettings* depth_attachment = NULL;
	vector<AttachmentSettings> color_attachments;
};

struct Framebuffer {
	unsigned int fbo = 0;
	unsigned int rbo = 0;
	unsigned int width = 0;
	unsigned int height = 0;

	void operator=(Framebuffer&&) noexcept;
	Framebuffer(World&, FramebufferSettings&);
	Framebuffer();

	void bind();
	void clear_color(glm::vec4);
	void clear_depth(glm::vec4);
	void unbind();
};